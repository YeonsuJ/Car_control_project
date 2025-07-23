/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include "mpu6050.h"
#include "input_handler.h"
#include "comm_handler.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

MPU6050_t MPU6050;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define TRANSMIT_INTERVAL_MS 1

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

uint32_t lastTransmitTick = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
static float App_GetRollAngle(void);
static void App_BuildPacket(uint8_t* packet_buffer, float roll_angle);
static void App_HandleAckPayload(uint8_t* ack_payload);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
// =============================
// NRF24 송신 패킷 구조 정의
// -----------------------------
// Byte 0   : 메시지 식별자 (1)
// Byte 1~2 : roll (int16_t, 센서 측에서 ×100 배율 인코딩)
// Byte 3~4 : accel_ms (uint16_t, 엑셀 유지 시간)
// Byte 5~6 : brake_ms (uint16_t, 브레이크 유지 시간)
// Byte 7   : direction (0: BACKWARD, 1: FORWARD)
// 나머지 Byte 8~31 : 예약 or 미사용
// =============================
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C2_Init();
  MX_TIM2_Init();
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */

  InputHandler_Init();
  CommHandler_Init();

  if (MPU6050_Init(&hi2c2) != 0) {
      Error_Handler();
  }

  HAL_TIM_Base_Start_IT(&htim2);

  uint8_t tx_packet[PAYLOAD_SIZE] = {0};
  uint8_t ack_packet[ACK_PAYLOAD_SIZE] = {0};

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
    while (1)
    {
    	   uint32_t now = HAL_GetTick();

            if (now - lastTransmitTick >= TRANSMIT_INTERVAL_MS)
            {
                lastTransmitTick = now;

                float roll = App_GetRollAngle();

                // 2. 전송 패킷 생성 (어떻게?) -> 내부 헬퍼 함수가 알아서
                App_BuildPacket(tx_packet, roll);

                // 3. 데이터 전송 (누구에게?) -> 통신 핸들러에게 위임
                CommHandler_Transmit(tx_packet, PAYLOAD_SIZE);

                // 4. 통신 결과 확인 및 처리
                CommStatus_t status = CommHandler_CheckStatus(ack_packet, ACK_PAYLOAD_SIZE);
                if (status == COMM_TX_SUCCESS)
                {
                    App_HandleAckPayload(ack_packet);
                }
                else if (status == COMM_TX_FAIL)
                {
                    // TODO: 통신 실패 시 처리 로직
                }
            }

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
 }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

// ★★★ main 루프의 상세 로직을 담당하는 내부 헬퍼 함수들 ★★★

/**
  * @brief  MPU6050 센서로부터 Roll 각도를 읽어옵니다.
  */
static float App_GetRollAngle(void)
{
    MPU6050_Read_All(&hi2c2, &MPU6050);
    return MPU6050.KalmanAngleX;
}

/**
  * @brief  각 모듈에서 데이터를 가져와 전송 패킷을 구성합니다.
  */
static void App_BuildPacket(uint8_t* packet_buffer, float roll_angle)
{
    memset(packet_buffer, 0, PAYLOAD_SIZE);

    packet_buffer[0] = 1; // ID

    // Roll 값 인코딩
    int16_t roll_encoded = (int16_t)(roll_angle * 100.0f);
    memcpy(&packet_buffer[1], &roll_encoded, sizeof(int16_t));

    // 버튼 입력 시간 가져오기
    uint16_t accel_ms = InputHandler_GetAccelMillis();
    uint16_t brake_ms = InputHandler_GetBrakeMillis();
    memcpy(&packet_buffer[3], &accel_ms, sizeof(uint16_t));
    memcpy(&packet_buffer[5], &brake_ms, sizeof(uint16_t));

    // 방향 정보 가져오기
    packet_buffer[7] = InputHandler_GetDirection();
}

/**
  * @brief  수신된 ACK 페이로드를 처리합니다.
  */
static void App_HandleAckPayload(uint8_t* ack_payload)
{
    if (ack_payload[0] == 1)
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);
    else
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
}

// --- 콜백 함수들은 각 핸들러에게 작업을 위임 ---
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == GPIO_PIN_3)
    {
        CommHandler_IrqCallback();
        return;
    }
    InputHandler_GpioCallback(GPIO_Pin);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2)
    {
        InputHandler_TimerCallback();
    }
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

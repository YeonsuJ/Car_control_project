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
#include "fonts.h"
#include "ssd1306.h"
#include "mpu6050.h"
#include "NRF24.h"
#include "NRF24_reg_addresses.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
// MPU6050 구조체 인스턴스 (외부 라이브러리에서 정의)
MPU6050_t MPU6050;

// 방향 전송값 정의
typedef enum{
	LOCKER_FORWARD = 1,
	LOCKER_BACKWARD = 0
} LockerDirection;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
// NRF24 페이로드 크기 (byte)
#define PLD_S 32

// NRF24 송신 주기 (ms)
#define TRANSMIT_INTERVAL_MS 20
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
volatile uint8_t accel_pressed = 0;
volatile uint8_t brake_pressed = 0;
volatile uint32_t accel_count = 0;   // 20ms 단위
volatile uint32_t brake_count = 0;   // 20ms 단위

volatile uint32_t prev_tick_accel = 0;
volatile uint32_t prev_tick_brake = 0;

uint8_t tx_addr[5] = {0x45, 0x55, 0x67, 0x10, 0x21};
uint8_t dataT[PLD_S];  // 전송 버퍼

uint32_t lastTransmitTick = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void NRF24_Tx_Init(void);
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
  MX_I2C1_Init();
  MX_I2C2_Init();
  MX_TIM2_Init();
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */
  SSD1306_Init();

  // MPU6050 초기화 실패 시 Error_Handler
  if (MPU6050_Init(&hi2c2) !=0){
	  Error_Handler();
  }

  HAL_TIM_Base_Start_IT(&htim2);

  //NRF24L01 무선 송신 설정
  NRF24_Tx_Init();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
    while (1)
    {
        uint32_t now = HAL_GetTick();

        if (now - lastTransmitTick >= TRANSMIT_INTERVAL_MS)
        {
            lastTransmitTick = now;

            // 1. 자이로센서 데이터 읽기 및 유효성 검사
            MPU6050_Read_All(&hi2c2, &MPU6050);
            float roll = MPU6050.KalmanAngleX;

            // 2. 전송 버퍼 구성
            memset(dataT, 0, PLD_S);
            dataT[0] = 1;  // 식별자

            // 3. roll (float → int16_t)
            int16_t roll_encoded = (int16_t)(roll * 100.0f);  // 예: 32.53° → 3253
            memcpy(&dataT[1], &roll_encoded, sizeof(int16_t));  // roll : 1~2 byte

            // 4. accel_count, brake_count (uint16_t로 압축 전송, 단위: 20ms)
            uint16_t accel_ms = (uint16_t)(accel_count * 20);
            uint16_t brake_ms = (uint16_t)(brake_count * 20);
            memcpy(&dataT[3], &accel_ms, sizeof(uint16_t));    // accel_ms : 3~4 byte
            memcpy(&dataT[5], &brake_ms, sizeof(uint16_t));    // brake_ms : 5~6 byte

            // 5. 락커 스위치 판별 (PB8: LOCKER_BACKWARD == 0, PB9: LOCKER_FORWARD == 1)
            // direction : 7 byte
            if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_9) == GPIO_PIN_RESET)
                dataT[7] = LOCKER_FORWARD;
            else
                dataT[7] = LOCKER_BACKWARD;

            // 6. NRF24 전송
            nrf24_transmit(dataT, PLD_S);
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
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    uint32_t now = HAL_GetTick();

    if (GPIO_Pin == GPIO_PIN_0)  // Accel 버튼
    {
        if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) == GPIO_PIN_RESET)
        {
            // 눌림이 감지된 순간(하강), 디바운싱 여부와 상관없이 초기화
            accel_pressed = 1;
            accel_count = 0;
            prev_tick_accel = now;
        }
        else
        {
            // 떼짐(상승)은 즉시 pressed=0 처리 및 카운트 초기화
            accel_pressed = 0;
            accel_count = 0;
            prev_tick_accel = now;
        }
    }
    else if (GPIO_Pin == GPIO_PIN_1)  // Brake 버튼
    {
        if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1) == GPIO_PIN_RESET)
        {
            // 눌림이 감지된 순간(하강), 디바운싱 여부와 상관없이 초기화
            brake_pressed = 1;
            brake_count = 0;
            prev_tick_brake = now;
        }
        else
        {
            // 떼짐(상승)은 즉시 pressed=0 처리 및 카운트 초기화
            brake_pressed = 0;
            brake_count = 0;
            prev_tick_brake = now;
        }
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2) // 20ms 주기 타이머
    {
        if (accel_pressed) accel_count++;
        if (brake_pressed) brake_count++;
    }
}

// RF 초기화 함수
void NRF24_Tx_Init(void)
{
	csn_high();
	ce_high();
	HAL_Delay(5);
	ce_low();

	nrf24_init();
	nrf24_stop_listen();  // 송신기이므로 listen 비활성화

	nrf24_auto_ack_all(auto_ack);
	nrf24_en_ack_pld(disable);
	nrf24_dpl(disable);
	nrf24_set_crc(no_crc, _1byte);

	nrf24_tx_pwr(_0dbm);
	nrf24_data_rate(_1mbps);
	nrf24_set_channel(90);
	nrf24_set_addr_width(5);
	nrf24_set_rx_dpl(0, disable);
	nrf24_pipe_pld_size(0, PLD_S);
	nrf24_auto_retr_delay(4);
	nrf24_auto_retr_limit(10);

	nrf24_open_tx_pipe(tx_addr);
	ce_high();
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

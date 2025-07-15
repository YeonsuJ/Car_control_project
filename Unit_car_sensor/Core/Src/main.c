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
#include "can.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
#define FRONT_TRIG_PIN GPIO_PIN_9
#define FRONT_TRIG_PORT GPIOA
#define REAR_TRIG_PIN GPIO_PIN_8
#define REAR_TRIG_PORT GPIOA

volatile uint32_t ic_val1_front = 0;
volatile uint32_t ic_val2_front = 0;
volatile uint8_t is_first_captured_front = 0;
volatile uint32_t distance_front = 0;

volatile uint32_t ic_val1_rear = 0;
volatile uint32_t ic_val2_rear = 0;
volatile uint8_t is_first_captured_rear = 0;
volatile uint32_t distance_rear = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void delay_us(uint16_t us);
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
CAN_TxHeaderTypeDef TxHeader;
uint32_t TxMailbox;
uint8_t TxData[8]= {0};
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
  MX_CAN_Init();
  MX_TIM4_Init();
  MX_TIM2_Init(); // delay 전용
  /* USER CODE BEGIN 2 */
  HAL_CAN_Start(&hcan);
  TxHeader.DLC = 8;  // data length
  TxHeader.IDE = CAN_ID_STD;
  TxHeader.RTR = CAN_RTR_DATA;
  TxHeader.StdId = 0x6A5;  // ID can be between Hex1 and Hex7FF (1-2047 decimal)

  HAL_TIM_Base_Start(&htim2);  // 타이머 카운터만 시작
  HAL_TIM_IC_Start_IT(&htim4, TIM_CHANNEL_1);
  HAL_TIM_IC_Start_IT(&htim4, TIM_CHANNEL_2);

  uint32_t prev_tick = 0;

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  // 20ms 주기 체크
	  if (HAL_GetTick() - prev_tick >= 100)
	  {
		  prev_tick = HAL_GetTick();

		  // 1) 전방 트리거 (PA9)
		  HAL_GPIO_WritePin(FRONT_TRIG_PORT, FRONT_TRIG_PIN, GPIO_PIN_SET);
		  delay_us(10);
		  HAL_GPIO_WritePin(FRONT_TRIG_PORT, FRONT_TRIG_PIN, GPIO_PIN_RESET);

		  // 2) 후방 트리거 (PA8)
		  HAL_GPIO_WritePin(REAR_TRIG_PORT, REAR_TRIG_PIN, GPIO_PIN_SET);
		  delay_us(10);
		  HAL_GPIO_WritePin(REAR_TRIG_PORT, REAR_TRIG_PIN, GPIO_PIN_RESET);

		  // 3) 거리 조건 확인 후 CAN 데이터 설정
		  if (distance_front <= 10 || distance_rear <= 10)
			TxData[0] = 1;
		  else
			TxData[0] = 0;

		  // 4) CAN 메시지 전송
		  HAL_CAN_AddTxMessage(&hcan, &TxHeader, TxData, &TxMailbox);
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
// tim2를 delay로 사용
void delay_us(uint16_t us)
{
  __HAL_TIM_SET_COUNTER(&htim2, 0);
  while (__HAL_TIM_GET_COUNTER(&htim2) < us);
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM4)
  {
    if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)  // 전방 Echo
    {
      if (is_first_captured_front == 0)
      {
        ic_val1_front = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
        __HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_FALLING);
        is_first_captured_front = 1;
      }
      else
      {
        ic_val2_front = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
        __HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_RISING);
        __HAL_TIM_DISABLE_IT(htim, TIM_IT_CC1);
        uint32_t diff = (ic_val2_front > ic_val1_front) ? (ic_val2_front - ic_val1_front) : (0xFFFF - ic_val1_front + ic_val2_front);
        distance_front = (diff * 0.0343f) / 2.0f;
        is_first_captured_front = 0;
        __HAL_TIM_ENABLE_IT(htim, TIM_IT_CC1);
      }
    }

    else if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)  // 후방 Echo ← 수정됨
    {
      if (is_first_captured_rear == 0)
      {
        ic_val1_rear = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);
        __HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_2, TIM_INPUTCHANNELPOLARITY_FALLING);
        is_first_captured_rear = 1;
      }
      else
      {
        ic_val2_rear = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);
        __HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_2, TIM_INPUTCHANNELPOLARITY_RISING);
        __HAL_TIM_DISABLE_IT(htim, TIM_IT_CC2);
        uint32_t diff = (ic_val2_rear > ic_val1_rear) ? (ic_val2_rear - ic_val1_rear) : (0xFFFF - ic_val1_rear + ic_val2_rear);
        distance_rear = (diff * 0.0343f) / 2.0f;
        is_first_captured_rear = 0;
        __HAL_TIM_ENABLE_IT(htim, TIM_IT_CC2);
      }
    }
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

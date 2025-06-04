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
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "fonts.h"
#include "ssd1306.h"
#include "mpu6050.h"
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
MPU6050_t MPU6050;

volatile uint8_t accel_pressed = 0;
volatile uint8_t brake_pressed = 0;
volatile uint32_t accel_count = 0;   // 20ms 단위
volatile uint32_t brake_count = 0;   // 20ms 단위

volatile uint32_t prev_tick_accel = 0;
volatile uint32_t prev_tick_brake = 0;

uint32_t previousMillis = 0;
uint32_t currentMillis = 0;
uint32_t counterOutside = 0; //For testing only
uint32_t counterInside = 0; //For testing only
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  /* USER CODE BEGIN 2 */
  SSD1306_Init();
//    while (MPU6050_Init(&hi2c2) == 1);
//    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);
    HAL_TIM_Base_Start_IT(&htim2);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
    while (1)
    {
//        // 자이로 센서 데이터 읽기
//        MPU6050_Read_All(&hi2c2, &MPU6050);
//
//        // Roll 각도 OLED 출력
//        char buffer_roll[20];
//        snprintf(buffer_roll, sizeof(buffer_roll), "Roll: %.2f", MPU6050.KalmanAngleX);
//        SSD1306_GotoXY(0, 0);
//        SSD1306_Puts(buffer_roll, &Font_11x18, 1);
//        SSD1306_UpdateScreen();
//
//        // Roll 값 (-90 ~ 90)
//        float roll = MPU6050.KalmanAngleX;
//        if (roll < -90.0f) roll = -90.0f;
//        if (roll >  90.0f) roll = 90.0f;
//
//        // 각도 0 ~ 180으로 변환
//        float angle = roll + 90.0f;
//
//        // angle 0~180 → PWM 500~2500으로 매핑
//        uint16_t pwm = (uint16_t)(500 + (angle / 180.0f) * 2000.0f);
//
//        // PWM 출력
//        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, pwm);
//    	HAL_Delay(100);
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
            // 떼짐(상승)은 즉시 pressed=0 처리
            accel_pressed = 0;
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
            // 떼짐(상승)은 즉시 pressed=0 처리
            brake_pressed = 0;
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

        char buffer1[20], buffer2[20];

        snprintf(buffer1, sizeof(buffer1), "ACC: %lums", accel_count * 20);
        SSD1306_GotoXY(0, 0);
        SSD1306_Puts(buffer1, &Font_11x18, 1);

        snprintf(buffer2, sizeof(buffer2), "BRK: %lums", brake_count * 20);
        SSD1306_GotoXY(0, 20);
        SSD1306_Puts(buffer2, &Font_11x18, 1);

        SSD1306_UpdateScreen();
    }
}

//void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
//{
//  counterOutside++; //For testing only
//  currentMillis = HAL_GetTick();
//  if (GPIO_Pin == GPIO_PIN_0 && (currentMillis - previousMillis > 10))
//  {
//    counterInside++; //For testing only
//    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_0);
//    previousMillis = currentMillis;
//  }
//
//  if (GPIO_Pin == GPIO_PIN_1 && (currentMillis - previousMillis > 10))
//  {
//    counterInside++; //For testing only
//    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_1);
//    previousMillis = currentMillis;
//  }
//}
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

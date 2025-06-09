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
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define PPR 11
#define GEAR_RATIO 21.3f
#define TICKS_PER_REV (PPR * GEAR_RATIO * 4)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
int16_t last_encoder = 0;
float motor_rpm = 0;
int motor_direction = 0; // 0: forward, 1: backward

uint8_t accel_pressed = 0, brake_pressed = 0;
uint32_t accel_count = 0, brake_count = 0;
uint32_t prev_tick_accel = 0, prev_tick_brake = 0;
uint32_t last_tick = 0;

int duty = 0;
int accel_step = 5;
int max_duty = 1000;
int min_duty = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
//void Update_RPMA_Direction(void)
//{
//    int16_t enc = __HAL_TIM_GET_COUNTER(&htim3);
//    int32_t delta = (int16_t)(enc - last_encoder);  // 오버플로우 보정
//    last_encoder = enc;
//
//    motor_rpm = ((float)delta / TICKS_PER_REV) * 10.0f * 60.0f;
//
//    if (__HAL_TIM_IS_TIM_COUNTING_DOWN(&htim3))
//        motor_direction = 1;
//    else
//        motor_direction = 0;
//
//    last_tick = HAL_GetTick();
//}
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
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_I2C2_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
  SSD1306_Init();

  HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL);
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);
  HAL_TIM_Base_Start_IT(&htim2);

//  uint32_t last_tick = HAL_GetTick();
//  uint32_t motor_control_tick = 0;
//  uint8_t state = 0;
//
//  static uint32_t state_durations[] = {5000, 2000, 5000, 2000};  // 각 상태 지속시간 (ms)

  // 모터 전진 방향 설정
   HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);  // in1 = 0
   HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);    // in2 = 1
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
//	  if (HAL_GetTick() - last_tick >= 20)
//	      {
//	          Update_RPMA_Direction();
//	      }

//	  if (HAL_GetTick() - last_tick >= 20)
//	  	    {
//		  	  Update_RPMA_Direction();
//	  	    }
//
//	  	  // 상태 기반으로 항상 핀/모터 상태 적용
//	  	  switch (state)
//	  	  {
//	  	    case 0: // forward
//	  	      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);
//	  	      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);
//	  	      __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, 300);
//	  	      break;
//	  	    case 1: // stop
//	  	      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);
//	  	      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
//	  	      __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, 0);
//	  	      break;
//	  	    case 2: // backward
//	  	      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET);
//	  	      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
//	  	      __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, 600);
//	  	      break;
//	  	    case 3: // stop again
//	  	      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);
//	  	      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
//	  	      __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, 0);
//	  	      break;
//	  	  }
//
//	  	  // 상태 전이 조건은 별도로 유지
//	  	  if (HAL_GetTick() - motor_control_tick >= state_durations[state])
//	  	  {
//	  	      state = (state + 1) % 4;
//	  	      motor_control_tick = HAL_GetTick();
//}

    /* USER CODE END WHILE */
 }
    /* USER CODE BEGIN 3 */
}
  /* USER CODE END 3 */


/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 180;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
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

	if (htim->Instance == TIM2)
	    {
			// 1) RPM 업데이트
			int16_t enc = __HAL_TIM_GET_COUNTER(&htim3);
			int32_t delta = (int16_t)(enc - last_encoder);
			last_encoder = enc;

			motor_rpm = ((float)delta / TICKS_PER_REV) * 50.0f * 60.0f;
			// 주기 20ms이므로 1초에 50번 호출됨 → ×50 ×60 으로 환산

	        // 2) 버튼 누름 시간 카운트
	        if (accel_pressed) accel_count++;
	        if (brake_pressed) brake_count++;


	        // 3) PWM duty 제어
	        if (brake_pressed) //brake부터 체크해야 동시에 누를 때 brake가 우선처리됨
	        {
	        	duty -= accel_step * 5;
	        }
	        else if (accel_pressed)
	        {
	        	if (duty ==0) duty = 200; // deadzone으로 인해 초기값 수정
	        	else duty += accel_step;
	        }
	        else
	        {
	        	duty -= accel_step;
	        }

	        // 4) duty 값 클램핑 (음수 또는 max 이상 방지)
			if (duty < 0) duty = 0;
			if (duty > max_duty) duty = max_duty;

	        __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, duty);


			// 4) OLED 출력
			char buffer1[20], buffer2[20];
			SSD1306_Clear(); // 화면 초기화 설정
			snprintf(buffer1, sizeof(buffer1), "DUTY: %.1f", (float)duty);
			SSD1306_GotoXY(0, 0);
			SSD1306_Puts(buffer1, &Font_11x18, 1);
			snprintf(buffer2, sizeof(buffer2), "RPM: %.1f", motor_rpm);
			SSD1306_GotoXY(0, 20);
	        SSD1306_Puts(buffer2, &Font_11x18, 1);
			SSD1306_UpdateScreen();

	        }


//    if (htim->Instance == TIM2) // 20ms 주기 타이머
//    {
//        if (accel_pressed) accel_count++;
//        if (brake_pressed) brake_count++;
//
//        char buffer1[20], buffer2[20];
//
//        snprintf(buffer1, sizeof(buffer1), "ACC: %lums", accel_count * 20);
//        SSD1306_GotoXY(0, 0);
//        SSD1306_Puts(buffer1, &Font_11x18, 1);
//
//        snprintf(buffer2, sizeof(buffer2), "BRK: %lums", brake_count * 20);
//        SSD1306_GotoXY(0, 20);
//        SSD1306_Puts(buffer2, &Font_11x18, 1);
//
//        SSD1306_UpdateScreen();
//    }
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

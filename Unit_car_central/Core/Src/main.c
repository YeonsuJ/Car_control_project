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

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

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

uint32_t previousMillis = 0;
uint32_t currentMillis = 0;
uint32_t counterOutside = 0; //For testing only
uint32_t counterInside = 0; //For testing only

volatile uint16_t received_accel_ms = 0;
volatile uint16_t received_brake_ms = 0;

volatile uint8_t motor_dir = 0; // 0: 전진, 1: 후진
uint16_t current_pwm_dc = 0;    // 현재 DC 모터 PWM (감속용 상태 변수)
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void Control_Servo(float roll);
void Control_DC_Motor(uint16_t accel_ms, uint16_t brake_ms);
void Update_Motor_Direction(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#define PLD_S 32
uint8_t rx_buffer[PLD_S] = {0};
volatile uint8_t new_data_flag = 0;
volatile float received_roll = 0.0f;

float roll = 0;
uint16_t accel_ms = 0;
uint16_t brake_ms = 0;

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
  MX_TIM1_Init();
  MX_TIM4_Init();
  /* USER CODE BEGIN 2 */
  SSD1306_Init();
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3); // servo pwm
  HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_ALL); //encoder
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4); // dc motor pwm
  HAL_TIM_Base_Start_IT(&htim2);
  HAL_TIM_Base_Start_IT(&htim4);

  // 모터 전진 방향 설정
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);  // in1 = 0
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);    // in2 = 1

  csn_high();
  ce_high();
  HAL_Delay(5);
  ce_low();

  nrf24_init();

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

  uint8_t rx_addr[5] = {0x45, 0x55, 0x67, 0x10, 0x21};
  nrf24_open_rx_pipe(0, rx_addr);

  nrf24_listen();   // 수신 대기 시작



  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
    while (1)
    {
    	SSD1306_Clear();
    	//test
//    	Control_Servo(-30);
//    	Control_DC_Motor(500, 0);
        if (new_data_flag)
        {
            new_data_flag = 0;

            float roll = received_roll;
            uint16_t accel_ms = received_accel_ms;
            uint16_t brake_ms = received_brake_ms;

            // OLED 출력
            char buffer1[20], buffer2[20], buffer3[20];

            snprintf(buffer1, sizeof(buffer1), "Roll: %.2f", roll);
            snprintf(buffer2, sizeof(buffer2), "ACC: %ums", accel_ms);
            snprintf(buffer3, sizeof(buffer3), "BRK: %ums", brake_ms);

            SSD1306_GotoXY(0, 0);
            SSD1306_Puts(buffer1, &Font_11x18, 1);

            SSD1306_GotoXY(0, 20);
            SSD1306_Puts(buffer2, &Font_11x18, 1);

            SSD1306_GotoXY(0, 40);
            SSD1306_Puts(buffer3, &Font_11x18, 1);

            SSD1306_UpdateScreen();

            // 서보 모터 제어
            Control_Servo(roll);
            Control_DC_Motor(accel_ms, brake_ms);
            SSD1306_Clear();
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
    if (GPIO_Pin == GPIO_PIN_3)  // IRQ 핀
    {
        if (nrf24_data_available())
        {
            nrf24_receive(rx_buffer, PLD_S);

            if (rx_buffer[0] == 1)
            {
                int16_t roll_encoded = 0;
                memcpy(&roll_encoded, &rx_buffer[1], sizeof(int16_t));
                received_roll = ((float)roll_encoded) / 100.0f;

                memcpy(&received_accel_ms, &rx_buffer[3], sizeof(uint16_t));
                memcpy(&received_brake_ms, &rx_buffer[5], sizeof(uint16_t));

                new_data_flag = 1;
            }
        }
    }
}

//void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) // RX
//{
//  if (GPIO_Pin == GPIO_PIN_3) // IRQ
//  {
//    if (nrf24_data_available())
//    {
//      nrf24_receive(rx_buffer, PLD_S);
//
//      if (rx_buffer[0] == 1)
//      {
//        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_4);  // PB4 LED 토글
//      }
//    }
//  }
//}

// 서보 모터 제어 함수
void Control_Servo(float roll)
{
	if (roll < -90.0f) roll = -90.0f;
	if (roll >  90.0f) roll = 90.0f;

	// roll [-90 ~ 90] → angle [0 ~ 180]
	float angle = roll + 90.0f;

	// servo pwm : 우회전최대 = 1200, 중립 = 1550, 좌회전최대 = 1900
	uint16_t pwm_servo = (uint16_t)(1200 + (angle / 180.0f) * (1900-1200));

	// 출력 제한 (안정성 보장)
	if (pwm_servo < 1200) pwm_servo = 1200;
	if (pwm_servo > 1900) pwm_servo = 1900;

	__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, pwm_servo);
}

// DC 모터 제어 함수
void Control_DC_Motor(uint16_t accel_ms, uint16_t brake_ms)
{
    static int16_t duty = 0;  // PWM duty값 (static으로 유지)
    const int16_t accel_step = 10;     // 가속 증가폭
    const int16_t brake_step = 30;     // 브레이크 감소폭
    const int16_t max_duty = 1000;     // 최대 duty (ARR와 맞춰야 함)

    // 1. 브레이크 우선 처리
    if (brake_ms > 0)
    {
        duty -= brake_step;
    }
    // 2. 가속 입력 처리
    else if (accel_ms > 0)
    {
        if (duty == 0)
            duty = 200;  // Deadzone 극복을 위한 초기값 설정

        duty += accel_step;
    }
    // 3. 엑셀에서 발 뗀 상태 → 감속
    else if (accel_ms == 0)
    {
        duty -= accel_step;
    }

    // 4. PWM duty 범위 제한
    if (duty < 0)
        duty = 0;
    if (duty > max_duty)
        duty = max_duty;

    // 5. PWM 출력 적용 (htim1, TIM_CHANNEL_4 사용)
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, duty);
}

// 전/후진 설정 (PWM이 0일 때만)
void Update_Motor_Direction(void)
{
	if (current_pwm_dc == 0)
	{
		if (motor_dir == 0)
		{
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET); // IN1 = 0
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);   // IN2 = 1
		}
		else
		{
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);   // IN1 = 1
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET); // IN2 = 0
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

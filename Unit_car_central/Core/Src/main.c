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
#include <stdio.h>
#include <math.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
// 상수 정의
// (45~47) rpm 계산용
#define PPR 11
#define GEAR_RATIO 21.3f
#define TICKS_PER_REV (PPR * GEAR_RATIO * 4)

// NRF24 수신 페이로드 크기 정의 (Byte)
#define PLD_S 32
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
// (56~64)방향 제어 매크로
#define MOTOR_FORWARD()   do { \
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET); /* IN1 = 0 */ \
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);   /* IN2 = 1 */ \
} while (0)

#define MOTOR_BACKWARD()  do { \
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);   /* IN1 = 1 */ \
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET); /* IN2 = 0 */ \
} while (0)

// (67~72) 방향 enum
typedef enum {
    DIRECTION_BACKWARD = 0,
    DIRECTION_FORWARD = 1
} MotorDirection;

volatile MotorDirection motor_direction = DIRECTION_FORWARD;

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
// 전역변수
// (82~85) RF 수신 관련
volatile uint16_t received_accel_ms = 0;
volatile uint16_t received_brake_ms = 0;
volatile float received_roll = 0.0f;
volatile uint8_t new_data_flag = 0;
uint8_t rx_buffer[PLD_S] = {0};

// 모터 RPM
float motor_rpm = 0;

// (95) encoder debug 시 활성화
//uint8_t encoder_direction = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void Control_Servo(float roll);
void Control_DC_Motor(uint16_t accel_ms, uint16_t brake_ms);
void Update_Motor_Direction(MotorDirection motor_direction);
void Parse_RF_Data(uint8_t* buffer);
void NRF24_Rx_Init(void);
void Update_Motor_RPM(void);
void OLED_Display_Status(float roll, uint16_t accel_ms, uint16_t brake_ms, float motor_rpm, MotorDirection motor_direction);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
// =============================
// NRF24 수신 패킷 구조 정의
// -----------------------------
// Byte 0   : 메시지 식별자 (1이면 주행 명령 수신)
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
  MX_TIM1_Init();
  MX_TIM4_Init();
  /* USER CODE BEGIN 2 */

  // 1. 주변 장치 초기화
  SSD1306_Init();
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3); // 서보 PWM 시작
  HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_ALL); //encoder 시작
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4); // DC모터 PWM 시작

  // 2. 초기 모터 방향 설정 (실제 제어 핀 설정)
  MOTOR_FORWARD();

  // 3. NRF24 설정 (안정성을 위해 초기화 실패 처리 루틴 추가)
  NRF24_Rx_Init();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
    while (1)
    {
    	// 1. 모터 RPM 계산
    	Update_Motor_RPM();

		// 2. 모터 방향 반영 (변화 시만)
		Update_Motor_Direction(motor_direction);

		// 3. RF 수신 처리 플래그 확인
        if (new_data_flag)
        {
            new_data_flag = 0;

            float roll = received_roll;
            uint16_t accel_ms = received_accel_ms;
            uint16_t brake_ms = received_brake_ms;

            // 4. OLED 출력
            OLED_Display_Status(roll, accel_ms, brake_ms, motor_rpm, motor_direction);

	        // 5. 서보 및 DC 모터 제어
            Control_Servo(roll);
            Control_DC_Motor(accel_ms, brake_ms);
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

            Parse_RF_Data(rx_buffer);
        }
    }
}

// RF 송신 데이터 파싱 함수
void Parse_RF_Data(uint8_t* rx_buffer)
{
	if (rx_buffer[0] == 1)
	{
		int16_t roll_encoded = 0;
		memcpy(&roll_encoded, &rx_buffer[1], sizeof(int16_t));
		received_roll = ((float)roll_encoded) / 100.0f;

		memcpy((void*)&received_accel_ms, &rx_buffer[3], sizeof(uint16_t));
		memcpy((void*)&received_brake_ms, &rx_buffer[5], sizeof(uint16_t));

		motor_direction = (MotorDirection)rx_buffer[7];  // enum 적용, forward : 1, backward : 0

		new_data_flag = 1;
	}
}

// RF 초기화 함수
void NRF24_Rx_Init(void)
{
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

	static uint8_t rx_addr[5] = {0x45, 0x55, 0x67, 0x10, 0x21};
	nrf24_open_rx_pipe(0, rx_addr);
	nrf24_listen();   // 수신 대기 시작
}

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
    const int16_t accel_step = 50;     // 가속 증가폭
    const int16_t brake_step = 100;     // 브레이크 감소폭
    const int16_t max_duty = 1000;     // 최대 duty (ARR와 맞춰야 함)

    // 1. 브레이크
    if (brake_ms > 0)
    {
    	duty -= brake_step;
    }
    // 2. 가속 입력 처리
    else if (accel_ms > 0)
    {
        if (duty == 0)
            duty = 300; // 모터 정지 상태에서 일정 수준 이상의 PWM이 있어야 회전이 시작되기 때문에 Deadzone 극복을 위한 초기값 설정

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

// 모터 방향 상태 변화 시에만 GPIO 출력 수행
void Update_Motor_Direction(MotorDirection motor_dir)
{
    static MotorDirection prev = DIRECTION_FORWARD;

    if (motor_direction != prev) {
        prev = motor_direction;
        if (motor_direction == DIRECTION_FORWARD)
            MOTOR_FORWARD();
        else
            MOTOR_BACKWARD();
    }
}

// Encoder 모터 RPM 계산 함수
void Update_Motor_RPM(void)
{
	static int16_t last_encoder = 0;
	static uint32_t last_rpm_tick =0;

	uint32_t now = HAL_GetTick();

	// 100ms마다 계산
	if (now - last_rpm_tick >= 100)
	{
		last_rpm_tick = now;

		int16_t enc = (int16_t)__HAL_TIM_GET_COUNTER(&htim4);
		int16_t delta = (int16_t)(enc - last_encoder);
		last_encoder = enc;

//			// 방향 추정 encoder debug시 필요
//			if (delta > 0) encoder_direction = 1;     // FWD
//			else if (delta < 0) encoder_direction = 0; // REV

		// RPM 계산 (100ms 기준 → 10회/초 → ×10 ×60) (math.h -> fabsf : 절댓값)
		motor_rpm = fabsf(((float)delta / TICKS_PER_REV) * 10.0f * 60.0f);

	}

}

// OLED 출력 함수
void OLED_Display_Status(float roll, uint16_t accel_ms, uint16_t brake_ms, float motor_rpm, MotorDirection motor_direction)
{
	SSD1306_Clear();  // 여기에서만 Clear (화면 깜빡임 최소화)

	char buffer[32];

	SSD1306_GotoXY(0, 0);
	snprintf(buffer, sizeof(buffer), "Roll: %.2f", roll);
	SSD1306_Puts(buffer, &Font_7x10, 1);

	SSD1306_GotoXY(0, 10);
	snprintf(buffer, sizeof(buffer), "ACC: %ums", accel_ms);
	SSD1306_Puts(buffer, &Font_7x10, 1);

	SSD1306_GotoXY(0, 20);
	snprintf(buffer, sizeof(buffer), "BRK: %ums", brake_ms);
	SSD1306_Puts(buffer, &Font_7x10, 1);

	SSD1306_GotoXY(0, 30);
	snprintf(buffer, sizeof(buffer), "RPM: %.1f", motor_rpm);
	SSD1306_Puts(buffer, &Font_7x10, 1);

	SSD1306_GotoXY(0, 40);
	snprintf(buffer, sizeof(buffer), "DIR: %s", motor_direction == DIRECTION_FORWARD ? "FWD" : "BWD");
	SSD1306_Puts(buffer, &Font_7x10, 1);

	SSD1306_UpdateScreen();
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

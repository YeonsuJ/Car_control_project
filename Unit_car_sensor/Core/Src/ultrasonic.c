#include "ultrasonic.h"
#include "tim.h"

/* 외부 핸들 */
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim4;

// 전역변수
volatile uint32_t ic_val1_front = 0;
volatile uint32_t ic_val2_front = 0;
volatile uint8_t is_first_captured_front = 0;
volatile uint32_t distance_front = 0;

volatile uint32_t ic_val1_rear = 0;
volatile uint32_t ic_val2_rear = 0;
volatile uint8_t is_first_captured_rear = 0;
volatile uint32_t distance_rear = 0;

// tim2를 delay로 사용
void delay_us(uint16_t us)
{
  __HAL_TIM_SET_COUNTER(&htim2, 0);
  while (__HAL_TIM_GET_COUNTER(&htim2) < us);
}

// 모듈 초기화
void Ultrasonic_Init(void)
{
	HAL_TIM_Base_Start(&htim2);  // 타이머 카운터만 시작
	HAL_TIM_IC_Start_IT(&htim4, TIM_CHANNEL_1);
	HAL_TIM_IC_Start_IT(&htim4, TIM_CHANNEL_2);
}

void Ultrasonic_Trigger(void)
{
	// 1) 전방 트리거
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET);
	delay_us(10);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET);

	// 2) 후방 트리거
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
	delay_us(10);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
}

// input 캡처 콜백 함수
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

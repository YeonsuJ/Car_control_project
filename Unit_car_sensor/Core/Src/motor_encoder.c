/*
 * motor_encoder.c
 *
 * Created on: Aug 15, 2025
 * Author: itnc
 */
#include <math.h>
#include "motor_encoder.h"
#include "tim.h"

// --- 상수 정의 ---
#define PPR 8
#define GEAR_RATIO 21.3f
#define TICKS_PER_REV (PPR * GEAR_RATIO * 4)
// 저주파 통과 필터 계수 (0.0 ~ 1.0 사이, 값이 클수록 반응이 빠르고 작을수록 부드러워짐)
#define RPM_FILTER_ALPHA 0.3f

// --- 타이머 핸들 ---
extern TIM_HandleTypeDef htim1; // DC 모터 엔코더 타이머

// --- static 변수 ---
static float motor_rpm = 0.0f;
static float filtered_rpm = 0.0f;

/**
 * @brief DWT Cycle Counter를 활성화하여 초정밀 타이머로 사용
 * @note  이 함수는 HAL_Init() 이후, RTOS 스케줄러 시작 전에 한 번만 호출하면 됩니다.
 */
void DWT_Init(void)
{
    // DWT 기능 활성화
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    // Cycle Counter 활성화
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    // Cycle Counter 리셋
    DWT->CYCCNT = 0;
}

/**
 * @brief 개선된 RPM 계산 함수
 */
void Update_Motor_RPM(void)
{
    static int16_t last_encoder_count = 0;
    static uint32_t last_cycle_count = 0;

    uint32_t current_cycle_count = DWT->CYCCNT;
    int16_t current_encoder_count = (int16_t)__HAL_TIM_GET_COUNTER(&htim1);

    // 경과 시간 및 엔코더 변화량 계산
    uint32_t delta_cycles = current_cycle_count - last_cycle_count;
    int16_t delta_encoder = current_encoder_count - last_encoder_count;

    // 다음 측정을 위해 현재 값 저장
    last_encoder_count = current_encoder_count;
    last_cycle_count = current_cycle_count;

    // 경과 시간(초) 계산
    float delta_time_s = (float)delta_cycles / (float)SystemCoreClock;

    if (delta_time_s > 0)
    {
        // 1. 실제 경과 시간을 이용해 정확한 RPM 계산
        float pps = (float)delta_encoder / delta_time_s; // Pulses Per Second
        float raw_rpm = fabsf((pps / TICKS_PER_REV) * 60.0f);

        // 2. 저주파 통과 필터(IIR) 적용하여 값을 부드럽게 만듦
        filtered_rpm = (RPM_FILTER_ALPHA * raw_rpm) + ((1.0f - RPM_FILTER_ALPHA) * filtered_rpm);
        motor_rpm = filtered_rpm;
    }
}

float MotorControl_GetRPM(void)
{
    return motor_rpm;
}

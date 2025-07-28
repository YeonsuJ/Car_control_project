#include "battery_monitor.h"
#include "adc.h"

// 이동 평균 필터 내부 변수
#define FILTER_SIZE 10
#define VOUT_CORRECTION_FACTOR 1.010f

// 내부 변수
float vout_buffer[FILTER_SIZE] = {0};  // 최근 10개 저장
uint8_t filter_index = 0;
uint8_t valid_sample_count = 0;

// 이동 평균 계산 함수
float Get_Averaged_Vout(float new_sample)
{
    vout_buffer[filter_index] = new_sample;
    filter_index = (filter_index + 1) % FILTER_SIZE;

    if (valid_sample_count < FILTER_SIZE)
        valid_sample_count++;

    float sum = 0.0f;
    for (int i = 0; i < valid_sample_count; i++)
        sum += vout_buffer[i];

    return sum / valid_sample_count;
}

//--- adc 전압출력 및 배터리 전압 → 퍼센트 계산 함수 ---
float Read_Battery_Percentage(float* vout_ret)
{
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
    uint16_t adc_val = HAL_ADC_GetValue(&hadc1);
    HAL_ADC_Stop(&hadc1);

    // 1. ADC → STM32 입력 전압 (Vout raw)
    float vout_raw = (3.3f * adc_val) / 4095.0f;

    // 2. 고정 보정 계수 적용
    float vout = vout_raw * VOUT_CORRECTION_FACTOR;

    // 3. 이동 평균 필터 적용
    vout = Get_Averaged_Vout(vout);

    // 4. 분압 비율에 따라 Vbat 복원 (ex: 11.46V → 2.92V 였다면)
    float vbat = vout * (11.46f / 2.92f);  // 실측 기반 비율

    // 5. Vbat → 퍼센트 환산 (최대 12.6V, 최소 9.6V 기준)
    float percent = ((vbat - 9.6f) / (12.6f - 9.6f)) * 100.0f;

    // 6. 범위 제한
    if (percent > 100.0f) percent = 100.0f;
    if (percent < 0.0f) percent = 0.0f;

    if (vout_ret != NULL)
        *vout_ret = vout;

    return percent;
}

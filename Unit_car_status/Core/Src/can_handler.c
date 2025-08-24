#include "can_handler.h"
#include "can.h"
#include "oled_display.h"
#include "led_control.h"
#include <stdbool.h>
// =============================
// CAN 수신 패킷 구조
// - 0x6A5 : [0] 거리 위험, [1] LDR 기반 어두움 여부
// - 0x321 : [0] 전진/후진, [1] 브레이크 여부
// =============================


// can 수신을 위한 필터 및 수신 버퍼 구성
CAN_FilterTypeDef sFilterConfig;
CAN_RxHeaderTypeDef RxHeader;
uint8_t RxData[8];

extern char oled_line1[21];

// 상태 변수
static uint8_t status_distance = 0;
static uint8_t status_light = 0;
static uint8_t status_direction = 0;
static uint8_t status_brake = 0;

// 마지막 CAN 수신 시간을 저장할 전역 변수
volatile uint32_t g_last_can_rx_time = 0;

void CANHandler_Init(void)
{
	extern CAN_HandleTypeDef hcan;
	HAL_CAN_Start(&hcan);
	CAN_Filter_Config(&hcan);
}

// can 필터 설정 및 인터럽트 활성화 함수
void CAN_Filter_Config(CAN_HandleTypeDef *hcan_ptr)
{
	// Configure the filter
	sFilterConfig.FilterActivation = CAN_FILTER_ENABLE;
	sFilterConfig.FilterFIFOAssignment = CAN_FILTER_FIFO1;
	sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
	sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;

	// 모든 ID 수신을 허용 (센서 0x6A5, 중앙 0x321)
	sFilterConfig.FilterIdHigh     = 0x0000;
	sFilterConfig.FilterIdLow      = 0x0000;
	sFilterConfig.FilterMaskIdHigh = 0x0000;
	sFilterConfig.FilterMaskIdLow  = 0x0000;

//	sFilterConfig.FilterIdHigh = 0x6A5<<5;
//	sFilterConfig.FilterIdLow = 0;
//	sFilterConfig.FilterMaskIdHigh = 0x7FF<<5; // SET 0 to unfilter
//	sFilterConfig.FilterMaskIdLow = 0;

	if (HAL_CAN_ConfigFilter(&hcan, &sFilterConfig) != HAL_OK)
	        Error_Handler(); // 실패 시 무한 루프 등 처리

	// Activate the notification
	if (HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO1_MSG_PENDING) != HAL_OK)
	        Error_Handler(); // 실패 시 처리
}

// CAN 수신 인터럽트 콜백 함수 (FIFO1)
void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	// 메시지를 받았으므로 현재 시간을 기록
	g_last_can_rx_time = HAL_GetTick();

	// 각 상태를 나타내는 비트 마스크 정의
	#define STATUS_FRONT_DETECT (1 << 0)
	#define STATUS_REAR_DETECT  (1 << 1)

    HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO1, &RxHeader, RxData);

    if (RxHeader.StdId == 0x6A5 && RxHeader.DLC >=1)
    {
        status_distance = RxData[0];
        status_light = RxData[1];

        // 비트 마스크를 사용하여 상태 확인
		bool isFrontDetected = (status_distance & STATUS_FRONT_DETECT);
		bool isRearDetected = (status_distance & STATUS_REAR_DETECT);

		if (isFrontDetected && isRearDetected) // 둘 다 감지
		{
			OLED_SetObstacleStatus("BOTH");
		}
		else if (isFrontDetected) // 전방만 감지
		{
			OLED_SetObstacleStatus("FRONT");
		}
		else if (isRearDetected) // 후방만 감지
		{
			OLED_SetObstacleStatus("REAR");
		}
		else // 둘 다 미감지
		{
			OLED_SetObstacleStatus("NONE");
		}
    }
    else if (RxHeader.StdId == 0x321 && RxHeader.DLC >=3)
    {
        status_direction = RxData[0];
        status_brake = RxData[1];
        bool rf_ok = (bool)RxData[2];
        OLED_SetRFStatus(rf_ok);
    }

    // LED 상태 업데이트 (독립 처리)
    LEDControl_Update(status_light, status_direction, status_brake);
}

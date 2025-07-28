#include "can_handler.h"
#include "can.h"
#include "oled_display.h"

// =============================
// CAN 수신(Rx) 패킷 구조 정의
// -----------------------------
// ▶ CAN 수신 ID:
//   - StdId: 0x6A5 (슬레이브 → 마스터)
//   - 필터 모드: 11비트 표준 ID, 마스크 0x7FF (전비트 일치)
// ▶ 수신 헤더 (RxHeader):
//   - RxHeader.StdId : 송신 측 ID 확인용 (0x6A5 expected)
//   - RxHeader.DLC   : 데이터 길이 (보통 1)
// ▶ 수신 데이터 (RxData):
//   - RxData[0] : 거리 조건 결과 (uint8_t, 거리값에따라 1 or 0 수신)
//   - RxData[1] ~ RxData[7] : 예비, 현재 미사용
// =============================

// can 수신을 위한 필터 및 수신 버퍼 구성
CAN_FilterTypeDef sFilterConfig;
CAN_RxHeaderTypeDef RxHeader;
uint8_t RxData[8];

extern char oled_line1[21];


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
	sFilterConfig.FilterIdHigh = 0x6A5<<5;
	sFilterConfig.FilterIdLow = 0;
	sFilterConfig.FilterMaskIdHigh = 0x7FF<<5; // SET 0 to unfilter
	sFilterConfig.FilterMaskIdLow = 0;
	sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;

	if (HAL_CAN_ConfigFilter(&hcan, &sFilterConfig) != HAL_OK)
	        Error_Handler(); // 실패 시 무한 루프 등 처리

	// Activate the notification
	if (HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO1_MSG_PENDING) != HAL_OK)
	        Error_Handler(); // 실패 시 처리
}

// CAN 수신 인터럽트 콜백 함수 (FIFO1)
void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO1, &RxHeader, RxData);

    if (RxHeader.StdId == 0x6A5 && RxHeader.DLC == 1)
    {
    	if (RxData[0] == 1)
			OLED_SetStatus("DETECTED");
		else if (RxData[0] == 0)
			OLED_SetStatus("SAFE");
		else
			OLED_SetStatus("UNKNOWN");
    }
}

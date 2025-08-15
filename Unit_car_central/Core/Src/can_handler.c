#include "can_handler.h"
#include "can.h" // hcan 정의가 여기에 있을 수 있습니다.
#include "cmsis_os.h" // osSemaphoreRelease를 사용하기 위해 추가

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

// ⭐️ freertos.c에 정의된 CANRxQueueHandle을 외부에서 참조
extern osMessageQueueId_t CANRxQueueHandle;

CAN_FilterTypeDef sFilterConfig;
CAN_RxHeaderTypeDef RxHeader;
uint8_t RxData[8];

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

/**
 * @brief CAN 수신 인터럽트 콜백 함수 (FIFO1)
 */
void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
  HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO1, &RxHeader, RxData);

  if (RxHeader.StdId == 0x6A5 && RxHeader.DLC >= 1)
  {
      // ⭐️ 수신한 데이터(RxData[0])를 CANRxQueue에 직접 전송
      // 이 함수 호출 하나로 데이터 전송과 CANTask 깨우기가 모두 완료됩니다.
      osMessageQueuePut(CANRxQueueHandle, &RxData[0], 0U, 0U);
  }
}

// 차량 상태 can 송신 함수 구현
void CAN_Send_DriveStatus(uint8_t direction, uint8_t brake_status)
{
    CAN_TxHeaderTypeDef TxHeader;
    uint8_t TxData[2];
    uint32_t TxMailbox;

    TxHeader.StdId = 0x321;  // 예시 ID, sensor에서 수신 필터 설정 필요
    TxHeader.IDE = CAN_ID_STD;
    TxHeader.RTR = CAN_RTR_DATA;
    TxHeader.DLC = 2;
    TxHeader.TransmitGlobalTime = DISABLE;

    TxData[0] = direction;
    TxData[1] = brake_status;

    if (HAL_CAN_AddTxMessage(&hcan, &TxHeader, TxData, &TxMailbox) != HAL_OK)
    {
        Error_Handler();  // 전송 실패 시 처리
    }
}


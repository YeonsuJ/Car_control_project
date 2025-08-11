#include "can_handler.h"
#include "can.h" // extern CAN_HandleTypeDef hcan; 이 헤더에 존재해야 함

// 전역 변수는 그대로
CAN_FilterTypeDef sFilterConfig;
CAN_RxHeaderTypeDef RxHeader;
uint8_t RxData[8];
volatile uint8_t can_distance_signal = 0;

/* CANHandler_Init: 외부에 정의된 hcan을 사용하거나,
   필요하면 CAN_HandleTypeDef*를 인자로 받아 호출하도록 변경 가능 */
void CANHandler_Init(void)
{
    extern CAN_HandleTypeDef hcan; // can.h에 extern 선언이 있어야 함

    if (HAL_CAN_Start(&hcan) != HAL_OK) {
        Error_Handler(); // 실패 시 원인 확인
    }

    // 필터 설정에 전역 hcan 대신 포인터를 명시적으로 전달
    CAN_Filter_Config(&hcan);
}

// CAN 필터 설정 및 인터럽트 활성화 함수 (핸들 파라미터 사용)
void CAN_Filter_Config(CAN_HandleTypeDef *hcan_ptr)
{
   // Configure the filter (32-bit scale, mask mode -> exact match)
   sFilterConfig.FilterActivation = CAN_FILTER_ENABLE;
   sFilterConfig.FilterFIFOAssignment = CAN_FILTER_FIFO1;
   sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
   sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;

   // For 11-bit standard IDs -> left shift by 5 (per STM HAL convention)
   sFilterConfig.FilterIdHigh = (uint16_t)((0x6A5 << 5) & 0xFFFF);
   sFilterConfig.FilterIdLow  = 0x0000;
   sFilterConfig.FilterMaskIdHigh = (uint16_t)((0x7FF << 5) & 0xFFFF); // mask all bits (exact match)
   sFilterConfig.FilterMaskIdLow  = 0x0000;
   sFilterConfig.FilterBank = 0; // 필요시 필터 뱅크 지정 (CubeMX 설정과 일치)

   if (HAL_CAN_ConfigFilter(hcan_ptr, &sFilterConfig) != HAL_OK) {
       // 필터 설정 실패 시 에러 원인을 로그/디버그 확인
       Error_Handler();
   }

   // Activate the notification for FIFO1 message pending
   if (HAL_CAN_ActivateNotification(hcan_ptr, CAN_IT_RX_FIFO1_MSG_PENDING) != HAL_OK) {
       Error_Handler();
   }
}

// CAN 수신 콜백 (FIFO1)
void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
  if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO1, &RxHeader, RxData) != HAL_OK) {
      // 수신 읽기 실패 시 처리(로그 등)
      return;
  }

  if ( (RxHeader.StdId == 0x6A5) && (RxHeader.DLC >= 1) ) {
      can_distance_signal = RxData[0];
  }
}

// CAN 송신 예제 (전역 hcan 사용)
void CAN_Send_DriveStatus(uint8_t direction, uint8_t brake_status)
{
    extern CAN_HandleTypeDef hcan;
    CAN_TxHeaderTypeDef TxHeader;
    uint8_t TxData[2];
    uint32_t TxMailbox;

    TxHeader.StdId = 0x321;
    TxHeader.IDE = CAN_ID_STD;
    TxHeader.RTR = CAN_RTR_DATA;
    TxHeader.DLC = 2;
    TxHeader.TransmitGlobalTime = DISABLE;

    TxData[0] = direction;
    TxData[1] = brake_status;

    if (HAL_CAN_AddTxMessage(&hcan, &TxHeader, TxData, &TxMailbox) != HAL_OK) {
        // 전송 실패 또는 메일박스 가득 참 -> 처리
        Error_Handler();
    }
}

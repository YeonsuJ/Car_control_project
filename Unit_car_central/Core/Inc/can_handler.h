#ifndef INC_CAN_HANDLER_H_
#define INC_CAN_HANDLER_H_

#include "main.h"
// =============================
// CAN 수신 메시지 설명:
// ID 0x6A5, RxData[0] = 거리 조건 (1: 위험, 0: 안전)
// =============================

// can 수신을 위한 필터 및 수신 버퍼 구성
extern CAN_FilterTypeDef sFilterConfig;
extern CAN_RxHeaderTypeDef RxHeader;
extern uint8_t RxData[8];
extern volatile uint8_t can_distance_signal;

void CANHandler_Init(void);
void CAN_Filter_Config(CAN_HandleTypeDef *hcan_ptr);
void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan);
void CAN_Send_DriveStatus(uint8_t direction, uint8_t brake_status);

#endif /* INC_CAN_HANDLER_H_ */

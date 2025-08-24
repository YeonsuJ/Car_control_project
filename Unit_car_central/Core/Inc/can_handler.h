#ifndef INC_CAN_HANDLER_H_
#define INC_CAN_HANDLER_H_

#include "main.h"
// =============================
// CAN 수신 메시지 설명:
// ID 0x6A5, RxData[0] = 거리 조건 (1: 위험, 0: 안전) , RxData[2] = 모터 RPM 값
// =============================

typedef struct {
    uint8_t distance_signal; // RxData[0]를 저장할 변수
    uint16_t motor_rpm;   // RxData[2]를 저장할 변수
} CAN_RxPacket_t;

// can 수신을 위한 필터 및 수신 버퍼 구성
extern CAN_FilterTypeDef sFilterConfig;
extern CAN_RxHeaderTypeDef RxHeader;
extern uint8_t RxData[8];

void CANHandler_Init(void);
void CAN_Filter_Config(CAN_HandleTypeDef *hcan_ptr);
void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan);
void CAN_Send_DriveStatus(uint8_t direction, uint8_t brake_status, uint8_t rf_status);

#endif /* INC_CAN_HANDLER_H_ */

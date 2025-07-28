#include "can_handler.h"
#include "can.h"

CAN_TxHeaderTypeDef TxHeader;
uint32_t TxMailbox;
uint8_t TxData[1]; // 외부 접근을 위해 전역 선언


void CAN_tx_Init(void)
{
	HAL_CAN_Start(&hcan);

	TxHeader.DLC = 1;  // data length : 1바이트만 전송
	TxHeader.IDE = CAN_ID_STD;
	TxHeader.RTR = CAN_RTR_DATA;
	TxHeader.StdId = 0x6A5;  // ID can be between Hex1 and Hex7FF (1-2047 decimal)
}

void CAN_Send(void)
{
	HAL_CAN_AddTxMessage(&hcan, &TxHeader, TxData, &TxMailbox);
}

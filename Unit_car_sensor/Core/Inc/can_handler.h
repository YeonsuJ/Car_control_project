/*
 * can_tx_handler.h
 *
 *  Created on: Jul 24, 2025
 *      Author: itnc
 */

#ifndef INC_CAN_TX_HANDLER_H_
#define INC_CAN_TX_HANDLER_H_

#include "main.h"

extern CAN_TxHeaderTypeDef TxHeader;
extern uint32_t TxMailbox;
extern uint8_t TxData[3];

void CAN_tx_Init(void);   /* HAL_CAN_Start + 헤더 설정 */
void CAN_Send(void);   /* TxData[0] 전송 */

#endif /* INC_CAN_TX_HANDLER_H_ */

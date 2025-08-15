#ifndef INC_RF_HANDLER_H_
#define INC_RF_HANDLER_H_

#include "main.h"
#include <stdbool.h> // bool 타입을 위해 추가

// 수신된 주행 명령 데이터를 담을 구조체
typedef struct {
    float roll;
    uint16_t accel_ms;
    uint16_t brake_ms;
    uint8_t direction;
} VehicleCommand_t;

/**
 * @brief  통신 모듈을 수신(Rx) 모드로 초기화
 */

void RFHandler_Init(void);

/**
 * @brief  NRF24 IRQ 핀 인터럽트 발생 시 호출될 콜백 함수
 */
void RFHandler_IrqCallback(void);

/**
 * @brief  다음에 전송할 ACK 페이로드 데이터를 설정
 * @param  signal: ACK 페이로드에 담을 8비트 신호 (can_distance_signal)
 */
void RFHandler_SetAckPayload(uint8_t signal);

/**
 * @brief  새로운 주행 명령이 수신되었는지 확인하고, 있다면 데이터를 파싱하여 반환
 * @param  command: 파싱된 명령 데이터를 저장할 구조체의 포인터
 * @retval bool: 새로운 명령이 수신되어 처리되었으면 true, 아니면 false
 */
bool RFHandler_GetNewCommand(VehicleCommand_t* command);

#endif /* INC_RF_HANDLER_H_ */

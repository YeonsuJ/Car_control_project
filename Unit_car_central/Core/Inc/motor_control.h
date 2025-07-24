#ifndef INC_MOTOR_CONTROL_H_
#define INC_MOTOR_CONTROL_H_

#include "main.h"
#include "rf_handler.h" // VehicleCommand_t 구조체를 사용하기 위해 포함

/**
 * @brief  Servo, DC 모터 제어에 필요한 모든 주변장치를 초기화
 */
void MotorControl_Init(void);

/**
 * @brief  수신된 주행 명령에 따라 모든 모터의 상태를 업데이트
 * @param  command: CommHandler로부터 받은 주행 명령 구조체의 포인터
 */
void MotorControl_Update(const VehicleCommand_t* command);

/**
 * @brief 현재 모터의 RPM 값을 반환함
 * @retval float 현재 RPM 값
 */
float MotorControl_GetRPM(void);

#endif /* INC_MOTOR_CONTROL_H_ */

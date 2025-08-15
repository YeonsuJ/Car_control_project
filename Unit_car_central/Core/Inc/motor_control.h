#ifndef INC_MOTOR_CONTROL_H_
#define INC_MOTOR_CONTROL_H_

#include "main.h"
#include "rf_handler.h" // VehicleCommand_t 구조체를 사용하기 위해 포함

typedef enum {
    DIRECTION_BACKWARD = 0,
    DIRECTION_FORWARD = 1
} MotorDirection_t;

/**
 * @brief  Servo, DC 모터 제어에 필요한 모든 주변장치를 초기화
 */
void MotorControl_Init(void);

/**
 * @brief  수신된 주행 명령에 따라 모든 모터의 상태를 업데이트
 * @param  command: CommHandler로부터 받은 주행 명령 구조체의 포인터
 */
void MotorControl_Update(const VehicleCommand_t* command);

void Control_Servo(float roll);
void Control_DcMotor(uint16_t accel_ms, uint16_t brake_ms);
void Update_MotorDirection(MotorDirection_t direction);

#endif /* INC_MOTOR_CONTROL_H_ */

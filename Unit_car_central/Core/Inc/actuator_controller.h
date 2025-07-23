// actuator_controller.h

#ifndef INC_ACTUATOR_CONTROLLER_H_
#define INC_ACTUATOR_CONTROLLER_H_

#include "main.h"
#include "rf_handler.h" // VehicleCommand_t 구조체를 사용하기 위해 포함

/**
 * @brief  액추에이터(모터, 서보) 제어에 필요한 모든 주변장치를 초기화합니다.
 */
void ActuatorController_Init(void);

/**
 * @brief  수신된 주행 명령에 따라 모든 액추에이터의 상태를 업데이트합니다.
 * @param  command: CommHandler로부터 받은 주행 명령 구조체의 포인터
 */
void ActuatorController_Update(const VehicleCommand_t* command);

#endif /* INC_ACTUATOR_CONTROLLER_H_ */

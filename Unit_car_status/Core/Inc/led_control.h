
#ifndef INC_LED_CONTROL_H_
#define INC_LED_CONTROL_H_

#include "main.h"

// 외부에서 전달할 상태값
void LEDControl_Update(uint8_t ldr_dark, uint8_t direction, uint8_t brake);

#endif /* INC_LED_CONTROL_H_ */

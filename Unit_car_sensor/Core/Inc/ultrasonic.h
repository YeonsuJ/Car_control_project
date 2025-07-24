
#ifndef INC_ULTRASONIC_H_
#define INC_ULTRASONIC_H_

#include "main.h"

// 다른 파일(main.c 등)에서 사용할 수 있게 extern 선언
extern volatile uint32_t distance_front;
extern volatile uint32_t distance_rear;

void delay_us(uint16_t us);
void Ultrasonic_Init(void);
void Ultrasonic_Trigger(void);

#endif /* INC_ULTRASONIC_H_ */

/*
 * motor_encoder.h
 *
 *  Created on: Aug 15, 2025
 *      Author: itnc
 */

#ifndef INC_MOTOR_ENCODER_H_
#define INC_MOTOR_ENCODER_H_


/**
 * @brief 현재 모터의 RPM 값을 반환함
 * @retval float 현재 RPM 값
 */
float MotorControl_GetRPM(void);

void DWT_Init(void);


void Update_Motor_RPM(void);


#endif /* INC_MOTOR_ENCODER_H_ */

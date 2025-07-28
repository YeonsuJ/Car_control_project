/*
 * battery_monitor.h
 *
 *  Created on: Jul 28, 2025
 *      Author: itnc
 */

#ifndef INC_BATTERY_MONITOR_H_
#define INC_BATTERY_MONITOR_H_

#include "main.h"

float Read_Battery_Percentage(float* vout_ret);
float Get_Averaged_Vout(float new_sample);

#endif /* INC_BATTERY_MONITOR_H_ */

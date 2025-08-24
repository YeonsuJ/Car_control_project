#ifndef INC_OLED_DISPLAY_H_
#define INC_OLED_DISPLAY_H_
#include <stdbool.h>
#include <stdint.h>

void OLED_Init(void);  // 외부에서 호출할 초기화 함수
// OLED 상태 정보 표시 함수
void OLED_BatteryStatus(float percent, float vout);
void OLED_SetStatus(const char* status);
void OLED_SetCANStatus(bool is_ok);
void OLED_SetRFStatus(bool is_ok);
void OLED_SetObstacleStatus(const char* status);

#endif /* INC_OLED_DISPLAY_H_ */

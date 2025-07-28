#ifndef INC_OLED_DISPLAY_H_
#define INC_OLED_DISPLAY_H_

#include <stdint.h>

void OLED_Init(void);  // 외부에서 호출할 초기화 함수
// OLED 상태 정보 표시 함수
void OLED_BatteryStatus(const char* status, float percent, float vout);
void OLED_SetStatus(const char* status);


#endif /* INC_OLED_DISPLAY_H_ */

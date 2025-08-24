#include "oled_display.h"
#include "ssd1306.h"
#include "fonts.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h> // bool 타입을 사용하기 위해 추가


static char oled_line1[21];
static char oled_line2[21];
static char oled_line3[21];
static char oled_line4[21];

void OLED_Init(void)
{
    SSD1306_Init();
    SSD1306_Clear();
    SSD1306_UpdateScreen();
}

void OLED_SetObstacleStatus(const char* status)
{
    snprintf(oled_line1, sizeof(oled_line1), "Obstacle: %s", status);
}

/**
 * @brief CAN 통신 상태 문자열을 설정합니다.
 * @param is_ok: true이면 "SUC", false이면 "FAIL"로 설정됩니다.
 */
void OLED_SetCANStatus(bool is_ok)
{
    if (is_ok)
        snprintf(oled_line2, sizeof(oled_line2), "CAN : SUC");
    else
        snprintf(oled_line2, sizeof(oled_line2), "CAN : FAIL");
}

//RF 통신 상태를 설정 함수
/**
 * @brief RF 통신 상태 문자열을 설정합니다.
 * @param is_ok: true이면 "SUC", false이면 "FAIL"로 설정됩니다.
 */
void OLED_SetRFStatus(bool is_ok)
{
    if (is_ok)
        snprintf(oled_line3, sizeof(oled_line3), "RF : SUC");
    else
        snprintf(oled_line3, sizeof(oled_line3), "RF : FAIL");
}

void OLED_BatteryStatus(float percent, float vout)
{
    // 배터리 계산 후 문자열 구성
    snprintf(oled_line4, sizeof(oled_line4), "BAT:%3d%% (%.2fV)", (uint8_t)(percent + 0.5f), vout);

	SSD1306_Clear();
	SSD1306_GotoXY(0, 0);  SSD1306_Puts(oled_line1, &Font_7x10, 1);
	SSD1306_GotoXY(0, 20); SSD1306_Puts(oled_line2, &Font_7x10, 1);
	SSD1306_GotoXY(0, 30); SSD1306_Puts(oled_line3, &Font_7x10, 1);
	SSD1306_GotoXY(0, 45); SSD1306_Puts(oled_line4, &Font_7x10, 1);
	SSD1306_UpdateScreen();
}

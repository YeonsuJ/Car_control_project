#include "oled_display.h"
#include "ssd1306.h"
#include "fonts.h"
#include <stdio.h>
#include <string.h>

static char oled_line1[21] = "Status: ---";
static char oled_line2[21];
static char oled_line3[21];

void OLED_Init(void)
{
    SSD1306_Init();
    SSD1306_Clear();
    SSD1306_UpdateScreen();
}

void OLED_SetStatus(const char* status)
{
    snprintf(oled_line1, sizeof(oled_line1), "Status: %s", status);
}

void OLED_BatteryStatus(const char* status, float percent, float vout)
{
    if (status != NULL)
        OLED_SetStatus(status); // 상태 문자열 갱신

    // 배터리 계산 후 문자열 구성
    snprintf(oled_line2, sizeof(oled_line2), "BAT: %3d%%", (uint8_t)(percent + 0.5f)); // 소수점 반올림
	snprintf(oled_line3, sizeof(oled_line3), "VOL: %.2fV", vout);

	SSD1306_Clear();
	SSD1306_GotoXY(0, 0);  SSD1306_Puts(oled_line1, &Font_7x10, 1);
	SSD1306_GotoXY(0, 20); SSD1306_Puts(oled_line2, &Font_7x10, 1);
	SSD1306_GotoXY(0, 30); SSD1306_Puts(oled_line3, &Font_7x10, 1);
	SSD1306_UpdateScreen();
}

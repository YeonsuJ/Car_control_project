// boot_logger.c

#include "boot_logger.h"
#include <stdio.h>
#include <string.h>

extern I2C_HandleTypeDef hi2c2;
extern UART_HandleTypeDef huart2;

// 모듈 내부 전역 변수
static BootInfoHeader bootHeader;
static uint32_t current_boot_record_addr;
static uint32_t last_uptime_save_time = 0;

// 내부에서만 사용할 함수들
static void EEPROM_Write(uint16_t addr, uint8_t *data, uint16_t len);
static void EEPROM_Read(uint16_t addr, uint8_t *data, uint16_t len);
static void Format_Uptime(char* buf, uint32_t uptime_ms);

// --- 공개 함수 구현 ---

void BootLogger_Init(void)
{
    char uart_buf[256];
    char time_str[40]; // [추가] 포맷된 시간 문자열을 담을 버퍼
    char *msg;

    // [Init Step 1] 팩토리 리셋 플래그 확인
#if (PERFORM_EEPROM_FACTORY_RESET == 1)
    BootLogger_EraseAllRecords(); // 리셋 함수 호출 후 멈춤
#endif

    // [Init Step 2] EEPROM에서 부팅 헤더 정보 읽기
    EEPROM_Read(BOOT_HEADER_ADDR, (uint8_t*)&bootHeader, sizeof(BootInfoHeader));

    // [Init Step 3] 최초 부팅 여부 확인 및 초기화
    if (bootHeader.magic_number != BOOT_INFO_MAGIC_NUMBER)
    {
        msg = "\r\n--- First Boot Detected! Initializing EEPROM... ---\r\n";
        HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 100);

        bootHeader.magic_number = BOOT_INFO_MAGIC_NUMBER;
        bootHeader.boot_count = 0;
        EEPROM_Write(BOOT_HEADER_ADDR, (uint8_t*)&bootHeader, sizeof(BootInfoHeader));
    }

    // [Init Step 4] 데이터 덤프 플래그 확인
#if (ENABLE_BOOT_LOG_DUMP_ON_STARTUP == 1)
    if (bootHeader.boot_count > 0)
    {
        BootLogger_DumpAllRecords();
    }
#endif

    // [Init Step 5] 이전 부팅 기록 출력
    if (bootHeader.boot_count > 0)
    {
        uint32_t prev_boot_record_addr = BOOT_RECORDS_START_ADDR + (bootHeader.boot_count - 1) * sizeof(BootRecord);
        BootRecord prev_record;
        EEPROM_Read(prev_boot_record_addr, (uint8_t*)&prev_record, sizeof(BootRecord));

        // 헬퍼 함수를 사용하여 시간 포맷 변경
        Format_Uptime(time_str, prev_record.uptime_ms);

        sprintf(uart_buf, "\r\n--- Previous Boot Information ---\r\nBoot Count: #%lu\r\nUptime: %s\r\n", bootHeader.boot_count, time_str); // ms 대신 포맷된 문자열 사용
        HAL_UART_Transmit(&huart2, (uint8_t*)uart_buf, strlen(uart_buf), 1000);
    }

    // [Init Step 6] 현재 부팅 세션 준비
    bootHeader.boot_count++;
    EEPROM_Write(BOOT_HEADER_ADDR, (uint8_t*)&bootHeader, sizeof(BootInfoHeader));
    current_boot_record_addr = BOOT_RECORDS_START_ADDR + (bootHeader.boot_count - 1) * sizeof(BootRecord);

    sprintf(uart_buf, "\r\n--- Current Boot Session ---\r\nBoot Count: #%lu\r\nRecording uptime...\r\n\r\n", bootHeader.boot_count);
    HAL_UART_Transmit(&huart2, (uint8_t*)uart_buf, strlen(uart_buf), 1000);

    BootRecord initial_record = {0};
    EEPROM_Write(current_boot_record_addr, (uint8_t*)&initial_record, sizeof(BootRecord));
}

void BootLogger_UpdateUptime(void)
{
    uint32_t now = HAL_GetTick();
    if (now - last_uptime_save_time >= UPTIME_SAVE_INTERVAL_MS)
    {
        last_uptime_save_time = now;
        BootRecord current_record = { .uptime_ms = now };
        EEPROM_Write(current_boot_record_addr, (uint8_t*)&current_record, sizeof(BootRecord));
    }
}

/**
  * @brief EEPROM에 데이터를 쓰는 헬퍼 함수
  * @param addr: EEPROM 내부 메모리 주소
  * @param data: 쓸 데이터 포인터
  * @param len: 데이터 길이
  * @retval None
  */
void EEPROM_Write(uint16_t addr, uint8_t *data, uint16_t len)
{
    // 참고: 24LCxx 시리즈 EEPROM은 페이지 쓰기 제한이 있을 수 있습니다.
    // 여기서는 간단하게 구현했지만, 대용량 데이터를 한 번에 쓸 때는 페이지 경계를 고려해야 합니다.
    HAL_I2C_Mem_Write(&hi2c2, EEPROM_ADDR, addr, I2C_MEMADD_SIZE_16BIT, data, len, 1000);
    HAL_Delay(5); // EEPROM 내부 쓰기 완료 대기 시간
}

/**
  * @brief EEPROM에서 데이터를 읽는 헬퍼 함수
  * @param addr: EEPROM 내부 메모리 주소
  * @param data: 읽은 데이터를 저장할 버퍼 포인터
  * @param len: 읽을 데이터 길이
  * @retval None
  */
void EEPROM_Read(uint16_t addr, uint8_t *data, uint16_t len)
{
    HAL_I2C_Mem_Read(&hi2c2, EEPROM_ADDR, addr, I2C_MEMADD_SIZE_16BIT, data, len, 1000);
}

/**
  * @brief EEPROM에 저장된 모든 부팅 기록을 UART로 출력합니다.
  * @retval None
  */
void BootLogger_DumpAllRecords(void)
{
    char dump_buf[128];
    char time_str[40]; // 포맷된 시간 문자열을 담을 버퍼

    const char* start_msg = "\r\n\r\n==========[ EEPROM BOOT LOG DUMP ]==========\r\n";
    HAL_UART_Transmit(&huart2, (uint8_t*)start_msg, strlen(start_msg), 1000);

    // 헤더 정보 출력
    sprintf(dump_buf, "[Header] Magic: 0x%lX, Total Boots: %lu\r\n", bootHeader.magic_number, bootHeader.boot_count);
    HAL_UART_Transmit(&huart2, (uint8_t*)dump_buf, strlen(dump_buf), 1000);

    const char* my_str = "----------------------------------------------\r\n";
    HAL_UART_Transmit(&huart2, (uint8_t*)my_str, strlen(my_str), 1000);

    // 저장된 각 부팅 기록 순회하며 출력
    for (uint32_t i = 0; i < bootHeader.boot_count; i++)
    {
        uint32_t record_addr = BOOT_RECORDS_START_ADDR + i * sizeof(BootRecord);
        BootRecord record;
        EEPROM_Read(record_addr, (uint8_t*)&record, sizeof(BootRecord));

        // 헬퍼 함수를 사용하여 시간 포맷 변경
        Format_Uptime(time_str, record.uptime_ms);

        sprintf(dump_buf, "Record #%-3lu | Addr: %-5lu | Uptime: %s\r\n", i + 1, record_addr, time_str); // ms 대신 포맷된 문자열 사용
        HAL_UART_Transmit(&huart2, (uint8_t*)dump_buf, strlen(dump_buf), 1000);
    }
    const char* end_msg = "==========[ END DUMP ]==========\r\n";
    HAL_UART_Transmit(&huart2, (uint8_t*)end_msg, strlen(end_msg), 1000);
}

/**
  * @brief EEPROM의 모든 부팅 기록을 초기화(삭제)합니다.
  * @retval None
  */
void BootLogger_EraseAllRecords(void)
{
    char msg[128];
    sprintf(msg, "\r\n!!! PERFORMING EEPROM FACTORY RESET !!!\r\n");
    HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 1000);

    // 초기화된 새로운 헤더를 생성합니다.
    BootInfoHeader freshHeader;
    freshHeader.magic_number = BOOT_INFO_MAGIC_NUMBER;
    freshHeader.boot_count = 0; // 부팅 카운트를 0으로 리셋

    // 이 헤더 정보를 EEPROM의 시작 주소에 덮어씁니다.
    EEPROM_Write(BOOT_HEADER_ADDR, (uint8_t*)&freshHeader, sizeof(BootInfoHeader));
    sprintf(msg, "EEPROM reset complete. Please set PERFORM_EEPROM_FACTORY_RESET to 0 and re-flash.\r\n");
    HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 1000);

    // 초기화 작업 후 의도치 않은 동작을 방지하기 위해 무한 루프에 진입합니다.
    // 반드시 코드를 원상 복구하고 다시 펌웨어를 다운로드해야 합니다.
    while (1)
    {
      HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13); // On-board LED
      HAL_Delay(100);
    }
}

// Uptime을 시/분/초 형식의 문자열로 변환하는 헬퍼 함수
static void Format_Uptime(char* buf, uint32_t uptime_ms)
{
    uint32_t total_seconds = uptime_ms / 1000;
    uint32_t hours = total_seconds / 3600;
    uint32_t minutes = (total_seconds % 3600) / 60;
    uint32_t seconds = total_seconds % 60;

    if (total_seconds >= 3600) // 1시간 이상일 경우
        sprintf(buf, "%lu h %lu m %lu s", hours, minutes, seconds);
    else if (total_seconds >= 60) // 1분 이상일 경우
        sprintf(buf, "%lu m %lu s", minutes, seconds);
    else // 60초 미만일 경우
        sprintf(buf, "%.2f s", (float)uptime_ms / 1000.0f);
}

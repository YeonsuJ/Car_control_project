// boot_logger.h

#ifndef INC_BOOT_LOGGER_H_
#define INC_BOOT_LOGGER_H_

#include "main.h"

// --- EEPROM 데이터 덤프 기능 활성화 플래그 ---
#define ENABLE_BOOT_LOG_DUMP_ON_STARTUP 1

// --- EEPROM 모든 데이터 삭제 기능 활성화 플래그 ---
#define PERFORM_EEPROM_FACTORY_RESET 0

/*
 * =================================================================================
 * 구조체 및 상수 정의
 * =================================================================================
 */
#define EEPROM_ADDR 0xA0

typedef struct {
    uint32_t magic_number;
    uint32_t boot_count;
} BootInfoHeader;

typedef struct {
    uint32_t uptime_ms;
} BootRecord;

#define BOOT_INFO_MAGIC_NUMBER      0xDEADBEEF
#define BOOT_HEADER_ADDR            0
#define BOOT_RECORDS_START_ADDR     (sizeof(BootInfoHeader))
#define UPTIME_SAVE_INTERVAL_MS     1000

/*
 * =================================================================================
 * 함수 원형
 * =================================================================================
 */

void BootLogger_Init(void);
void BootLogger_UpdateUptime(void);
// 참고: Dump와 Erase 함수는 Init 함수 내부에서만 사용되므로 굳이 외부에 노출할 필요는 없습니다.
// 하지만 디버깅을 위해 남겨두는 것도 좋습니다. 여기서는 이름을 통일합니다.
void BootLogger_DumpAllRecords(void);
void BootLogger_EraseAllRecords(void);

#endif /* INC_BOOT_LOGGER_H_ */

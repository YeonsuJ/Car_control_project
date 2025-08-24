#include "rf_handler.h"
#include "NRF24.h" // 저수준 드라이버 포함
#include "NRF24_reg_addresses.h"
#include <string.h> // memcpy를 위해 추가
#include "cmsis_os.h" // ★ 세마포어 사용

// =============================
// NRF24 수신(Rx) 패킷 구조 정의
// -----------------------------
// Byte 0   : 메시지 식별자 (1이면 주행 명령 수신)
// Byte 1~2 : roll (int16_t, 센서 측에서 ×100 배율 인코딩)
// Byte 3~4 : accel_ms (uint16_t, 엑셀 유지 시간)
// Byte 5~6 : brake_ms (uint16_t, 브레이크 유지 시간)
// Byte 7   : direction (0: BACKWARD, 1: FORWARD)
// 나머지 Byte 8~31 : 예약 or 미사용
// =============================

// 페이로드 크기 정의
#define RX_PAYLOAD_SIZE 8
#define ACK_PAYLOAD_SIZE 3

// ★ FreeRTOS 세마포어
extern osSemaphoreId_t RFSemHandle;

// 내부 ACK 응답 버퍼
static uint8_t ack_response[ACK_PAYLOAD_SIZE] = {0};

// NRF24 모듈 수신 초기화
void RFHandler_Init(void)
{
    static uint8_t rx_addr[5] = {0x45, 0x55, 0x67, 0x10, 0x21};

    csn_high();
    HAL_Delay(5);
    ce_low();

    nrf24_init();
    nrf24_auto_ack_all(auto_ack);
    nrf24_en_ack_pld(enable);
    nrf24_dpl(disable);
    nrf24_set_crc(enable, _1byte);
    nrf24_tx_pwr(_0dbm);
    nrf24_data_rate(_2mbps); // 조종기와 동일하게 설정
    nrf24_set_channel(90);
    nrf24_set_addr_width(5);
    nrf24_open_rx_pipe(1, rx_addr);
    nrf24_pipe_pld_size(1, RX_PAYLOAD_SIZE);

    nrf24_listen(); // 수신 대기 시작
}

// ★ EXTI에서 호출되는 IRQ 콜백: 플래그 대신 세마포어 릴리즈
void RFHandler_IrqCallback(void)
{
    // 커널 실행 중 + 핸들 유효할 때만 ISR-safe 릴리즈
    if (osKernelGetState() == osKernelRunning && RFSemHandle) {
        (void)osSemaphoreRelease(RFSemHandle);
    }
}

// 다음 ACK 페이로드에 실릴 데이터 배열 설정
void RFHandler_SetAckPayload(uint8_t* payload, uint8_t length)
{
	// 1. 복사할 길이를 결정 (버퍼 오버플로우 방지)
	uint8_t len_to_copy = length;
	if (len_to_copy > ACK_PAYLOAD_SIZE)
		len_to_copy = ACK_PAYLOAD_SIZE; // ACK 버퍼 크기보다 크면 잘라냄

	// 2. 전달받은 payload 데이터를 내부 ack_response 버퍼로 복사
	memcpy(ack_response, payload, len_to_copy);
}

// ★ 새 명령 유무는 RX_DR 비트로 판단 (플래그 제거)
bool RFHandler_GetNewCommand(VehicleCommand_t* command)
{
    // 수신 데이터 준비 여부 확인
    uint8_t status = nrf24_r_status();
    if ((status & (1U << RX_DR)) == 0U) {
        return false; // 새 데이터 없음
    }

    // 데이터 수신
    uint8_t rx_buffer[RX_PAYLOAD_SIZE] = {0};
    nrf24_receive(rx_buffer, RX_PAYLOAD_SIZE);

    // 미리 준비된 ACK 페이로드 송신
    nrf24_transmit_rx_ack_pld(1, ack_response, ACK_PAYLOAD_SIZE);

    // RX_DR 클리어
    nrf24_clear_rx_dr();

    // 파싱
    if (rx_buffer[0] == 1)  // ID == 1 : 주행 명령
    {
        int16_t roll_encoded = 0;
        memcpy(&roll_encoded, &rx_buffer[1], sizeof(int16_t));
        command->roll = ((float)roll_encoded) / 100.0f;

        memcpy(&command->accel_ms,  &rx_buffer[3], sizeof(uint16_t));
        memcpy(&command->brake_ms,  &rx_buffer[5], sizeof(uint16_t));
        command->direction = rx_buffer[7];

        return true;
    }

    return false; // 유효하지 않은 ID
}

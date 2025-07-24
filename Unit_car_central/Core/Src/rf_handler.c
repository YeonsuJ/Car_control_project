#include "rf_handler.h"
#include "NRF24.h" // 저수준 드라이버 포함
#include "NRF24_reg_addresses.h"
#include <string.h> // memcpy를 위해 추가

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
#define ACK_PAYLOAD_SIZE 2

// === 내부 전용 변수들 (static으로 외부 접근 차단) ===
static volatile uint8_t nrf_irq_flag = 0;
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

// IRQ 핀 EXTI 발생 시 호출
void RFHandler_IrqCallback(void)
{
    nrf_irq_flag = 1;
}

// 다음 ACK 페이로드에 실릴 신호 설정
void RFHandler_SetAckPayload(uint8_t signal)
{
    // ACK 페이로드 버퍼의 첫 바이트에 신호값을 저장
    ack_response[0] = signal;
}

// 새 명령 패킷이 수신되었을 때만 true 반환 및 값 복사
bool RFHandler_GetNewCommand(VehicleCommand_t* command)
{
    if (!nrf_irq_flag) {
        return false; // 새 데이터 없음
    }

    nrf_irq_flag = 0; // 플래그 즉시 초기화
    uint8_t status = nrf24_r_status();

    if (status & (1 << RX_DR))
    {
        uint8_t rx_buffer[RX_PAYLOAD_SIZE] = {0};
        nrf24_receive(rx_buffer, RX_PAYLOAD_SIZE);

        // ACK 페이로드 전송 (미리 설정된 ack_response 값을 보냄)
        nrf24_transmit_rx_ack_pld(1, ack_response, ACK_PAYLOAD_SIZE);
        nrf24_clear_rx_dr();

        if (rx_buffer[0] == 1) // ID가 1인 주행 명령일 경우 데이터 파싱
        {
            int16_t roll_encoded = 0;
            memcpy(&roll_encoded, &rx_buffer[1], sizeof(int16_t));
            command->roll = ((float)roll_encoded) / 100.0f;

            memcpy(&command->accel_ms, &rx_buffer[3], sizeof(uint16_t));
            memcpy(&command->brake_ms, &rx_buffer[5], sizeof(uint16_t));
            command->direction = rx_buffer[7];

            return true; // 파싱 성공, 새 데이터 있음
        }
    }

    return false; // 유효한 데이터 없음
}

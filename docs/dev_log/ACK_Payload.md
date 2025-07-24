# NRF24L01+ 양방향 통신: ACK Payload 기능 적용 (v1.5.0)

## 프로젝트 목표: ACK Payload를 활용한 양방향 통신

본 프로젝트는 nRF24L01+의 **ACK Payload** 기능을 사용하여 기존 단방향 통신을 **저지연 양방향 통신**으로 개선합니다.

-   **핸들 (송신)**: 조향 및 속도 데이터를 RC카로 전송합니다.
-   **RC카 (수신)**: 수신 응답(ACK)에 실시간 **배터리 전압** 정보를 담아 핸들로 즉시 회신합니다.
-   **기대 효과**: 사용자는 조종기에서 RC카의 상태를 실시간으로 모니터링하고, 통신 연결 상태를 안정적으로 확인할 수 있습니다.

---

## ACK Payload: 핵심 원리

### 정의

**ACK Payload**란, 데이터 수신 성공을 알리는 ACK 신호에 **사용자 데이터를 함께 실어 보내는** Enhanced ShockBurst™의 핵심 기능입니다. 이를 통해 별도의 송/수신 역할 전환 없이 효율적인 양방향 통신이 가능합니다.

### 통신 흐름

1.  **설정 (양쪽 공통)**
    -   `FEATURE` 레지스터에서 `EN_DPL` (동적 페이로드) 및 `EN_ACK_PAY` (ACK 페이로드) 기능을 모두 활성화합니다.

2.  **통신 사이클**
    -   **핸들 (TX)**: `TX_FIFO`에 조종 데이터를 넣고 `nrf24_transmit()`을 호출하여 RC카로 전송합니다.
    -   **RC카 (RX)**: 데이터 수신 성공 시 **`RX_DR` 인터럽트**가 발생합니다. 인터럽트 루틴에서 `RX_FIFO`의 조종 데이터를 읽어낸 직후, 응답으로 보낼 **배터리 전압 데이터를 `nrf24_write_ack_pld()` 함수로 `TX_FIFO`에 미리 써넣습니다.**
    -   **RC카 (RX) → 핸들 (TX)**: 하드웨어는 ACK를 보낼 때, `TX_FIFO`에 미리 준비된 배터리 전압 데이터를 **자동으로 함께 담아 전송**합니다.
    -   **핸들 (TX)**: ACK 수신 시 **`TX_DS` 인터럽트**가 발생하고, 동시에 함께 전송된 배터리 전압 데이터가 `RX_FIFO`에 들어옵니다. `nrf24_read_ack_pld()` 함수로 이 데이터를 읽습니다.

---

## 전체 시스템 동작 흐름

### 송신부 (핸들)

1.  20ms 주기로 자이로/버튼 데이터를 `tx_buffer`에 패킹합니다.
2.  `nrf24_transmit()` 함수로 데이터를 전송합니다.
3.  전송 성공 시(`TX_DS`), `nrf24_data_available()` 함수로 ACK Payload 수신 여부를 확인합니다.
4.  수신된 ACK Payload(RC카 배터리 전압)가 있다면, 이를 읽어와 핸들의 OLED에 표시합니다.

### 수신부 (RC카)

1.  데이터 수신 시 발생하는 EXTI 인터럽트를 감지합니다.
2.  `nrf24_receive()`로 핸들의 조종 데이터를 읽고 서보모터/DC 모터를 제어합니다.
3.  ADC를 이용해 RC카의 배터리 전압을 상시 측정합니다.
4.  `nrf24_write_ack_pld()` 함수를 호출하여, 측정된 전압 데이터를 다음 ACK 패킷에 실릴 페이로드로 미리 등록합니다.

---

## 송신부(핸들) 상세 로직

### 초기화 절차

`NRF24_Tx_Init` 함수는 모듈을 송신 모드로 설정하고, **ACK Payload를 수신할 수 있도록** 준비합니다. `EN_ACK_PAY` 기능을 활성화하고, ACK를 수신할 파이프 0번을 송신 주소와 동일하게 열어주는 것이 핵심입니다.

```c
void NRF24_Tx_Init(void)
{
    csn_high();
    HAL_Delay(5); 
    ce_low();

    nrf24_init();
    nrf24_stop_listen();

    nrf24_en_ack_pld(enable);         // 1. ACK 페이로드 활성화
    nrf24_dpl(disable);               // 2. 고정 페이로드 크기 사용
    nrf24_set_crc(enable, _1byte);    // 3. CRC 활성화

    nrf24_open_tx_pipe(tx_addr);
    nrf24_open_rx_pipe(0, tx_addr);   // ★ 추가: ACK 수신을 위해 Pipe 0 열기

    nrf24_pipe_pld_size(0, ACK_PAYLOAD_SIZE); // ★ 추가: ACK 수신 파이프 크기
}
```

---
### 주기적 데이터 전송  
메인 while(1) 루프는 20ms 주기로 조종 데이터를 패키징하여 전송합니다. 자이로 센서 값, 버튼 유지 시간, 락커 스위치 상태를 하나의 데이터 패킷으로 만듭니다.

```c
while (1)
{
    uint32_t now = HAL_GetTick();

    if (now - lastTransmitTick >= TRANSMIT_INTERVAL_MS)
    {
        lastTransmitTick = now;

        // 1. 센서 데이터 읽기 및 패키징
        MPU6050_Read_All(&hi2c2, &MPU6050);
        float roll = MPU6050.KalmanAngleX;
        
        memset(dataT, 0, PLD_S);
        dataT[0] = 1;

        int16_t roll_encoded = (int16_t)(roll * 100.0f);
        memcpy(&dataT[1], &roll_encoded, sizeof(int16_t));

        uint16_t accel_ms = (uint16_t)(accel_count * 20);
        uint16_t brake_ms = (uint16_t)(brake_count * 20);
        memcpy(&dataT[3], &accel_ms, sizeof(uint16_t));
        memcpy(&dataT[5], &brake_ms, sizeof(uint16_t));
        
        if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_9) == GPIO_PIN_RESET)
            dataT[7] = LOCKER_FORWARD;
        else
            dataT[7] = LOCKER_BACKWARD;

        // 2. 데이터 전송
        nrf24_transmit(dataT, PLD_S);
        HAL_Delay(5);
    }
}
```
---
### 전송 결과 및 ACK Payload 처리
nRF24L01의 IRQ 인터럽트가 발생하면 nrf_irq_flag가 1이 됩니다. main 루프에서 이 플래그를 감지하여 송신 성공/실패를 판단하고, 성공 시 수신된 ACK Payload를 처리합니다.

```c
if (nrf_irq_flag)
{
    nrf_irq_flag = 0; // 플래그 초기화
    uint8_t status = nrf24_r_status();

    if (status & (1 << TX_DS)) // 송신 성공 (ACK 수신)
    {
        if (nrf24_data_available())
        {
            nrf24_receive(ack_payload, ACK_PAYLOAD_SIZE);
            // ack_payload[0] 값을 OLED 등에 표시하는 로직 추가
        }
        nrf24_clear_tx_ds();
    }
    else if (status & (1 << MAX_RT)) // 송신 실패
    {
        nrf24_flush_tx();
        nrf24_clear_max_rt();
    }
}
```
---
## 수신부(RC카) 상세 로직

### 초기화 절차

수신부의 `main` 함수는 CAN, SPI, Timer 등 모든 주변 장치를 초기화합니다. 특히 RF 통신을 위해 `NRF24_Rx_Init`을, CAN 통신을 위해 `CAN_Filter_Config`를 호출하여 각각의 통신 채널을 설정하고 인터럽트를 활성화합니다.

-   **`NRF24_Rx_Init`**: nRF24L01 모듈을 **데이터 수신 모드**로 설정하고, 송신부로 **ACK Payload를 보내기 위한 준비**를 합니다. `EN_ACK_PAY` 기능을 활성화하고, 주 데이터를 수신할 파이프 1번을 엽니다.
-   **`CAN_Filter_Config`**: 특정 CAN ID(0x6A5)를 가진 메시지만 수신하도록 필터를 설정하고, 수신 인터럽트를 활성화합니다.

```c
void NRF24_Rx_Init(void)
{
    static uint8_t rx_addr[5] = {0x45, 0x55, 0x67, 0x10, 0x21};

    csn_high();
    HAL_Delay(5);
    ce_low();

    nrf24_init();

    // 송신부와 동일하게 ACK 페이로드 관련 기능 활성화
    nrf24_auto_ack_all(auto_ack);
    nrf24_en_ack_pld(enable);
    nrf24_dpl(disable); 
    nrf24_set_crc(enable, _1byte);

    // 수신 파이프 설정
    nrf24_open_rx_pipe(1, rx_addr); // Pipe 1로 주 데이터 수신

    // 파이프별 페이로드 크기 설정
    nrf24_pipe_pld_size(1, RX_PAYLOAD_SIZE);   // Pipe 1은 32바이트
    nrf24_pipe_pld_size(0, ACK_PAYLOAD_SIZE); // Pipe 0은 응답용 2바이트

    nrf24_listen(); // 수신 대기 시작
}
```
---
### 데이터 수신 및 처리
#### RF 데이터 수신 및 ACK 응답
수신 로직은 인터럽트 기반으로 동작합니다. nRF 모듈이 데이터를 수신하면 IRQ 핀을 통해 인터럽트를 발생시키고, 콜백 함수에서 nrf_irq_flag를 1로 설정합니다. main 루프는 이 플래그를 감지하여 데이터 처리   및 ACK 응답 로직을 수행합니다.

- 데이터 파싱: nrf24_receive로 수신된 rx_buffer에서 memcpy를 이용해 roll, accel, brake, direction 값을 추출하여 각각의 제어 변수에 저장합니다.

- 조건부 ACK 응답: CAN 통신으로 받은 can_distance_signal 값에 따라 ACK 응답 데이터를 구성하고, nrf24_transmit_rx_ack_pld 함수를 사용해 수신 성공 신호(ACK)와 함께 송신부로 보낼 페이로드를 미리 TX FIFO에 써넣습니다.

```c
// main.c 내 while(1) 루프
if (nrf_irq_flag)
{
    nrf_irq_flag = 0; // 플래그 즉시 초기화
    uint8_t status = nrf24_r_status();

    if (status & (1 << RX_DR)) // 데이터 수신 인터럽트(RX_DR)가 발생했다면
    {
        // 1. 데이터 수신 및 파싱
        nrf24_receive(rx_buffer, RX_PAYLOAD_SIZE);
        if (rx_buffer[0] == 1) // 식별자 확인
        {
            int16_t roll_encoded = 0;
            memcpy(&roll_encoded, &rx_buffer[1], sizeof(int16_t));
            received_roll = ((float)roll_encoded) / 100.0f;
            memcpy((void*)&received_accel_ms, &rx_buffer[3], sizeof(uint16_t));
            memcpy((void*)&received_brake_ms, &rx_buffer[5], sizeof(uint16_t));
            motor_direction = (MotorDirection)rx_buffer[7];
        }

        // 2. 조건부 ACK 페이로드 응답 준비
        uint8_t ack_response[ACK_PAYLOAD_SIZE] = {0};
        if (can_distance_signal == 1)
        {
            ack_response[0] = 1;
        }
        nrf24_transmit_rx_ack_pld(1, ack_response, ACK_PAYLOAD_SIZE);
        nrf24_clear_rx_dr();
    }
}
```
---
### CAN 데이터 수신
CAN 메시지 수신 인터럽트가 발생하면 HAL_CAN_RxFifo1MsgPendingCallback 함수가 호출됩니다.  
 이 함수는 수신된 데이터(RxData[0])에 따라 햅틱 모터를 즉시 제어하고, 이 상태를   
 can_distance_signal 변수에 저장하여 ACK Payload로 전송될 수 있도록 합니다.

```c
void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO1, &RxHeader, RxData);

    // 1. 햅틱 모터는 즉시 제어
    if (RxData[0] == 1)
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_SET);
    else
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_RESET);

    // 2. ACK 페이로드에 보낼 신호를 변수에 저장
    can_distance_signal = RxData[0];
}
```
---
## 향후 확장 및 개선 아이디어
###   **통신 주기 최적화 및 지연 최소화**

- `HAL_Delay()`를 타이머 기반의 **비차단(Non-blocking) 방식**으로 변경하여 데이터 처리 중 발생하는 불필요한 지연을 제거합니다. 이를 바탕으로 아래와 같은 통신 주기 최적화를 적용할 수 있습니다.
- 반응성을 극대화하기 위해, 현재 20ms인 고정 전송 주기를 **10ms 혹은 그 이하로 단축**하여 더 빠르게 조종 상태를 반영할 수 있습니다.

###   **통신 두절 감지 및 페일세이프(Failsafe) 구현**
 -   **문제점**: 현재는 조종기와의 통신이 끊겼을 때 RC카가 마지막 수신 명령을 계속 수행할 위험이 있습니다.  

-   **개선 방안**: 수신부에서 일정 시간(예: 100ms) 동안 새로운 데이터 패킷이 수신되지 않으면 통신 두절로 간주하고, 모터를 자동으로 정지시키는 **페일세이프 로직**을 추가합니다. 이는 시스템의 안정성을 크게 향상시킵니다.




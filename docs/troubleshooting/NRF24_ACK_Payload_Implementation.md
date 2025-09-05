# 차량-컨트롤러 간 무선 양방향 통신 구현

## 문제점
초기에는 컨트롤러가 차량에 데이터를 송신(TX)한 후, 수신(RX) 모드로 직접 전환하여 차량의 응답을 기다리는 방식을 시도했다. 하지만 이 방식은 두 모듈 간의 정밀한 타이밍 동기화가 필수적이었고, 소프트웨어 기반의 모드 전환 지연 시간으로 인해 빈번한 통신 오류를 야기했다. 결과적으로 데이터 손실이 발생하고, 차량 상태(RPM 등)를 실시간으로 피드백받아 디스플레이에 표시하거나 충돌 경고에 즉각적으로 반응하는 데 실패했다.

## 해결 방안
이 문제를 해결하기 위해, nRF24L01+ 모듈이 하드웨어 수준에서 지원하는 ‘ACK Payload’ 기능을 활용했다. 이 기능은 송신 측이 데이터를 보내면, 수신 측이 응답(ACK) 신호에 작은 크기의 데이터(Payload)를 실어 자동으로 회신하는 방식이다. 이를 통해 송신기와 수신기의 역할을 소프트웨어로 전환할 필요 없이, 송신 동작 한 번으로 안정적인 양방향 통신을 구현할 수 있었다.

## 주요 코드
먼저 `comm_handler.c`에서 통신 모듈을 초기화할 때, 자동 응답(ACK)과 ACK 페이로드 기능을 활성화했다. 특히, 송신 파이프와 동일한 주소로 수신 파이프 0번을 설정하여 컨트롤러가 보낸 패킷에 대한 ACK를 하드웨어적으로 수신할 준비를 마쳤다.

```c
// [ comm_handler.c ]

/**
 * @brief NRF24 모듈을 송신(Tx) 모드로 초기화한다.
 * @note 주소, 채널, 데이터 속도, 자동 재전송 등 통신 파라미터를 설정한다.
 * ACK 페이로드를 수신하기 위해 Rx 파이프 0번도 함께 설정한다.
 */
void CommHandler_Init(void)
{
    // ... (기타 설정)
    nrf24_auto_ack_all(auto_ack);       // 모든 파이프에 대해 자동 ACK 활성화
    nrf24_en_ack_pld(enable);           // ACK 페이로드 기능 활성화
    // ...
    nrf24_open_tx_pipe(tx_addr);        // 송신 파이프 열기
    nrf24_open_rx_pipe(0, tx_addr);     // ACK 페이로드 수신을 위한 Rx 파이프 0번 열기
    nrf24_pipe_pld_size(0, ACK_PAYLOAD_SIZE); // 파이프 0번의 페이로드 크기 설정
}
```

컨트롤러는 주행 명령이 담긴 8바이트 패킷을 송신하고, 차량은 이에 대한 응답으로 햅틱 신호와 현재 RPM이 담긴 3바이트 ACK 페이로드를 회신하도록 데이터 구조를 정의했다.

```c
// [ comm_handler.c ]

/**
 * @brief NRF24 송신(Tx) 패킷 구조 정의 (컨트롤러 -> 차량)
 * @details
 * Byte | 내용        | 타입       | 비고            |
 * 0    | 메세지ID    | (uint8_t)  | 1: 주행명령     |
 * 1~2  | roll        | int16_t    | x100 인코딩     |
 * ...
 */

/**
 * @brief NRF24 수신(Rx) ACK 페이로드 구조 정의 (차량 -> 컨트롤러)
 * @details
 * Byte | 내용        | 타입       | 비고                          |
 * 0    | haptic      | uint8_t    | 햅틱 피드백 신호              |
 * 1~2  | rpm         | uint16_t   | 모터 RPM (Little Endian)      |
 */
```

## 결과: 즉각적인 반응성 확보
ACK Payload를 적용한 결과, 모드 전환 없이 안정적이고 신뢰성 높은 양방향 통신 채널을 구축할 수 있었다. 컨트롤러에서 통신 성공 인터럽트가 발생하면 ackHandlerTask가 즉시 실행된다. 이 태스크는 수신된 ACK 페이로드를 확인하여, 차량에서 보낸 햅틱 신호(ack_payload[0])에 따라 진동 모터를 제어하고, RPM 데이터는 디스플레이용 데이터 구조체에 업데이트했다.

이러한 구조를 통해 즉각적인 충돌 경고 피드백과 실시간 데이터 디스플레이라는 두 가지 목표를 모두 달성할 수 있었다.

```c
// [ freertos.c ]

void StartackHandlerTask(void *argument)
{
  uint8_t ack_packet[ACK_PAYLOAD_SIZE] = {0};

  for(;;)
  {
      // ACK 관련 인터럽트(TX_DS 또는 MAX_RT)가 발생할 때까지 대기
      osSemaphoreAcquire(ackSemHandle, osWaitForever);

      // 통신 상태 확인 및 ACK 페이로드 수신
      CommStatus_t status = CommHandler_CheckStatus(ack_packet, ACK_PAYLOAD_SIZE);

      if (status == COMM_TX_SUCCESS)
      {
          // 수신된 ACK 페이로드 처리
          App_HandleAckPayload(ack_packet);
      }
      // ... (통신 실패 처리)
  }
}

// [ app_logic.c ]

 void App_HandleAckPayload(uint8_t* ack_payload) // ACK 신호 기반 진동 생성 함수
 {
     // 햅틱 신호(ack_payload[0])에 따라 진동 모터 제어
     if (ack_payload[0] == 1)
     {
         HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);
     }
     else
     {
         HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
     }

     // RPM 데이터(ack_payload[1], ack_payload[2])를 디스플레이용 데이터에 업데이트
     if (g_displayDataMutexHandle != NULL)
     {
         if (osMutexAcquire(g_displayDataMutexHandle, 10) == osOK)
         {
             memcpy(&g_displayData.rpm, &ack_payload[1], sizeof(uint16_t));
             osMutexRelease(g_displayDataMutexHandle);
         }
     }
 }
```
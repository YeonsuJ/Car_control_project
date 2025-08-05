# RTOS 포팅 완료 및 주요 변경사항

## 실시간 운영체제(RTOS)

**실시간 운영체제(Real-Time Operating System, RTOS)** 는 정해진 시간 제약 조건, 즉 **'데드라인(Deadline)'** 안에 작업의 완료를 보장하는 것을 최우선 목표로 하는 운영체제입니다.

일반적인 OS(Windows, Linux 등)가 처리량(Throughput)을 높이는 데 중점을 두는 반면, RTOS는 결과의 **정확성**뿐만 아니라 결과가 **정해진 시간 안에 나오는가**를 시스템 성공의 핵심 척도로 삼습니다. 이러한 특성을 **결정성(Determinism)**이라고 부릅니다.

> 본 프로젝트는 핸들의 기울기, 악셀 및 브레이크의 눌림 시간, 전후방 장애물 감지 정보들을 병렬적으로 처리함에 있어서 RTOS를 도입하여 실시간성을 확보하고자 했습니다.

### RTOS의 핵심 구성 요소

-   **스케줄러 (Scheduler)**: 어떤 태스크를 언제 실행할지 결정하는 RTOS의 핵심 엔진입니다. 우선순위 기반의 선점형 스케줄링을 주로 사용합니다.
-   **태스크 (Task)**: 독립적으로 실행될 수 있는 최소 작업 단위입니다. 각 태스크는 자신만의 스택 메모리를 가집니다.
-   **동기화 기법 (Synchronization)**: 여러 태스크가 공유 자원(예: SPI 버스)에 안전하게 접근하도록 하는 **뮤텍스(Mutex)** 나, 특정 이벤트 발생을 알리는 **세마포어(Semaphore)** 등을 제공합니다.
-   **태스크 간 통신 (Inter-Task Communication)**: 태스크들이 서로 데이터를 안전하게 주고받을 수 있도록 **메시지 큐(Message Queue)** 와 같은 통신 채널을 제공합니다.

---

## FreeRTOS의 주요 특징

**FreeRTOS**는 임베디드 시스템, 특히 마이크로컨트롤러(MCU) 환경에서 가장 널리 사용되는 오픈 소스 RTOS 중 하나이며, 다음과 같은 특징을 가집니다.

-   **경량성 및 높은 이식성 (Lightweight & Portable)**
    -   커널의 크기가 매우 작아(수 KB 수준) 메모리가 제한적인 소형 MCU에도 쉽게 올릴 수 있습니다.
    -   대부분 C언어로 작성되어 있고, 특정 아키텍처에 의존하는 코드가 최소화되어 있어 ARM Cortex-M, RISC-V 등 다양한 CPU 아키텍처로의 이식이 용이합니다.

-   **우선순위 기반의 선점형 스케줄러 (Priority-based Preemptive Scheduler)**
    -   각 태스크에 우선순위를 할당할 수 있으며, 스케줄러는 항상 준비(Ready) 상태에 있는 태스크 중 가장 높은 우선순위를 가진 태스크를 실행합니다.
    -   만약 더 높은 우선순위의 태스크가 실행될 준비가 되면, 현재 실행 중인 낮은 우선순위의 태스크를 즉시 중단시키고(선점, Preemption) 높은 우선순위의 태스크를 실행하여 실시간성을 보장합니다.

-   **풍부하고 직관적인 API 제공**
    -   태스크 생성/삭제, 메시지 큐, 세마포어, 뮤텍스, 소프트웨어 타이머 등 멀티태스킹 어플리케이션을 개발하는 데 필요한 대부분의 기능을 표준 API 형태로 제공하여 개발자가 복잡한 내부 동작을 몰라도 쉽게 사용할 수 있습니다.

-   **관대한 라이선스 정책 (MIT License)**
    -   매우 자유로운 MIT 라이선스를 채택하여, 개인 프로젝트는 물론 상업용 제품에도 아무런 제약 없이 무료로 사용할 수 있습니다. 이는 FreeRTOS가 널리 확산된 주요 원인 중 하나입니다.  

## 주요 성과

RTOS를 도입하여 시스템을 멀티태스킹 환경으로 전환함으로써, 기존의 순차적 처리 방식에서 발생하던 여러 제약을 극복하고 다음과 같은 주요 성능 개선을 달성했습니다.

---

-   **조향 응답성 및 해상도 향상**
    -   IMU 센서 데이터 처리를 **독립적인 고우선순위 태스크**로 분리하여, 다른 기능의 영향을 받지 않고 일정한 주기로 센서 값을 갱신할 수 있게 되었습니다.
    -   그 결과, 조작에 즉각적으로 반응하는 **실시간 조향**과 더 부드러운 **조향 해상도**를 확보했습니다.

-   **햅틱 피드백 지연 최소화**
    -   차량부의 초음파 센서 데이터를 담은 ACK Payload 수신 처리를 기존의 폴링 (Polling) 방식이 아닌, **세마포어 (Semaphore) 기반의 이벤트 구동 방식**으로 변경했습니다.
    -   이로 인해 차량의 센서 데이터가 수신되는 즉시 햅틱 모터가 반응하여, **장애물 감지 피드백의 실시간성**이 크게 향상되었습니다.

-   **양방향 통신 안정성 및 효율 증대**
    -   결론적으로, 독립적인 센서 데이터 수집 태스크와 세마포어 기반의 통신 이벤트 처리 방식을 통해 시스템의 중추인 **RF 양방향 통신의 안정성과 전체적인 처리 효율**을 개선하는 성과를 이뤘습니다.  


# `main.c` : RTOS 도입에 따른 구조적 변화

FreeRTOS(CMSIS-OS)가 적용된 `main.c` 파일은 기존의 순차적 `while(1)` 루프 구조에서, 여러 작업이 동시에 실행되는 **RTOS 기반의 멀티태스킹 구조**로 완전히 변경되었습니다.

---

## 1. `main()` 함수의 역할 축소: 시스템 로더(Loader)

`main()` 함수의 역할이 대폭 축소되었습니다. 이제 `main()`은 더 이상 어플리케이션의 주된 로직을 담지 않으며, 다음과 같은 **초기화 및 부팅** 역할만 수행합니다.

1.  **하드웨어 초기화**: `HAL_Init()`, `SystemClock_Config()`, 각종 `MX_..._Init()` 함수를 호출합니다.
2.  **커스텀 모듈 초기화**: 기존의 `InputHandler_Init()`, `CommHandler_Init()`을 호출합니다.
3.  **RTOS 초기화 및 실행**: `osKernelInitialize()`로 커널을 초기화하고, `MX_FREERTOS_Init()`에서 생성된 태스크(Task), 세마포어(Semaphore) 등의 객체를 로드한 뒤, `osKernelStart()`로 스케줄러를 시작시킵니다.

`osKernelStart()`가 호출된 이후에는 제어권이 RTOS 스케줄러로 완전히 넘어가므로, `main()` 함수의 `while(1)` 루프는 더 이상 실행되지 않는 **데드 코드(Dead Code)** 가 됩니다.

```c
int main(void)
{
    // ... (1) 하드웨어 및 (2) 커스텀 모듈 초기화
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    // ...
    InputHandler_Init();
    CommHandler_Init();

    // ... (3) RTOS 초기화 및 실행
    osKernelInitialize();
    MX_FREERTOS_Init(); // 태스크, 세마포어 등 생성
    osKernelStart();    // 스케줄러 시작 (이후 코드는 실행되지 않음)

    while (1)
    {
        // 이 루프는 이제 실행되지 않음
    }
}
```

## 2. 인터럽트 처리 방식 변경: Flag에서 Semaphore로
가장 핵심적인 변화는 인터럽트 처리 방식입니다.

- 기존 방식: IRQ 발생 시 콜백 함수가 **플래그 변수(`nrf_irq_flag`)** 를 1로 만들고, `main` 루프가 이 플래그를 계속 확인(Polling)하는 방식이었습니다. 이는 불필요한 CPU 자원을 낭비합니다.

- RTOS 방식: `HAL_GPIO_EXTI_Callback` 함수는 이제 플래그를 설정하는 대신 **세마포어(`ackSemHandle`)** 를 **해제(Release)** 합니다. 이는 특정 이벤트 발생을 기다리며 잠들어 있던(Blocked) 전용 처리 태스크를 즉시 깨우는 신호 역할을 합니다. 이 방식은 CPU가 불필요한 폴링 작업을 하지 않아 매우 효율적이며, 이벤트 발생 시 반응 속도가 빠릅니다.

```c
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    extern osSemaphoreId_t ackSemHandle; // RTOS가 관리하는 세마포어 핸들

    if (GPIO_Pin == GPIO_PIN_3) // nRF IRQ 핀
    {
        // CommHandler_IrqCallback(); // 기존 플래그 설정 방식 대신
        osSemaphoreRelease(ackSemHandle); // 세마포어를 해제하여 대기중인 태스크를 깨움
        return;
    }
    InputHandler_GpioCallback(GPIO_Pin);
}
```
## 3. 어플리케이션 로직의 완전한 분리
기존 main()의 while 루프 안에 있던 모든 어플리케이션 로직(센서 읽기, 패킷 구성, 전송 요청, 상태 확인)은 이제 각각의 독립된 RTOS 태스크 내부로 전부 이동되었습니다. 예를 들어, InputTask, CommTask 와 같은 태스크들이 각자의 역할을 독립적으로 수행하게 되어 main.c는 어플리케이션의 상세 로직과 완전히 분리되었습니다.

# `freertos.c` : 멀티태스킹 시스템의 구현

`freertos.c` 파일은 RTOS 어플리케이션의 **핵심 로직** 을 담고 있습니다. 기존의 순차적인 `while` 루프를 대체하여, 각 기능이 독립적인 **태스크(Task)** 로 분리되어 동시에 실행되는 멀티태스킹 시스템을 구현합니다.

본 시스템의 태스크 우선순위는 **'조작의 실시간성'** 을 최우선으로 고려하여 설계되었습니다.

1.  **`sensorTask` / `commTask` (높은 우선순위)**
    -   `sensorTask`는 사용자 입력을 감지하고, `commTask`는 이 데이터를 즉시 전송하는, 시스템의 가장 핵심적인 역할을 수행합니다. 조작 지연을 최소화하기 위해 이 두 태스크는 가장 높은 우선순위를 가집니다. 이를 통해 다른 작업에 의해 실행이 방해받지 않고, 사용자 입력이 즉각적으로 처리 및 전송될 수 있습니다.

2.  **`ackHandlerTask` (보통 우선순위)**
    -   `ackHandlerTask`는 수신된 ACK Payload(예: 배터리 전압)를 처리하는 역할을 합니다. 이 작업은 조종 자체보다는 **피드백 정보 처리**에 해당하므로, `sensorTask`나 `commTask`에 비해 상대적으로 시간 제약이 덜 엄격합니다.
    -   따라서 보통 우선순위로 설정하여, 시스템의 핵심인 센서 처리와 통신 작업이 먼저 수행되도록 보장하고, CPU가 유휴 상태일 때 ACK 처리 작업이 수행되도록 합니다.

---

## 1. RTOS 객체 생성 (`MX_FREERTOS_Init`)

이 함수는 스케줄러가 시작되기 전에 시스템에 필요한 모든 RTOS 구성 요소(객체)를 생성하고 초기화합니다.

-   **태스크 (Threads)** : 시스템의 주요 기능들을 3개의 독립적인 실행 단위로 분리하여 생성합니다.
    -   `sensorTask`: MPU6050 센서 값을 주기적으로 읽는 태스크.
    -   `commTask`: `sensorTask`로부터 데이터를 받아 무선으로 전송하는 태스크.
    -   `ackHandlerTask`: nRF24L01의 IRQ 발생 시 ACK/NACK을 처리하는 태스크.

-   **메시지 큐 (Message Queue)** : `sensorQueueHandle`
    -   **역할** : `sensorTask`와 `commTask` 사이의 데이터 통신 채널입니다. `sensorTask`가 읽은 센서 값을 이 큐에 넣으면, `commTask`가 큐에서 데이터를 꺼내 사용합니다.
    -   **효과** : 두 태스크의 동작을 완전히 분리(Decoupling)하여, 센서 측정 주기와 통신 주기가 서로에게 영향을 주지 않도록 합니다.

-   **세마포어 (Semaphore)** : `ackSemHandle`
    -   **역할** : 인터럽트 서비스 루틴(ISR)과 `ackHandlerTask` 사이의 동기화를 위한 신호입니다. 평소에는 `ackHandlerTask`가 이 세마포어를 기다리며 잠들어 있고, IRQ가 발생하면 ISR이 세마포어를 반납하여 태스크를 깨웁니다.
    -   **효과** : 불필요한 폴링(Polling) 없이 이벤트가 발생했을 때만 CPU를 사용하여 효율적인 처리가 가능합니다.


## 2. 태스크 우선순위
본 시스템의 태스크 우선순위는 **조작의 실시간성** 을 최우선으로 고려하여 설계되었습니다.

1.  **`sensorTask` / `commTask` (높은 우선순위)**
    -   `sensorTask`는 사용자 입력을 감지하고, `commTask`는 이 데이터를 즉시 전송하는, 시스템의 가장 핵심적인 역할을 수행합니다. 조작 지연을 최소화하기 위해 이 두 태스크는 가장 높은 우선순위를 가집니다. 이를 통해 다른 작업에 의해 실행이 방해받지 않고, 사용자 입력이 즉각적으로 처리 및 전송될 수 있습니다.

2.  **`ackHandlerTask` (보통 우선순위)**
    -   `ackHandlerTask`는 수신된 ACK Payload(예: 배터리 전압)를 처리하는 역할을 합니다. 이 작업은 조종 자체보다는 **피드백 정보 처리**에 해당하므로, `sensorTask`나 `commTask`에 비해 상대적으로 시간 제약이 덜 엄격합니다.
    -   따라서 보통 우선순위로 설정하여, 시스템의 핵심인 센서 처리와 통신 작업이 먼저 수행되도록 보장하고, CPU가 유휴 상태일 때 ACK 처리 작업이 수행되도록 합니다.

---

## 3. 태스크 구현

### `StartsensorTask` (데이터 생산자)

-   5ms마다 주기적으로 `App_GetRollAngle()` 함수를 호출하여 MPU6050의 Roll 각도를 측정합니다.
-   측정한 `roll` 값을 `osMessageQueuePut()` 함수를 통해 `sensorQueueHandle` 메시지 큐에 넣습니다.
-   이 태스크는 시스템에 필요한 데이터를 꾸준히 **생산(Produce)** 하는 역할을 합니다.

```c
void StartsensorTask(void *argument)
{
  for(;;)
  {
    // 1. 센서 값을 읽어옵니다.
    float roll = App_GetRollAngle();

    // 2. 읽어온 값을 메시지 큐에 넣습니다.
    osMessageQueuePut(sensorQueueHandle, &roll, 0U, 0U);

    // 3. 5ms 주기로 실행합니다.
    osDelay(5);
  }
}
```
### `StartcommTask` (데이터 소비자)  
- `osMessageQueueGet()` 함수를 호출하여 `sensorQueueHandle`에 새로운 데이터가 들어올 때까지 무한정 대기합니다. (휴면 상태)

- 메시지 큐로부터 `roll` 값을 수신하면, `App_BuildPacket()`으로 전송 패킷을 만들고 `CommHandler_Transmit()`으로 데이터를 전송합니다.

- 이 태스크는 `sensorTask`가 생산한 데이터를 **소비(Consume)** 하는 역할을 합니다.

```c
void StartcommTask(void *argument)
{
  uint8_t tx_packet[PAYLOAD_SIZE] = {0};
  float roll = 0.0f;

  for(;;)
  {
    // 큐에 데이터가 들어올 때까지 대기
    osMessageQueueGet(sensorQueueHandle, &roll, NULL, osWaitForever);
    // 데이터 수신 후 처리
    App_BuildPacket(tx_packet, roll);
    CommHandler_Transmit(tx_packet, PAYLOAD_SIZE);
  }
}
```
### `StartackHandlerTask` (이벤트 처리기)  
- `osSemaphoreAcquire()` 함수를 호출하여 `ackSemHandle` 세마포어가 반납될 때까지 무한정 대기합니다. (휴면 상태)

- `HAL_GPIO_EXTI_Callback`에서 nRF IRQ가 발생하여 세마포어가 반납되면 즉시 깨어납니다.

- `CommHandler_CheckStatus()`를 호출하여 통신 결과를 처리합니다.

- 이 태스크는 특정 이벤트가 발생했을 때만 동작하는 효율적인 이벤트 핸들러 역할을 합니다.

```c
void StartackHandlerTask(void *argument)
{
  uint8_t ack_packet[ACK_PAYLOAD_SIZE] = {0};

  for(;;)
  {
    // 1. 세마포어 신호가 올 때까지 대기
    osSemaphoreAcquire(ackSemHandle, osWaitForever);

    // 2. 신호가 와서 깨어나면, ACK 처리 로직을 수행
    CommStatus_t status = CommHandler_CheckStatus(ack_packet, ACK_PAYLOAD_SIZE);
    if (status == COMM_TX_SUCCESS)
    {
        App_HandleAckPayload(ack_packet);
    }
    // ...
  }
}
```

## 추후 개선 사항
- 차량부의 여러 센서 데이터를 수집하여 핸들부의 OLED에 상태를 출력하는 테스크 생성 가능
- RPM, 기어 D / R 상태, 배터리 잔량 등

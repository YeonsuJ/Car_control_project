
# [HC-SR04] 초음파센서 구현 로그

[▶️ 폴링방식 기반 초음파센서 거리값 출력](#-폴링방식-기반-초음파센서-거리값-출력)

## 🎯 프로젝트 활용 방안

초음파 센서는 차량 전방의 장애물을 감지하기 위해 사용되며,
측정된 거리 데이터는 핸들에 연결된 진동모터 기반 충돌 방지 경고 시스템에 전달되어 운전자에게 피드백을 제공하는 데 활용된다.

---

## 📖 이론 개요

### HC-SR04 동작원리
HC-SR04 초음파센서 모듈은 총 4개의 핀으로 구성되어 있으며, 각각 VCC, GND, Trig(트리거 핀), Echo(에코 핀)으로 구성된다. 초음파 센서는 자신과 앞에 놓인 물체 사이의 거리를 측정하며, 이는 40kHz 주파수의 초음파를 8회 발사함으로써 이루어진다. 방출된 신호가 송신된 직후 타이머가 시작되고, 초음파가 물체에 반사되어 수신기에 감지되면 타이머가 종료된다. 이때, 초음파가 왕복하는 데 걸린 시간을 측정하여 센서와 물체 사이의 거리를 계산할 수 있다.

HC-SR04 초음파 센서를 사용하기 위해서는 먼저 Trig 핀에 10마이크로초(μs)의 펄스 신호를 입력해야 하며, 이후 Echo 핀에서 출력되는 펄스의 폭(pulse width) 을 측정해야 한다.
이를 위해 Trig 핀에 연결된 GPIO를 출력 모드로 설정하고, 먼저 LOW 상태로 초기화한 후, 10μs 동안 HIGH 상태를 유지한 뒤 다시 LOW로 전환하여 트리거 펄스를 생성한다.
그 다음, Echo 핀에 연결된 GPIO를 입력 모드로 설정하고, 해당 핀이 HIGH 상태를 유지하는 시간을 측정함으로써 초음파가 반사되어 돌아오는 시간을 알 수 있다.
 
8회의 초음파 펄스는 공기 중에서 음속인 초속 340m의 속도로 이동하게 되며, 거리 = 속력 × 시간 공식을 통해 거리를 계산할 수 있다. 
단, 이 공식에서 사용되는 시간은 초음파가 물체까지 가는 시간뿐만 아니라 왕복 시간 (물체까지 가는 시간 + 다시 돌아오는 시간)을 포함하므로, 측정된 시간을 2로 나누어야 실제 거리를 얻을 수 있다.

공식 : Distance = (pulse_time × speed_of_sound) / 2 

> [HC-SR04 datasheet](../datasheets/HCSR04_datasheet.pdf)

---

## 🔌 하드웨어 연결

STM32F103에서 초음파 센서 HC-SR04의 Echo 핀은 5V 신호를 출력하므로, 이를 직접 GPIO에 연결할 경우 보드 손상의 위험이 있다. 
따라서 STM32F103의 데이터시트에서 해당 GPIO 핀이 FT(Five Volt Tolerant)로 명시되어 있는지를 반드시 확인해야 하며, <br>
FT로 표시된 핀은 5V 입력을 허용하므로 별도의 로직 레벨 시프터 없이도 안전하게 사용할 수 있다. <br>
반면 FT가 아닌 3.3V 전용 핀에 5V 신호를 입력하면 내부 회로가 손상될 수 있으므로 주의가 필요하다.

|초음파센서|F103보드|
|:---:|:---:|
|Vcc|5V|
|Gnd|Gnd|
|Trig|A9|
|Echo|A8|

---

## ⚙️ STM32CubeMX 설정
systemcore > sys > debug > Serial Wire
               > RCC > HSE > Crystal/Ceramic

clock configuration > HCLK를 72로, enter, ok

Timers > TIM1 > clock source : internal clock

parameter settings > prescaler : 71

PA9 -> gpio_output
PA8 -> gpio_input

save and generate code

참고한 링크에서 code copy, save and build로 오류 확인

debug > debugger > st-link (st-link gdb server) > reset behavior : sofware system reset > apply and ok

switch 누르고 live expressions 탭 > 코드의 distance 변수값 복사 > expressions에 붙여넣기 > start하면 거리값 출력됨

---
#### ▶️ 폴링방식 기반 초음파센서 거리값 출력
## 💻 코드 설명

본 프로젝트의 초음파센서 동작은 정확한 거리값이 필요한 것이 아니라 차량 충돌 방지를 위한 특정 범위의 거리값만 반환하면 되는 것이기 때문에 원활한 코드의 유지보수 목적으로  HAL 방식을 적용하였으며, 초기의 코드는 HC-SR04 초음파 센서의 동작을 GPIO polling 방식으로 구현한 것으로, STM32의 타이머(TIM1)를 이용해 펄스 폭을 직접 측정하여 거리(cm)를 계산하였다.

### 1. 핀 정의
```c
    #define TRIG_PIN GPIO_PIN_9
    #define ECHO_PIN GPIO_PIN_8
```
- TRIG_PIN은 초음파를 발사할 트리거 핀 (출력)
- ECHO_PIN은 반사된 초음파를 감지할 핀 (입력)

### 2. TRIG 펄스 생성
```c
HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_SET);
__HAL_TIM_SET_COUNTER(&htim1, 0);
while (__HAL_TIM_GET_COUNTER (&htim1) < 10);
HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_RESET);
```
- TRIG 핀을 10μs 동안 HIGH로 설정하여 초음파 발사 신호를 보냄

### 3. ECHO 핀의 펄스 폭 측정 (Polling 방식)
```c
while (!(HAL_GPIO_ReadPin (ECHO_PORT, ECHO_PIN)) && pMillis + 10 > HAL_GetTick());
Value1 = __HAL_TIM_GET_COUNTER (&htim1);

while ((HAL_GPIO_ReadPin (ECHO_PORT, ECHO_PIN)) && pMillis + 50 > HAL_GetTick());
Value2 = __HAL_TIM_GET_COUNTER (&htim1);
```
- 상승 에지(ECHO 핀이 HIGH 되는 시점): Value1에 저장
- 하강 에지(ECHO 핀이 LOW 되는 시점): Value2에 저장
- 타이머 값은 1 tick = 1μs 기준 → 펄스 폭 = Value2 - Value1

### 4. 거리 계산
```c
Distance = (Value2 - Value1) * 0.034 / 2;
```
- 초음파 왕복 시간에 음속(0.034 cm/μs) 을 곱한 뒤, 2로 나누어 편도 거리 계산

### 5. 오류 방지: Timeout 처리
```c
pMillis = HAL_GetTick();  // 현재 시간 기준
while (...) && pMillis + 10 > HAL_GetTick());
```
- ECHO 신호가 오지 않거나 너무 오래 걸리는 경우 무한 루프에 빠지지 않도록 timeout 구현


> 코드 참고 사이트 -> https://www.micropeta.com/video42<br>
> 참고링크 : https://www.youtube.com/watch?v=oD0mc6FXQ0s

---

## ~~⚠️ 문제 및 해결~~

---

## 🛠️ 확장 및 개선

### 1. 초기 코드 방식의 문제점
HC-SR04 초음파 센서의 Echo 신호의 상승과 하강을 while 루프 + 타이머로 직접 감지함

이 방식의 단점 : 
- while로 핀 상태를 폴링하기 때문에 CPU 점유율이 높음
- 디지털 신호의 미세한 변화(마이크로sec 단위)는 폴링 방식으로는 잡기 어려움
- 중간에 인터럽트가 들어오거나 오버헤드가 있으면 오차 발생


### 2. 코드 개선 방향
" Timer input capture 사용해서 업그레이드 "

Timer input capture를 사용하면
- 타이머가 외부 핀의 상승/하강 엣지를 자동으로 캡처함

- 캡처 순간의 정확한 타이머 카운터 값이 저장되어 μs 단위의 정확한 시간 측정 가능

- 인터럽트 기반이므로 CPU 점유율이 거의 없음

(STM32F103C8T6 기준, TIM2~TIM4에서 Input Capture 가능)<br><br>

### 3. 하드웨어 연결 및 CubeMX 설정

Timer Input Capture가 가능한 핀을 Echo에 연결 (PB6 선택)

|초음파센서|F103보드|
|:---:|:---:|
|Vcc|5V|
|Gnd|Gnd|
|Trig|A9|
|Echo|PB6|


Timer를 Input Capture 모드로 설정 (상승 엣지/하강 엣지 감지 목적)

(tim4, ch1, echo를 pb6으로 사용)<br>
TIM4 → Channel1 → Input Capture 설정

PB6 → Alternate Function, Input Capture 모드

NVIC → TIM4 global interrupt enable

GPIOA9 → Output (TRIG)

> [Ultrasonic_wiring_diagram](../wiring_diagram/ultrasonic.png)

### 4. 코드의 핵심 기능 요약

**Timer Input Capture 기반 거리 측정**

1. Trig 신호 발생 (출력)
TRIG_PIN (GPIOA PIN9)을 10μs 동안 HIGH로 설정하여 초음파 발사 신호를 보냄

```c
    HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_SET);
    delay_us(10);  // 10μs 유지
    HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_RESET);
```

2. Echo 신호 측정 (입력 캡처)
초음파가 반사되어 돌아오면, Echo 핀에서 HIGH 펄스가 발생. <br> TIM4의 CH1 (GPIOA PIN6 등)을 Input Capture 모드로 설정하여 펄스 폭을 측정함

```c
    htim4.Init.Prescaler = 71;     // 72MHz / (71+1) = 1MHz → 1 tick = 1μs
    htim4.Init.Period = 65535;     // 최대 65535μs 측정 가능
```

3. Input Capture 동작 흐름
```c
    HAL_TIM_IC_Start_IT(&htim4, TIM_CHANNEL_1);  // 인터럽트 기반 입력 캡처 시작
```
- 첫 번째 인터럽트 발생 시: 
    - 상승 에지(RISING) 감지 → ic_val1 저장
    - 다음 캡처를 위해 감지 방향을 하강 에지(FALLING)로 전환

- 두 번째 인터럽트 발생 시:
    - 하강 에지(FALLING) 감지 → ic_val2 저장
    - 펄스 폭 계산: ic_val2 - ic_val1 (단, 오버플로우 보정 포함)

- 거리 계산:
```c
    distance = (diff * 0.0343f) / 2.0f;
```
※ 여기서 0.0343f는 음속 343 m/s를 μs 단위로 변환한 값 (cm/μs)

> 커밋 해시 [82d4624](https://github.com/YeonsuJ/Car_control_project/commit/82d462479a9bef4ad92decd2cc37d7a1186b770f)

---

## 💡 향후 확장 및 개선 아이디어
- 거리 제한값 초과 시 이벤트 처리 추가
- FreeRTOS task로 나누는 방식
- 거리 값 평균 필터 적용



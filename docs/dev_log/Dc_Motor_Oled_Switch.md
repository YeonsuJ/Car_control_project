# 버튼 누름 시간 기반 DC 모터 속도 제어 및 로커 스위치 전/후진 전환 + OLED 상태 표시 기능 구현	

## 🎯 프로젝트 활용 방안
본 프로젝트에서 
---

## 📖 이론 개요

- 로커스위치로 전/후진 기어변속 구현
    - 3핀 로커스위치(토글 스위치)는 가운데 공통(COM) 핀과 양쪽 단자를 갖고 있어서 전진/후진 스위치로 적합함

### 로커 스위치 동작 원리

스위치가 FWD에 닿으면 PC9는 GND로 연결되어 LOW 입력 발생

스위치가 REV에 닿으면 PC8는 GND로 연결되어 LOW 입력 발생

### 로커 스위치 (폴링 방식 유리)
로커스위치는 구조적으로 천천히 누르거나 길게 눌리는 경우가 많고, 접점 떨림(bounce)이나 노이즈가 거의 없어 오작동 가능성이 낮으며, 상태가 고정되는 특성 때문에 전진(FWD) 또는 후진(REV)과 같은 지속적인 상태 입력에 적합하다. 이러한 이유로 빠른 응답이 필요한 인터럽트 방식보다는, 10~20ms 주기로 상태를 주기적으로 확인하는 폴링 방식이 구현이 간단하고 훨씬 안정적인 선택일 수 있다.

---

## 🔌 하드웨어 연결

<img src="../wiring_diagram/dc_motor_oled_switch.png" alt="DC모터 스위치 OLED 결선도" width="500"/>

|외부전원(5V)|L298N|F446RE보드|DC모터|
|:---:|:---:|:---:|:---:|
|+12V|+12v|||
|Gnd|Gnd|Gnd|encoder Gnd(black)|
||Out1||Motor-(white)|
||Out2||Motor+(red)|
||IN1|PB4||
||IN2|PB5||
||ENA|PB8||
|||+5v|encoder +5v(blue)|
|||PA7|encoder signal A(yellow)|
|||PA6|encoder signal B(green)|

|F446RE보드|택트 스위치|3핀 로커 스위치|OLED|
|:---:|:---:|:---:|:---:|
|PB0|Accel 버튼|||
|PB1|Brake 버튼|||
|PC12|||SDA|
|PB10|||SCL|
|Gnd|Gnd|Gnd(중앙,COM)|Gnd|
|+3.3V|||Vcc|
|PC8||우측||
|PC9||좌측핀||


- 택트 스위치는 외부 풀업 저항(9.4kΩ)에 연결되어 있으며,
눌렀을 때 LOW(0V) 상태가 되도록 연결
- HW 디바운싱 처리를 위해 0.1μF 커패시터 장착 
---  

## ⚙️ STM32CubeMX 설정

- GPIO
    - PB0, PB1: EXTI Interrupt (Rising and Falling edge)
    - 내부 풀업 저항 비활성화 (No pull-up and no pull-down)

- NVIC
    - EXTI line0 interrupt
    - EXTI line1 interrupt

- I2C
    - I2C2: SSD1306 연결 (400kHz : fast mode)
    - I2C2 - SCL PB10
    - I2C2 - SDA PC12

- TIM2
    - Prescaler: 89 -> 20ms로 맞춰주기위해 
    - Period: 19999+

- file 수정
    - fonts.h, ssd1306.h 내에서 stm32f4xx_hal.h로 수정
    - ssd1306.c 에서 extern I2C_HandleTypeDef hi2c2; 로 수정

> [이전 dc모터 ioc setting 참고](./Dc_Motor.md)
---
## 💻 코드 설명
동작요약 : 


---

## ⚠️🛠️ 문제 해결 및 개선/확장

### 문제상황1.
while 무한루프 블로킹 문제 (로커스위치를 폴링방식으로 적용하려는데 계속  동작이 안됨)<br>
해결 : 

-> 하드웨어는 그대로, 빈 프로젝트로는 정상적으로 동작함

-> HAL_TIM_Base_Start_IT(&htim2); 를 주석처리하니까 정상적으로 동작함 >> 타이머2에서 문제가 발생했던거임

>> 현재 코드에서 HAL_TIM_PeriodElapsedCallback() 인터럽트 콜백 함수 안에서 I2C 기반 OLED 제어 
함수들을 호출하고 있으며, 이로 인해 CPU가 인터럽트 서비스 루틴(ISR)에 블로킹된 채 
빠져나오지 못하는 상황이 발생합니다.

SSD1306_UpdateScreen();   // I2C 통신 포함!

➡ 이 중 SSD1306_UpdateScreen() 함수는 내부에서 HAL_I2C_Master_Transmit()을 사용합니다.
이 함수는 **블로킹 방식(BLOCKING I2C API)**이며, 인터럽트 루틴 안에서 호출되면 다음과 같은 문제가 생깁니다:

인터럽트가 끝나야만 main loop (while(1))으로 복귀 가능

하지만 I2C는 내부적으로도 인터럽트를 사용하기 때문에 인터럽트 내에서 I2C 호출 시 데드락이 발생

결과: MCU는 응답이 없는 것처럼 정지

해결 : OLED 화면 업데이트는 while(1) 루프에서만 수행하고, 타이머 인터럽트에서는 플래그만 설정하세요.

인터럽트 기반(TIM2)과 폴링 기반(while-loop)은 우선순위나 정확도에서 차이가 있을 수 있으므로, 중요한 제어는 인터럽트에서 처리하고, 
디스플레이나 보조 상태 확인 등은 while 루프에서 처리하는 것이 일반적인 구조입니다.


### 개선사항
- CubeMX에서 Peripheral 별 파일 분할

### 설정 방법
1. CubeMX 실행 후 .ioc 파일 열기
2. 상단 메뉴에서<br>
    Project Manager > Code Generator 탭 선택
3. Generate peripheral initialization as a pair of '.c/.h' files per peripheral   
4. 저장 후 GENERATE CODE 클릭

- 이 설정이 되어 있지 않으면 main.c 내부나 stm32f4xx_hal_msp.c 등에 모든 초기화 코드가 모이게 된다.

### Peripheral 별 파일 분할의 장점
- 가독성 향상
    - 각 주변장치의 설정이 해당 전용 파일에 명확히 구분되므로, 코드를 파악하고 수정하기 쉬워짐
- 유지보수 용이
    - 예를 들어 I2C 설정만 바꾸고 싶을 때 i2c.c만 보면 되며, 다른 설정과 충돌하지 않도록 관리가 용이함
- 빌드 속도 개선
    - 변경이 발생한 .c 파일만 컴파일되므로, 프로젝트 규모가 클수록 빌드 시간 감소에 효과적임
- Git 버전관리 효율 
    - 기능별로 커밋이 가능하므로 이력 관리가 체계적으로 됨. (예: tim.c만 수정한 경우 diff 확인이 쉬움)
- 팀 협업에 유리
    - 여러 사람이 동시에 I2C, TIM 등 서로 다른 기능을 작업할 수 있어 충돌 가능성이 줄어들게 됨


---

## 💡 향후 확장 및 개선 아이디어
- 해결못한사항 -> timer2 내부의 하드웨어 주기와 while 내부의 주기가 다르기 때문에 oled 출력에 rpm max 값이 230정도로 낮게 출력됨.


 
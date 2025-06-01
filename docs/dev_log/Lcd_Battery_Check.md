
# [I2C 16x2 LCD] 배터리 잔량 측정 LCD 모니터링 로직 구현


## 🎯 프로젝트 활용 방안
LCD 디스플레이는 차량 후방에 장착되어, 주행 전 차량의 배터리 잔량, 각 MCU 간의 통신 상태, 차량과 핸들 간의 통신 상태 등을 확인하는 용도로 사용된다. 해당 정보는 실시간 출력이 요구되지 않기 때문에, 다중 슬레이브 연결이 가능하고 하드웨어 자원 소모가 적은 I2C 방식의 LCD 디스플레이를 채택하였다.

---

## 📖 이론 개요

### LCD display 동작원리

이 디스플레이 모듈은 HD44780 문자 LCD와 PCF8574T I/O 확장 칩이 결합된 구조로, MCU에서 I2C를 통해 전송한 데이터를 PCF8574T가 8비트 병렬 신호로 변환하여 HD44780에 전달하고, 이를 통해 문자가 LCD에 출력되는 방식으로 동작한다. 5V의 전압에서 동작 하며, SDA, SCL 두 핀만으로 제어가 가능해 I/O 핀을 절약할 수 있다.

### ADC(아날로그-디지털 변환)로 배터리 잔량 측정 원리
STM32F103 보드의 ADC는 입력된 아날로그 전압을 기준 전압(Vref, 일반적으로 3.3V)을 기준으로 12비트 해상도(0~4095의 정수)로 디지털 값으로 변환하는 장치로, 예를 들어 1.65V 입력 시 2047 정도의 값을 출력한다. 본 프로젝트에서 사용하는 3.7V 리튬이온 배터리는 방전 상태에 따라 전압이 달라지므로, 이 전압을 측정하여 계산하면 대략적인 잔량(%)을 추정할 수 있다.


> [LCD display datasheet](../datasheets/I2C_1602_LCD.pdf)<br>
> [STM32F103C8T6 datasheet](../datasheets/STM32F103C8T6_datasheet.pdf)
---

## 🔌 하드웨어 연결

### LCD 디스플레이

datasheet를 통해 FT(five volt tolerant 핀 확인해야함)

|LCD display|F103보드|
|:---:|:---:|
|Vcc|5V|
|Gnd|Gnd|
|SDA|B7|
|SCL|B6|

### ADC 분압회로 연결
STM32의 ADC 핀은 3.3V를 초과하는 전압을 허용하지 않기 때문에, 최대 12.6V(3.7v 직렬 3구)까지 도달하는 배터리 전압을 안전하게 측정하기 위해 저항 분압 회로를 구성하여 이를 약 3.2V 수준으로 낮춘 후 ADC에 입력한다.

### 원하는 분압 비율: R2/(R1+R2) = 3.2/12.6 = 0.254

### R1, R2 계산식

R2/(R1+R2)= 0.254= 1/(R1/R2)+1 = 약0.254
R1/R2 = 1/0.254-1 = 약 2.94
즉, R1은 2.94xR2와 근접함

표준저항에서 선택하면, R1=20k, R2=6.8k로 선택할 수 있음


```yaml
(+) 배터리 (3.7v 3S = 12.6V)
   |
   |
  [ R1 = 20kΩ ]
   |
   |---------> Vout 3.2V (F103보드 PA1)
   |
  [ R2 = 6.8kΩ ]
   |
   |
  GND (공통)
```
---

## ⚙️ STM32CubeMX 설정

### LCD 디스플레이
system core > sys > debug: serial wire

connectivity > i2c1 > i2c로 설정

<라이브러리 추가>
Core/Inc/liquidcrystal_i2c.h
Core/Src/liquidcrystal_i2c.h 

<liquidcrystal_i2c.h의 device i2c address 수정>
datasheet을 통해 i2c 주소를 0x27로 설정하면 되는 것을 확인<br>
#define DEVICE_ADDR     (0x3F << 1) 을 (0x27 << 1)로 바꾸기<br>
#include "stm32f1xx_hal.h" 로 수정

※ **I2C 장치중에서 Address를 확실하게 모를때**<br>
아래 코드를 넣고 출력을 하면 간단하게 7비트 Address를 확인할 수 있다.

```c
uint8_t address = 0;
  while(HAL_I2C_IsDeviceReady(&hi2c1, address, 10, 1000)!=HAL_OK) {
      address++;
  }
  printf("I2C Address is 0x%02X(7bit value)\r\n", address>>1);
  while(1);
```

### ADC 배터리 잔량측정

sys > serial Wire (안하면 target is not responding retrying 오류 발생)

analog > adc1 > in1
Scan Conversion Mode: Disabled

Continuous Conversion Mode: Disabled

Discontinuous Conversion Mode: Disabled

External Trigger Conversion: Software Start

DMA Continuous Requests: Disabled

Data Alignment: Right

Number of Conversion: 1

channel : ADC1_IN1
Rank : 1
Sampling Time : 239.5 cycles (정확도 ↑)

connectivity > i2c1 > speed : 100kHz (표준속도)


> 참고링크 : https://blog.naver.com/kiatwins/221118252542<br>
> 참고링크 : https://www.micropeta.com/video57

---
#### ▶️ 선형 보간 방식을 적용한 ADC 배터리 측정
## 💻 코드 설명
동작 요약 : ADC 기능을 이용해 배터리 전압을 측정하고, 계산된 전압과 잔량 퍼센트를 I2C 방식의 LCD에 1초마다 출력한다.

### 1. ADC 전압출력 및 배터리 전압 → 퍼센트 계산 함수

#### 보정 계수 정의
```c
float correction = 3.15/3.11f;
```
- 멀티미터로 측정한 ADC 핀으로 입력되는 전압은 3.15v인 반면, ADC 출력은 3.11v이기 때문에 오차를 보정하기 위한 비례 계수를 적용

#### ADC 값 읽기
```c
HAL_ADC_Start(&hadc1);
HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
uint16_t adc_val = HAL_ADC_GetValue(&hadc1);
HAL_ADC_Stop(&hadc1);
```
- ADC 변환 시작 → 완료 대기 → ADC 값 읽기 → 변환 종료 순서

#### ADC 값 → 분압된 전압(Vout) 계산
```c
float vout_raw = (3.3f * adc_val) / 4095.0f;
float vout = vout_raw * correction;
```
- 12비트 ADC 기준 최대값은 4095이므로, 3.3V * (adc_val / 4095)로 ADC 핀의 실제 전압을 구하고 보정 계수로 실제 측정값에 더 근접한 값으로 보정

#### 분압 비율을 통해 실제 배터리 전압(Vbat) 계산
```c
float vbat = vout * ((19.7f + 6.73f) / 6.73f);  // ≈ 3.927
```
- 저항 분압 회로의 비율에 따라 원래 배터리 전압을 복원

- Vout은 분압된 값이므로, 역으로 Vbat = Vout * (R1+R2)/R2

#### 전압 → 퍼센트로 환산, 범위제한
```c
float percent = ((vbat - 9.0f) / (12.6f - 9.0f)) * 100.0f;

if (percent > 100.0f) percent = 100.0f;
if (percent < 0.0f) percent = 0.0f;
```
- 배터리 전압이 9.0V~12.6V 범위일 때, 해당 전압이 몇 퍼센트에 해당하는지 계산
- 실제 전압이 범위를 벗어날 수 있으므로 출력값은 0~100%로 클램핑

### 2. main 함수

#### LCD 초기화
```c
HD44780_Init(2);   // 2줄 LCD 설정
HD44780_Clear();   // LCD 화면 초기화
```
- liquidcrystal_i2c.h 기반 라이브러리 사용하여 I2C LCD 모듈 초기화

#### 출력 문자열 변수 선언
```c
char line1[16], line2[16];
```
- LCD에 출력할 문자열 버퍼
- HD44780 LCD는 16x2 문자형 디스플레이이므로 16바이트 확보

#### 무한루프 내부 동작

##### 1. 배터리 상태 측정
```c
float vout = 0.0f;
float percent = Read_Battery_Percentage(&vout);
```
- Read_Battery_Percentage() 호출로
vout에 보정된 ADC 출력 전압 저장
- 반환값으로 percent에 배터리 잔량 계산

##### 2. 문자열 구성
```c
uint8_t display_percent = (uint8_t)(percent + 0.5f);
snprintf(line1, sizeof(line1), "Battery: %3d%%", display_percent);
snprintf(line2, sizeof(line2), "Voltage: %.2f V", vout);
```
- percent를 반올림한 뒤 정수 형태로 출력
- line1: 잔량, line2: 전압

##### 3. LCD 출력
```c
HD44780_Clear(); //초기화
// 각 줄에 문자열 출력
HD44780_SetCursor(0, 0);
HD44780_PrintStr(line1);

HD44780_SetCursor(0, 1);
HD44780_PrintStr(line2);

// 매 1초마다 업데이트 주기 설정
HAL_Delay(1000); 
```

> 참고링크 : https://www.micropeta.com/video57 <br>
> 커밋 해시 [8bd32fd](https://github.com/YeonsuJ/Car_control_project/commit/8bd32fd9d1ee292344dcbc0b8382e334841c31ea)


---


## ⚠️🛠️ 문제 해결 및 개선/확장

### 문제상황 1.
리튬배터리의 정상 사용범위는 12.6v ~ 9.6v이며, 배터리 보호 회로 BMS의 컷오프 전압은 보통 9.0V 근처이다. STM32의 외부전원과 dc모터 전원으로 함께 사용하기 때문에 9.6V 정도가 실질적인 사용 한계로 봐야 하며 그 이하로 떨어지면 출력이 급격히 약해짐<br>
**해결** : 이전 코드의 전압범위를 12.6v to 9.6v로 수정하여 해결하였다.

### 문제상황 2.
STM32CubeIDE (GCC toolchain 기반)에서 printf()는 기본적으로 float 포맷팅 (%f, %.2f 등)을 지원하지 않도록 설정되어 있음 <br>
→ 코드에는 문제 없지만 링커 설정이 부족해서 컴파일러가 지원하지 못함<br>
**해결** : 프로젝트 우클릭 > properties > c/c++ build -> settings > tool settings > mcu/mpu settings > use float printf, scanf 두 개 체크


---

## 💡 향후 확장 및 개선 아이디어
- 룩업 테이블 + 선형 보간 방식 적용<br>
(비선형 전압-잔량 특성을 반영한 LUT 기반 방식
오차 줄이기 위해 전압→% 보간)

- 전압 히스테리시스 + 디지털 필터 <br>
(ADC 입력값 평균 필터링 적용 → 노이즈 대응
급격한 전압 하락에 대해 히스테리시스 적용해 잔량 출력 안정화)

- ADC 내부 채널을 활용한 칩 내부 온도센서 값 디스플레이
-> https://blog.naver.com/leeinje66/223139324543

- LED low battery 경고 등 추가 및 9.6v 이하에서는 모터 동작 강제 중단 기능 고려





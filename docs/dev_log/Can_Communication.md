#  CAN 통신 송/수신 초기 기능 구현 (m3)

## 🎯 프로젝트 활용 방안
본 프로젝트는 RC카 핸들 제어를 위한 무선 컨트롤러에 자이로센서(MPU6050)를 장착하고, 차량의 회전 상태(Roll 각도)를 실시간으로 I2C 방식 OLED(SSD1306)에 시각적으로 출력하는 기능을 구현한다. 운전자의 직관적인 제어를 돕고, 자이로센서 데이터가 정상적으로 수신되고 있는지를 현장에서 즉시 확인하는 데 활용할 수 있다.


## 📖 이론 개요
MPU6050 자이로센서 동작 원리
MPU6050은 3축 가속도계와 3축 자이로스코프가 통합된 6축 센서로, I2C 인터페이스를 통해 데이터를 송수신한다. 내부에 칼만 필터(Kalman filter)가 적용된 연산을 통해 자세(Roll, Pitch)를 비교적 안정적으로 계산할 수 있으며, 본 프로젝트에서는 Roll 각도만 추출하여 OLED에 출력한다.

SSD1306 OLED 디스플레이 동작 원리
SSD1306은 128x64 해상도의 모노크롬 OLED 디스플레이로, SPI 또는 I2C로 MCU와 통신한다. 본 프로젝트에서는 I2C를 채택하여 데이터와 명령어를 전송하고, 그래픽 라이브러리(fonts.h, ssd1306.h)를 통해 텍스트를 출력한다.


---

## 🔌 하드웨어 연결

### 송신부(Rx)
<img src="../wiring_diagram/central_v1.1.0.png" alt="Locker, Encoder RPM 측정 추가 배선도" width="500"/>

※ 수신부(Rx)는 [V1.0.0_RF_MotorCtrl.md](./V1.0.0_RF_MotorCtrl.md)와 동일

---

## ⚙️ STM32CubeMX 설정

### 송신부 변경 사항
- Motor_Forward 
    - PB8 : GPIO_Input (Pull-Up)
- Motor_Backward 
    - PB9 : GPIO_Input (Pull-Up)


### 수신부 변경사항

DC모터 Encoder
- combined channels > encoder mode
- pb6 (tim4_ch1) : encoderA
- pb7 (tim4_ch2) : encoderB
- encoder mode TI1 adn TI2
- prescaler : 0
- Counter period(ARR) : 65535 or 0xFFFF (엔코더 overflow 방지)
- Counter Mode : UP
- IC1 Polarity: Rising Edge
- IC1 Selection: Direct TI
- IC1 Prescaler: No division (1번마다 1번 인식 = 모든 엣지를 다 인식함)
- IC1 Filter: 10 (노이즈가 심하면 값을 올려줘야함)
- (IC2 항목도 동일하게 설정)

> 참고 [V1.0.0_RF_MotorCtrl.md](./V1.0.0_RF_MotorCtrl.md)
---

## 💻 동작 코드
다음은 기존의 v1.0.0 버전의 코드를 리팩토링한 사항을 요약한 것이다.

## 루프백 모드를 활성화 내부 송/수신 test






---

## 💡 향후 확장 및 개선 아이디어
- 장애물을 초음파 센서로 감지 -> RF 통신을 통해 핸들로 전송, 핸들의 햅틱 센서로 운전자에게 진동 경고를 제공 
- (파이프라인 추가해 Rx와 Tx 역할을 교체하여 양방향 통신을 구현)

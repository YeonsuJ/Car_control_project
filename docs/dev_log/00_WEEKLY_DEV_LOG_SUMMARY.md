# Weekly Dev_log Summary 

## 1주차 (25-05-12 ~ 25-05-18) 
|내용|문서 링크|커밋해시|
|---|:---:|:---:|
|- 초기 아이디어 구상<br>- 보드 및 부품 선정 후 구매<br>- 마일드스톤 개괄 작성|[Initial_Planning.md](./Initial_Planning.md)|-|

---

## 2주차 (25-05-19 ~ 25-05-25)
|내용|문서 링크|커밋해시|
|---|:---:|:---:|
|Git commands 학습|[Github_Manuals.md](./Github_Manuals.md)|-|
|Github을 활용한 버전관리법 학습|[Managing_Ver.md](./Managing_Ver.md)|-|
|STM32CubeIDE와 Git & Github 관리/연동|[Managing_Git_CubeIDE.md](./Managing_Git_CubeIDE.md)|-|
|폴링방식 기반 -> 인터럽트 기반 타이머 인풋 캡쳐 활용 초음파센서 거리값 출력 로직 구현 |[Ultrasonic.md](./Ultrasonic.md)|[`7165ee7`](https://github.com/YeonsuJ/Car_control_project/commit/7165ee7f55b6ba93f8e09380047e063f922466cb)|

---

## 3주차 (25-05-26 ~ 25-06-01)
|내용|문서 링크|커밋해시|
|---|:---:|:---:|
|자이로센싱값 -> OLED 모니터링 로직 구현|[Gyro_Display.md](./Gyro_Display.md)|[`229aaa8`](https://github.com/YeonsuJ/Car_control_project/commit/229aaa8b4e313b1294b8de80c3d619d1e4bc61c4)|
|배터리 잔량 측정 -> LCD 모니터링 로직 구현|[Lcd_Battery_Check.md](./Lcd_Battery_Check.md)|[`3cadc21`](https://github.com/YeonsuJ/Car_control_project/commit/3cadc214babbb125b15034e2ebce0408e0c439cd)|
|서보모터 PWM 듀티비 기반 조향 각도 매핑 및 좌/우 조향 한계각 보정|[Servo_Motor.md](./Servo_Motor.md)|[`1d18eda`](https://github.com/YeonsuJ/Car_control_project/commit/1d18eda88c05c9aac707a6153e64591776d5ae05)|
|자이로 roll 값 -> 서보모터 구동|[Gyro_Pwm_Mapping.md](./Gyro_Pwm_Mapping.md)|[`7dc0c12`](https://github.com/YeonsuJ/Car_control_project/commit/7dc0c1298043c86b4969e85140554ca7ac16d90a)|

---

## 4주차 (25-06-02 ~ 25-06-08)
|내용|문서 링크|커밋해시|
|---|:---:|:---:|
|버튼 눌림 시간 측정 로직 구현|[Timer_Switch.md](./Timer_Switch.md)|[`7dcde54`](https://github.com/YeonsuJ/Car_control_project/commit/7dcde5411003d99339164ecea6fa701451369387)|
|PWM + 엔코더 기반 DC모터 동작 및 RPM 측정 로직 구현|[Dc_Motor.md](./Dc_Motor.md)|[`45c4e16`](https://github.com/YeonsuJ/Car_control_project/commit/45c4e164ce43241788da8687f2dbf96ccc241a7f)|

---

## 5주차 (25-06-09 ~ 25-06-15)
|내용|문서 링크|커밋해시|
|---|:---:|:---:|
|버튼 누름 시간 기반 DC 모터 속도 제어 및 로커 스위치 전/후진 전환 + OLED 상태 표시 기능 구현|[Dc_Motor_Oled_Switch.md](./Dc_Motor_Oled_Switch.md)|[`ad9f4bc`](https://github.com/YeonsuJ/Car_control_project/commit/ad9f4bc341844dbb1dcecefefb23643b87a2f076)|
|초음파센서 거리설정값 기반 진동모터 햅틱 피드백 로직 추가|[Ultrasonic_Vibration.md](./Ultrasonic_Vibration.md)|[`3be3675`](https://github.com/YeonsuJ/Car_control_project/commit/3be3675e9acbf0c19c06f436050e4edda30a9942)|

---

## 6주차 (25-06-16 ~ 25-06-22)
|내용|문서 링크|커밋해시|
|---|:---:|:---:|
|선형 보간 방식 기반 ADC 배터리 측정 후 OLED 출력 로직으로 수정|[Adc_Oled_Battery_Check.md](./Adc_Oled_Battery_Check.md)|[`7bd584e`](https://github.com/YeonsuJ/Car_control_project/commit/7bd584ef51410f1fcd1aae941d038a377702ed81)|
|Shift-lock 시스템 적용: DC모터 전/후진 전환 시 브레이크 입력 필수 조건 추가|[Dc_Motor_Oled_Switch.md](./Dc_Motor_Oled_Switch.md)|[`3c3b16f`](https://github.com/YeonsuJ/Car_control_project/commit/3c3b16f653379466fd9c0d39f7b53c472481e6a6)|

---

## 7주차 (25-06-23 ~ 25-06-29)
|내용|문서 링크|커밋해시|
|---|:---:|:---:|
|DC모터와 Servo모터 동작 기능 통합 및 핀 라벨링 추가|-|[`b474df6`](https://github.com/YeonsuJ/Car_control_project/commit/b474df67b9a24d75a08725cf00dbaa94170cabac)|
|RF 통신을 통한 roll 값, 악셀 / 브레이크 버튼 눌림 시간 송수신 및 실시간 서보모터 조향 구현 (m3)|[M3_RF_Communication.md](./M3_RF_Communication.md)|[`RX`](https://github.com/YeonsuJ/Car_control_project/commit/cbc18035c71e6f3c50977fd6d9abb157786e36d2) / [`TX`](https://github.com/YeonsuJ/Car_control_project/commit/88aaa169f57b4323f77507a8af936f329d8cdfcc)|

---

## 8주차 (25-06-30 ~ 25-07-06)
|내용|문서 링크|커밋해시|
|---|:---:|:---:|
|CAN 통신 노말모드 기반 송/수신 초기 기능 구현 (m3)|[Can_Communication.md](./Can_Communication.md)|[`RX`](https://github.com/YeonsuJ/Car_control_project/commit/89800bed5bbd43df382bfc1d63d63a506a9933f4) / [`TX`](https://github.com/YeonsuJ/Car_control_project/commit/e1316b007e2b4ca462092e7ce0a50db136938af6)|

---

## 9주차 (25-07-07 ~ 25-07-13)
|내용|문서 링크|커밋해시|
|---|:---:|:---:|
|M3보드 기반 RF 송/수신 모터 제어 통합 (v1.0.0)|[V1.0.0_RF_MotorCtrl.md](./V1.0.0_RF_MotorCtrl.md)|[`central_v1.0.0`](https://github.com/YeonsuJ/Car_control_project/commit/087c00a452b5e8d1e8889b24831789548ee6a3f0) / [`controller_v1.0.0`](https://github.com/YeonsuJ/Car_control_project/commit/de77deb8e3d7acceab46b517a5e457805b62f0cb)|
|코드 리팩토링 및 Locekr 스위치 기반 전/후진 기능 추가, <br>DC모터 Encoder 기반 RPM 측정 로직 추가 (v1.1.0)|[V1.1.0_Locker_Encoder.md](./V1.1.0_Locker_Encoder.md)|[`central_v1.1.0`](https://github.com/YeonsuJ/Car_control_project/commit/94a4a332562ac80192bba8cc971c298cab48cbfd) / [`controller_v1.1.0`](https://github.com/YeonsuJ/Car_control_project/commit/241c4c5877d608a3723825619a0ed84717d12e22)|

---

## 10주차 (25-07-14 ~ 25-07-20)
|내용|문서 링크|커밋해시|
|---|:---:|:---:|
|ACK 페이로드 기반 RF 양방향 송수신 구현|[null.md](./null.md)|[`controller_v1.5.0`](https://github.com/YeonsuJ/Car_control_project/commit/0745e3354e6c7a4ae12c31953c2b0a166a373d1a) / [`central_v1.5.0`](https://github.com/YeonsuJ/Car_control_project/commit/fb063337c0dd13055ecf7afc8d4cca3d3b347613)|
|CAN 송/수신 초음파센서 -> 햅틱센서 제어 통합|[null.md](./null.md)|[`central_v1.5.0`](https://github.com/YeonsuJ/Car_control_project/commit/fb063337c0dd13055ecf7afc8d4cca3d3b347613) / [`sensor_v1.0.0`](https://github.com/YeonsuJ/Car_control_project/commit/13f38c18cb6364108f1dbb893375f56113965320) / [`status_v1.0.0`](https://github.com/YeonsuJ/Car_control_project/commit/d7810ab06a289a489a91285c02ddfea00daa1e14)|

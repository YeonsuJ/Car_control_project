# 개발 간 사용한 보드 및 모듈

**Board**
| 품목 | 용도 | 수량 | 이미지 |
| :------: | :------: |:------: |:------: |
| STM32F103C8T6 | controller MCU <br> car_central ECU <br> car_status ECU <br> car_sensor ECU |4| <img src="./images/part_list/stm32f103.png" alt="부품 이미지" width="200" />|
<br/>

**Sensor**
| 품목 | 용도 | 수량 | 이미지 |
| :------: | :------: |:------: |:------: |
| [MPU-6050] 6축 IMU 센서 | controller - 모션 제어|1|<img src="./images/part_list/mpu-6050.jpg" alt="부품 이미지" width="90" />|
|[HC-SRO4] 초음파센서|전/후방 장애물 감지|2|<img src="./images/part_list/hc-sro4.png" alt="부품 이미지" width="100" />|
|[LM393 ldr] 조도센서|주변 밝기 판단|1|<img src="./images/part_list/gl5528(LDR).png" alt="부품 이미지" width="120" />|
<br/>

**Communication module**
| 품목 | 용도 | 수량 | 이미지 |
| :------: | :------: |:------: |:------: |
|[NRF24L01+PA+LNA] 2.4G 무선 모듈|car-controller RF 통신|2|<img src="./images/part_list/nrf24l01+pa+lna(rf_module).jpg" alt="부품 이미지" width="120" />|
|[MCP2551] CAN 트랜시버 모듈| car - can 통신 |3|<img src="./images/part_list/(MCP2551)can_transceiver.png" alt="부품 이미지" width="120" />|
<br/>


**Motor**
| 품목 | 용도 | 수량 | 이미지 |
| :------: | :------: |:------: |:------: |
|[JGA25-370] DC모터|내부 인코더를 통해 dc모터의 RPM 확인가능|1|<img src="./images/part_list/jga25-370(dc_motor).png" alt="부품 이미지" width="100" />|
|L298N|Dc motor driver - DC모터 PWM 제어 및 방향 변경|1|<img src="./images/part_list/l298n(motor_driver).png" alt="부품 이미지" width="100" />|
|진동모터|controller - 충돌 경고 핸들 진동|2|<img src="./images/part_list/vibration_motor_module.png" alt="부품 이미지" width="100" />|
|[MG996R]servo 모터|전륜 조향|1|<img src="./images/part_list/mg996r(servo_motor).png" alt="부품 이미지" width="100" />|
<br/>


**Etc**
| 품목 | 용도 | 수량 | 이미지 |
| :------: | :------: |:------: |:------: |
|RC카 섀시, 휠|차량의 뼈대|1|<img src="./images/part_list/rc_car_chassis.png" alt="부품 이미지" width="150" />|
|Locker 스위치| car - 배터리 on/off |1|<img src="./images/part_list/locker_switch.png" alt="부품 이미지" width="70" />|
|토글 스위치| controller - 기어변속|1|<img src="./images/part_list/toggle_switch.png" alt="부품 이미지" width="70" />|
|tactile 스위치|브레이크,악셀 버튼용|2|<img src="./images/part_list/tactile_switch.png" alt="부품 이미지" width="70" />|
|LED|전조등, 후미등, 브레이크등|6|<img src="./images/part_list/10mm_led.jpg" alt="부품 이미지" width="100" />|
|OLED 디스플레이| controller - 주행 중 현황 확인 <br> car - 차량 상태 확인 |2|<img src="./images/part_list/oled_display.png" alt="부품 이미지" width="100" />|
|충전형 18650 리튬 배터리(3000mAh)| car - 3s <br> controller - 2S|5|<img src="./images/part_list/18650_batteries.png" alt="부품 이미지" width="100" />|
|DC 스텝다운 컨버터| MCU 5V 전압공급 목적 강압 <br> car : 12v->5v <br> controller : 8v->5v |2|<img src="./images/part_list/(lm2596s)dc_stepdown_converter.png" alt="부품 이미지" width="100" />|
|저항|120옴 - CAN 종단저항(3) <br> 100옴 - LED용(6)|9|<img src="./images/part_list/resistor.png" alt="부품 이미지" width="100" />|



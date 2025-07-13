# 차량 메인 제어 유닛

## 모듈 테스트 히스토리

| 커밋해시값 | 날짜 | 변경 내용 |
|-----|------|------|
| [`e97e7fc`](https://github.com/YeonsuJ/Car_control_project/commit/e97e7fc52aa7a19635b8d633540d07a471143fa9) | 2025-05-23 | Unit_car_central 프로젝트 파일 생성 |
| [`c666fd3`](https://github.com/YeonsuJ/Car_control_project/commit/c666fd3a62c6b6f6c9f868a21637e81b8ca79b3c) | 2025-05-30 | 서보모터 PWM 듀티비 기반 조향 각도 매핑 및 좌/우 조향 한계각 보정 |
| [`1d18eda`](https://github.com/YeonsuJ/Car_control_project/commit/1d18eda88c05c9aac707a6153e64591776d5ae05) | 2025-06-02 | 서보모터 좌우 한계각 범위 내의 펄스 폭에 따른 Compare 값 조정 |
| [`45c4e16`](https://github.com/YeonsuJ/Car_control_project/commit/45c4e164ce43241788da8687f2dbf96ccc241a7f) | 2025-06-05 | DC 모터 제어 및 엔코더 기반 RPM 측정 기능 구현 |
| [`3d1299e`](https://github.com/YeonsuJ/Car_control_project/commit/3d1299e5b8503f3396420f090a27b58ee6edd679) | 2025-06-09 | 버튼 눌림 시간 기반 DC 모터 PWM 제어 및 RPM 계산 후 OLED 실시간 상태 출력 로직 추가 |
| [`ad9f4bc`](https://github.com/YeonsuJ/Car_control_project/commit/ad9f4bc341844dbb1dcecefefb23643b87a2f076) | 2025-06-11 | 로커 스위치를 이용한 DC 모터 전/후진 제어 및 OLED 실시간 상태 표시 기능 추가|
| [`3c3b16f`](https://github.com/YeonsuJ/Car_control_project/commit/3c3b16f653379466fd9c0d39f7b53c472481e6a6) | 2025-06-22 | Shift-lock 시스템 적용: DC모터 전/후진 전환 시 브레이크 입력 필수 조건 추가|
| [`b474df6`](https://github.com/YeonsuJ/Car_control_project/commit/b474df67b9a24d75a08725cf00dbaa94170cabac) | 2025-06-27 | DC모터와 Servo모터 동작 기능 통합 및 핀 라벨링 추가|

---

## 버전 히스토리
| 버전 | 날짜 | 변경 내용 |
|----------|--------------|-----------|
| [`central_v1.0.0`](https://github.com/YeonsuJ/Car_control_project/commit/087c00a452b5e8d1e8889b24831789548ee6a3f0) | 2025-07-08 | M3로 변경 및 RF 수신 기반 DC, Servo모터 제어 통합 (v1.0.0)|
| [`central_v1.1.0`](https://github.com/YeonsuJ/Car_control_project/commit/94a4a332562ac80192bba8cc971c298cab48cbfd) | 2025-07-11 | 코드 리팩토링 및 전/후진 기능, 모터 RPM 측정 로직 추가 (v1.1.0)|

# 차량 센서 제어 유닛

## 모듈 테스트 히스토리

| 커밋해시값 | 날짜         | 변경 내용 |
|----------|--------------|-----------|
| [`34db804`](https://github.com/YeonsuJ/Car_control_project/commit/34db80414a831400c01b22d808842b0f45d03a0e) | 2025-05-23 | Unit_car_sensor 프로젝트 파일 생성 |
| [`7165ee7`](https://github.com/YeonsuJ/Car_control_project/commit/7165ee7f55b6ba93f8e09380047e063f922466cb) | 2025-05-24 | 인터럽트 기반 타이머 인풋 캡쳐를 활용한 초음파센서 거리값 출력 로직 구현|
| [`3be3675`](https://github.com/YeonsuJ/Car_control_project/commit/3be3675e9acbf0c19c06f436050e4edda30a9942) | 2025-06-13 | 초음파센서 거리설정값 기반 진동모터 햅틱 피드백 로직 추가|
| [`e1316b0`](https://github.com/YeonsuJ/Car_control_project/commit/e1316b007e2b4ca462092e7ce0a50db136938af6) | 2025-07-01 | (F103C8T6) CAN 통신 송신부(TX) 초기 기능 구현|


## 버전 히스토리
| 버전 | 날짜   | 변경 내용 |
|----------|--------------|-----------|
| [`sensor_v1.5.0`](https://github.com/YeonsuJ/Car_control_project/commit/13f38c18cb6364108f1dbb893375f56113965320) | 2025-07-17 | CAN 송신 기반 초음파센서 -> 햅틱센서 기능 통합 (v1.5.0)|
| [`sensor_v1.6.0`](https://github.com/YeonsuJ/Car_control_project/commit/22e10d68004d4dc19ce922d08332d6f769065621) | 2025-07-24 | 기능별 파일 분할 및 코드 최적화 + CAN 통신 개선 및 속도 상향(500kbps) (v1.6.0)|

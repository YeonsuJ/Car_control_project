# 차량 상태 디스플레이 유닛

## 모듈 테스트 히스토리

| 커밋해시값 | 날짜         | 변경 내용 |
|----------|--------------|-----------|
| [`297fb7a`](https://github.com/YeonsuJ/Car_control_project/commit/297fb7a5fb488cf9ed558b2aad0e361e1f59dd34) | 2025-05-23 | Unit_car_status 프로젝트 파일 생성 |
| [`3cadc21`](https://github.com/YeonsuJ/Car_control_project/commit/3cadc214babbb125b15034e2ebce0408e0c439cd) | 2025-05-29 | 선형 보간 방식을 적용한 ADC 배터리 측정 후 LCD 출력 로직 구현 |
| [`7bd584e`](https://github.com/YeonsuJ/Car_control_project/commit/7bd584ef51410f1fcd1aae941d038a377702ed81) | 2025-06-19 | 선형 보간 방식 기반 ADC 배터리 측정 후 OLED 출력 로직으로 수정|
| [`89800be`](https://github.com/YeonsuJ/Car_control_project/commit/89800bed5bbd43df382bfc1d63d63a506a9933f4) | 2025-07-01 | (F103C8T6) CAN 통신 수신부(RX) 초기 기능 구현|


## 버전 히스토리
| 버전 | 날짜         | 변경 내용 |
|----------|--------------|-----------|
| [`status_v1.5.0`](https://github.com/YeonsuJ/Car_control_project/commit/d7810ab06a289a489a91285c02ddfea00daa1e14) | 2025-07-17 | CAN 수신 기반 초음파센서 -> OLED 디버깅 기능 통합 (v1.5.0)|
| [`status_v1.6.0`](https://github.com/YeonsuJ/Car_control_project/commit/ff784f4d59c5a10102964285e873eb8930eb3fa3) | 2025-07-24 | 파일분할 및 코드최적화, CAN 개선, ADC배터리측정 추가 (v1.6.0)|

# 핸들링 모션 기반 RC카 네트워크 시스템 개발
이 리포지토리는 `핸들링 모션 기반 RC카 네트워크 시스템` 프로젝트의 기획부터 최종 구현까지 모든 개발 과정, 소스 코드, 기술 문서를 기록하는 메인 개발 아카이브입니다.

## 프로젝트 문서
프로젝트 개발 과정에서 작성된 모든 문서는 docs 폴더 내에 체계적으로 정리되어 있습니다.
```
docs/
├── 3d_model/           # 컨트롤러 외형 설계 도면
├── datasheets/         # 개발에 참고한 부품 데이터시트
├── dev_logs/           # 상세 개발 일지
├── images/             # 프로젝트 관련 사진
├── troubleshooting/    # 문제 해결 과정 기록
├── wiring_diagram/     # 하드웨어 배선도
├── dev_log_summary.md  # 주간 개발 일지 요약
└── parts_list.md       # 전체 부품 목록
```
**주간 개발 일지 요약** : 전체 진행 상황은 [dev_log_summary.md](./docs/dev_log_summary.md) 에서 확인하실 수 있습니다.

---

## 브랜치 전략
기능 단위 개발을 위해 아래와 같이 브랜치를 나누어 관리합니다. 각 브랜치에는 해당 기능의 개발 소스 코드가 커밋 단위로 기록되어 있으며, 상세 개발 히스토리는 별도 문서로 확인할 수 있습니다.

- `main` : 최종 안정화 버전의 코드가 병합되는 기본 브랜치 
- `develop` : 개발 과정에서 작성된 모든 문서를 관리하는 브랜치
- `Unit_controller` : 컨트롤러 기능 개발 → [[상세 히스토리](https://github.com/YeonsuJ/Car_control_project/tree/Unit_controller/ver_Unit_controller.md)]
- `Unit_car_central` : 차량 중앙 제어 시스템 개발 → [[상세 히스토리](https://github.com/YeonsuJ/Car_control_project/tree/Unit_car_central/ver_Unit_car_central.md)]
- `Unit_car_sensor` : 차량 센서 데이터 처리 기능 개발 → [[상세 히스토리](https://github.com/YeonsuJ/Car_control_project/tree/Unit_car_sensor/ver_Unit_car_sensor.md)]
- `Unit_car_status` : 차량 상태 모니터링 기능 개발 → [[상세 히스토리](https://github.com/YeonsuJ/Car_control_project/tree/Unit_car_status/ver_Unit_car_status.md)]





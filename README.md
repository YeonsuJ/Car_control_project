# Car_control_project

## 기존의 문제
기존의 can_handler.c 파일을 사용할 때, main 함수의 initialize 부에서 CANHandler_Init();을 선언하면  
커널 태스크로 진입을 할 수 없는 문제가 발생함. 
```c
  // 1. Can 시작, 필터 설정 및 인터럽트 등록 핸들러 초기화
  CANHandler_Init();  <---  문제 발생

  // 2. RF 핸들러 초기화
  RFHandler_Init();

  // 3. 모터 제어기 초기화 (PWM, 엔코더 시작)
  MotorControl_Init();
  .
  .
  .
  osKernelStart();  < ---  여기로 진입을 못 함
```
> 그래서 can_handler.c 파일을 수정함. 이전 코드에서는 CAN_Filter_Config 함수가 인자로 받은 포인터(hcan_ptr)를  
사용하지 않고, 초기화되지 않은 전역 변수 hcan을 사용하려다 실패하여 Error_Handler에 빠짐.  

## 진행 상황

 1. 수정한 can_handler.c 파일은 커널로 정상 진입하여 태스크 내부에 있는 LED 토글 테스트를 정상 동작시킴.  
 2. 이 파일을 마지막 커밋 (베어멘탈)에 이식하여 핸들과 정상 동작하는 것을 확인
 3. 커밋 & 푸시함
 4. 수정한 can_handler.c 파일을 기반으로 새로 작성한 RF_Task, CAN_Task 실행
 5. **작동 안함**

> 현재 로컬에는 작동하지 않는 코드 부분을 주석처리 해놈 RF_Task / CAN_Task  
커널의 정상진입을 확인하는 LED 테스트 코드만 활성화

## Comment
작동하지 않는 코드는 2개의 세마포어 및 하나의 뮤텍스 변수를 사용 RFSem & CanSem / CanMutex  
인터럽트가 발생할 때 세마포어 신호를 활성화하는 구조가 뭔가 잘못된 거 같음.  
 can_handler.c & rf_handler.c 의 콜백 함수에서 이 로직을 다룸   
- 뮤텍스 기법은 사용 안 해도 될 거 같음 (central은 read만 하기 때문에 동시접근 이론 상 문제 없어 보임)
- 기존의 안 되는 코드를 수정할 시 rtos.c / can_handler.c / rf_handler.c 의 세마포어 로직을 손보는 거 추천
- 새로 코드 짜는 거도 추천
- 새로 짤 때는 핸들러.c 파일에서 인터럽트를 세마포어 신호로 태스크로 넘기는 구조말고  
 기존의 인터럽트 구조에서 irq변수가 토글되는 것을 태스크에서 판단하여 로직을 짜는 것도 고려해볼만 함. 


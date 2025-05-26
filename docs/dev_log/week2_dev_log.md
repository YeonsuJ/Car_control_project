# [2주차] 주간 개발 일지 (25-05-19 ~ 25-05-25)

## 1. 작업 내용
1. Git commands 학습 및 STM32CubeIDE 와 Git & Github 관리/연동, 버전관리법 학습
2. 초음파센서 test
3. OLED test
4. 택트 스위치 test

## 2. 배경지식 정리 
-> [CubeIDE & git 연동 관리 가이드](../backgrounds/Managing_Git_CubeIDE.md)  
-> [버전 관리 가이드](../backgrounds/Managing_Ver.md)  
-> [git 명령어 가이드](../backgrounds/github_manuals.md)<br>
-> [HC-SR04 초음파센서 동작 원리](..backgroudns/Ultrasonic_sensor.md)


## 3. 소스코드 주요 사항
### 스위치 test
두개의 스위치를 입력, 두개의 LED를 출력으로 설정하여 폴링 방식으로 간단히 구현하였으며  
ioc를 통해 입력을 풀업 저항을 설정하여 외부 저항을 달지 않고 스위치를 동작시켰다.  
버튼이 눌릴 때 LED가 켜지고 버튼에서 손을 떼면 LED가 꺼진다.   
추후에 타이머 인터럽트를 기반으로 스위치가 눌린 시간을 측정하는 코드를 구현할 예정이다.
> 커밋 해시 [00041ac](https://github.com/YeonsuJ/Car_control_project/commit/00041ac7af3b44cfdb9ed819bdfbda69035885cd)

---
### OLED test
오픈 소스를 활용하여 기본적인 헤더파일과 c파일을 추가하고  
OLED 라이브러리를 사용하여 간단한 OLED 구동 코드를 구현하였다.    
OLED 라이브러리의 기능에 관한 코드 설명은 추후 이 블록에 업로드 예정.
~~~

~~~
- fonts.h, ssd1306.h 파일 Inc 폴더에 추가
- fonts.c, ssd1306.c 파일 Src 폴더에 추가
> 오픈소스 경로 -> https://www.micropeta.com/video19  
> [유튜브 참고 자료](https://www.youtube.com/watch?v=Mt6U30Yxrr8&t=34s)   
> 커밋 해시 [a89ee61](https://github.com/YeonsuJ/Car_control_project/commit/a89ee61071d8992d62197c93fb81851c717782c4)

---
### 초음파 test
기존 폴링방식으로 작성 -> Timer input caputure 기능 사용해서 구현함. 
> 커밋 해시 [82d4624](https://github.com/YeonsuJ/Car_control_project/commit/82d462479a9bef4ad92decd2cc37d7a1186b770f)

---

## 4. 하드웨어 연결도
[스위치](docs/wiring_diagram/tactile_switch.png)<br>
[OLED](docs/wiring_diagram/oled.png)<br>
[초음파센서](docs/wiring_diagram/ultrasonic.png)<br>

## 5. 다음주 목표
- 자이로센싱값 -> OLED 모니터링 로직 구현
- 스위치 눌림 시간 카운트(타이머 인터럽트 기반) -> OLED 모니터링 로직 구현


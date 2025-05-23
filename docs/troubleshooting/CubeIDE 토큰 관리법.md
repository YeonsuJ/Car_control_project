# Cube IDE 토큰 생성, 적용

### 환경 구축
Git과 STM32CubeIDE를 연동하여 프로젝트 버전 관리를 원활하게 하기 위해 아래의 가이드를 참고하여 환경을 구성  
➡ 참고 블로그: https://m.blog.naver.com/musk95/221900982612  
<br>

### 인증 토큰 생성

GitHub에서는 보안 강화를 위해 2021년부터 **패스워드 인증 방식 대신 Personal Access Token(PAT)**을 사용하도록 권장하고 있음.  
STM32CubeIDE에서 GitHub와 연동하려면, GitHub 계정의 패스워드 대신 이 토큰을 사용하여 인증을 수행.

➡ 토큰 생성 페이지: https://github.com/settings/tokens  
<br>

#### 토큰 생성 시 설정

- **Repository access**: All repositories 체크  
- **Repository permissions**: 전체 항목 체크 (읽기/쓰기 권한 필요)  
- **Account permissions**: 전체 항목 체크  
- 일반적으로 read & write 권한을 부여해야 하지만, **읽기 전용 용도일 경우 read만 체크**  
<br>

#### 생성 후 사용 방법

- 생성이 완료되면 토큰 문자열이 제공됨 → **복사해둘 것 (다시 볼 수 없음)**  
- STM32CubeIDE에서 Git 기능 사용 시 로그인 창이 뜨면,  
  - **Username**: GitHub ID  
  - **Password**: 방금 생성한 토큰 문자열을 붙여넣기  
<br>

이 과정을 통해 GitHub 저장소에 안전하게 접근하고, CubeIDE 내부에서 **Push, Pull, Clone 등의 Git 기능을 직접 사용할 수 있음.**

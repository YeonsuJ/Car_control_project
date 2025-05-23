# 🛠️ CubeIDE 연동 Git 작업공간 관리 가이드

개발 중인 Git 작업공간(원격 저장소 연동)에서  
환경구축, 토큰생성 및 다른 프로젝트를 임시로 열람하거나, 특정 커밋 상태로 안전하게 **덮어쓰기**하는 방법을 정리한 문서

## Cube IDE 환경 구축 및 토큰 생성, 적용

### 초기 환경 구축
Git과 STM32CubeIDE를 연동하여 프로젝트 버전 관리를 원활하게 하기 위해 아래의 가이드를 참고하여 환경을 구성  
➡ 참고 블로그: https://m.blog.naver.com/musk95/221900982612  
<br>


1. 로컬 c > 사용자 > jysjy > git 폴더 만들고 이동
~~~
git bash 환경 명령어 사이클
- git clone https://github.com/YeonsuJ/Car_control_project.git
- cd Car_control_project
- git remote -v 	// 확인용
- git fetch
- for branch in $(git branch -r | grep -v '\->'); do
  git branch --track "${branch#origin/}" "$branch"
done
- git checkout 원하는 브랜치
~~~
2. 파일 시스템 GUI를 통해 로컬 repo에서 .project 파일 실행 or CubeIDE에서 open 프로젝트 후 브라우징    
3. ide에서 환경 구축 및 토큰 설정

- git plugin 설치 (help > eclipse marketplace > egit 검색 > install > restart now클릭 후 cubeide 재시작

- window > show view > other > git repositories, staging, history, debugger console 추가


4. 프로젝트 새로 생성
- 새 프로젝트 생성 > 경로를 git/Car_control_project > Unit_car_sensor 폴더 생성후 여기로 > 프로젝트명도 Unit_car_sensor로 동일하게

- staging탭에서 아무것도 설정하지 않은 기본 프로젝트를 모든 파일을 staging > commit and push


주의)
!!! 삭제하기전에 git status로 staging 중인게 있는지 확인해야함 ( 삭제된 커밋까지 남기 때문에 지저분해짐) 
```

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

---

## 🔍 다른 프로젝트를 임시로 열람할 때

특정 커밋 상태의 프로젝트를 확인할 때 사용하는 절차

### 1️⃣ 현재 변경사항이 있는 경우

변경사항을 임시로 보관하여 작업공간을 clean 상태로 전환

```bash
git stash
```

### 2️⃣ 브랜치 생성 및 커밋 열람

원하는 커밋 해시로부터 브랜치를 생성하여 해당 상태로 이동

```bash
git checkout -b test_from_3f26e88 3f26e88
```

- `test_from_3f26e88`: 새 브랜치 이름 (임의 지정)
- `3f26e88`: 열람 대상 커밋 해시

---

### ✅ 열람 후 브랜치 정리

#### ✔️ 커밋 및 Push까지 완료한 경우

```bash
git checkout Unit_Controller
git branch -d test_from_3f26e88
git push origin --delete test_from_3f26e88
```

#### ✔️ 단순 열람만 한 경우

```bash
git checkout Unit_Controller
git branch -d test_from_3f26e88
```

---

### ♻️ stash 복원

```bash
git stash pop
```

또는 CubeIDE의 Apply Stashed Changes 기능 사용

---

## 💾 작업공간에 특정 분기 커밋 프로젝트 덮어쓰기

작업공간을 특정 커밋 상태의 프로젝트로 완전히 교체할 때 사용하는 절차

### 1️⃣ 현재 작업 파일 언스테이징

```bash
git reset HEAD
```

> 필요 시 `git stash` 또는 커밋으로 백업

---

### 2️⃣ 기존 프로젝트 삭제

파일 탐색기 또는 IDE에서 기존 프로젝트 폴더 삭제 또는 휴지통 이동

---

### 3️⃣ 특정 커밋 프로젝트 다운로드 및 덮어쓰기

1. GitHub에서 커밋 페이지 진입  
2. `Browse files` 클릭  
3. `Code > Download ZIP`  
4. 압축 해제 후 작업공간에 덮어쓰기

---

### 4️⃣ CubeIDE에서 프로젝트 Import

`.project` 파일 더블 클릭  

---

### 5️⃣ 변경사항 스테이징 및 커밋

```bash
git add .
git commit -m "특정 커밋 프로젝트로 덮어쓰기 완료"
```

---

## ✅ 요약

| 목적             | 작업 요약                                                    |
|------------------|---------------------------------------------------------------|
| 🔍 임시 열람     | `stash` → `checkout` → 열람 후 브랜치 삭제                    |
| ♻️ 덮어쓰기      | `reset` → 기존 프로젝트 삭제 → ZIP 다운로드 후 덮어쓰기        |

---

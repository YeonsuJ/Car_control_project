# git & github manuals

=============================================================================
## branch 별 기능 

### 1. main
최종 완성 때 develop에서 main으로 merge해줌
### 2. develop
docs, media, readme 등 관리, feature branch 코드 merge

---
==================================================================================
## 초기설정 

1. git 설치 (홈페이지에서 x64 download) <br/>
-> git --version

2. git 기본설정<br/>
	git config --global user.name "이름"<br/>
	git config --global user.email "이메일@example.com"<br/>
처음에 딱 한 번만, "내가 누군지" Git에 알려야 함, 여기 입력한 이름/이메일이 나중에 "커밋한 사람"으로 기록됨

3. 프로젝트 폴더 준비<br/>
ex) 문서 > car_control_project 폴더 생성

4. Git 초기화 (프로젝트 폴더 안에서 터미널(cmd) 열고)<br/>
-> git init<br/>
폴더가 Git이 추적할 수 있는 "저장소(repository)"가 됨.

5. 로컬 코드 GitHub에 연결<br/>
-> git remote add origin https://github.com/YeonSd/STM32_Car_Control_Project.git<br/>
(내 컴퓨터 코드와 GitHub 리포지터리가 연결됨)

6. 원격에서 내 로컬로 모든 브랜치 설정 전부 가져오기<br/>
```bash
   for branch in $(git branch -r | grep -v '\->'); do git branch --track "${branch#origin/}" "$branch" done
```

==================================================================================
## commands 

### 1. 매번 작업 시작 전<br/>
(다른 사람이 수정했을 수도 있으니까 먼저 가져오기)<br/>
-> git pull origin 브랜치명		

### 2. 작업 후
2-1. 원하는브랜치로 이동했는지 재차확인<br/>
-> git checkout 브랜치명<br/><br/>
2-2. 특정폴더만추가 ( git add . 하면 모든 폴더 추가됨 )<br/><br/>
-> git add 특정폴더이름 <br/><br/>
2-3. 커밋 & 메세지 입력 ex) Add RF communication"<br/>
-> git commit -m "message"<br/><br/>
2-4. 이동했던 브랜치로 push<br/>
-> git push origin 브랜치명		

2-5. upstream 옵션

처음 브랜치를 푸시할 땐 반드시 -u 옵션 사용하는 게 좋음
로컬 브랜치와 원격 브랜치를 추적 관계(upstream)로 설정하는거임<br/>
-> git push -u origin 브랜치명

이 명령을 한 번만 실행하면, 이후에는:

git push<br/>
git pull

만 입력해도 자동으로 추적관계가 설정됨

==================================================================================
### fork
남의 저장소를 복제해서 내 저장소로 만드는 기능
앞에서 말한 클론은 깃허브  저장소에 있는 내용을 내컴퓨터로 복사해오는거고, fork는 남의 깃허브 저장소를 복사해서 내 깃허브 계정에 저장소를 하나 새로 만드는 것이다.

### PR (Pull Request)
fork로 남의 저장소를 내 저장소로 복제한 후. 이 코드를 가지고 내가 이런저런 작업들을 진행한다.
근데 이렇게 내가 작업한 내용들이 내가 복사해온 원본 저장소에도 내용이 업데이트 되길 원할경우
해당 저장소로 pull request를 하고, 해당 저장소에서는 이 요청을 검토해서 승인되면 내가 작업한내용이 원본 저장소에도 업데이트가 된다.


### ※ 유의사항
명확한 커밋 메시지 달기
Pull 먼저, Push는 나중에 : 같이 하는 사람 코드가 최신일 수도 있음. 덮어쓰는 사고 방지
기능별로 커밋 자주하기 : 한 번에 너무 많은 걸 몰아서 커밋하지 말기.


==================================================================================
## 원격(깃헙페이지)에서 수정하고 로컬(pc 디렉토리)로 동기화하는 법

1. 깃헙페이지에서 연필모양으로 내용 수정
2. 수정했던 브랜치로 이동 (같은 브랜치에 있어야 pull 시 충돌 없이 동기화됨)<br/>
-> git checkout 브랜치명
3. 원격(origin)의 develop 브랜치 변경사항을 가져오고, 현재 로컬 브랜치에 병합<br/>
-> git pull origin 브랜치명		
<br/><br/>
결과 -> 수정 내용이 로컬에도 반영되며, .git 폴더 내 히스토리도 최신 상태로 동기화됨

-> + 만약 로컬 파일을 동시에 수정 중이었다면?<br/>
-> git pull 도중 merge conflict(병합 충돌) 발생가능 <br/><br/>
-> 이 경우 git이 충돌 부분을 표시하고, 수동으로 해결해야함

## ! 충돌 발생 시 해결 법 
-> 원격보다 로컬이 뒤쳐진 경우<br/>
ex) develop의 경우

1. git fetch origin
2. git merge origin/develop
3. git log HEAD..origin/develop --online
4. git reset --hard origin/develop
5. git pull

==================================================================================

## 새로운 Repository 생성시

1. 해당 디렉토리 내부에서 터미널열고<br/>
-> git init
2. 디렉토리 내부에 폴더나 파일 넣고<br/>
-> git add 폴더명(파일명은 확장자까지)
3. 커밋<br/>
-> git commit -m "message"
4. main 브랜치 생성<br/>
-> git branch -M main
5. 원격과 연동<br/>
-> git remote add origin https://github.com/YeonsuJ/리포지토리명.git
 6. 로컬에서 원격으로 push<br/>
 -> git push -u origin main

==================================================================================

## 로컬에서 md파일 수정하고 원격으로 push

ex)develop 브랜치의 docs/dev_log/weekly_dev_log.md을 수정했다면

1. md파일이 있는 브랜치로 이동<br/>
-> git checkout develop
2. 로컬 내의 디렉토리에서 md파일수정
3. modified: docs/dev_log/weekly_dev_log.md 뜨는지 확인<br/>
-> git status
4. 파일 경로로 add<br/>
-> git add docs/dev_log/weekly_dev_log.md
5. 커밋<br/>
-> git commit -m "Fix : Update weekly_dev_log"
6. 푸시<br/>
-> git push origin develop

==================================================================================

## 로컬에서 branch 삭제 후 원격에 반영

ex) feature/button 브랜치를 삭제하려면

1. 삭제하는 브랜치 말고 다른 브랜치로 이동<br/>
-> git checkout 다른 브랜치
2. 로컬 브랜치 삭제<br/>
-> git branch -d feature/button
3. 원격 브랜치 삭제<br/>
-> git push origin --delete feature/button
4. 확인<br/>
-> git branch	// 로컬 브랜치 목록<br/>
   git branch -r	// 원격 브랜치 목록<br/>
   git branch -a     // 전체 브랜치 목록<br/>

==================================================================================
## 기타 

원하는 브랜치로 이동<br/>
-> git checkout 기존브랜치이름

새로운 branch 추가 (!!! 상위 develop branch에서 생성해야 종속관계가됨 !!!)<br/>
-> git checkout -b 생성할이름

현재 브랜치와 변경 상태 확인<br/>
-> git status					

최근 커밋 내역 확인<br/>
-> git log --online



	



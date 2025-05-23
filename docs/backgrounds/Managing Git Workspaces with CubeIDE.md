# 🛠️ CubeIDE 연동 Git 작업공간 관리 가이드

개발 중인 Git 작업공간(원격 저장소 연동)에서  
다른 프로젝트를 임시로 열람하거나, 특정 커밋 상태로 안전하게 **덮어쓰기**하는 방법을 정리한 문서

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

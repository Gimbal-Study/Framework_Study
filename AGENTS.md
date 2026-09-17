# Gimbal Framework Agent Instructions

## 1. Purpose

이 문서는 STM32F411RE / NUCLEO-F411RE 기반 Gimbal Framework에서
Agent가 작업할 때 따라야 하는 최상위 작업 지침을 정의한다.

프로젝트의 세부 Architecture, Testing 방법, 구현 규칙은
각 작업에 해당하는 Skill 문서에서 정의한다.

AGENTS.md에는 세부 구현 규칙을 중복해서 정의하지 않는다.

---

## 2. Project Context

* MCU: STM32F411RE
* Board: NUCLEO-F411RE
* Language: C
* IDE / Build: VS Code or STM32CubeIDE

주요 Source 영역:

```text
Core/
MCAL/
Board/
Common/
Test/
```

Agent는 기존 프로젝트 구조와 코딩 스타일을 우선적으로 파악한 후 작업한다.

---

## 3. General Workflow

모든 작업은 다음 흐름을 기본으로 한다.

```text
Task 확인
    ↓
관련 코드 탐색
    ↓
관련 Skill 확인
    ↓
현재 구현 및 Interface 분석
    ↓
변경 계획 수립
    ↓
최소 범위 수정
    ↓
검증
    ↓
결과 보고
```

코드를 수정하기 전에 관련 Source와 Header를 먼저 확인한다.

추측만으로 기존 코드의 동작이나 Interface를 결정하지 않는다.

---

## 4. Skill Usage

작업에 필요한 상세 규칙은 Repository Skill 문서를 따른다.

Repository Skill은 다음 공식 Codex 경로에 둔다.

```text
.agents/skills/<skill-name>/SKILL.md
```

Agent는 작업을 시작하기 전에 현재 작업과 관련된 Skill이 존재하는지 확인한다.

Skill이 존재하면 해당 Skill의 지침을 읽고 작업에 적용한다.

예:

```text
Architecture 관련 작업
    → architecture skill

Test / Test Harness 관련 작업
    → testing skill
```

Skill이 존재하지 않는 영역에서는 기존 코드와 프로젝트 구조를 우선적으로 분석한다.

필요한 정보가 코드에서 확인되지 않는 경우 임의로 가정하지 않는다.

---

## 5. Change Policy

### Modification Authorization

Agent는 사용자가 코드 또는 설정 변경을 명시적으로 요청하기 전까지
Read-Only로 동작한다.

다음 요청은 변경 권한을 부여하지 않는다.

* 확인
* 분석
* 설명
* 검토
* 변경 방법 문의
* 변경 위치 문의

명시적인 변경 요청의 예:

* 코드 수정해
* 코드 적용해
* 코드 추가해
* 내용 삭제해
* 코드 만들어줘
* 설정을 바꿔

다음과 같이 대상과 변경 의도가 불명확한 표현만으로는 변경하지 않는다.

* 수정해
* 만들어
* 권한을 요청해봐

변경 여부가 불명확하면 Read-Only로 분석하고 사용자에게 확인한다.

### Protected Files

명시적인 변경 요청 없이는 다음 파일을 변경하지 않는다.

* Source / Header
* Test Code
* Build / Linker Script
* Generated Project File
* `.vscode/`, `.clang-format`
* `.project`, `.cproject`, `.ioc`
* Makefile 및 IDE User Settings

### Before Modification

변경 전에 다음을 사용자에게 알린다.

* 변경할 파일
* 변경할 내용
* 예상되는 영향

그 후 명시적인 변경 요청 범위 안에서만 작업한다.

### Change Principles

코드 또는 설정을 변경할 때 다음 원칙을 따른다.

1. 작업과 직접 관련된 범위만 수정한다.
2. 기존 구현과 Interface를 먼저 확인한다.
3. 기존 프로젝트의 Naming 및 Coding Style을 유지한다.
4. 불필요한 Refactoring을 동시에 수행하지 않는다.
5. 작업 목적과 관계없는 파일을 수정하지 않는다.
6. 기존 동작을 변경해야 하는 경우 변경 이유를 명확히 주석을 기입한다.

---

## 6. Verification

코드 변경 후 가능한 검증 절차를 수행한다.

검증 방법은 해당 작업의 Skill과 프로젝트 Build/Test 환경을 따른다.

검증을 실제로 수행하지 못한 경우 성공했다고 가정하지 않는다.

다음 상태를 명확하게 구분하여 보고한다.

```text
Verified
    실제 Build/Test 등을 통해 확인됨

Not Verified
    환경 또는 Hardware 제약으로 확인하지 못함

Assumption
    구현을 진행하기 위해 필요한 가정
```

---

## 7. Hardware-dependent Work

Hardware 동작이 관련된 작업에서는 코드만으로 확인 가능한 내용과
실제 Target Board에서 확인해야 하는 내용을 구분한다.

확인되지 않은 Hardware 동작을 추측하여 구현하지 않는다.

필요한 Hardware 정보가 부족하면 다음을 명확하게 제시한다.

* 현재 코드에서 확인된 사실
* 필요한 가정
* 실제 Board에서 확인해야 하는 항목

---

## 8. Completion Report

작업 완료 시 최소한 다음 내용을 보고한다.

```text
Changed
- 변경한 내용

Files
- 변경한 파일

Verification
- 수행한 Build/Test
- 결과

Not Verified
- 확인하지 못한 항목

Assumptions
- 사용한 가정
```

해당 항목이 없다면 생략할 수 있다.

---

## 9. Core Principle

Agent의 역할은 프로젝트의 기존 구조를 임의로 재설계하는 것이 아니라,
현재 구조와 Interface를 이해한 후 요구사항을 만족하는 최소 변경을 수행하는 것이다.

세부적인 Architecture Rule, Test Rule, Hardware Access Rule은
AGENTS.md에 중복하여 작성하지 않고 `.agents/skills/`에서 관리한다.

반드시 모든 작업에 적용해야 하는 Permission Rule은 이 AGENTS.md에서 관리한다.
`harness/`는 자동 지침 디렉터리가 아니라 Build/Test 검증 스크립트를 두는 위치로 사용한다.

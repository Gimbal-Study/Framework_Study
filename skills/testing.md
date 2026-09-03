# Testing Skill

## Purpose

Gimbal Framework의 Embedded Test Harness와 Test Case 작성 규칙을 정의한다.

Test 추가·수정·실행·검토 시 이 문서를 따른다.

## Test Harness Goals

Test Harness는 다음 기능을 제공한다.

- 공통 Test Runner
- 통일된 Test Function 형식
- Test Name + Function Pointer 기반 관리
- PASS / FAIL / SKIP 결과
- 전체 Test 실행
- 특정 Test 선택 실행
- 실행/성공/실패/Skip 수 집계
- 최소 Assertion
- 실패 Test Name 기록
- 실패 File / Line 기록
- 정적 메모리 기반 동작

## Test Result

최소 다음 결과를 사용한다.

```c
typedef enum
{
    TEST_RESULT_PASS = 0,
    TEST_RESULT_FAIL,
    TEST_RESULT_SKIP
} test_result_t;
```

- `PASS`: 정의된 기대 조건을 만족했다.
- `FAIL`: 정의된 기대 조건을 만족하지 못했다.
- `SKIP`: 현재 환경 또는 Hardware 조건 때문에 검증할 수 없다.

검증하지 못한 결과를 PASS로 처리하지 않는다.

## Test Function

모든 자동 Test는 동일한 형태를 사용하고 실행 후 종료한다.

```c
typedef test_result_t (*test_function_t)(void);
```

권장 Test 구조:

```text
Arrange
  ↓
Act
  ↓
Assert
  ↓
PASS / FAIL / SKIP
  ↓
Return
```

무한 루프 기반 Hardware Toggle 코드는 자동 Test Case로 사용하지 않는다.

## Test Case

Test는 이름과 Function Pointer를 갖는 정적 구조체로 관리한다.

```c
typedef struct
{
    const char *name;
    test_function_t function;
} test_case_t;
```

동적 메모리를 사용한 Test 등록은 하지 않는다.

## Assertions

최소 다음 Assertion을 제공한다.

```text
EXPECT_TRUE(condition)
EXPECT_FALSE(condition)
EXPECT_EQ(expected, actual)
```

Assertion 실패 시 가능한 한 다음 정보를 기록한다.

- 현재 Test Name
- Source File
- Source Line

Assertion 실패는 현재 Test를 `TEST_RESULT_FAIL`로 종료시키고,
Runner는 다른 Test를 계속 실행할 수 있어야 한다.

## Test Runner

Test Runner의 책임:

- Test 목록 관리
- Function Pointer 호출
- 전체 Test 실행
- 특정 Test 선택 실행
- PASS / FAIL / SKIP 집계
- 실패 정보 기록

Runner는 GPIO, UART 등 개별 Peripheral의 Test Logic을 직접 구현하지 않는다.

## Test Summary

최소 다음 정보를 관리한다.

```text
total
passed
failed
skipped
```

초기 구현에서는 최초 실패 Test/File/Line만 기록해도 된다.

추가 기록이 필요하면 고정 크기 정적 배열을 우선 사용한다.

## GPIO Tests

현재 검증 대상 GPIO Public API:

```text
mcal_gpio_init()
mcal_gpio_high()
mcal_gpio_low()
mcal_gpio_read()
```

Target 구현:

```text
MCAL/Target/mcal_gpio.c
```

Pin Definition:

```text
Board/NUCLEO_F411RE/pinmap_config.h
```

GPIO Test에서는 정상 입력, 경계값, 잘못된 입력을 검토한다.

단, 경계값과 잘못된 입력의 기대 동작은 기존 Header/Source/API Contract에서 확인된 경우에만 Assertion으로 만든다.

예를 들어 Invalid Channel에 대한 동작이 정의되어 있지 않다면 다음을 임의로 가정하지 않는다.

```text
Error 반환
무시
Assert
특정 기본값 반환
```

Test를 위해 기존 Public API의 Return Type이나 의미를 변경하지 않는다.

## Hardware-dependent Tests

다음 두 종류를 구분한다.

### Automatically Verifiable

기존 Public Interface만으로 결과를 확인할 수 있고 그 동작이 명확하게 정의된 경우.

### Board Verification Required

실제 Target Board 또는 외부 회로 확인이 필요한 경우.

예:

- LED 실제 점등
- 외부 Pin 전압
- 외부 Pull-up/Pull-down
- Output Pin Read-back이 MCU/MCAL 계약상 보장되는지 불명확한 경우

확인되지 않은 Hardware 동작을 추측해서 Assertion으로 만들지 않는다.

필요하면 SKIP으로 처리하고 Board에서 확인해야 하는 항목으로 보고한다.

## Result Reporting

UART 또는 LED를 이용한 결과 출력이 필요하면 해당 MCAL Public Interface를 사용한다.

Test에서 HAL/CMSIS/Register를 직접 사용하여 Reporter를 만들지 않는다.

예상 결과 형식:

```text
[PASS] gpio_output_high
[PASS] gpio_output_low
[FAIL] gpio_invalid_channel
[SKIP] gpio_external_input

Total   : 4
Passed  : 2
Failed  : 1
Skipped : 1
```

## Adding a Test

새 Test 추가 순서:

1. 검증할 Public Interface를 확인한다.
2. Header와 구현에서 API Contract를 확인한다.
3. 기존 Test Pattern을 확인한다.
4. 정상 입력 Test를 작성한다.
5. 정의된 경우 경계값 Test를 작성한다.
6. 정의된 경우 Invalid Input Test를 작성한다.
7. Test Case 목록에 등록한다.
8. Build한다.
9. 관련 Test를 실행한다.
10. 결과를 확인한다.

## Failure Handling

Test가 실패하면 다음을 구분한다.

```text
Implementation Bug
Test Bug
Undefined Contract
Hardware Verification Required
```

다음 방식으로 실패를 숨기지 않는다.

- Test 삭제
- Assertion 삭제
- 기대 조건 완화
- 기대값 임의 변경
- FAIL을 PASS로 변경

## Completion Check

Test 관련 작업 후 확인한다.

- Test가 종료되는가?
- PASS / FAIL / SKIP을 반환하는가?
- Runner에서 실행 가능한가?
- Assertion 실패 위치를 추적할 수 있는가?
- Test에서 Register에 직접 접근하지 않는가?
- Hardware 접근이 MCAL을 통하는가?
- Build가 성공하는가?
- 실제 실행한 Test 결과와 미검증 항목이 구분되어 있는가?

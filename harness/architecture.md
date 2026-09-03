# Architecture Skill

## Purpose

Gimbal Framework의 저장소 구조, 계층별 책임, 의존 관계와 코드 배치 기준을 정의한다.

Architecture 관련 코드 추가·수정·검토 시 이 문서를 따른다.

## Project Layers

```text
Core/
MCAL/
Board/
Common/
Test/
```

### Core

Firmware의 진입점과 상위 실행 흐름을 담당한다.

- 시스템 초기화와 실행 흐름을 관리한다.
- Test Mode에서는 Test Runner의 공개 진입점만 호출한다.
- 개별 Test Case의 세부 구현을 포함하지 않는다.

### MCAL

STM32F411RE의 MCU Peripheral을 추상화한다.

예:

- GPIO
- UART
- Timer
- PWM
- SPI
- I2C
- DMA

MCU Register에 대한 직접 접근이 필요한 Hardware 제어는 MCAL 구현 내부에 둔다.

기존 MCAL Public Interface는 명시적인 요구 없이 변경하지 않는다.

### Board

NUCLEO-F411RE의 Board-specific Hardware Configuration을 담당한다.

예:

- Pin Mapping
- Peripheral과 물리 Pin 연결
- Board-specific Configuration

현재 GPIO Pin 정의는 `Board/NUCLEO_F411RE/pinmap_config.h`에 존재한다.

예:

```text
GPIO_A_05
    ↓
105
    ↓
PA5
```

### Common

여러 계층에서 재사용 가능한 Hardware-independent 공용 요소를 담당한다.

예:

- 공용 Type
- Macro
- Error Code
- 범용 Utility

특정 Peripheral 또는 Test에만 필요한 기능을 Common에 배치하지 않는다.

### Test

Test Harness와 Test Case를 담당한다.

- Application Logic을 구현하지 않는다.
- 기존 Public Interface를 검증한다.
- Hardware 접근이 필요한 경우 MCAL Public Interface를 사용한다.

## Dependency Rules

허용되는 기본 흐름:

```text
Core
  ↓
Test Runner
  ↓
Test Case
  ↓
MCAL Public API
  ↓
MCAL Target Implementation
  ↓
Hardware
```

일반 Firmware 코드에서는 상위 계층이 필요한 MCAL Public API를 사용한다.

금지되는 의존성:

```text
Test   ─X→ MCU Register 직접 접근
Test   ─X→ CMSIS Register API 직접 사용
Core   ─X→ 개별 Test Case 구현
MCAL   ─X→ Test
Board  ─X→ Test
Common ─X→ Test-specific implementation
```

Production Code가 Test Code에 의존하도록 만들지 않는다.

## Hardware Access Boundary

Hardware 접근은 MCAL을 경계로 한다.

허용:

```c
mcal_gpio_high(GPIO_A_05);
```

Test 또는 Core에서 다음과 같은 직접 Register Access를 추가하지 않는다.

```c
GPIOA->BSRR = (1UL << 5);
```

새로운 Hardware 접근이 필요하다면 해당 기능이 MCAL 책임인지 판단한 후 MCAL에 구현한다.

## File Placement

새 코드는 책임을 기준으로 배치한다.

```text
Firmware 실행 흐름
    → Core/

MCU Peripheral 제어
    → MCAL/

Board Pin/Hardware Configuration
    → Board/

범용 Utility
    → Common/

Test Runner / Assertion / Test Case
    → Test/
```

코드가 무엇을 사용하는지가 아니라 무엇을 책임지는지를 기준으로 위치를 결정한다.

## Public Interface

기존 Public Interface는 요구 없이 변경하지 않는다.

현재 GPIO Public API:

```text
mcal_gpio_init()
mcal_gpio_high()
mcal_gpio_low()
mcal_gpio_read()
```

Test 편의를 위해 Return Type, Parameter 또는 의미를 임의로 변경하지 않는다.

## Architecture Check

Architecture 관련 변경 후 다음을 확인한다.

- 코드가 올바른 계층에 위치하는가?
- 새로운 역방향 Dependency가 생기지 않았는가?
- Test-specific 코드가 Production 계층에 들어가지 않았는가?
- Hardware Register 접근이 MCAL 밖으로 노출되지 않았는가?
- 기존 Public Interface를 불필요하게 변경하지 않았는가?

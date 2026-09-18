# MPU6050 센서 드라이버

사용자가 지정한 `Devie/Sensor` 경로입니다. `MPU6050.c`, `MPU6050.h`는
현재 프로젝트의 `MCAL/mcal_i2c.h` 공개 함수 선언에 맞췄습니다.
요청의 `MPU650.h`는 참고 저장소의 `MPU6050.h`로 해석했습니다.

참고: https://github.com/jrowberg/i2cdevlib/tree/master/STM32/MPU6050

원본의 초기화/WHO_AM_I 확인/14바이트 burst read 방식을 참고했습니다.
원본 API 전체를 복제하지 않았으며, STM32 HAL과 `I2Cdev` 의존성 대신
프로젝트 MCAL을 호출합니다. 저작권 및 MIT 고지는 `LICENSE`에 있습니다.
스케일은 첨부한 InvenSense 레지스터 맵 revision 4.2의 p.29, p.31을 기준으로 합니다.

## 제공 기능

| API | 역할 |
|---|---|
| `MPU6050_DefaultConfig` | 채널/주소/범위/필터/출력 주기 기본값 |
| `MPU6050_Init` | 연결 확인, reset, 절전 해제, PLL, 범위/필터/주기 설정 및 read-back 검증 |
| `MPU6050_TestConnection` | 초기화된 장치의 WHO_AM_I 재확인 |
| `MPU6050_DataReady` | INT_STATUS의 새로운 데이터 여부 확인 및 플래그 해제 |
| `MPU6050_ReadRaw` | 가속도/온도/자이로 14바이트 일괄 읽기, signed 16-bit 변환 |
| `MPU6050_Read` | 가속도 g, 각속도 degrees/s, 칩 온도 Celsius 변환 |
| `MPU6050_CalibrateGyro` | 정지 상태에서 새로운 샘플들의 평균으로 소프트웨어 자이로 bias 계산 |
| `MPU6050_ClearGyroBias` | 자이로 소프트웨어 보정값 0으로 초기화 |

기본값: I2C 채널 1, 7비트 주소 `0x68`, 가속도 ±2g, 자이로 ±500°/s,
DLPF=3(자이로 42 Hz/가속도 44 Hz), 분주값 4(200 Hz), I2C timeout 20 ms.
짐벌의 첫 데이터 수집용 시작값이며 최적 제어 튜닝값을 의미하지는 않습니다.
필터 지연은 자이로 약 4.8 ms이므로 제어 응답을 보며 조정하세요.

장치 주소는 **시프트하지 않습니다**. 현재 MCAL 구현 내부에서 `address << 1`을 합니다.
AD0 HIGH이면 config.address만 `0x69`로 설정합니다. WHO_AM_I는 두 주소에서 모두 `0x68`입니다.
원본의 `0x34` 비교는 WHO_AM_I의 bit 6:1만 추출한 결과라는 차이가 있습니다.

## 먼저 해결해야 할 기존 MCAL 통합 문제

센서 파일은 실제 `mcal_i2c.h`로 컴파일 검증했지만, 현재 저장소의 **MCAL 구현 자체는
컴파일 및 하드웨어 초기화가 완료되지 않은 상태**입니다. 기존 MCAL/메인/Makefile은 수정하지 않았습니다.

1. `MCAL/Target/STM32F4xx/mcal_i2c.c`의 `mcal_i2c_init()`에 정의되지 않은 `uint32`가
   사용됩니다. 공개 헤더와 동일한 `uint32_t`가 필요합니다.
2. `mcal_i2c_read()`의 공개 헤더에는 `mem_addr_size`가 없지만 구현에는 있습니다.
   이 드라이버는 **현재 공개 헤더**를 호출합니다. 헤더를 유지할 경우 구현을
   아래 서명으로 맞추고 함수 내부에서 `const uint8_t mem_addr_size = 1U;`로
   고정하는 방법이 있습니다. 16비트 주소가 필요한 다른 장치를 위해서는 별도 확장 API가 적합합니다.

   ```c
   mcal_i2c_status_t mcal_i2c_read(uint8_t channel, uint16_t dev_addr,
       uint16_t reg_addr, uint8_t *data, uint16_t len, uint32_t timeout);
   ```

3. I2C 초기화의 GPIO MODER 쓰기에 여분의 닫는 괄호가 있고,
   PB7이 Alternate Function의 `10b`가 아닌 `11b`로 설정되어 있습니다.
   CCR/TRISE/PE 설정, 성공 반환, 채널 및 mode 처리도 아직 완성되지 않았습니다.
   현재 구현이 핀을 설정하는 대상은 I2C1(PB6/PB7)뿐입니다.
4. MCAL read/write가 호출하는 `mcal_time_ms()`의 구현이 없습니다.
   대기 중에도 증가하는 단조로운 millisecond timebase가 필요합니다.
   기존 `delay_ms()`는 SysTick을 시작/정지하는 방식이므로 이를 그대로
   상시 millisecond tick으로 간주할 수 없습니다. MPU delay 콜백과 MCAL
   timeout timebase가 서로의 타이머를 중지/재설정하지 않도록 구성해야 합니다.
5. 현재 루트 Makefile은 UART 테스트를 빌드하며 I2C/MPU 소스가 포함되어 있지 않습니다.
   I2C와 시간 기반 구현을 완성한 뒤 `MCAL/Target/STM32F4xx/mcal_i2c.c`,
   `Devie/Sensor/MPU6050.c`, timebase 소스를 빌드에 포함하고
   `Devie/Sensor`, `MCAL`, `Common` 등의 include 경로를 설정하세요.

이 문제들을 해결하기 전에는 센서 드라이버만 추가해도 보드에서 바로 실행되지 않습니다.

## 사용 예시 (MCAL/보드 초기화 완료 후)

아래 `app_delay_ms()`는 보드에서 구현할 실제 지연 함수입니다.
센서 드라이버는 GPIO/클록/I2C 초기화나 ISR을 소유하지 않습니다.

```c
#include "MPU6050.h"

extern void app_delay_ms(uint32_t ms);

static MPU6050_Device imu = {0};

MPU6050_Status sensor_start(void)
{
    MPU6050_Config config;
    MPU6050_Status status;

    MPU6050_DefaultConfig(&config);
    config.delay_ms = app_delay_ms;

    /* 여기 오기 전에 보드 전원, GPIO, I2C1, timeout용 tick을 초기화한다. */
    status = MPU6050_Init(&imu, &config);
    if (status != MPU6050_OK)
        return status;

    /* 모터 OFF, 센서 정지 상태. 기본 200 Hz에서 약 2.5초 이상 소요. */
    return MPU6050_CalibrateGyro(&imu, 500U);
}

/* 주기적 메인 루프에서 호출. 출력은 성공 + fresh=true일 때만 사용한다. */
MPU6050_Status sensor_poll(MPU6050_Data *sample, bool *fresh)
{
    bool ready;
    MPU6050_Status status;
    if (sample == 0 || fresh == 0)
        return MPU6050_ERROR_ARGUMENT;
    *fresh = false;
    status = MPU6050_DataReady(&imu, &ready);
    if (status != MPU6050_OK || !ready)
        return status;
    status = MPU6050_Read(&imu, sample);
    if (status == MPU6050_OK)
        *fresh = true;
    return status;
}
```

설정 변경은 `MPU6050_Config` 수정 후 `MPU6050_Init()` 재호출로 합니다.
재초기화는 reset과 대기, bias 초기화를 포함하므로 제어 중간에 호출하지 마세요.
device 구조체는 0으로 초기화하고 초기화 이후 필드를 직접 변경하지 마세요.

## 짐벌 제어에 연결할 때

- 반환 상태가 OK일 때만 측정값을 사용하세요. 통신 실패 시 출력 구조체는 그대로 유지되므로
  반환 상태를 무시하면 과거 데이터를 새 데이터로 오인할 수 있습니다.
- `ReadRaw`/`Read`는 최신 레지스터를 읽습니다. 자체적으로 새 샘플을 기다리지 않습니다.
  `DataReady` 또는 별도로 구성한 데이터 준비 인터럽트를 사용하세요. 출력 레지스터 방식은
  폴링이 늦으면 샘플을 놓칩니다. 정확한 샘플 개수 보장이 필요하면 이후 FIFO를 추가하세요.
- `DataReady`는 읽으면서 인터럽트 상태를 지웁니다. 다른 태스크가 상태를 동시에 읽지 않게 하세요.
- 모든 API는 blocking입니다. 제어 ISR에서 호출하지 말고 같은 버스 접근을 직렬화하세요.
- 자이로 보정 중에는 실제 회전이 없어야 합니다. 자동 움직임 감지는 하지 않으며 움직임도
  bias로 평균됩니다. 실패하면 이전 bias를 유지합니다. 보정은 가속도 중력을 제거하지 않습니다.
- 보정 샘플별 준비 대기는 `timeout_ms`회의 1 ms 지연과 각 I2C 호출 시간으로 제한됩니다.
  전체 보정의 wall-clock timeout이 아닙니다. 느린 출력 주기에서는 timeout도 늘리세요.
- 출력은 센서 XYZ 기준입니다. 장착 방향에 따른 축 교환/부호 변경, timestamp와 실제 dt,
  자세 추정(상보 필터 등), PID, 모터 출력은 상위 계층에서 처리하세요.
  자이로 각속도는 각도가 아니며, MPU6050만으로 절대 yaw를 안정적으로 얻을 수는 없습니다.
- DMP/FIFO/외부 자력계 제어는 현재 범위에 포함하지 않았습니다.

## 검증

프로젝트 루트 PowerShell에서:

```powershell
& ./Testbench/mpu6050/test.ps1
```

호스트 GCC로 실제 공개 MCAL 헤더와 모의 버스를 링크하여 초기화 단계별 I2C 실패,
설정 read-back 불일치, 주소 0x68/0x69, signed 극값, 모든 범위의 단위 변환,
부분 수신 실패 시 출력 보존, BUSY/TIMEOUT 전달, 보정 성공/실패를 검사합니다.
ARM GCC로 Cortex-M4/FPU 대상 센서 소스도 `-Wall -Wextra -Werror -pedantic` 컴파일합니다.
**MCAL 하드웨어 구현을 링크하는 전체 펌웨어 테스트나 실제 보드 통신 테스트는 아닙니다.**
실보드에서는 WHO_AM_I, 설정 read-back, 정지 시 가속도 크기 약 1g, 보정 후 자이로 약 0°/s,
케이블 분리 시 오류/timeout을 순서대로 확인하세요.

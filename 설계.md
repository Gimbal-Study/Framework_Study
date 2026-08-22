# Zimbal-Project
짐벌 제어 프로젝트

1. 프로젝트 전체 구조
GimbalDoorFramework/
│
├── Board/                              # [HW] 실제 보드에 종속
│   └── NUCLEO_F411RE/
│       ├── board_init.c                # 보드 초기화
│       ├── board_init.h
│       ├── pinmap_config.h             # 실제 핀 연결
│       ├── startup_stm32f411xe.s       # Reset → main 실행 준비
│       └── STM32F411RE_FLASH.ld        # Flash/RAM 메모리 배치
│
├── CMSIS/                              # [MCU] ARM/ST 저수준 정의
│   ├── Core/
│   └── Device/
│       └── STM32F4xx/
│
├── Core/                               # [System] 프로그램 진입점
│   ├── System/
│   │   ├── system.c
│   │   ├── system.h
│   │   ├── syscalls.c                 # C Library → 시스템 호출 연결
│   │   └── sysmem.c                   # malloc 등 Heap 관련 연결
│   └── main.c
│
├── Config/                             # [CONFIG] 소프트웨어 설정
│   ├── system_config.h                 # Gimbal / Door 선택
│   └── control_config.h                # PID / Filter 파라미터
│
├── MCAL/                               # [MCU HAL] MCU Peripheral 추상화
│   ├── mcal_gpio.h
│   ├── mcal_uart.h
│   ├── mcal_i2c.h
│   ├── mcal_spi.h
│   ├── mcal_pwm.h
│   └── Target/
│       └── STM32F4xx/
│           ├── mcal_gpio.c
│           ├── mcal_uart.c
│           ├── mcal_i2c.c
│           ├── mcal_spi.c
│           └── mcal_pwm.c
│
├── Device/                             # [Device] 외부 소자 Driver
│   ├── Actuator/
│   │   ├── Motor_BLDC/
│   │   ├── Motor_DC/
│   │   └── SolenoidLock/
│   └── Sensor/
│       ├── IMU_MPU6050/
│       ├── Encoder/
│       ├── Ultrasonic/
│       ├── PIR/
│       └── LimitSwitch/
│
├── Framework/                          # [CORE LOGIC] 재사용 핵심
│   ├── Scheduler/
│   ├── StateMachine/
│   ├── Math/
│   ├── Control/
│   │   ├── PID/
│   │   └── Filter/
│   ├── SensorProcessing/
│   │   └── Fusion/
│   ├── Safety/
│   ├── Communication/
│   └── Logger/
│
├── Common/                             # [COMMON] 최소 공통 요소
│   ├── def.h
│   ├── error_code.h
│   └── ring_buffer.h
│
├── Application/                        # [PRODUCT] 제품별 로직
│   ├── Gimbal/
│   │   ├── gimbal_app.c
│   │   └── gimbal_app.h
│   └── AutomaticDoor/
│       ├── door_app.c
│       └── door_app.h
│
├── Test/                               # [TEST] PC/Target 테스트
│   ├── Unit/
│   ├── Integration/
│   └── Mocks/
│
├── docs/                               # [DOC] 설계 문서
├── CMakeLists.txt                      # 빌드
├── .gitignore
└── README.md
2. 폴더를 역할별로 다시 묶으면
폴더를 전부 같은 레벨에서 바라보면 복잡해 보인다.

실제로는 다음 5개의 큰 영역으로 생각하면 쉽다.

┌──────────────────────────────────────────────────────────┐
│                    PRODUCT SOFTWARE                      │
│                                                          │
│  Application                                             │
│      ↓                                                   │
│  Framework                                                │
│                                                          │
├──────────────────────────────────────────────────────────┤
│                    HARDWARE ABSTRACTION                   │
│                                                          │
│  Device                                                   │
│      ↓                                                   │
│  MCAL                                                     │
│      ↓                                                   │
│  CMSIS                                                    │
│                                                          │
├──────────────────────────────────────────────────────────┤
│                    PLATFORM / BOARD                      │
│                                                          │
│  Board / Core / Config / Common                           │
│                                                          │
├──────────────────────────────────────────────────────────┤
│                       TEST                               │
│                                                          │
│  Unit / Integration / Mocks                               │
└──────────────────────────────────────────────────────────┘
3. 의존성 방향
3.1 정상적인 방향
┌─────────────────┐
│   Application   │
└────────┬────────┘
         │ uses
         ▼
┌─────────────────┐
│   Framework     │
└────────┬────────┘
         │
         ├───────────────────┐
         │                   │
         ▼                   ▼
┌─────────────────┐   ┌─────────────────┐
│     Device      │   │ Hardware-free   │
│  IMU / Motor    │   │ Algorithms      │
└────────┬────────┘   └─────────────────┘
         │
         ▼
┌─────────────────┐
│      MCAL       │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│     CMSIS       │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│     STM32       │
└─────────────────┘
3.2 절대로 올라가면 안 되는 의존
Framework
    │
    └──────X──────→ STM32 Register


Device
    │
    └──────X──────→ STM32 Register


Application
    │
    └──────X──────→ TIM1->CCR1


Application
    │
    └──────X──────→ HAL_GPIO_WritePin()


Framework
    │
    └──────X──────→ HAL_I2C_Master_Transmit()
상위 계층에서 아래 계층의 구현 방식을 알아버리면 추상화가 깨진다.

4. Board vs MCU vs CMSIS vs MCAL
이 부분은 가장 헷갈리기 쉬우므로 별도로 정리한다.

             ┌────────────────────────────┐
             │           BOARD            │
             │                            │
             │ NUCLEO-F411RE              │
             │ PA5 = LED                  │
             │ PB8 = I2C1_SCL             │
             │ PB9 = I2C1_SDA             │
             └─────────────┬──────────────┘
                           │
                           ▼
             ┌────────────────────────────┐
             │            MCU             │
             │                            │
             │ STM32F411RE                │
             │ GPIO / I2C / TIM / UART    │
             └─────────────┬──────────────┘
                           │
                           ▼
             ┌────────────────────────────┐
             │           CMSIS            │
             │                            │
             │ Register / Core Definition │
             └─────────────┬──────────────┘
                           │
                           ▼
             ┌────────────────────────────┐
             │           MCAL             │
             │                            │
             │ mcal_gpio_*()              │
             │ mcal_i2c_*()               │
             │ mcal_pwm_*()               │
             └────────────────────────────┘
쉽게 기억하기

Board = 어디에 연결되어 있는가?
MCU   = 무엇으로 제어하는가?
CMSIS = Register를 어떻게 표현하는가?
MCAL  = Peripheral을 어떤 API로 제공할 것인가?
5. ST HAL/LL을 어디에 놓는가?
이 프로젝트에서는 ST HAL/LL을 무조건 배제하지 않는다.

다만 프로젝트의 상위 계층에 노출시키지 않는다.

             Application
                  │
             Framework
                  │
               Device
                  │
                  ▼
        ┌──────────────────┐
        │       MCAL       │
        │                  │
        │  직접 Register   │
        │       또는       │
        │   ST HAL / LL    │
        └────────┬─────────┘
                 │
                 ▼
               CMSIS
즉,

ST HAL/LL = 구현 도구
MCAL      = 프로젝트가 정의한 경계
로 생각한다.

6. Device Driver의 위치
Device와 MCAL은 비슷해 보이지만 담당 대상이 다르다.

┌────────────────────────────────────┐
│              Device                │
│                                    │
│  "MPU6050을 어떻게 사용하는가?"    │
│  "Motor를 어떻게 움직이는가?"       │
└─────────────────┬──────────────────┘
                  │
                  ▼
┌────────────────────────────────────┐
│               MCAL                 │
│                                    │
│  "I2C를 어떻게 전송하는가?"         │
│  "PWM을 어떻게 출력하는가?"         │
└─────────────────┬──────────────────┘
                  │
                  ▼
┌────────────────────────────────────┐
│             STM32 MCU              │
└────────────────────────────────────┘
예를 들어 MPU6050:

mpu6050_read_accel()
        │
        ▼
mcal_i2c_read()
        │
        ▼
I2C1 Register
        │
        ▼
MPU6050
Motor:

motor_set_output(50%)
        │
        ▼
mcal_pwm_set_duty(50%)
        │
        ▼
TIMx->CCRx
        │
        ▼
Motor Driver
        │
        ▼
Motor
7. Framework가 이 프로젝트의 핵심
                ┌─────────────────────┐
                │      Framework      │
                │                     │
                │ Scheduler           │
                │ StateMachine        │
                │ Math                │
                │ PID                 │
                │ Filter              │
                │ Sensor Fusion       │
                │ Safety              │
                │ Communication       │
                │ Logger              │
                └──────────┬──────────┘
                           │
              ┌────────────┴────────────┐
              ▼                         ▼
       ┌─────────────┐           ┌─────────────┐
       │   Gimbal    │           │     Door    │
       └─────────────┘           └─────────────┘
Framework는 특정 제품의 코드를 넣는 곳이 아니다.

좋은 예

PID
Filter
Quaternion
StateMachine
Scheduler
Safety
Logger
나쁜 예

gimbal_roll_pid()
door_open_motor()
mpu6050_specific_logic()
제품에 종속되는 것은 Application 또는 Device로 내려간다.

8. Gimbal 전체 구조
                    ┌───────────────┐
                    │ Raspberry Pi  │
                    │               │
                    │ Camera        │
                    │ GUI           │
                    │ Log           │
                    │ PID Tuning    │
                    └───────┬───────┘
                            │ UART
                            ▼
┌────────────────────────────────────────────────────┐
│                    STM32F411                       │
│                                                    │
│  ┌──────────────────────────────────────────────┐  │
│  │              Gimbal Application              │  │
│  └──────────────────────┬───────────────────────┘  │
│                         ▼                          │
│  ┌──────────────────────────────────────────────┐  │
│  │                  Framework                   │  │
│  │ Scheduler / Fusion / PID / Safety / Logger  │  │
│  └──────────────┬───────────────────┬───────────┘  │
│                 │                   │              │
│                 ▼                   ▼              │
│          ┌─────────────┐     ┌─────────────┐       │
│          │ IMU Device  │     │Motor Device │       │
│          └──────┬──────┘     └──────┬──────┘       │
│                 │                   │              │
│                 └─────────┬─────────┘              │
│                           ▼                        │
│                     ┌──────────┐                   │
│                     │   MCAL   │                   │
│                     │ I2C/PWM  │                   │
│                     └──────────┘                   │
└────────────────────────────────────────────────────┘
9. Gimbal 제어 Loop
             IMU
              │
              ▼
        Raw Acc/Gyro
              │
              ▼
        Calibration
              │
              ▼
          Filtering
              │
              ▼
       Sensor Fusion
              │
              ▼
       Current Angle
              │
              ▼
       ┌──────────────┐
Target │  Angle PID   │
──────►│              │
       └──────┬───────┘
              │
         Target Rate
              │
              ▼
       ┌──────────────┐
       │   Rate PID   │
       └──────┬───────┘
              │
         Motor Output
              │
              ▼
       ┌──────────────┐
       │ Motor Driver │
       └──────┬───────┘
              │
              ▼
            Motor
              │
              └───────────────┐
                              │
                              ▼
                             IMU
이 구조가 바로 Cascade PID 구조다.

10. Automatic Door 전체 구조
             ┌──────────────┐
             │ Human Detect │
             │ PIR / Ultra  │
             └──────┬───────┘
                    │
                    ▼
             ┌──────────────┐
             │   Sensor     │
             │ Processing   │
             └──────┬───────┘
                    │
                    ▼
             ┌──────────────┐
             │ StateMachine │
             └──────┬───────┘
                    │
          ┌─────────┴─────────┐
          ▼                   ▼
       OPENING              CLOSING
          │                   │
          ▼                   ▼
        Motor               Motor
          │                   │
          └─────────┬─────────┘
                    ▼
             Encoder / Limit
                    │
                    ▼
                 Safety
                    │
                    ▼
             Stop / Reverse
11. Gimbal과 Door의 Framework 재사용
                         ┌──────────────────┐
                         │    Framework     │
                         ├──────────────────┤
                         │ Scheduler        │
                         │ StateMachine     │
                         │ Safety           │
                         │ Communication    │
                         │ Logger           │
                         └────────┬─────────┘
                                  │
                 ┌────────────────┼────────────────┐
                 │                │                │
                 ▼                ▼                ▼
              Gimbal             Door          Future App
                 │                │
                 ▼                ▼
              IMU/Motor        PIR/Motor
Framework은 제품을 모르고, Application이 Framework를 사용한다.

12. Scheduler 구조
Scheduler는 "무엇을 할지"가 아니라 **"언제 실행할지"**를 관리한다.

                 Scheduler
                     │
       ┌─────────────┼─────────────┐
       │             │             │
       ▼             ▼             ▼
    1 kHz          100 Hz         10 Hz
       │             │             │
       ▼             ▼             ▼
 Control Loop    State Update   Telemetry
       │             │             │
       ▼             ▼             ▼
      PID       StateMachine     UART
예:
1 ms   → IMU / Rate PID
10 ms  → Angle PID / State
100 ms → Telemetry
실제 주기는 제어 대상과 CPU 부하에 맞춰 결정한다.

13. StateMachine 구조
Gimbal

             ┌──────────────┐
             │     INIT     │
             └──────┬───────┘
                    ▼
             ┌──────────────┐
             │ CALIBRATION  │
             └──────┬───────┘
                    ▼
             ┌──────────────┐
             │    READY     │
             └──────┬───────┘
                    ▼
             ┌──────────────┐
             │    ACTIVE    │
             └──────┬───────┘
                    │ fault
                    ▼
             ┌──────────────┐
             │    ERROR     │
             └──────────────┘
Automatic Door

CLOSED
  │ person detected
  ▼
OPENING
  │ reached open position
  ▼
OPEN
  │ timeout
  ▼
CLOSING
  │ reached closed position
  ▼
CLOSED


CLOSING
  │ obstacle detected
  └──────────────→ OPENING
14. Safety 구조
Safety는 별도 기능처럼 보이지만 실제로는 모든 제품에서 필요하다.

                 ┌───────────────┐
                 │    Sensors    │
                 └───────┬───────┘
                         │
                         ▼
                 ┌───────────────┐
                 │ Fault Monitor │
                 └───────┬───────┘
                         │
             ┌───────────┼───────────┐
             ▼           ▼           ▼
         IMU Timeout  Motor Fault  Comm Timeout
             │           │           │
             └───────────┼───────────┘
                         ▼
                 ┌───────────────┐
                 │ SAFE / ERROR  │
                 └───────┬───────┘
                         ▼
                 Output Disable
15. Communication 구조
Raspberry Pi와 STM32는 역할을 분리한다.

┌──────────────────────┐
│     Raspberry Pi     │
├──────────────────────┤
│ Camera Capture       │
│ Video Display        │
│ Video Save           │
│ Real-time Graph      │
│ PID Tuning GUI       │
│ Log Storage          │
└──────────┬───────────┘
           │ UART
           ▼
┌──────────────────────┐
│      STM32F411       │
├──────────────────────┤
│ Sensor               │
│ Control              │
│ Scheduler            │
│ Motor                │
│ Safety               │
│ Telemetry            │
└──────────────────────┘
STM32              Raspberry Pi
실시간 제어          GUI
센서 처리            영상
모터 제어            로그
Safety              튜닝
                    상위 명령
16. Telemetry 흐름
Sensor
   │
   ▼
Control
   │
   ├──────────────┐
   │              │
   ▼              ▼
 Motor         Logger
                  │
                  ▼
             Ring Buffer
                  │
                  ▼
               UART
                  │
                  ▼
           Raspberry Pi
                  │
       ┌──────────┼──────────┐
       ▼          ▼          ▼
     Graph       Log       Monitor
고속 Control Loop에서 UART를 직접 blocking 방식으로 처리하지 않도록 한다.

17. Host Test 구조
Framework의 가장 큰 장점 중 하나다.

                  Framework
                      │
          ┌───────────┴───────────┐
          │                       │
          ▼                       ▼
     STM32 Target             PC Target
          │                       │
          ▼                       ▼
      Real Hardware           Unit Test
          │                       │
          ▼                       ▼
       Sensor/Motor          Mock Sensor
예:
PID
 │
 ├── STM32에서 실제 Motor 제어
 │
 └── PC에서 입력값을 넣고 수학적으로 검증
18. Test가 가능한 코드의 기준
좋은 Framework:

float pid_update(PID_t *pid,
                 float target,
                 float measurement,
                 float dt);
이 함수는 GPIO, UART, TIM, STM32 Register를 몰라도 된다.

따라서 PC에서도 테스트할 수 있다.

Input
  ↓
PID
  ↓
Output
반대로 다음과 같이 만들면 테스트하기 어렵다.

void gimbal_pid(void)
{
    TIM1->CCR1 = ...;
    HAL_UART_Transmit(...);
}
알고리즘과 Hardware I/O를 분리하는 이유가 바로 Host Testability다.

19. Startup → main 실행 흐름
              Power ON / Reset
                     │
                     ▼
          startup_stm32f411xe.s
                     │
             Stack / Vector
                     │
                     ▼
              SystemInit()
                     │
                     ▼
                  main()
                     │
                     ▼
              Board_Init()
                     │
                     ▼
               MCAL_Init()
                     │
                     ▼
            Framework_Init()
                     │
                     ▼
          Application_Init()
                     │
                     ▼
             Scheduler_Run()
                     │
                     ▼
                Main Loop
20. Linker Script는 왜 필요한가?
Startup만 있다고 실행 파일이 완성되는 것은 아니다.

startup.s
   │
   │ Reset Handler
   ▼
main()
과 동시에 Linker Script가 메모리 배치를 결정한다.

STM32F411 Flash
┌─────────────────────┐
│ Vector Table        │
├─────────────────────┤
│ .text               │
│ Program Code        │
├─────────────────────┤
│ .rodata             │
│ Const Data          │
└─────────────────────┘


STM32F411 SRAM
┌─────────────────────┐
│ .data               │
│ Initialized Data    │
├─────────────────────┤
│ .bss                │
│ Zero Initialized    │
├─────────────────────┤
│ Heap                │
├─────────────────────┤
│ Stack               │
└─────────────────────┘
따라서:

Startup = CPU를 C 프로그램 실행 상태로 준비
Linker  = 프로그램을 MCU 메모리에 배치
21. syscalls.c / sysmem.c의 위치
                 C Standard Library
                        │
             ┌──────────┴──────────┐
             ▼                     ▼
         syscalls.c             sysmem.c
             │                     │
             ▼                     ▼
       low-level I/O             Heap
             │                     │
             ▼                     ▼
        UART / File          _sbrk / malloc
이 파일들은 일반적인 Application/Framework 로직이 아니라 C Library와 Bare-metal System을 연결하는 시스템 Glue Code에 가깝다.

따라서 Core/System/에 배치하는 것이 자연스럽다.

22. Common의 사용 범위
                 Common
              ┌────┼────┐
              │    │    │
              ▼    ▼    ▼
           Device Framework App
단, Common은 모든 코드가 의존하는 "쓰레기통 폴더"가 되면 안 된다.

넣어도 되는 것

기본 타입
Error Code
Bit Utility
Ring Buffer
공통 작은 자료구조
넣으면 안 되는 것

MPU6050 코드
PID 코드
Motor 코드
Gimbal 코드
Door 코드
23. Configuration 원칙
             Configuration
                   │
        ┌──────────┴──────────┐
        ▼                     ▼
 system_config.h       control_config.h
        │                     │
        ▼                     ▼
 Application 선택       PID / Filter 값
예:

/* system_config.h */

/* 어떤 Application을 빌드할 것인가? */
#define APPLICATION_GIMBAL
/* #define APPLICATION_AUTOMATIC_DOOR */
/* control_config.h */

/* Gimbal Rate PID */
#define GIMBAL_RATE_KP    ...
#define GIMBAL_RATE_KI    ...
#define GIMBAL_RATE_KD    ...
Pin은:

Board/NUCLEO_F411RE/pinmap_config.h
에서 관리한다.

24. 실제 코드 책임 경계
Application

void Gimbal_Run(void)
{
    sensor = gimbal_get_sensor();
    angle  = fusion_update(sensor);
    output = gimbal_controller_update(angle);
    gimbal_motor_set(output);
}
"무엇을 할지"를 결정한다.

Framework

output = pid_update(&pid, target, measurement, dt);
"알고리즘을 어떻게 수행할지" 담당한다.

Device

mpu6050_read(&imu);
motor_set_output(output);
"외부 Device를 어떻게 사용하는지" 담당한다.

MCAL

mcal_i2c_read(...);
mcal_pwm_set_duty(...);
"MCU Peripheral을 어떻게 사용하는지" 담당한다.

CMSIS

I2C1->CR1
TIM1->CCR1
GPIOA->MODER
"Register와 Core를 어떻게 표현하는지" 담당한다.

25. 전체 실행 구조를 한 장으로 표현
                     ┌──────────────────────────┐
                     │      Raspberry Pi        │
                     │                          │
                     │ Camera / GUI / Log       │
                     └────────────┬─────────────┘
                                  │ UART
                                  ▼
┌──────────────────────────────────────────────────────────────┐
│                        STM32F411                             │
│                                                              │
│  ┌────────────────────────────────────────────────────────┐  │
│  │                    Application                         │  │
│  │                                                        │  │
│  │       Gimbal                    Automatic Door          │  │
│  └───────────────────────┬────────────────────────────────┘  │
│                          │                                   │
│  ┌───────────────────────▼────────────────────────────────┐  │
│  │                     Framework                          │  │
│  │                                                        │  │
│  │ Scheduler │ StateMachine │ PID │ Filter │ Fusion      │  │
│  │ Safety    │ Logger       │ Communication │ Math       │  │
│  └───────────────┬───────────────────────────┬────────────┘  │
│                  │                           │               │
│                  ▼                           ▼               │
│           ┌─────────────┐             ┌─────────────┐       │
│           │   Device    │             │   Device    │       │
│           │ IMU / Motor │             │ PIR / Motor │       │
│           └──────┬──────┘             └──────┬──────┘       │
│                  │                           │              │
│                  └─────────────┬─────────────┘              │
│                                ▼                            │
│                         ┌────────────┐                      │
│                         │    MCAL    │                      │
│                         │ I2C/PWM... │                      │
│                         └─────┬──────┘                      │
│                               ▼                             │
│                         ┌────────────┐                      │
│                         │   CMSIS    │                      │
│                         └─────┬──────┘                      │
└───────────────────────────────┼─────────────────────────────┘
                                ▼
                         STM32 Registers
26. 개발 순서
Phase 1 — MCU Boot
CMSIS
  ↓
Startup
  ↓
Linker
  ↓
System
  ↓
main()
목표: Build → Flash → Debug 성공

Phase 2 — MCAL
GPIO
 ↓
UART
 ↓
Timer
 ↓
I2C
 ↓
SPI
 ↓
PWM
Phase 3 — Device

Gimbal            Door
MPU6050           PIR
Motor             Ultrasonic
Encoder           Motor
                  LimitSwitch
                  Encoder
Phase 4 — Framework
Scheduler
StateMachine
Math
Filter
PID
Sensor Fusion
Safety
Communication
Logger
Phase 5 — Gimbal End-to-End
IMU
 ↓
Fusion
 ↓
Angle PID
 ↓
Rate PID
 ↓
Motor
Phase 6 — Automatic Door
Sensor
 ↓
StateMachine
 ↓
Motor
 ↓
Encoder / Limit
Phase 7 — Host Test
PID
Quaternion
Filter
StateMachine
Math
을 PC에서 테스트한다.

27. 처음부터 만들지 않을 것
┌─────────────────────────────────────────┐
│       현재는 구현하지 않는 것              │
├─────────────────────────────────────────┤
│ STM32G4 동시 지원                        │
│ STM32H7 지원                            │
│ 복잡한 RTOS Abstraction                  │
│ 과도한 Mock Framework                    │
│ 지나치게 세분화된 Math Folder             │
│ 여러 Board 동시 지원                      │
│ 필요 없는 Generic Driver                 │
└─────────────────────────────────────────┘
이유: 추상화를 위한 추상화는 하지 않는다.

현재:

STM32F411
+
NUCLEO-F411RE
+
Gimbal
을 먼저 완성한다.

이후 실제 Porting 요구가 생겼을 때:

STM32F411
      ↓
STM32G431

또는

NUCLEO-F411RE
      ↓
CUSTOM BOARD
를 추가한다.

28. 최종 Architecture 원칙
┌────────────────────────────────────────────────────┐
│                   10 RULES                         │
├────────────────────────────────────────────────────┤
│ 1. Hardware dependency는 아래로 내린다.              │
│ 2. Application은 Register를 직접 접근하지 않는다.     │
│ 3. Framework는 Hardware-independent하게 만든다.      │
│ 4. Device는 외부 소자를 추상화한다.                   │
│ 5. MCAL은 MCU Peripheral을 추상화한다.               │
│ 6. ST HAL/LL은 MCAL 내부 구현으로 사용할 수 있다.      │
│ 7. Board 설정과 Software Config를 분리한다.          │
│ 8. Framework는 Gimbal/Door 모두가 재사용한다.         │
│ 9. Pure Logic은 PC에서 Test 가능하게 만든다.          │
│10. 실제 필요 이상의 추상화는 만들지 않는다.             │
└────────────────────────────────────────────────────┘
29. 이 프로젝트가 최종적으로 증명하는 것
단순히 "STM32로 짐벌을 만들었다."가 아니다.

목표는:

              ┌────────────────────┐
              │ Reusable Framework │
              └─────────┬──────────┘
                        │
             ┌──────────┴──────────┐
             ▼                     ▼
          Gimbal                 Door
             │                     │
             └──────────┬──────────┘
                        ▼
                 동일 Framework
                 재사용 및 검증
그리고:

Hardware
   │
   ▼
MCAL
   │
   ▼
Device
   │
   ▼
Framework
   │
   ▼
Application
이라는 명확한 계층 구조와 의존성 방향을 실제 프로젝트에서 증명하는 것이다.

30. 최종 요약
가장 중요한 구조

             APPLICATION
            /           \
        Gimbal           Door
            \             /
             \           /
              FRAMEWORK
                  │
               DEVICE
                  │
                MCAL
                  │
               CMSIS
                  │
                MCU
Hardware 설정

BOARD
 ├── PinMap
 ├── Board Init
 ├── Startup
 └── Linker
System

CORE
 ├── main
 ├── System
 ├── syscalls
 └── sysmem
Configuration

CONFIG
 ├── Application Selection
 └── Control Parameters
Test

FRAMEWORK
    │
    ├────────→ STM32 Target
    │
    └────────→ PC Test Target
결론
이 구조의 핵심은 폴더를 많이 만드는 것이 아니라,

"각 계층이 무엇을 알고 있어야 하고, 무엇을 몰라야 하는가"

를 명확하게 만드는 것이다.

특히 Application → Framework → Device → MCAL → CMSIS의 방향을 유지하고, Framework의 Hardware Independence를 확보하는 것이 이 프로젝트의 가장 중요한 설계 목표다.

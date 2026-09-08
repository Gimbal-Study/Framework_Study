#include "mcal_i2c.h"
#include "stm32f411xe.h"
#include <common.h>
#include "mcal_macro.h"


bool mcal_i2c_init(uint8_t channel, uint8_t mode, uint32 Freq)
{
    if(/* channel == ??? ||*/ Freq == 0)
    {
        return false;
    }

    // 2. 하드웨어 주변장치 클록 인가 (I2C1, GPIOB)
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN; //  (0x1UL << 21U)
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN; // (0x1UL << 1U)

    // 3. GPIO 핀 설정 (PB6 = SCL, PB7 = SDA)
    // MODER: PB6, PB7을 대체 기능 모드(Alternate Function, 10b)로 설정
    GPIOB->MODER &= ~((0x3U << (6 * 2)) | (0x3U << (7 * 2)));
    GPIOB->MODER |= ((0x2U << (6 * 2)) | (0x3U << (7 * 2)));

    // OTYPER: I2C 필수 조건인 Open-Drain(1) 설정
    GPIOB->OTYPER |= (0x1U << 6) | (0x1U << 7);

    // PUPDR: MCU 내부 풀업
    GPIOB->PUPDR &= ~((0x3U << (6 * 2)) | (0x3U << (7 * 2)));
    GPIOB->PUPDR |= ((0x1U << (6 * 2)) | (0x1U << (7 * 2)));

    // AFR[0]: PB6, PB7에 AF4(I2C1 기능 번호) 부여
    GPIOB->AFR[0] &= ~((0xFU << (6 * 4)) | (0xFU << (7 * 4)));
    GPIOB->AFR[0] |= ((0x4U << (6 * 4)) | (0x4U << (7 * 4)));

    // I2C 하드웨어 타이밍 설정
    // 레지스터 설정 전 반드시 I2C 모듈을 잠시 꺼두어야 함
    I2C1->CR1 &= ~I2C_CR1_PE; // (0x1UL << 0U)

    // I2C에 APB1 클록 주파수(MHz)를 알려줌
    // 해당 프로젝트에서는 STM32에 공급되는 주파수가 96MHz. 이것의 절반.
    I2C1->CR2 = 48; //
}

/*
 * STM32F411 I2C polling reference implementation
 *
 * 참고용 파일: 프로젝트 소스에는 적용하지 않았으며 보드 검증 전이다.
 * 근거: RM0383, I2C controller transfer sequences and register descriptions.
 *
 * 사용 조건
 * - RCC, GPIO, CCR, TRISE 등 초기화 완료, PE=1, 일반 I2C 모드.
 * - 같은 채널을 다른 태스크/ISR/DMA가 동시에 사용하지 않는다.
 *   BUSY 검사는 소프트웨어 mutex를 대신하지 않는다.
 * - mcal_time_ms()를 플랫폼에서 구현해야 한다. 대기 중에도 증가해야 한다.
 * - dev_addr는 이동 전 7비트 주소이다. 10비트 주소는 지원하지 않는다.
 * - mem_addr_size: 0=주소 없음, 1=8비트, 2=16비트(MSB 먼저).
 * - 전송 방향은 write/read 함수가 내부에서 결정한다. rw 인수는 없다.
 * - len=0은 ERROR, timeout=0은 TIMEOUT. timeout 단위는 ms이다.
 * - 오류 후 버스/주변장치 복구가 필요할 수 있다. 자동 복구 드라이버가 아니다.
 * - 실패 시 송신 일부가 반영되거나 수신 버퍼 일부만 갱신될 수 있다.
 *
 * 통합 시 주의
 * - 기존 mcal_i2c.h의 함수 선언을 이 파일의 두 공개 함수 정의와 일치시켜야 한다.
 * - 이 파일은 기존 구현과 동시에 빌드하면 중복 심볼이 발생한다.
 * - STM32 환경에서 unsigned int가 32비트인 조건을 사용한다.
 * - 실제 적용 전 read 길이 1/2/3/4 이상 및 NACK/timeout을 검증한다.
 */

/* 구현 필요: 1 ms 단위로 증가하는 시간을 반환한다. */
extern uint32_t mcal_time_ms(void);

#define I2C_DIRECTION_WRITE 0U
#define I2C_DIRECTION_READ  1U
#define I2C_ERRORS (I2C_SR1_AF | I2C_SR1_BERR | I2C_SR1_ARLO | I2C_SR1_OVR)
#define I2C_PENDING (I2C_CR1_START | I2C_CR1_STOP | I2C_CR1_PEC)

/* 논리 채널을 주변장치 레지스터 주소로 변환한다. */
static I2C_TypeDef *i2c_get_instance(uint8_t channel)
{
    switch (channel)
    {
        case 1U: return I2C1;
        case 2U: return I2C2;
        case 3U: return I2C3;
        default: return NULL;
    }
}

/* unsigned 뺄셈으로 tick wraparound를 처리한다. */
static bool i2c_expired(uint32_t started, uint32_t timeout_ms)
{
    return (uint32_t)(mcal_time_ms() - started) >= timeout_ms;
}

/* 오류를 우선 검사하며 SR1 이벤트를 기다린다. */
static mcal_i2c_status_t i2c_wait_event(I2C_TypeDef *i2c,
    uint32_t flag, uint32_t started, uint32_t timeout_ms)
{
    for (;;)
    {
        uint32_t sr1 = i2c->SR1;
        if ((sr1 & I2C_ERRORS) != 0U)
            return MCAL_I2C_ERROR;
        if (i2c_expired(started, timeout_ms))
            return MCAL_I2C_TIMEOUT;
        if ((sr1 & flag) == flag)
            return MCAL_I2C_OK;
    }
}

/* STOP을 요청하는 함수가 아니라 요청 완료를 기다리는 함수이다. */
static mcal_i2c_status_t i2c_wait_stop(I2C_TypeDef *i2c,
    uint32_t started, uint32_t timeout_ms)
{
    while ((i2c->CR1 & I2C_CR1_STOP) != 0U)
    {
        if (i2c_expired(started, timeout_ms))
            return MCAL_I2C_TIMEOUT;
    }
    return MCAL_I2C_OK;
}

/* ADDR는 SR1 -> SR2 읽기로 해제한다. */
static void i2c_clear_addr(I2C_TypeDef *i2c)
{
    (void)i2c->SR1;
    (void)i2c->SR2;
}

/* 짧은 레지스터 시퀀스만 보호하며 대기 루프에는 사용하지 않는다. */
static uint32_t i2c_enter_critical(void)
{
    uint32_t saved = __get_PRIMASK();
    __disable_irq();
    return saved;
}

static void i2c_leave_critical(uint32_t saved)
{
    __set_PRIMASK(saved);
}

static mcal_i2c_status_t i2c_check_ready(I2C_TypeDef *i2c)
{
    if (i2c == NULL)
        return MCAL_I2C_ERROR;
    if ((i2c->CR1 & I2C_CR1_PE) == 0U)
        return MCAL_I2C_ERROR;
    if ((i2c->CR1 & I2C_PENDING) != 0U)
        return MCAL_I2C_BUSY;
    if ((i2c->SR2 & I2C_SR2_BUSY) != 0U)
        return MCAL_I2C_BUSY;
    if ((i2c->SR1 & I2C_ERRORS) != 0U)
        return MCAL_I2C_ERROR;
    return MCAL_I2C_OK;
}

static bool i2c_mem_addr_valid(uint16_t mem_addr, uint8_t mem_addr_size)
{
    if (mem_addr_size > 2U)
        return false;
    if ((mem_addr_size == 1U) && (mem_addr > 0xFFU))
        return false;
    return true;
}

/* write/read 공통 인수 검사. START 이전에 호출한다. */
static mcal_i2c_status_t i2c_validate_transfer(uint16_t dev_addr,
    uint16_t mem_addr, uint8_t mem_addr_size,
    const uint8_t *data, uint16_t len, uint32_t timeout_ms)
{
    if ((data == NULL) || (len == 0U) || (dev_addr > 0x7FU) ||
        !i2c_mem_addr_valid(mem_addr, mem_addr_size))
        return MCAL_I2C_ERROR;
    if (timeout_ms == 0U)
        return MCAL_I2C_TIMEOUT;
    return MCAL_I2C_OK;
}

/*
 * START/Repeated START 후 장치 주소를 보낸다.
 * 성공 시 ADDR를 남겨두며 호출자가 방향/수신 길이에 맞춰 해제한다.
 */
static mcal_i2c_status_t i2c_address_begin(I2C_TypeDef *i2c,
    uint16_t dev_addr, uint8_t direction,
    uint32_t started, uint32_t timeout_ms)
{
    mcal_i2c_status_t status;
    i2c->CR1 |= I2C_CR1_START;
    status = i2c_wait_event(i2c, I2C_SR1_SB, started, timeout_ms);
    if (status != MCAL_I2C_OK)
        return status;
    /* SR1 읽기 후 DR 쓰기로 SB가 해제된다. */
    i2c->DR = (uint32_t)((dev_addr << 1U) | direction);
    return i2c_wait_event(i2c, I2C_SR1_ADDR, started, timeout_ms);
}

/*
 * 장치주소(W)의 ADDR 해제 후 호출한다.
 * size=1/2이면 마지막 주소 바이트의 BTF까지 확인한다.
 * 읽기 함수는 성공 직후 Repeated START를 요청할 수 있다.
 */
static mcal_i2c_status_t i2c_mem_addr_write(I2C_TypeDef *i2c,
    uint16_t mem_addr, uint8_t mem_addr_size,
    uint32_t started, uint32_t timeout_ms)
{
    mcal_i2c_status_t status;
    if (!i2c_mem_addr_valid(mem_addr, mem_addr_size))
        return MCAL_I2C_ERROR;
    if (mem_addr_size == 0U)
        return MCAL_I2C_OK;
    if (mem_addr_size == 2U)
    {
        status = i2c_wait_event(i2c, I2C_SR1_TXE, started, timeout_ms);
        if (status != MCAL_I2C_OK)
            return status;
        i2c->DR = (uint32_t)((mem_addr >> 8U) & 0xFFU);
    }
    status = i2c_wait_event(i2c, I2C_SR1_TXE, started, timeout_ms);
    if (status != MCAL_I2C_OK)
        return status;
    i2c->DR = (uint32_t)(mem_addr & 0xFFU);
    return i2c_wait_event(i2c, I2C_SR1_BTF, started, timeout_ms);
}

/* STOP 완료 후에만 CR1의 수신 설정을 정리한다. */
static mcal_i2c_status_t i2c_finish(I2C_TypeDef *i2c,
    uint32_t started, uint32_t timeout_ms)
{
    mcal_i2c_status_t status = i2c_wait_stop(i2c, started, timeout_ms);
    if (status != MCAL_I2C_OK)
        return status;
    i2c->CR1 &= ~(I2C_CR1_ACK | I2C_CR1_POS);
    return MCAL_I2C_OK;
}

/*
 * 종료 시도만 수행한다. 원래 제한 시간 이상 새 대기 시간을 부여하지 않는다.
 * START/STOP 잔류 또는 버스 고정 시 재호출 전에 별도 복구가 필요하다.
 */
static void i2c_abort(I2C_TypeDef *i2c,
    uint32_t started, uint32_t timeout_ms)
{
    uint32_t sr1 = i2c->SR1;
    uint32_t cr1 = i2c->CR1;
    if (((cr1 & I2C_PENDING) == 0U) && ((sr1 & I2C_SR1_ARLO) == 0U))
    {
        if ((sr1 & I2C_SR1_ADDR) != 0U)
        {
            i2c->CR1 &= ~I2C_CR1_ACK;
            i2c_clear_addr(i2c);
        }
        if ((i2c->SR2 & I2C_SR2_MSL) != 0U)
            i2c->CR1 = (i2c->CR1 & ~I2C_CR1_ACK) | I2C_CR1_STOP;
    }
    i2c->SR1 &= ~I2C_ERRORS;
    (void)i2c_wait_stop(i2c, started, timeout_ms);
}

/* START -> device(W) -> optional memory address -> data -> STOP */
mcal_i2c_status_t mcal_i2c_write(uint8_t channel, uint16_t dev_addr,
    uint16_t mem_addr, uint8_t mem_addr_size,
    const uint8_t *data, uint16_t len, unsigned int timeout)
{
    I2C_TypeDef *i2c;
    mcal_i2c_status_t status;
    uint32_t started;
    uint32_t timeout_ms = (uint32_t)timeout;
    status = i2c_validate_transfer(dev_addr, mem_addr, mem_addr_size,
        data, len, timeout_ms);
    if (status != MCAL_I2C_OK)
        return status;

    i2c = i2c_get_instance(channel);
    status = i2c_check_ready(i2c);
    if (status != MCAL_I2C_OK)
        return status;
    started = mcal_time_ms();
    i2c->CR1 &= ~(I2C_CR1_ACK | I2C_CR1_POS);
    status = i2c_address_begin(i2c, dev_addr, I2C_DIRECTION_WRITE,
        started, timeout_ms);
    if (status != MCAL_I2C_OK)
        goto fail;
    i2c_clear_addr(i2c);
    status = i2c_mem_addr_write(i2c, mem_addr, mem_addr_size,
        started, timeout_ms);
    if (status != MCAL_I2C_OK)
        goto fail;
    for (uint16_t index = 0U; index < len; ++index)
    {
        status = i2c_wait_event(i2c, I2C_SR1_TXE, started, timeout_ms);
        if (status != MCAL_I2C_OK)
            goto fail;
        i2c->DR = data[index];
    }
    /* TXE는 DR 여유, BTF는 마지막 바이트 전송 완료를 확인한다. */
    status = i2c_wait_event(i2c, I2C_SR1_BTF, started, timeout_ms);
    if (status != MCAL_I2C_OK)
        goto fail;
    i2c->CR1 |= I2C_CR1_STOP;
    return i2c_finish(i2c, started, timeout_ms);
fail:
    i2c_abort(i2c, started, timeout_ms);
    return status;
}

/*
 * size=0: START -> device(R) -> receive -> NACK/STOP
 * size>0: START -> device(W) -> memory -> RESTART -> device(R) -> receive
 * data는 수신 결과를 저장하므로 const가 아니다.
 */
mcal_i2c_status_t mcal_i2c_read(uint8_t channel, uint16_t dev_addr,
    uint16_t mem_addr, uint8_t mem_addr_size,
    uint8_t *data, uint16_t len, unsigned int timeout)
{
    I2C_TypeDef *i2c;
    mcal_i2c_status_t status;
    uint32_t started;
    uint32_t timeout_ms = (uint32_t)timeout;
    uint32_t saved_irq;          /* 임계 구역 진입 전 PRIMASK */
    uint16_t remaining = len;    /* 아직 읽지 않은 바이트 수 */
    uint8_t *next = data;        /* 다음 바이트를 저장할 위치 */
    status = i2c_validate_transfer(dev_addr, mem_addr, mem_addr_size,
        data, len, timeout_ms);
    if (status != MCAL_I2C_OK)
        return status;

    i2c = i2c_get_instance(channel);
    status = i2c_check_ready(i2c);
    if (status != MCAL_I2C_OK)
        return status;
    started = mcal_time_ms();
    i2c->CR1 = (i2c->CR1 & ~I2C_CR1_POS) | I2C_CR1_ACK;
    if (mem_addr_size != 0U)
    {
        status = i2c_address_begin(i2c, dev_addr, I2C_DIRECTION_WRITE,
            started, timeout_ms);
        if (status != MCAL_I2C_OK)
            goto fail;
        i2c_clear_addr(i2c);
        status = i2c_mem_addr_write(i2c, mem_addr, mem_addr_size,
            started, timeout_ms);
        if (status != MCAL_I2C_OK)
            goto fail;
        /* STOP 없이 다음 START를 요청하여 Repeated START로 전환한다. */
    }
    status = i2c_address_begin(i2c, dev_addr, I2C_DIRECTION_READ,
        started, timeout_ms);
    if (status != MCAL_I2C_OK)
        goto fail;

    if (remaining == 1U)
    {
        /* 첫 바이트가 마지막: ADDR 해제 전에 ACK를 끈다. */
        saved_irq = i2c_enter_critical();
        i2c->CR1 &= ~I2C_CR1_ACK;
        i2c_clear_addr(i2c);
        i2c->CR1 |= I2C_CR1_STOP;
        i2c_leave_critical(saved_irq);
        status = i2c_wait_event(i2c, I2C_SR1_RXNE, started, timeout_ms);
        if (status != MCAL_I2C_OK)
            goto fail;
        *next = (uint8_t)i2c->DR;
    }
    else if (remaining == 2U)
    {
        /* 두 바이트: ACK=0/POS=1 -> ADDR clear -> BTF -> STOP -> DR x2. */
        saved_irq = i2c_enter_critical();
        i2c->CR1 = (i2c->CR1 & ~I2C_CR1_ACK) | I2C_CR1_POS;
        i2c_clear_addr(i2c);
        i2c_leave_critical(saved_irq);
        status = i2c_wait_event(i2c, I2C_SR1_BTF, started, timeout_ms);
        if (status != MCAL_I2C_OK)
            goto fail;
        saved_irq = i2c_enter_critical();
        i2c->CR1 |= I2C_CR1_STOP;
        *next++ = (uint8_t)i2c->DR;
        *next = (uint8_t)i2c->DR;
        i2c_leave_critical(saved_irq);
    }
    else
    {
        i2c_clear_addr(i2c);
        while (remaining > 3U)
        {
            status = i2c_wait_event(i2c, I2C_SR1_RXNE, started, timeout_ms);
            if (status != MCAL_I2C_OK)
                goto fail;
            *next++ = (uint8_t)i2c->DR;
            --remaining;
        }
        /* 마지막 세 바이트: BTF 상태에서 ACK를 끄고 N-2를 읽는다. */
        status = i2c_wait_event(i2c, I2C_SR1_BTF, started, timeout_ms);
        if (status != MCAL_I2C_OK)
            goto fail;
        saved_irq = i2c_enter_critical();
        i2c->CR1 &= ~I2C_CR1_ACK;
        *next++ = (uint8_t)i2c->DR;
        --remaining;
        i2c_leave_critical(saved_irq);
        /* 마지막 두 바이트 준비를 기다리는 동안 인터럽트는 복원되어 있다. */
        status = i2c_wait_event(i2c, I2C_SR1_BTF, started, timeout_ms);
        if (status != MCAL_I2C_OK)
            goto fail;
        saved_irq = i2c_enter_critical();
        i2c->CR1 |= I2C_CR1_STOP;
        *next++ = (uint8_t)i2c->DR;
        *next = (uint8_t)i2c->DR;
        i2c_leave_critical(saved_irq);
    }
    return i2c_finish(i2c, started, timeout_ms);
fail:
    i2c_abort(i2c, started, timeout_ms);
    return status;
}

/*
 * 호출 예시 (장치 주소/내부 주소는 실제 장치에 맞춰 변경):
 *
 * uint8_t tx[] = { 0x12U, 0x34U };
 * uint8_t rx[4];
 * mcal_i2c_status_t status;
 * status = mcal_i2c_write(1U, 0x50U, 0x20U, 1U, tx, sizeof(tx), 100U);
 * status = mcal_i2c_read(1U, 0x50U, 0x1234U, 2U, rx, sizeof(rx), 100U);
 * status = mcal_i2c_read(1U, 0x50U, 0U, 0U, rx, 1U, 100U);
 */

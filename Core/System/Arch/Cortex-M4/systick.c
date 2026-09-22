/*
 * Author  : sghyeon
 * Version : v1.0
 * Target  : STM32F411RE
 */
#include <stdint.h>
#include "option.h"
#include "systick.h"
#include "clock.h"
#include "mcal_macro.h"
#include "stm32f411xe.h"

#define SYSTICK_CLOCK_HZ              (HCLK / 8U)
#define SYSTICK_TICKS_PER_MSEC        (SYSTICK_CLOCK_HZ / 1000U)
#define SYSTICK_TICKS_PER_100_USEC    (SYSTICK_CLOCK_HZ / 10000U)

static volatile uint32_t systick_elapsed_msec = 0U;
static volatile uint32_t systick_period_msec = 0U;

void systick_run(unsigned int msec)
{
    MCAL_CLEAR_BIT(SysTick->CTRL, 2);
    MCAL_CLEAR_BIT(SysTick->CTRL, 1);
    MCAL_CLEAR_BIT(SysTick->CTRL, 0);

    SysTick->LOAD = (unsigned int)((SYSTICK_TICKS_PER_MSEC * msec) - 1U);

    SysTick->VAL = 0U;

    MCAL_SET_BIT(SysTick->CTRL, 0);
}

void systick_run_interrupt(unsigned int period_msec)
{
    MCAL_CLEAR_BIT(SysTick->CTRL, 2);
    MCAL_CLEAR_BIT(SysTick->CTRL, 1);
    MCAL_CLEAR_BIT(SysTick->CTRL, 0);

    SysTick->LOAD = (unsigned int)((SYSTICK_TICKS_PER_MSEC * period_msec) - 1U);

    SysTick->VAL = 0U;

    systick_elapsed_msec = period_msec;
    systick_elapsed_msec = 0U;

    MCAL_SET_BIT(SysTick->CTRL, 1);
    MCAL_SET_BIT(SysTick->CTRL, 0);
}

int systick_check_timeout(void)
{
    return MCAL_CHECK_BIT_SET(SysTick->CTRL, 16);
}

unsigned int systick_get_time(void)
{
    return SysTick->VAL;
}

unsigned int systick_get_load_time(void)
{
    return SysTick->LOAD;
}

void systick_stop(void)
{
    MCAL_CLEAR_BIT(SysTick->CTRL, 0);
}

void delay_ms(unsigned int msec)
{
    systick_run(msec);

    while (!systick_check_timeout())
    {
    }

    systick_stop();
}

void systick_irq_handler(void)
{
    systick_elapsed_msec += systick_period_msec;
}

uint32_t systick_get_elapsed_msec(void)
{
    return systick_elapsed_msec;
}

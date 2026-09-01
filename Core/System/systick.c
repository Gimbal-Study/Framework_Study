/*
 * Author  : sghyeon
 * Version : v1.0
 * Target  : STM32F411RE
 */

#include "systick.h"
#include "clock.h"
#include "mcal_macro.h"
#include "stm32f411xe.h"

void systick_run(unsigned int msec)
{
    MCAL_CLEAR_BIT(SysTick->CTRL, 2);
    MCAL_CLEAR_BIT(SysTick->CTRL, 1);
    MCAL_CLEAR_BIT(SysTick->CTRL, 0);

    SysTick->LOAD = (unsigned int)(((HCLK / (8U * 1000U)) * msec) - 1U);

    SysTick->VAL = 0U;

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

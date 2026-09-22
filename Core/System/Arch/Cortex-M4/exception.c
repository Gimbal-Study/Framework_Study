#include "systick.h"
#include "mcal_i2c.h"
#include "stm32f411xe.h"

void SysTick_Handler(void)
{
    systick_irq_handler();
}

void TIM4_IRQHandler(void)
{
    mcal_i2c_tim4_irq_handler();
}
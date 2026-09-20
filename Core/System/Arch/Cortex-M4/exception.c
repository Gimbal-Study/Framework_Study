#include "systick.h"

void SysTick_Handler(void)
{
    systick_irq_handler();
}


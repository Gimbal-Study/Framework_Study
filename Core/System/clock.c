#include "clock.h"

void Clock_Init(void)
{
    RCC->CR |= (1 << 0); 
    while(!MCAL_CHECK_BIT_SET(RCC->CR, 1));

    FLASH->ACR = (1<<12)|(1<<11);
    FLASH->ACR = (1<<10)|(1<<9)|(1<<8)|(0x3 << 0);

    RCC->PLLCFGR = (8<<24)|(0<<22)|(1<<16)|(192<<6)|(8<<0);

    MCAL_SET_BIT(RCC->CR, 24);
    while(!MCAL_CHECK_BIT_SET(RCC->CR, 25));

    RCC->CFGR = (0<<13)|(4<<10)|(0<<4);

    MCAL_WRITE_BLOCK(RCC->CFGR, 0x3, 0x2, 0);
    while(MCAL_EXTRACT_AREA(RCC->CFGR, 0x3, 2) != 0x2);
}
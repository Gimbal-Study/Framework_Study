/*
 * clock.c
 *
 *  Created on: 2026. 9. 1.
 *      Author: kccistc
 */


#include "clock.h"
#include "stm32f411xe.h"
#include "mcal_macro.h"


void clock_init(void)
{
    /*
     * ============================================================
     * 1. Enable HSI
     * ============================================================
     *
     * RCC_CR
     *   HSION : bit 0
     *   HSIRDY: bit 1
     */

    MCAL_SET_BIT(RCC->CR, 0);

    while (MCAL_CHECK_BIT_CLEAR(RCC->CR, 1))
    {
    }


    /*
     * ============================================================
     * 2. Configure FLASH
     * ============================================================
     *
     * FLASH_ACR
     *
     * LATENCY : bits [3:0]
     * PRFTEN  : bit 8
     * ICEN    : bit 9
     * DCEN    : bit 10
     */

    MCAL_WRITE_BLOCK(FLASH->ACR, 0xF, 3, 0);

    MCAL_SET_BIT(FLASH->ACR, 8);     /* Prefetch enable */
    MCAL_SET_BIT(FLASH->ACR, 9);     /* Instruction cache */
    MCAL_SET_BIT(FLASH->ACR, 10);    /* Data cache */


    /*
     * ============================================================
     * 3. Configure PLL
     * ============================================================
     *
     * PLL source = HSI
     *
     * HSI = 16 MHz
     *
     * PLLM = 8
     * PLLN = 96
     * PLLP = 2
     *
     * VCO input  = 16 / 8  = 2 MHz
     * VCO output = 2 * 96  = 192 MHz
     * SYSCLK     = 192 / 2 = 96 MHz
     *
     * PLLCFGR
     *
     * PLLM : bits [5:0]
     * PLLN : bits [14:6]
     * PLLP : bits [17:16]
     * PLLSRC: bit 22
     */

    uint32_t pll_p_bits;

    pll_p_bits = (PLLP / 2U) - 1U;

    MCAL_WRITE_BLOCK(RCC->PLLCFGR, 0x3F, PLLM, 0);
    MCAL_WRITE_BLOCK(RCC->PLLCFGR, 0x1FF, PLLN, 6);
    MCAL_WRITE_BLOCK(RCC->PLLCFGR, 0x3, pll_p_bits, 16);


    /*
     * ============================================================
     * 4. Enable PLL
     * ============================================================
     *
     * RCC_CR
     *
     * PLLON  : bit 24
     * PLLRDY : bit 25
     */

    MCAL_SET_BIT(RCC->CR, 24);

    while (MCAL_CHECK_BIT_CLEAR(RCC->CR, 25))
    {
    }


    /*
     * ============================================================
     * 5. Configure AHB / APB Prescaler
     * ============================================================
     *
     * RCC_CFGR
     *
     * HPRE  : bits [7:4]
     * PPRE1 : bits [12:10]
     * PPRE2 : bits [15:13]
     *
     * AHB  = SYSCLK / 1  = 96 MHz
     * APB1 = HCLK   / 2  = 48 MHz
     * APB2 = HCLK   / 1  = 96 MHz
     */

    /*
     * HPRE = 0000 → AHB / 1
     */
    MCAL_WRITE_BLOCK(RCC->CFGR, 0xF, 0, 4);

    /*
     * PPRE1 = 100 → APB1 / 2
     */
    MCAL_WRITE_BLOCK(RCC->CFGR, 0x7, 4, 10);

    /*
     * PPRE2 = 000 → APB2 / 1
     */
    MCAL_WRITE_BLOCK(RCC->CFGR, 0x7, 0, 13);


    /*
     * ============================================================
     * 6. Select PLL as SYSCLK
     * ============================================================
     *
     * SW : bits [1:0]
     *
     * 00 = HSI
     * 01 = HSE
     * 10 = PLL
     * 11 = reserved
     */

    MCAL_WRITE_BLOCK(RCC->CFGR, 0x3, 0x2, 0);

    /*
     * Wait until PLL becomes SYSCLK source
     *
     * SWS : bits [3:2]
     *
     * 10 = PLL
     */
    while (MCAL_EXTRACT_AREA(RCC->CFGR, 0x3, 2) != 0x2)
    {
    }
}

#include <stdio.h>
#include <string.h>
#include <common.h>
#include <mcal_uart.h>
#include <test.h>
#include <systick.h>
#include <stm32f411xe.h>
#include "macro.h"

#define BASIC       1
#define STRING_TEST 0

extern volatile uint8_t Uart2_Rx_Expired;

#if BASIC


uint8_t rx_data[10];


void uart_test_main(void)
{
    
    while(USART2->SR && USART_SR_RXNE)
    {
        (void)USART2->DR;
    }

    if(Uart2_Rx_Expired)
    {
        while(!MCAL_CHECK_BIT_SET(USART1->SR, 5))
        {
            if(mcal_uart_read(2, rx_data, 1, 0) == MCAL_UART_OK)
            {
                mcal_uart_write(2, rx_data, 1, 0);
                mcal_uart_write(2, (const uint8_t *)"\n", 1, 0);
            }
        }
        
       Uart2_Rx_Expired = 0;
    }
    delay_ms(2000);   
}
#endif

#if STRING_TEST
void uart_test_main(void)
{
    uint8_t rx_data[10];
    const char *msg = "mcal Uart String CallBack Test\n\n";
    
    /* Test 시작 문구 출력*/
    mcal_uart_init(2, 115200, 0, 0);
    mcal_uart_write(2,  (const uint8_t *)msg, (uint16_t)strlen(msg), 0);
    
    while(1)
    {
        mcal_uart_read(2, rx_data, 1, 0);
    }
    if( == MCAL_UART_OK)
    {
        mcal_uart_write(2, rx_data, 1, 0);
    }

    delay_ms(1000);   
}
#endif
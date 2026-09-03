#include <stdio.h>
#include <string.h>
#include <common.h>
#include <mcal_uart.h>
#include <pinmap_config.h>
#include <clock.h>
#include <test.h>

void Main(void)
{
    Clock_Init();


    while(1)
    {
        uart_test_main();
    }
        
}
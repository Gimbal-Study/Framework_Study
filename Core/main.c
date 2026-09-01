#include <stdio.h>
#include <string.h>
#include <common.h>
#include <mcal_uart.h>
#include <macro.h>
#include <pinmap_config.h>
#include <clock.h>

void Main(void)
{
    Clock_Init();

    const uint8_t StringArray[] = "asdfa;gnasas\n";
    const char *msg = "mcal_uart Test\n\n";

    mcal_uart_init(2, 115200, 0, 0);

    mcal_uart_write(2,  (const uint8_t *)msg, (uint16_t)strlen(msg), 0);
    mcal_uart_write(2,  StringArray, (uint16_t)strlen((const char *)StringArray), 0);
    
}
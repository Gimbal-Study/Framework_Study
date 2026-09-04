#include <test.h>

void Main(void)
{
    const char *msg = "mcal_uart Test\n\n";

    Clock_Init();
    mcal_uart_init(2, 115200, 0, 0);

    mcal_uart_write(2, (const uint8_t *)msg, (uint16_t)strlen(msg), 0);

    while(1)
    {
        uart_test_main();
    }
        
}
#include <stdio.h>
#include <string.h>
#include <common.h>
#include <mcal_uart.h>
#include <test.h>
#include <systick.h>
#include <stm32f411xe.h>
#include "macro.h"

#define BASIC 0
#define STRING_TEST 0
#define TEST_CODEX  1

extern volatile uint8_t Uart2_Rx_Expired;

#if BASIC

uint8_t rx_data[10];
mcal_uart_status_t mcal_write_result;
mcal_uart_status_t mcal_read_result;
const uint8_t *Uart2_Expired_msg = "Uart2_Expitred!!\n";

extern volatile uint8_t rx_cnt;

void uart_test_main(void)
{
    while (Uart2_Rx_Expired)
    {
        mcal_uart_write(2, Uart2_Expired_msg, strlen((char *)Uart2_Expired_msg), 0);

        mcal_read_result = mcal_uart_read(2, rx_data, rx_cnt, 0);

        if (mcal_read_result == MCAL_UART_OK)
        {
            mcal_uart_write(2, (const uint8_t *)&rx_cnt, 1, 0);
            printf("%d\n", rx_cnt);
            mcal_write_result = mcal_uart_write(2, rx_data, rx_cnt, 0);
        }

        else if(mcal_read_result == MCAL_UART_ERROR)
        {
             mcal_uart_write(2, (const uint8_t *)"Read_Error!!\n", strlen("Read_Error!!\n"), 0);
        }
        
        if(mcal_write_result == MCAL_UART_OK)
        {
            Uart2_Rx_Expired = 0;
            rx_cnt = 0;
            mcal_uart_write(2, (const uint8_t *)"UART CAll BACK SUCCESS!!\n", strlen("UART CAll BACK SUCCESS!!\n"), 0);
        }
        
        else if(mcal_write_result == MCAL_UART_ERROR)
        {
            mcal_uart_write(2, (const uint8_t *)"UART CAll BACK FAILED...\n", strlen("UART CAll BACK FAILED...\n"), 0);
        }
        
    }
}
#endif

#if STRING_TEST
void uart_test_main(void)
{
    uint8_t rx_data[10];
    const char *msg = "mcal Uart String CallBack Test\n\n";

    /* Test 시작 문구 출력*/
    mcal_uart_init(2, 115200, 0, 0);
    mcal_uart_write(2, (const uint8_t *)msg, (uint16_t)strlen(msg), 0);

    while (1)
    {
        mcal_uart_read(2, rx_data, 1, 0);
    }
    if (== MCAL_UART_OK)
    {
        mcal_uart_write(2, rx_data, 1, 0);
    }

    delay_ms(1000);
}
#endif

#if TEST_CODEX
/* Text echo testbench. Compile this INSTEAD OF Test/uart_test.c.
 * Core/main.c and the existing UART ISR/driver remain unchanged.
 */
#include <stdint.h>
#include <mcal_uart.h>

/* Inspect these in the debugger; do not print diagnostics into the echo. */
volatile uint32_t tb_rx_bytes = 0;
volatile uint32_t tb_tx_bytes = 0;
volatile uint32_t tb_skipped_nuls = 0;
volatile mcal_uart_status_t tb_last_read = MCAL_UART_OK;
volatile mcal_uart_status_t tb_last_write = MCAL_UART_OK;
volatile uint8_t tb_tx_failed = 0;

void uart_test_main(void)
{
    /* Current mcal_uart_read writes an extra NUL after the requested data. */
    uint8_t received[2];

    if (tb_tx_failed)
        return;

    /* A bounded batch returns control to the existing main loop. */
    for (uint16_t i = 0; i < 64U; ++i)
    {
        tb_last_read = mcal_uart_read(2U, received, 1U, 0U);
        /* The current driver uses ERROR for an empty queue, too. */
        if (tb_last_read != MCAL_UART_OK)
            return;

        ++tb_rx_bytes;

        /* Existing ISR inserts a synthetic NUL after LF.
         * Text-only test: discard all NULs, including any sent by the PC.
         */
        if (received[0] == 0U)
        {
            ++tb_skipped_nuls;
            continue;
        }

        tb_last_write = mcal_uart_write(2U, received, 1U, 0U);
        if (tb_last_write != MCAL_UART_OK)
        {
            tb_tx_failed = 1U;
            return;
        }
        /* Counts bytes handed to MCAL, not its inserted CR bytes. */
        ++tb_tx_bytes;
    }
}
#endif
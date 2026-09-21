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

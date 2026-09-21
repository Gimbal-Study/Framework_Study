/* Text batch echo; select this instead of either other uart_test_main. */
#include <common.h>
#include <mcal_uart.h>
#include <stm32f411xe.h>
#include <systick.h>

#define TB_BATCH_MAX 128U
#define TB_COLLECT_MS 2U

/* Test-only adapter: the current public MCAL API has no available-count API.
 * This test must be the ONLY consumer of uart2_q.
 */
extern queue_t uart2_q;

volatile uint16_t tb_last_read_len;
volatile uint16_t tb_last_write_len;
volatile uint32_t tb_batches;
volatile uint32_t tb_received;
volatile uint32_t tb_transmitted;
volatile uint32_t tb_skipped_nuls;
volatile mcal_uart_status_t tb_read_status;
volatile mcal_uart_status_t tb_write_status;
volatile uint8_t tb_failed;

static uint16_t available_count(void)
{
    uint32_t saved = __get_PRIMASK();
    __disable_irq();
    uint16_t front = uart2_q.front;
    uint16_t rear = uart2_q.rear;
    __set_PRIMASK(saved);
    return (uint16_t)((rear + QUEUE_MAX - front) % QUEUE_MAX);
}

void uart_test_main(void)
{
    /* Current read appends a NUL in addition to the requested length. */
    uint8_t buffer[TB_BATCH_MAX + 1U];

    if (tb_failed || available_count() == 0U)
        return;

    /* Fixed collection window, not an end-of-message detector.
     * IRQs remain enabled while bytes accumulate in the existing queue.
     */
    delay_ms(TB_COLLECT_MS);

    uint16_t read_len = available_count();
    if (read_len > TB_BATCH_MAX)
        read_len = TB_BATCH_MAX;
    if (read_len == 0U)
        return;

    tb_last_read_len = read_len;
    tb_last_write_len = 0U;
    tb_read_status = mcal_uart_read(2U, buffer, read_len, 0U);
    if (tb_read_status != MCAL_UART_OK)
    {
        tb_failed = 1U; /* Do not hide partial-consumption errors. */
        return;
    }
    tb_received += read_len;

    uint16_t write_len = 0U;
    for (uint16_t i = 0U; i < read_len; ++i)
    {
        /* Existing ISR inserts NUL after LF. Text-only: discard all NULs. */
        if (buffer[i] == 0U)
            ++tb_skipped_nuls;
        else
            buffer[write_len++] = buffer[i];
    }

    tb_last_write_len = write_len;
    if (write_len != 0U)
    {
        tb_write_status = mcal_uart_write(2U, buffer, write_len, 0U);
        if (tb_write_status != MCAL_UART_OK)
        {
            tb_failed = 1U;
            return;
        }
        tb_transmitted += write_len;
    }
    ++tb_batches;
}

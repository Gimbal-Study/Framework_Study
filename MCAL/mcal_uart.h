

//author: sgHyeon
//date: 2026/08/24
//version: 1


#ifndef MCAL_UART_H
#define MCAL_UART_H

#ifdef __cplusplus
extern "C" {
#endif


#include "mcal_common.h"


/* =========================================================================
 * UART Instance
 * ========================================================================= */

typedef uint8_t mcal_uart_instance_t;


/* =========================================================================
 * UART State
 * ========================================================================= */

typedef enum
{
    MCAL_UART_STATE_UNINITIALIZED = 0U,
    MCAL_UART_STATE_READY,
    MCAL_UART_STATE_BUSY,
    MCAL_UART_STATE_ERROR

} mcal_uart_state_t;


/* =========================================================================
 * UART Error
 * ========================================================================= */

typedef enum
{
    MCAL_UART_ERROR_NONE = 0U,
    MCAL_UART_ERROR_OVERRUN,
    MCAL_UART_ERROR_FRAMING,
    MCAL_UART_ERROR_PARITY,
    MCAL_UART_ERROR_NOISE

} mcal_uart_error_t;


/* =========================================================================
 * UART Callback
 * ========================================================================= */

typedef void (*mcal_uart_tx_callback_t)(mcal_uart_instance_t instance);

typedef void (*mcal_uart_rx_callback_t)(mcal_uart_instance_t instance);

typedef void (*mcal_uart_error_callback_t)(mcal_uart_instance_t instance, mcal_uart_error_t error);


/* =========================================================================
 * Initialization
 * ========================================================================= */

void mcal_uart_init(mcal_uart_instance_t instance);

void mcal_uart_deinit(mcal_uart_instance_t instance);


/* =========================================================================
 * Blocking API
 * ========================================================================= */

mcal_status_t mcal_uart_transmit(mcal_uart_instance_t instance, const uint8_t *data, size_t length, uint32_t timeout);

mcal_status_t mcal_uart_receive(mcal_uart_instance_t instance, uint8_t *data, size_t length, uint32_t timeout);


/* =========================================================================
 * Non-blocking API
 * ========================================================================= */

mcal_status_t mcal_uart_write(mcal_uart_instance_t instance, const uint8_t *data, size_t length);

mcal_status_t mcal_uart_read(mcal_uart_instance_t instance, uint8_t *data, size_t length);


/* =========================================================================
 * Callback
 * ========================================================================= */

void mcal_uart_register_tx_callback(mcal_uart_instance_t instance, mcal_uart_tx_callback_t callback);

void mcal_uart_register_rx_callback(mcal_uart_instance_t instance, mcal_uart_rx_callback_t callback);

void mcal_uart_register_error_callback(mcal_uart_instance_t instance, mcal_uart_error_callback_t callback);


/* =========================================================================
 * State
 * ========================================================================= */

mcal_uart_state_t mcal_uart_get_state(mcal_uart_instance_t instance);


#ifdef __cplusplus
}
#endif

#endif /* MCAL_UART_H */

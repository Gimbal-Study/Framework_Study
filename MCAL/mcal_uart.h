#ifndef MCAL_UART_H
#define MCAL_UART_H

typedef enum{
	NONE,
	ODD,
	EVEN
	} mcal_parity_t;
	
typedef enum {
    MCAL_UART_OK = 0,
    MCAL_UART_ERROR,
    MCAL_UART_BUSY,
    MCAL_UART_TIMEOUT    
} mcal_uart_status_t;

mcal_uart_status_t mcal_uart_init(uint8_t uart_instance, uint32_t baud, mcal_parity_t parity, uint8_t stopbit);
mcal_uart_status_t mcal_uart_write(uint8_t uart_instance,  const uint8_t *data, uint16_t len, uint16_t timeout);
mcal_uart_status_t mcal_uart_read(uint8_t uart_instance, uint8_t *data, uint16_t len, uint16_t timeout);

#endif /* MCAL_UART_H */
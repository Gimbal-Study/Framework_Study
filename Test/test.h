#ifndef TEST_H
#define TEST_H

#include <stdio.h>
#include <string.h>
#include <common.h>
#include <mcal_uart.h>
#include <pinmap_config.h>
#include <clock.h>
#include <systick.h>

int gpio_test_main(void);
void uart_test_main(void);

#endif /* TEST_H */
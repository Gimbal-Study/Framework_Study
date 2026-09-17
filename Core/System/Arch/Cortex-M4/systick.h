/*
 * Author  : sghyeon
 * Version : v1.0
 * Target  : STM32F411RE
 */

#ifndef SYSTICK_H
#define SYSTICK_H

#include <stdint.h>

void systick_run(unsigned int msec);

void systick_run_interrupt(unsigned int msec);

uint32_t systick_get_elapsed_msec(void);

int systick_check_timeout(void);

unsigned int systick_get_time(void);

unsigned int systick_get_load_time(void);

void systick_stop(void);

void delay_ms(unsigned int msec);

void systick_irq_handler(void);

#endif /* SYSTICK_H */

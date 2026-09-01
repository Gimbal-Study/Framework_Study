/*
 * Author  : sghyeon
 * Version : v1.0
 * Target  : STM32F411RE
 */

#ifndef SYSTICK_H
#define SYSTICK_H


void systick_run(unsigned int msec);

int systick_check_timeout(void);

unsigned int systick_get_time(void);

unsigned int systick_get_load_time(void);

void systick_stop(void);

void delay_ms(unsigned int msec);


#endif /* SYSTICK_H */

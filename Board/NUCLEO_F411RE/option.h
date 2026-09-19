#ifndef OPTION_H
#define OPTION_H

#define SYSCLK  96000000
#define HCLK	SYSCLK
#define PCLK2	HCLK
#define PCLK1	HCLK/2
#define TIMXCLK ((HCLK == PCLK1)?(PCLK1):(PCLK1*2))

#define RAM_START	0x20000000
#define RAM_END		0x20020000
#define HEAP_BASE	(((unsigned int)&__ZI_LIMIT__ + 0x7) & ~0x7)
#define HEAP_SIZE	(4*1024)
#define HEAP_LIMIT	(HEAP_BASE + HEAP_SIZE)
#define STACK_LIMIT	(HEAP_LIMIT + 8)
#define STACK_BASE	(RAM_END + 1)
#define STACK_SIZE	(STACK_BASE - STACK_LIMIT)

/*
 * 이 보드의 공통 시간원으로 TIM4 전체를 예약한다.
 * PWM, UART timeout 등 다른 기능에서 TIM4를 사용하지 않는다.
 *
 * 아래 항목들은 하나의 하드웨어 설정 묶음이다.
 */
#define BOARD_TIMEBASE_TIMER_INSTANCE  4U
#define BOARD_TIMEBASE_TIMER           TIM4
#define BOARD_TIMEBASE_TIMER_IRQ       TIM4_IRQn
#define BOARD_TIMEBASE_TIMER_HANDLER   TIM4_IRQHandler

#define BOARD_TIMEBASE_CLOCK_EN_REG    (RCC->APB1ENR)
#define BOARD_TIMEBASE_CLOCK_EN_MASK   RCC_APB1ENR_TIM4EN

#define BOARD_TIMEBASE_TIMER_CLOCK_HZ  (TIMXCLK)

#endif /* OPTION_H */
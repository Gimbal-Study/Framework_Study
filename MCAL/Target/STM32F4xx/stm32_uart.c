#include <common.h>
#include <mcal_uart.h>
#include <macro.h>
#include <pinmap_config.h>
#include <string.h>
#include <option.h>
#include <mcal_timer.h>

#define UART_TIMER_INSTANCE 2U

// static void Uart_Send_Byte(USART_TypeDef *uart_instance, char data);
bool uart_timeout_tick(uint8_t timer_instance, uint16_t time);

static volatile uint32_t g_uart_timer_ms = 0U;
static bool g_uart_timer_started = false;

static bool uart_timeout_expired(uint32_t started, uint16_t timeout)
{
  return (uint32_t)(g_uart_timer_ms - started) >= (uint32_t)timeout;
}

/*
 * TIM2 인터럽트와 UART 수신 인터럽트가 실행될 수 있는
 * 메인 코드에서 호출하도록 제한한다.
 */
static bool uart_timeout_context_ready(void)
{
  return g_uart_timer_started &&
         (__get_IPSR() == 0U) &&
         (__get_PRIMASK() == 0U) &&
         (__get_BASEPRI() == 0U) &&
         (__get_FAULTMASK() == 0U);
}

/* 송신 레지스터 여유(TXE) 또는 실제 전송 완료(TC)를 기다린다. */
static mcal_uart_status_t uart_wait_flag(
    USART_TypeDef *uart,
    uint32_t flag,
    uint32_t started,
    uint16_t timeout)
{
  for (;;)
  {
    if (uart_timeout_expired(started, timeout))
      return MCAL_UART_TIMEOUT;

    if ((uart->SR & flag) != 0U)
      return MCAL_UART_OK;
  }
}

/*USARTx_Pin_Map for STM32*/
static const Stm32_UartPinConfigType uart1_pinmap =
    {
        .tx_port = GPIOA,
        .tx_pin = 9U,
        .rx_port = GPIOA,
        .rx_pin = 10U,
        .alternate_function = 7U,
        .USART_target = USART1};

static const Stm32_UartPinConfigType uart2_pinmap =
    {
        .tx_port = GPIOA,
        .tx_pin = 2U,
        .rx_port = GPIOA,
        .rx_pin = 3U,
        .alternate_function = 7U,
        .USART_target = USART2};

static const Stm32_UartPinConfigType uart6_pinmap =
    {
        .tx_port = GPIOA,
        .tx_pin = 11U,
        .rx_port = GPIOA,
        .rx_pin = 12U,
        .alternate_function = 8U,
        .USART_target = USART6};

typedef enum
{
  UART_UNDESIRED_GPIO = -1,
  UART_GPIOA = 0,
  UART_GPIOB,
  UART_GPIOC,
  UART_GPIOD,
  UART_GPIOE,
  UART_GPIOF,
  UART_GPIOG,
  UART_GPIOH
} uart_gpio_t;

queue_t uart1_q;
queue_t uart2_q;
queue_t uart6_q;

volatile uint8_t Uart2_Rx_Expired = 0;
volatile uint8_t rx_cnt = 0;

uart_gpio_t uart_tx_gpio_confirm(uint8_t uart_instance, const Stm32_UartPinConfigType *uart_pinmap)
{
  switch (uart_instance)
  {
  case 1:
    if (uart_pinmap->tx_port == GPIOA)
      return UART_GPIOA;
    else if (uart_pinmap->tx_port == GPIOB)
      return UART_GPIOB;
    else
      return UART_UNDESIRED_GPIO;

  case 2:
    if (uart_pinmap->tx_port == GPIOA)
      return UART_GPIOA;
    else if (uart_pinmap->tx_port == GPIOD)
      return UART_GPIOD;
    else
      return UART_UNDESIRED_GPIO;

  case 6:
    if (uart_pinmap->tx_port == GPIOA)
      return UART_GPIOA;
    else if (uart_pinmap->tx_port == GPIOC)
      return UART_GPIOC;
    else
      return UART_UNDESIRED_GPIO;

  default:
    return UART_UNDESIRED_GPIO;
  }
}

uart_gpio_t uart_rx_gpio_confirm(uint8_t uart_instance, const Stm32_UartPinConfigType *uart_pinmap)
{
  switch (uart_instance)
  {
  case 1:
    if (uart_pinmap->rx_port == GPIOA)
      return UART_GPIOA;
    else if (uart_pinmap->rx_port == GPIOB)
      return UART_GPIOB;
    else
      return UART_UNDESIRED_GPIO;

  case 2:
    if (uart_pinmap->rx_port == GPIOA)
      return UART_GPIOA;
    else if (uart_pinmap->rx_port == GPIOD)
      return UART_GPIOD;
    else
      return UART_UNDESIRED_GPIO;

  case 6:
    if (uart_pinmap->rx_port == GPIOA)
      return UART_GPIOA;
    else if (uart_pinmap->rx_port == GPIOC)
      return UART_GPIOC;
    else
      return UART_UNDESIRED_GPIO;

  default:
    return UART_UNDESIRED_GPIO;
  }
}

mcal_uart_status_t mcal_uart_init(uint8_t uart_instance, uint32_t baud, mcal_parity_t parity, uint8_t stopbit)
{
  double div;
  unsigned int mant;
  unsigned int frac;
  uart_gpio_t uart_tx_gpio;
  uart_gpio_t uart_rx_gpio;
  uint8_t af_num = 0;
  IRQn_Type uart_irq;

  static const Stm32_UartPinConfigType *uart_s;

  switch (uart_instance)
  {
  case 1:
    MCAL_SET_BIT(RCC->APB2ENR, 4U); // USART2 ON
    uart_s = &uart1_pinmap;
    uart_irq = USART1_IRQn;

    queue_init(&uart1_q);
    break;

  case 2:
    MCAL_SET_BIT(RCC->APB1ENR, 17U); // USART2 ON
    uart_s = &uart2_pinmap;
    uart_irq = USART2_IRQn;

    queue_init(&uart2_q);
    break;

  case 6:
    MCAL_SET_BIT(RCC->APB2ENR, 5U); // USART2 ON
    uart_s = &uart6_pinmap;
    uart_irq = USART6_IRQn;
    af_num = 1;

    queue_init(&uart6_q);
    break;

  default:
    return MCAL_UART_ERROR;
    break;
  }

  if ((uart_tx_gpio = uart_tx_gpio_confirm(uart_instance, uart_s)) == UART_UNDESIRED_GPIO)
  {
    return MCAL_UART_ERROR;
  }
  if ((uart_rx_gpio = uart_rx_gpio_confirm(uart_instance, uart_s)) == UART_UNDESIRED_GPIO)
  {
    return MCAL_UART_ERROR;
  }

  /* UART_TX Init */
  MCAL_SET_BIT(RCC->AHB1ENR, uart_tx_gpio);
  MCAL_WRITE_BLOCK(uart_s->tx_port->MODER, 0x3, 0x2, uart_s->tx_pin * 2);                              // uart2의 tx_pin -> ALT
  MCAL_WRITE_BLOCK(uart_s->tx_port->AFR[af_num], 0xf, uart_s->alternate_function, uart_s->tx_pin * 4); // tx_pin => AF07
  MCAL_WRITE_BLOCK(uart_s->tx_port->PUPDR, 0x3, 0x1, uart_s->tx_pin * 2);                              // tx_pin => Pull-up

  /* UART_RX Init*/
  MCAL_SET_BIT(RCC->AHB1ENR, uart_rx_gpio);
  MCAL_WRITE_BLOCK(uart_s->rx_port->MODER, 0x3, 0x2, uart_s->rx_pin * 2);                              // uart2의 tx_pin -> ALT
  MCAL_WRITE_BLOCK(uart_s->rx_port->AFR[af_num], 0xf, uart_s->alternate_function, uart_s->rx_pin * 4); // tx_pin => AF07
  MCAL_WRITE_BLOCK(uart_s->rx_port->PUPDR, 0x3, 0x1, uart_s->rx_pin * 2);                              // tx_pin => Pull-up

  /* Baudrate, 기타 초기 설정*/
  div = PCLK1 / (16. * baud);
  mant = (int)div;
  frac = (int)((div - mant) * 16. + 0.5);
  mant += frac >> 4;
  frac &= 0xf;
  uart_s->USART_target->BRR = (mant << 4) | (frac << 0);
  uart_s->USART_target->CR1 = (1 << 13) | (0 << 12) | (parity << 10) | (1 << 3) | (1 << 2);
  uart_s->USART_target->CR2 = stopbit << 12;
  uart_s->USART_target->CR3 = 0;

  // USARTx TX, RX Interrupt Enable
  uart_s->USART_target->CR1 |= 0x1 << 5;
  // NIVC Pending Clear
  NVIC_ClearPendingIRQ(uart_irq);
  // NVIC Interrupt Enable
  NVIC_EnableIRQ(uart_irq);

  /* UART1/2/6이 공유하는 TIM2를 최초 한 번만 시작 */
  if (!g_uart_timer_started)
  {
    g_uart_timer_ms = 0U;

    if (!mcal_timer_repeat_init(
            UART_TIMER_INSTANCE, 1000000U, 1000U))
    {
      return MCAL_UART_ERROR;
    }

    if (!mcal_timer_start(UART_TIMER_INSTANCE))
    {
      mcal_timer_stop(UART_TIMER_INSTANCE);
      return MCAL_UART_ERROR;
    }

    g_uart_timer_started = true;
  }

  return MCAL_UART_OK;
}

/*uart_write*/
mcal_uart_status_t mcal_uart_write(uint8_t uart_instance, const uint8_t *data, uint16_t len, uint32_t timeout)
{
  mcal_timer_start(2);
  static const Stm32_UartPinConfigType *uart_s;
  mcal_uart_status_t status;
  uint32_t started;

  switch (uart_instance)
  {
  case 1:
    uart_s = &uart1_pinmap;
    break;

  case 2:
    uart_s = &uart2_pinmap;
    break;

  case 6:
    uart_s = &uart6_pinmap;
    break;

  default:
    return MCAL_UART_ERROR;
    break;
  }

  if (uart_s == NULL || data == NULL || len == 0U)
    return MCAL_UART_ERROR;

  if (timeout == 0U)
    return MCAL_UART_TIMEOUT;

  if (!uart_timeout_context_ready())
    return MCAL_UART_ERROR;

  /* UART와 송신 기능이 활성화되어 있어야 한다. */
  if ((uart_s->USART_target->CR1 & (USART_CR1_UE | USART_CR1_TE)) !=
      (USART_CR1_UE | USART_CR1_TE))
  {
    return MCAL_UART_ERROR;
  }

  /* 전체 송신에 사용할 시작 시각: 한 번만 저장 */
  started = g_uart_timer_ms;

  for (uint16_t i = 0; i < len; i++)
  {
    /* 기존 줄바꿈 처리 유지: LF 앞에 CR 추가 */
    if (data[i] == (uint8_t)'\n')
    {
      status = uart_wait_flag(
          uart_s->USART_target, USART_SR_TXE, started, timeout);

      if (status != MCAL_UART_OK)
        return status;

      uart_s->USART_target->DR = (uint8_t)'\r';
    }

    status = uart_wait_flag(
        uart_s->USART_target, USART_SR_TXE, started, timeout);

    if (status != MCAL_UART_OK)
      return status;

    uart_s->USART_target->DR = data[i];
  }

  /* 마지막 바이트가 핀으로 전송 완료될 때까지 확인 */
  return uart_wait_flag(
      uart_s->USART_target, USART_SR_TC, started, timeout);
}
/*
  {
    Uart_Send_Byte(uart_s->USART_target, *data8b_ptr++);
  }
  return MCAL_UART_OK;
  }
 */

mcal_uart_status_t mcal_uart_read(uint8_t uart_instance, uint8_t *data, uint16_t len, uint32_t timeout)
{
  static const Stm32_UartPinConfigType *uart_s;
  queue_t *uart_q;
  IRQn_Type uart_irq;
  uint32_t started;

  switch (uart_instance)
  {
  case 1:
    uart_s = &uart1_pinmap;
    uart_q = &uart1_q;
    uart_irq = USART1_IRQn;
    
    break;

  case 2:
    uart_s = &uart2_pinmap;
    uart_q = &uart2_q;
    uart_irq = USART2_IRQn;
    break;

  case 6:
    uart_s = &uart6_pinmap;
    uart_q = &uart6_q;
    uart_irq = USART6_IRQn;
    break;

  default:
    return MCAL_UART_ERROR;
  }

  if (uart_s == NULL || data == NULL || len == 0U)
    return MCAL_UART_ERROR;

  if (timeout == 0U)
    return MCAL_UART_TIMEOUT;

  if (!uart_timeout_context_ready())
    return MCAL_UART_ERROR;

  if ((uart_s->USART_target->CR1 &
       (USART_CR1_UE | USART_CR1_RE | USART_CR1_RXNEIE)) !=
      (USART_CR1_UE | USART_CR1_RE | USART_CR1_RXNEIE))
  {
    return MCAL_UART_ERROR;
  }

  if (NVIC_GetEnableIRQ(uart_irq) == 0U)
    return MCAL_UART_ERROR;

  started = g_uart_timer_ms;

  for (uint16_t i = 0U; i < len; ++i)
  {
    for (;;)
    {
      uint32_t saved_irq;
      bool received;

      if (uart_timeout_expired(started, timeout))
        return MCAL_UART_TIMEOUT;

      /*
       * 큐를 꺼내는 짧은 구간만 보호한다.
       * 데이터가 도착하기를 기다리는 동안에는
       * 인터럽트를 허용해야 한다.
       */
      saved_irq = __get_PRIMASK();
      __disable_irq();

      received = read_queue(uart_q, &data[i]);

      __set_PRIMASK(saved_irq);

      if (received)
        break;
    }
  }

  return MCAL_UART_OK;
}

#if 0
  for (int i = 0; i < len; i++)
  {
    if (queue_empty(uart_q))
      return MCAL_UART_ERROR;

    if (!read_queue(uart_q, data8b_ptr++))
      return MCAL_UART_ERROR;
  }

  *data8b_ptr = '\0';

  return MCAL_UART_OK;
}
#endif



#if 0
static void Uart_Send_Byte(USART_TypeDef *uart_instance, char data)
{
  if (data == '\n')
  {
    while (!MCAL_CHECK_BIT_SET(uart_instance->SR, 7))
      ;
    uart_instance->DR = 0x0d;
  }

  while (!MCAL_CHECK_BIT_SET(uart_instance->SR, 7))
    ;
  uart_instance->DR = data;
}
mcal_uart_status_t mcal_uart_Rx_Handler(uint8_t uart_instance)
{
  switch(uart_instance)
  {
    case 1:
      void USART1_IRQHandler(void)
      {
        if(MCAL_CHECK_BIT_SET(USART1->SR, 5))
        {
          uint8_t data = USART1->DR;

          insert_queue(&uart1_q, data);
          NVIC_ClearPendingIRQ(USART1_IRQn);
        }
      }
      return MCAL_UART_OK;
      break;
      
    case 2:
      void USART2_IRQHandler(void)
      {
        if(MCAL_CHECK_BIT_SET(USART2->SR, 5))
        {
          uint8_t data = USART2->DR;

          insert_queue(&uart2_q, data);
          NVIC_ClearPendingIRQ(USART2_IRQn);
        }
      }
      return MCAL_UART_OK;
      break;   
        
    case 6:
      void USART6_IRQHandler(void)
      {
        if(MCAL_CHECK_BIT_SET(USART6->SR, 5))
        {
          uint8_t data = USART6->DR;

          insert_queue(&uart6_q, data);
          NVIC_ClearPendingIRQ(USART6_IRQn);
        }
      }
      return MCAL_UART_OK;
      break;
     
    default:
      return MCAL_UART_ERROR;
      break;
  }  
}
#endif

bool uart_timeout_tick(uint8_t timer_instance, uint16_t time)
{
  bool tick_result;
  mcal_timer_oneshot_init(timer_instance, 50000, 40000);
  mcal_timer_int_enable(timer_instance, 1);
  tick_result = mcal_timer_start(timer_instance);

  return tick_result;
}

void USART1_IRQHandler(void)
{
  if (MCAL_CHECK_BIT_SET(USART1->SR, 5))
  {
    uint8_t data = USART1->DR;

    insert_queue(&uart1_q, data);

    NVIC_ClearPendingIRQ(USART1_IRQn);
  }
}

// const uint8_t *Uart2_IRQ_msg = "Uart_Inturrpt Occured\n";
// const uint8_t *Uart2_Expired_msg = "Uart2_Expitred!!\n";
void USART2_IRQHandler(void)
{
  uint8_t data = USART2->DR;

  if (!insert_queue(&uart2_q, data))
    return;
  if (data == '\n')
  {
    insert_queue(&uart2_q, '\0');
    Uart2_Rx_Expired = 1;
  }

  NVIC_ClearPendingIRQ(USART2_IRQn);
}

void USART6_IRQHandler(void)
{
  if (MCAL_CHECK_BIT_SET(USART6->SR, 5))
  {
    uint8_t data = USART6->DR;

    insert_queue(&uart6_q, data);
    NVIC_ClearPendingIRQ(USART6_IRQn);
  }
}

/* TIM2는 UART timeout용으로 전용 사용 */
void TIM2_IRQHandler(void)
{
  if ((TIM2->SR & TIM_SR_UIF) != 0U)
  {
    TIM2->SR = 0U;
    ++g_uart_timer_ms;
  }
}
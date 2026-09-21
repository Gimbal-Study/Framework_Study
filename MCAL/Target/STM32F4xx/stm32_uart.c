#include <common.h>
#include <mcal_uart.h>
#include <macro.h>
#include <pinmap_config.h>
#include <string.h>
#include <option.h>
#include <mcal_timer.h>

void Uart_Send_Byte(USART_TypeDef *uart_instance, char data);
bool uart_timeout_tick(uint8_t timer_instance, uint16_t time);

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

  return MCAL_UART_OK;
}

/*uart_write*/
mcal_uart_status_t mcal_uart_write(uint8_t uart_instance, const uint8_t *data, uint16_t len, uint16_t timeout)
{
  mcal_timer_start(2);
  uint8_t *data8b_ptr = (uint8_t *)data;
  static const Stm32_UartPinConfigType *uart_s;

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

  for (int i = 0; i < len; i++)
  {
    Uart_Send_Byte(uart_s->USART_target, *data8b_ptr++);
  }

  return MCAL_UART_OK;
}

mcal_uart_status_t mcal_uart_read(uint8_t uart_instance, uint8_t *data, uint16_t len, uint16_t timeout)
{
  // mcal_timer_
  queue_t *uart_q;
  uint8_t *data8b_ptr = data;

  switch (uart_instance)
  {
  case 1:
    uart_q = &uart1_q;
    break;

  case 2:
    uart_q = &uart2_q;
    break;

  case 6:
    uart_q = &uart6_q;
    break;

  default:
    return MCAL_UART_ERROR;
  }

  // while(시간이 timeout이 넘었거나 || uart_q-> rear - uart_q->front == len)

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

void Uart_Send_Byte(USART_TypeDef *uart_instance, char data)
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

#if 0
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

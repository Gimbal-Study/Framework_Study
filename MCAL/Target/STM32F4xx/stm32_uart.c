#include <common.h>
#include <mcal_uart.h>
#include <macro.h>
#include <pinmap_config.h>
#include <string.h>
#include <option.h>

void Uart_Send_Byte(USART_TypeDef *uart_instance, char data);

/*USARTx_Pin_Map for STM32*/
static const Stm32_UartPinConfigType uart1_pinmap =
{
  .tx_port = GPIOA,
  .tx_pin  = 9U,
  .rx_port = GPIOA,
  .rx_pin  = 10U,
  .alternate_function = 7U,
  .USART_target = USART1
};

static const Stm32_UartPinConfigType uart2_pinmap =
{
  .tx_port = GPIOA,
  .tx_pin  = 2U,
  .rx_port = GPIOA,
  .rx_pin  = 3U,
  .alternate_function = 7U,
  .USART_target = USART2
};

static const Stm32_UartPinConfigType uart6_pinmap =
{
  .tx_port = GPIOA,
  .tx_pin  = 11U,
  .rx_port = GPIOA,
  .rx_pin  = 12U,
  .alternate_function = 8U,
  .USART_target = USART6
};

typedef enum{
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

uart_gpio_t uart_tx_gpio_confirm(uint8_t uart_instance, const Stm32_UartPinConfigType *uart_pinmap)
{
  switch(uart_instance)
  {
    case 1:
      if(uart_pinmap->tx_port == GPIOA)
        return UART_GPIOA;
      else if(uart_pinmap->tx_port == GPIOB)
        return UART_GPIOB;
      else
        return UART_UNDESIRED_GPIO;

    case 2:
      if(uart_pinmap->tx_port == GPIOA)
        return UART_GPIOA;
      else if(uart_pinmap->tx_port == GPIOD)
        return UART_GPIOD;
      else
        return UART_UNDESIRED_GPIO;

    case 6:
      if(uart_pinmap->tx_port == GPIOA)
        return UART_GPIOA;
      else if(uart_pinmap->tx_port == GPIOC)
        return UART_GPIOC;
      else
        return UART_UNDESIRED_GPIO;
    
    default:
      return UART_UNDESIRED_GPIO;
  }
}

uart_gpio_t uart_rx_gpio_confirm(uint8_t uart_instance, const Stm32_UartPinConfigType *uart_pinmap)
{
  switch(uart_instance)
  {
    case 1:
      if(uart_pinmap->rx_port == GPIOA)
        return UART_GPIOA;
      else if(uart_pinmap->rx_port == GPIOB)
        return UART_GPIOB;
      else
        return UART_UNDESIRED_GPIO;

    case 2:
      if(uart_pinmap->rx_port == GPIOA)
        return UART_GPIOA;
      else if(uart_pinmap->rx_port == GPIOD)
        return UART_GPIOD;
      else
        return UART_UNDESIRED_GPIO;

    case 6:
      if(uart_pinmap->rx_port == GPIOA)
        return UART_GPIOA;
      else if(uart_pinmap->rx_port == GPIOC)
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

  switch(uart_instance)
  {
    case 1:
      MCAL_SET_BIT(RCC->APB2ENR, 4U);                // USART2 ON
      uart_s = &uart1_pinmap;
      uart_irq = USART1_IRQn;
      break;
      
    case 2:
      MCAL_SET_BIT(RCC->APB1ENR, 17U);                // USART2 ON
      uart_s = &uart2_pinmap;
      uart_irq = USART2_IRQn;
      break;   
        
    case 6:
      MCAL_SET_BIT(RCC->APB2ENR, 5U);                // USART2 ON
      uart_s = &uart6_pinmap;
      uart_irq = USART6_IRQn;
      af_num = 1;
      break;
     
    default:
      return MCAL_UART_ERROR;
      break;
  }

  if((uart_tx_gpio = uart_tx_gpio_confirm(uart_instance, uart_s)) == UART_UNDESIRED_GPIO)
  {
    return MCAL_UART_ERROR;
  }
  if((uart_rx_gpio = uart_rx_gpio_confirm(uart_instance, uart_s)) == UART_UNDESIRED_GPIO)
  {
    return MCAL_UART_ERROR;
  }

  /* UART_TX Init */
  MCAL_SET_BIT(RCC->AHB1ENR, uart_tx_gpio);
  MCAL_WRITE_BLOCK(uart_s->tx_port->MODER, 0x3, 0x2, uart_s->tx_pin * 2); // uart2의 tx_pin -> ALT
  MCAL_WRITE_BLOCK(uart_s->tx_port->AFR[af_num], 0xf, uart_s->alternate_function, uart_s->tx_pin * 4);  // tx_pin => AF07
  MCAL_WRITE_BLOCK(uart_s->tx_port->PUPDR, 0x3, 0x1, uart_s->tx_pin * 2); // tx_pin => Pull-up

  /* UART_RX Init*/
  MCAL_SET_BIT(RCC->AHB1ENR, uart_rx_gpio);
  MCAL_WRITE_BLOCK(uart_s->rx_port->MODER, 0x3, 0x2, uart_s->rx_pin * 2); // uart2의 tx_pin -> ALT
  MCAL_WRITE_BLOCK(uart_s->rx_port->AFR[af_num], 0xf, uart_s->alternate_function, uart_s->rx_pin * 4);  // tx_pin => AF07
  MCAL_WRITE_BLOCK(uart_s->rx_port->PUPDR, 0x3, 0x1, uart_s->rx_pin * 2); // tx_pin => Pull-up

  /* Baudrate, 기타 초기 설정*/
  div = PCLK1/(16. * baud);
  mant = (int)div;
  frac = (int)((div - mant) * 16. + 0.5);
  mant += frac >> 4;
  frac &= 0xf;
  uart_s->USART_target->BRR = (mant<<4)|(frac<<0);
  uart_s->USART_target->CR1 = (1 << 13)|(0 << 12)|(parity << 10)|(1 << 3)|(1 << 2);
  uart_s->USART_target->CR2 = stopbit << 12;
  uart_s->USART_target->CR3 = 0;


	// USARTx TX, RX Interrupt Enable
  uart_s->USART_target -> CR1 |= 0x1 << 5;
	// NIVC Pending Clear
  NVIC_ClearPendingIRQ(uart_irq);
	// NVIC Interrupt Enable
  NVIC_EnableIRQ(uart_irq);

  return MCAL_UART_OK;
}



/*uart_write*/
mcal_uart_status_t mcal_uart_write(uint8_t uart_instance,  const uint8_t *data, uint16_t len, uint16_t timeout)
{
  static const Stm32_UartPinConfigType *uart_s;

  switch(uart_instance)
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

  for(int i = 0; i < len; i ++)
  {
    Uart_Send_Byte(uart_s->USART_target, *data++);
  }

  return MCAL_UART_OK;
  
}

mcal_uart_status_t mcal_uart_read(uint8_t uart_instance, uint8_t *data, uint16_t len, uint16_t timeout)
{
  queue_t uart_q;
  
  switch(uart_instance)
  {
    case 1:
      uart_q = uart1_q;
      break;
      
    case 2:
      uart_q = uart2_q;
      break;   
        
    case 6:
      uart_q = uart6_q;
      break;
     
    default:
      return MCAL_UART_ERROR;
      break;
  }
  for(int i = 0; i < len; i ++)
  {
    read_queue(&uart_q, data++);
  }

  return MCAL_UART_OK;
}

void Uart_Send_Byte(USART_TypeDef *uart_instance, char data)
{
  if(data == '\n')
  {
    while(!MCAL_CHECK_BIT_SET(uart_instance->SR, 7));
    uart_instance->DR = 0x0d;
  }

  while(!MCAL_CHECK_BIT_SET(uart_instance->SR, 7));
  uart_instance->DR = data;
}


// void Uart_Send_String(USART_TypeDef *uart_instance, char *pt)
// {
//   while(*pt != 0)
//   {
//     Uart_Send_Byte(uart_instance, *pt++);
//   }
// }


#if 0
void Uart2_Send_Byte(char data)
{
  if(data == '\n')
  {
    while(!Macro_Check_Bit_Set(USART2->SR, 7));
    USART2->DR = 0x0d;
  }

  while(!Macro_Check_Bit_Set(USART2->SR, 7));
  USART2->DR = data;
}

void Uart2_RX_Interrupt_Enable(int en)
{
  if(en)
  {
    (void)USART2 -> DR;
		// USART2 RX Interrupt Enable
    USART2 -> CR1 |= 0x1 << 5;
		// NIVC Pending Clear
    NVIC_ClearPendingIRQ(38);
		// NVIC Interrupt Enable
    NVIC_EnableIRQ(38);

  }
  else
  {
    Macro_Clear_Bit(USART2->CR1, 5);
    NVIC_DisableIRQ(38);
  }
}
#endif

#if 0
void Uart1_Init(int baud)
{
  double div;
  unsigned int mant;
  unsigned int frac;

  Macro_Set_Bit(RCC->AHB1ENR, 0);                   // PA9,10
  Macro_Set_Bit(RCC->APB2ENR, 4);                   // USART1 ON
  Macro_Write_Block(GPIOA->MODER, 0xf, 0xa, 18);    // PA9,10 => ALT
  Macro_Write_Block(GPIOA->AFR[1], 0xff, 0x77, 4);  // PA9,10 => AF07
  Macro_Write_Block(GPIOA->PUPDR, 0xf, 0x5, 18);    // PA9,10 => Pull-Up
  
  volatile unsigned int t = GPIOA->LCKR & 0x7FFF;
  GPIOA->LCKR = (0x1<<16)|t|(0x3<<9);               // Lock PA9, 10 Configuration
  GPIOA->LCKR = (0x0<<16)|t|(0x3<<9);
  GPIOA->LCKR = (0x1<<16)|t|(0x3<<9);
  t = GPIOA->LCKR;

  div = PCLK2 / (16. * baud);
  mant = (int)div;
  frac = (int)((div - mant) * 16 + 0.5);
  mant += frac >> 4;
  frac &= 0xf;
  USART1->BRR = (mant<<4)|(frac<<0);

  USART1->CR1 = (1<<13)|(0<<12)|(0<<10)|(1<<3)|(1<<2);
  USART1->CR2 = 0 << 12;
  USART1->CR3 = 0;
}
#endif

#if 0
char Uart1_Get_Char(void)
{
	while(!Macro_Check_Bit_Set(USART1->SR, 5));
	return (char)USART1->DR;
}
#endif
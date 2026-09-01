
//author: sgHyeon
//date: 2026/08/29
//version: 1

#include "mcal_common.h"
#include "mcal_macro.h"
#include "mcal_gpio.h"
#include "pinmap_config.h"
#include "stm32f411xe.h"

static GPIO_TypeDef* get_gpio_port(mcal_gpio_channel_t channel)
{
	uint8_t port = channel / 100U;

    switch (port)
    {
        case 1U:
            return GPIOA;

        case 2U:
            return GPIOB;

        case 3U:
            return GPIOC;

        case 4U:
            return GPIOD;

        case 5U:
            return GPIOE;
/*
        case 6U:
            return GPIOF;

        case 7U:
            return GPIOG;
*/
        case 8U:
            return GPIOH;

        default:
            return NULL;
    }
}

static void gpio_enable_clock(uint8_t port)
{
    switch (port)
    {
        case 1U:
            MCAL_SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOAEN_Pos);
            break;

        case 2U:
            MCAL_SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOBEN_Pos);
            break;

        case 3U:
            MCAL_SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOCEN_Pos);
            break;

        case 4U:
            MCAL_SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIODEN_Pos);
            break;

        case 5U:
            MCAL_SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOEEN_Pos);
            break;

        /*
        case 6U:
            MCAL_SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOFEN);
            break;

        case 7U:
            MCAL_SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOGEN);
            break;
        */

        case 8U:
        	MCAL_SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOHEN_Pos);
            break;

        default:
            break;
    }
}


void mcal_gpio_init(mcal_gpio_channel_t channel)
{
	uint8_t port = channel / 100U;
	uint8_t pin  = channel % 100U;

    if ((port == 0U) || (pin > 15U))
    {
        return;
    }

    GPIO_TypeDef *gpio = get_gpio_port(channel);

    if (gpio == NULL)
    {
        return;
    }


	gpio_enable_clock(port);

    MCAL_CLEAR_AREA(gpio->MODER, 0x3U, pin * 2U);
    MCAL_SET_AREA(gpio->MODER, 0x1U, pin * 2U);

    MCAL_CLEAR_BIT(gpio->OTYPER, pin);
    MCAL_CLEAR_BIT(gpio->ODR, pin);
}

void mcal_gpio_high(mcal_gpio_channel_t channel)
{
    uint8_t pin = channel % 100U;

    GPIO_TypeDef *gpio = get_gpio_port(channel);

    if ((gpio == NULL) || (pin > 15U))
    {
        return;
    }

    MCAL_SET_BIT(gpio->ODR, pin);
}

void mcal_gpio_low(mcal_gpio_channel_t channel)
{
    uint8_t pin = channel % 100U;
    GPIO_TypeDef *gpio = get_gpio_port(channel);

    if ((gpio == NULL) || (pin > 15U))
    {
        return;
    }


    MCAL_CLEAR_BIT(gpio->ODR, pin);
}

bool mcal_gpio_read(mcal_gpio_channel_t channel)
{
    uint8_t pin = channel % 100U;
    GPIO_TypeDef *gpio = get_gpio_port(channel);

    if ((gpio == NULL) || (pin > 15U))
    {
        return false;
    }

	return MCAL_CHECK_BIT_SET(gpio->ODR, pin);
}

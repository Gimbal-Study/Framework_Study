
// author: sgHyeon
// date: 2026/08/29
// version: 1

#include "mcal_common.h"
#include "mcal_macro.h"
#include "mcal_gpio.h"
#include "pinmap_config.h"
#include "stm32f411xe.h"

static mcal_gpio_channel_t gpio_int_channels[16] = {0};
static mcal_gpio_int_callback_t gpio_int_callbacks[16] = {0};

static GPIO_TypeDef *get_gpio_port(mcal_gpio_channel_t channel)
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

static void gpio_configure_exti_routing(uint16_t channel)
{
    uint8_t port = channel / 100U;
    uint8_t pin = channel % 100U;

    uint8_t exticr_index;
    uint8_t exticr_pos;
    uint8_t exti_port_select;
    uint32_t exticr_field_mask;

    exticr_index = pin / 4U;
    exticr_pos = (pin % 4U) * 4U;
    exti_port_select = port - 1U;
    exticr_field_mask = SYSCFG_EXTICR1_EXTI0_Msk;

    MCAL_CLEAR_AREA(SYSCFG->EXTICR[exticr_index], exticr_field_mask, exticr_pos);

    MCAL_SET_AREA(SYSCFG->EXTICR[exticr_index], exti_port_select, exticr_pos);
}

void mcal_gpio_init(mcal_gpio_channel_t channel)
{
    uint8_t port = channel / 100U;
    uint8_t pin = channel % 100U;

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

bool mcal_gpio_int_enable(mcal_gpio_channel_t channel, mcal_gpio_int_trigger_t trigger,
                          mcal_gpio_int_callback_t callback)
{
    uint8_t port = channel / 100U;
    uint8_t pin = channel % 100U;

    if ((port == 0U) || (pin > 15U) || (callback == NULL))
    {
        return false;
    }

    GPIO_TypeDef *gpio = get_gpio_port(channel);

    if (gpio == NULL)
    {
        return false;
    }

    gpio_enable_clock(port);

    // 입력 모드
    MCAL_CLEAR_AREA(gpio->MODER, 0x3U, pin * 2U);

    // 기존 trigger 해제
    MCAL_CLEAR_BIT(EXTI->RTSR, pin);
    MCAL_CLEAR_BIT(EXTI->FTSR, pin);

    // 새로운 trigger 설정
    switch (trigger)
    {
    case MCAL_GPIO_INT_RISING:
        MCAL_SET_BIT(EXTI->RTSR, pin);
        break;
    case MCAL_GPIO_INT_FALLING:
        MCAL_SET_BIT(EXTI->FTSR, pin);
        break;
    case MCAL_GPIO_INT_BOTH:
        MCAL_SET_BIT(EXTI->RTSR, pin);
        MCAL_SET_BIT(EXTI->FTSR, pin);
        break;
    default:
        return false;
    }

    // EXTI 라인과 GPIO 연결
    gpio_int_channels[pin] = channel;
    gpio_int_callbacks[pin] = callback;

    // EXTI 라인 활성화
    MCAL_SET_BIT(RCC->APB2ENR, RCC_APB2ENR_SYSCFGEN_Pos);
    MCAL_SET_BIT(EXTI->IMR, pin);

    return true;
}

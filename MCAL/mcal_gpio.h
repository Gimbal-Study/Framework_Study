

//author: sgHyeon
//date: 2026/08/24
//version: 1


#ifndef MCAL_GPIO_H
#define MCAL_GPIO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "mcal_common.h"


/* =========================================================================
 * GPIO Channel
 * =========================================================================
 *
 * A GPIO Channel is a logical hardware-independent identifier.
 *
 * The actual hardware mapping is platform-dependent and must be resolved
 * by the platform-specific MCAL implementation.
 *
 * Example:
 *
 *     Channel XYZ
 *         ├── STM32     -> GPIOA / Pin 5 | X=0, YZ = 05
 *         ├── AVR       -> PORTB / Pin 5 | X=1, YZ = 05
 *         └── Raspberry -> GPIO17        | X=0, YZ = 17
 *
 * No runtime lookup table is required by this interface.
 * ========================================================================= */

typedef uint16_t mcal_gpio_channel_t;

typedef enum
{
    MCAL_GPIO_MODE_INPUT,
    MCAL_GPIO_MODE_OUTPUT
} mcal_gpio_mode_t;

typedef enum
{
    MCAL_GPIO_PULL_NONE,
    MCAL_GPIO_PULL_UP,
    MCAL_GPIO_PULL_DOWN
} mcal_gpio_pull_t;


void mcal_gpio_init(mcal_gpio_channel_t channel);

void mcal_gpio_high(mcal_gpio_channel_t channel);

void mcal_gpio_low(mcal_gpio_channel_t channel);

bool mcal_gpio_read(mcal_gpio_channel_t channel);

#ifdef __cplusplus
}
#endif

#endif /* MCAL_GPIO_H */

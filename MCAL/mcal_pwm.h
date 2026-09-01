

//author: sgHyeon
//date: 2026/08/24
//version: 1


#ifndef MCAL_PWM_H
#define MCAL_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "mcal_common.h"


/* =========================================================================
 * PWM Channel
 * ========================================================================= */

typedef uint8_t mcal_pwm_channel_t;


/* =========================================================================
 * PWM Configuration
 * ========================================================================= */

typedef struct
{
    uint32_t frequency_hz;
    uint8_t duty_percent;

} mcal_pwm_config_t;


/* =========================================================================
 * PWM API
 * ========================================================================= */

/**
 * @brief Initialize PWM channel.
 *
 * @param[in] channel PWM channel.
 * @param[in] config PWM configuration.
 *
 * @return MCAL status.
 */
mcal_status_t mcal_pwm_init(mcal_pwm_channel_t channel, const mcal_pwm_config_t *config);


/**
 * @brief Deinitialize PWM channel.
 *
 * @param[in] channel PWM channel.
 *
 * @return MCAL status.
 */
mcal_status_t mcal_pwm_deinit(mcal_pwm_channel_t channel);


/**
 * @brief Start PWM output.
 *
 * @param[in] channel PWM channel.
 *
 * @return MCAL status.
 */
mcal_status_t mcal_pwm_start(mcal_pwm_channel_t channel);


/**
 * @brief Stop PWM output.
 *
 * @param[in] channel PWM channel.
 *
 * @return MCAL status.
 */
mcal_status_t mcal_pwm_stop(mcal_pwm_channel_t channel);


/**
 * @brief Set PWM frequency.
 *
 * @param[in] channel PWM channel.
 * @param[in] frequency_hz PWM frequency in Hz.
 *
 * @return MCAL status.
 */
mcal_status_t mcal_pwm_set_frequency(mcal_pwm_channel_t channel, uint32_t frequency_hz);


/**
 * @brief Set PWM duty cycle.
 *
 * @param[in] channel PWM channel.
 * @param[in] duty_percent Duty cycle in percent [0 ~ 100].
 *
 * @return MCAL status.
 */
mcal_status_t mcal_pwm_set_duty(mcal_pwm_channel_t channel, uint8_t duty_percent);


/**
 * @brief Get current PWM frequency.
 *
 * @param[in] channel PWM channel.
 *
 * @return PWM frequency in Hz.
 */
uint32_t mcal_pwm_get_frequency(mcal_pwm_channel_t channel);


/**
 * @brief Get current PWM duty cycle.
 *
 * @param[in] channel PWM channel.
 *
 * @return Duty cycle in percent.
 */
uint8_t mcal_pwm_get_duty(mcal_pwm_channel_t channel);


#ifdef __cplusplus
}
#endif

#endif /* MCAL_PWM_H */

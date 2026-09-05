

//author: sgHyeon
//date: 2026/08/24
//version: 1



#ifndef MCAL_TIMER_H
#define MCAL_TIMER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"
#include "macro.h"

#define sg      0
#define ss      1
#if sg
/* =========================================================================
 * Timer Instance
 * ========================================================================= */

typedef uint8_t mcal_timer_instance_t;


/* =========================================================================
 * Timer Callback
 * ========================================================================= */

/**
 * @brief Timer periodic event callback.
 *
 * This callback is called when the configured timer period has elapsed.
 */
typedef void (*mcal_timer_callback_t)(void);


/* =========================================================================
 * Timer API
 * ========================================================================= */

/**
 * @brief Initialize timer.
 *
 * @param[in] instance Timer instance.
 */
void mcal_timer_init(mcal_timer_instance_t instance);


/**
 * @brief Deinitialize timer.
 *
 * @param[in] instance Timer instance.
 */
void mcal_timer_deinit(mcal_timer_instance_t instance);


/**
 * @brief Start periodic timer.
 *
 * @param[in] instance Timer instance.
 * @param[in] period_us Timer period in microseconds.
 * @param[in] callback Periodic callback.
 *
 * @return MCAL status.
 */
mcal_status_t mcal_timer_start_periodic(mcal_timer_instance_t instance, uint32_t period_us, mcal_timer_callback_t callback);


/**
 * @brief Stop periodic timer.
 *
 * @param[in] instance Timer instance.
 *
 * @return MCAL status.
 */
mcal_status_t mcal_timer_stop_periodic( mcal_timer_instance_t instance);


/**
 * @brief Get current timer counter value.
 *
 * @param[in] instance Timer instance.
 *
 * @return Current counter value.
 */
uint32_t mcal_timer_get_counter(mcal_timer_instance_t instance);


/**
 * @brief Reset timer counter.
 *
 * @param[in] instance Timer instance.
 */
void mcal_timer_reset(mcal_timer_instance_t instance);


#ifdef __cplusplus
}
#endif
#endif

#if ss
typedef enum {
    MCAL_TIMER_OK = 0,
    MCAL_TIMER_ERROR,
    MCAL_TIMER_BUSY,
    MCAL_TIMER_TIMEOUT    
} mcal_timer_status_t;

bool mcal_timer_repeat_init(uint8_t timer_instance, uint32_t freq, uint32_t arr);
bool mcal_timer_oneshot_init(uint8_t timer_instance, uint32_t freq, uint32_t arr);

bool mcal_timer_int_enable(uint8_t timer_instance, bool act);

bool mcal_timer_start(uint8_t timer_instance);
void mcal_timer_stop(uint8_t timer_instance);

#endif

#endif /* MCAL_TIMER_H */

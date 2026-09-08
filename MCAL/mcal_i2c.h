#ifndef MCAL_I2C_H
#define MCAL_I2C_H

#include "common.h"

typedef enum {
    MCAL_I2C_OK = 0,  /* 정상적으로 통신 완료 */
    MCAL_I2C_ERROR,   /* NACK 수신 또는 주소 불일치 등 오류 */
    MCAL_I2C_BUSY,    /* 아직 통신 중 */
    MCAL_I2C_TIMEOUT  /* 센서 응답 지연 */
} mcal_i2c_status_t;

bool mcal_i2c_init(uint8_t channel, uint8_t mode, uint32_t Freq);

mcal_i2c_status_t mcal_i2c_write(uint8_t channel, uint16_t dev_addr,
    uint16_t mem_addr, uint8_t mem_adddr_size, const uint8_t *data, uint16_t len, uint32_t timeout);

mcal_i2c_status_t mcal_i2c_read(uint8_t channel, uint16_t dev_addr, uint16_t reg_addr,
    uint8_t *data, uint16_t len, uint32_t timeout);


#if 0
//author: sgHyeon
//date: 2026/08/24
//version: 1




#ifdef __cplusplus
extern "C" {
#endif

#include "mcal_common.h"


/* =========================================================================
 * I2C Instance
 * =========================================================================
 *
 * Logical I2C peripheral identifier.
 *
 * Example:
 *
 *     MCAL_I2C_1
 *         ├── STM32 -> I2C1
 *         ├── AVR   -> TWI0
 *         └── RPi   -> /dev/i2c-1
 *
 * The actual hardware mapping is platform-dependent.
 * ========================================================================= */

typedef uint8_t mcal_i2c_instance_t;


/* =========================================================================
 * I2C Address
 * ========================================================================= */

typedef uint8_t mcal_i2c_address_t;


/* =========================================================================
 * I2C Status
 * ========================================================================= */

typedef enum
{
    MCAL_I2C_OK = 0,
    MCAL_I2C_BUSY,
    MCAL_I2C_ERROR,
    MCAL_I2C_TIMEOUT,
    MCAL_I2C_NACK
} mcal_i2c_status_t;


/* =========================================================================
 * I2C ERROR Status
 * ========================================================================= */

typedef enum
{
    MCAL_I2C_ERROR_NONE = 0,
    MCAL_I2C_ERROR_NACK,
    MCAL_I2C_ERROR_BUS,
    MCAL_I2C_ERROR_ARBITRATION,
    MCAL_I2C_ERROR_OVERRUN
} mcal_i2c_error_t;


/* =========================================================================
 * I2C Transfer Direction
 * ========================================================================= */

typedef enum
{
    MCAL_I2C_WRITE = 0,
    MCAL_I2C_READ

} mcal_i2c_direction_t;


/* =========================================================================
 * I2C Transaction
 * ========================================================================= */

typedef struct
{
    mcal_i2c_address_t address;

    mcal_i2c_direction_t direction;

    uint8_t *data;


    size_t length;

} mcal_i2c_transaction_t;


/* =========================================================================
 * Initialization
 * ========================================================================= */

/**
 * @brief Initialize I2C MCAL.
 */
void mcal_i2c_init(void);


/* =========================================================================
 * Blocking Transfer API
 * ========================================================================= */

/**
 * @brief Transmit data to an I2C slave.
 *
 * @param[in] instance I2C instance.
 * @param[in] address  7-bit slave address.
 * @param[in] data     Data buffer.
 * @param[in] length   Number of bytes.
 *
 * @return I2C transfer status.
 */
mcal_i2c_status_t mcal_i2c_write(mcal_i2c_instance_t instance, mcal_i2c_address_t address, uint8_t reg, uint8_t data);


/**
 * @brief Receive data from an I2C slave.
 *
 * @param[in] instance I2C instance.
 * @param[in] address  7-bit slave address.
 * @param[out] data    Receive buffer.
 * @param[in] length   Number of bytes.
 *
 * @return I2C transfer status.
 */
mcal_i2c_status_t mcal_i2c_read(mcal_i2c_instance_t instance, mcal_i2c_address_t address, uint8_t reg, uint8_t *data);


/* Burst */

mcal_status_t  mcal_i2c_burst_read(
    mcal_i2c_instance_t instance,
    mcal_i2c_address_t address,
    uint8_t start_reg,
    uint8_t *data,
    uint16_t size,
    mcal_i2c_error_t *error
);

mcal_status_t  mcal_i2c_burst_write(
    mcal_i2c_instance_t instance,
    mcal_i2c_address_t address,
    uint8_t start_reg,
    const uint8_t *data,
    uint16_t size,
    mcal_i2c_error_t *error
);

#ifdef __cplusplus
}
#endif
#endif



#endif /* MCAL_I2C_H */

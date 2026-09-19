/* MPU6050 device layer for Framework_Study. See README.md and LICENSE. */
#ifndef DEVICE_SENSOR_MPU6050_H
#define DEVICE_SENSOR_MPU6050_H

#include <common.h>

#ifdef __cplusplus
extern "C" {
#endif


/* REGISTER*/
#define MPU6050_REG_XG_OFFS_TC      0x00u
#define MPU6050_REG_SMPLRT_DIV      0x19u
#define MPU6050_REG_CONFIG          0x1Au
#define MPU6050_REG_GYRO_CONFIG     0x1Bu
#define MPU6050_REG_ACCEL_CONFIG    0x1CU

#define MPU6050_REG_MOT_THR         0x1Fu
#define MPU6050_REG_MOT_DUR         0x20u
#define MPU6050_REG_ZRMOT_THR       0x21u
#define MPU6050_REG_ZRMOT_DUR       0x22u

#define MPU6050_REG_I2C_SLV0_ADDR    0x25U

#define MPU6050_REG_INT_ENABLE      0x38u
#define MPU6050_REG_INT_STATUS      0x3Au
#define MPU6050_REG_USER_CTRL       0x6Au

#define REG_PWR_MGMT_2              0x6CU
#define MPU6050_REG_BANK_SEL        0x6Du
#define MPU6050_REG_MEM_START_ADDR  0x6Eu
#define MPU6050_REG_MEM_R_W         0x6Fu
#define MPU6050_REG_DMP_CFG_1       0x70u
#define MPU6050_REG_DMP_CFG_2       0x71u

#define MPU6050_REG_WHO_AM_I     0x75U
#define MPU6050_REG_PWR_MGMT_1   0x6BU
#define MPU6050_REG_ACCEL_XOUT_H 0x3BU

/* MASK*/
#define MPU6050_MASK_DMP_EN         0x80u
#define MPU6050_MASK_FIFO_EN        0x40u
#define MPU6050_MASK_DMP_RESET      0x08u
#define MPU6050_MASK_FIFO_RESET     0x04u

#define MPU6050_MASK_INT_DMP        0x02u
#define MPU6050_MASK_INT_FIFO_OFLOW 0x10u

#define MPU6050_MASK_I2C_MST_EN     0x20U
#define MPU6050_MASK_I2C_MST_RESET  0x02U
#define MPU6050_MASK_SLEEP          0x40U
#define MPU6050_MASK_CLKSEL         0x07U

/* DEVICE ADDRESS*/
#define MPU6050_ADDRESS_AD0_LOW  0x68U
#define MPU6050_ADDRESS_AD0_HIGH 0x69U

/* PARAMETER*/
#define MPU6050_DMP_CODE_SIZE       1929u
#define MPU6050_DMP_BANK_SIZE       256u
#define MPU6050_DMP_CHUNK_SIZE      16u
#define MPU6050_DMP_PACKET_SIZE     42u
#define MPU6050_DMP_FIFO_DIVISOR    1u

/* REGIETER BIT */
// associated with dmpinitialize, reset
#define MPU6050_PWR1_DEVICE_RESET_BIT 7U



typedef enum {
    MPU6050_OK = 0,
    MPU6050_ERROR_ARGUMENT,
    MPU6050_ERROR_NOT_INITIALIZED,
    MPU6050_ERROR_I2C,
    MPU6050_ERROR_BUSY,
    MPU6050_ERROR_TIMEOUT,
    MPU6050_ERROR_ID,
    MPU6050_ERROR_VERIFY
} MPU6050_Status;

typedef enum {
    MPU6050_ACCEL_2G = 0,
    MPU6050_ACCEL_4G,
    MPU6050_ACCEL_8G,
    MPU6050_ACCEL_16G
} MPU6050_AccelRange;

typedef enum {
    MPU6050_GYRO_250DPS = 0,
    MPU6050_GYRO_500DPS,
    MPU6050_GYRO_1000DPS,
    MPU6050_GYRO_2000DPS
} MPU6050_GyroRange;

/* Names are gyro bandwidths; accelerometer bandwidths differ slightly.
 * Only DLPF 1..6 are exposed, giving a consistent 1 kHz divider input. */
typedef enum {
    MPU6050_DLPF_188HZ = 1,
    MPU6050_DLPF_98HZ,
    MPU6050_DLPF_42HZ,
    MPU6050_DLPF_20HZ,
    MPU6050_DLPF_10HZ,
    MPU6050_DLPF_5HZ
} MPU6050_Dlpf;

typedef void (*MPU6050_DelayMs)(uint32_t milliseconds);

typedef struct {
    uint8_t channel;             /* MCAL logical channel: 1, 2 or 3. */
    uint8_t address;             /* Unshifted 7-bit address: 0x68 or 0x69. */
    uint32_t timeout_ms;         /* Per transaction; also ready-poll delay budget. */
    MPU6050_DelayMs delay_ms;    /* Required; must actually wait this long. */
    MPU6050_AccelRange accel_range;
    MPU6050_GyroRange gyro_range;
    MPU6050_Dlpf dlpf;
    uint8_t sample_rate_div;     /* Sample rate = 1000 / (1 + divider) Hz. */
} MPU6050_Config;

typedef struct {
    int16_t accel[3];            /* Sensor X, Y, Z, signed raw counts. */
    int16_t temperature;
    int16_t gyro[3];
} MPU6050_RawData;

typedef struct {
    float accel_g[3];            /* Includes gravity; sensor axes. */
    float gyro_dps[3];           /* Software bias subtracted; degrees/s. */
    float temperature_c;        /* Die temperature, not ambient. */
} MPU6050_Data;

/* Zero-initialize before first use. Fields are driver-owned after Init.
 * Serialize all access to a device and its I2C bus; no ISR/thread safety. */
typedef struct {
    MPU6050_Config config;
    float accel_lsb_per_g;
    float gyro_lsb_per_dps;
    float gyro_bias_dps[3];
    bool initialized;

    /* associated with dmp*/
    uint16_t dmp_packet_size;
    bool dmp_initialized;
    bool dmp_enabled;
} MPU6050_Device;

/* Defaults: channel 1, AD0 LOW, 20 ms timeout, +/-2g, +/-500dps,
 * DLPF 42 Hz (accel 44 Hz), 200 Hz output. Supply delay callback afterwards. */
void MPU6050_DefaultConfig(MPU6050_Config *config);

/* Board must initialize power/GPIO/I2C/timebase first. Blocking reset + setup;
 * waits 100 ms before probe, after reset and after wake. Verifies settings.
 * On failure, device is unusable until a successful Init. Init clears bias.
 * Reconfigure by calling Init again; do not write registers behind the driver. */
MPU6050_Status MPU6050_Init(MPU6050_Device *device,
                           const MPU6050_Config *config);
MPU6050_Status MPU6050_TestConnection(const MPU6050_Device *device);

/* DataReady reads INT_STATUS and clears latched interrupt flags. It does not
 * wait. ReadRaw/Read return the latest sample; they do not guarantee freshness.
 * Output arguments remain unchanged on error. Check every returned status. */
MPU6050_Status MPU6050_DataReady(const MPU6050_Device *device, bool *ready);
MPU6050_Status MPU6050_ReadRaw(const MPU6050_Device *device,
                              MPU6050_RawData *data);
MPU6050_Status MPU6050_Read(const MPU6050_Device *device, MPU6050_Data *data);

/* Blocking startup calibration, 1..4096 fresh samples. Keep sensor stationary
 * and motors OFF throughout. No automatic motion detection. Gravity is NOT
 * subtracted from accelerometer. Old bias survives any calibration error.
 * timeout_ms must exceed one configured sample period. Run outside control ISR.
 * Ready polling is bounded by timeout_ms one-ms delays, plus I2C call time;
 * timeout_ms is NOT a wall-clock deadline for the complete calibration. */
MPU6050_Status MPU6050_CalibrateGyro(MPU6050_Device *device, uint16_t samples);
MPU6050_Status MPU6050_ClearGyroBias(MPU6050_Device *device);

MPU6050_Status MPU6050_DMPInitialize(MPU6050_Device *dev);
MPU6050_Status MPU6050_SetDMPEnabled(MPU6050_Device *dev, bool enabled);
MPU6050_Status MPU6050_reset(MPU6050_Device *dev);


#ifdef __cplusplus
}
#endif
#endif

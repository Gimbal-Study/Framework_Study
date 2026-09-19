#include "MPU6050.h"
#include "mcal_i2c.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static uint8_t regs[128];
static unsigned int calls, fail_at, corrupt_reg, bursts;
static mcal_i2c_status_t failure;
static uint16_t expected_address = 0x68U;
static bool produce_samples;

static void reset_bus(void)
{
    memset(regs, 0, sizeof(regs));
    regs[0x75] = 0x68;
    calls = fail_at = bursts = 0U;
    corrupt_reg = 255U;
    failure = MCAL_I2C_ERROR;
    produce_samples = true;
}

static void delay_mock(uint32_t ms)
{
    assert(ms > 0U);
    if (produce_samples)
        regs[0x3A] = 1U;
}

mcal_i2c_status_t mcal_i2c_write(uint8_t channel, uint16_t address,
    uint16_t reg, uint8_t size, const uint8_t *data, uint16_t len, uint32_t timeout)
{
    assert(channel == 1U && address == expected_address);
    assert(size == 1U && len == 1U && timeout > 0U && reg < 128U);
    if (++calls == fail_at)
        return failure;
    if (reg == 0x6BU && data[0] == 0x80U) {
        memset(regs, 0, sizeof(regs));
        regs[0x75] = 0x68U;
        regs[0x6B] = 0x40U;
    } else {
        regs[reg] = data[0];
    }
    return MCAL_I2C_OK;
}

mcal_i2c_status_t mcal_i2c_read(uint8_t channel, uint16_t address,
    uint16_t reg, uint8_t size, uint8_t *data, uint16_t len, uint32_t timeout)
{
    assert(channel == 1U && address == expected_address);
    assert(size == 1U && timeout > 0U && reg + len <= 128U);
    if (++calls == fail_at) {
        data[0] = 0xEEU; /* Model a partially received buffer on error. */
        return failure;
    }
    memcpy(data, &regs[reg], len);
    if (reg == corrupt_reg)
        data[0] ^= 1U;
    if (reg == 0x3AU)
        regs[reg] = 0U;
    if (reg == 0x3BU) {
        assert(len == 14U);
        ++bursts;
    }
    return MCAL_I2C_OK;
}

static void set_word(unsigned int reg, int16_t value)
{
    regs[reg] = (uint8_t)((uint16_t)value >> 8U);
    regs[reg + 1U] = (uint8_t)value;
}

static void near(float actual, float expected)
{
    float difference = actual - expected;
    assert(difference < 0.001f && difference > -0.001f);
}

int main(void)
{
    MPU6050_Device device = {0};
    MPU6050_Config config;
    MPU6050_RawData raw, before_raw;
    MPU6050_Data data, before;
    bool ready;
    unsigned int init_calls;
    static const int16_t scales[] = {16384, 8192, 4096, 2048};
    static const float gyro_scales[] = {131.0f, 65.5f, 32.8f, 16.4f};

    MPU6050_DefaultConfig(&config);
    assert(MPU6050_Read(&device, &data) == MPU6050_ERROR_NOT_INITIALIZED);
    assert(MPU6050_Init(&device, &config) == MPU6050_ERROR_ARGUMENT);
    config.delay_ms = delay_mock;
    reset_bus();
    assert(MPU6050_Init(&device, &config) == MPU6050_OK);
    init_calls = calls;
    assert(regs[0x6B] == 1U && regs[0x6C] == 0U);
    assert(regs[0x19] == 4U && regs[0x1A] == 3U);
    assert(regs[0x1B] == 8U && regs[0x1C] == 0U);

    /* Every init bus operation must fail closed, including verification reads. */
    for (unsigned int index = 1U; index <= init_calls; ++index) {
        reset_bus();
        fail_at = index;
        assert(MPU6050_Init(&device, &config) == MPU6050_ERROR_I2C);
        assert(!device.initialized);
        assert(MPU6050_Read(&device, &data) == MPU6050_ERROR_NOT_INITIALIZED);
    }
    reset_bus();
    regs[0x75] = 0x70U;
    assert(MPU6050_Init(&device, &config) == MPU6050_ERROR_ID);
    reset_bus();
    corrupt_reg = 0x1BU;
    assert(MPU6050_Init(&device, &config) == MPU6050_ERROR_VERIFY);

    /* AD0 high changes the bus address but not the identity register. */
    reset_bus();
    expected_address = config.address = 0x69U;
    assert(MPU6050_Init(&device, &config) == MPU6050_OK);
    assert(MPU6050_TestConnection(&device) == MPU6050_OK);
    expected_address = config.address = 0x68U;

    for (unsigned int range = 0U; range < 4U; ++range) {
        reset_bus();
        config.accel_range = (MPU6050_AccelRange)range;
        config.gyro_range = (MPU6050_GyroRange)range;
        assert(MPU6050_Init(&device, &config) == MPU6050_OK);
        set_word(0x3B, scales[range]);
        set_word(0x3D, (int16_t)-scales[range]);
        set_word(0x3F, INT16_MIN);
        set_word(0x41, -340);
        set_word(0x43, INT16_MAX);
        set_word(0x45, INT16_MIN);
        set_word(0x47, -1);
        assert(MPU6050_ReadRaw(&device, &raw) == MPU6050_OK);
        assert(raw.accel[2] == INT16_MIN && raw.gyro[0] == INT16_MAX);
        assert(raw.gyro[1] == INT16_MIN && raw.gyro[2] == -1);
        assert(MPU6050_Read(&device, &data) == MPU6050_OK);
        near(data.accel_g[0], 1.0f);
        near(data.accel_g[1], -1.0f);
        near(data.temperature_c, 35.53f);
        near(data.gyro_dps[0], 32767.0f / gyro_scales[range]);
        near(data.gyro_dps[1], -32768.0f / gyro_scales[range]);
        assert(bursts == 2U);
    }

    before = data;
    before_raw = raw;
    for (unsigned int error = 1U; error <= 3U; ++error) {
        static const mcal_i2c_status_t errors[] = {
            MCAL_I2C_OK, MCAL_I2C_ERROR, MCAL_I2C_BUSY, MCAL_I2C_TIMEOUT};
        static const MPU6050_Status mapped[] = {
            MPU6050_OK, MPU6050_ERROR_I2C, MPU6050_ERROR_BUSY, MPU6050_ERROR_TIMEOUT};
        failure = errors[error];
        fail_at = calls + 1U;
        assert(MPU6050_Read(&device, &data) == mapped[error]);
        assert(memcmp(&before, &data, sizeof(data)) == 0);
        fail_at = calls + 1U;
        assert(MPU6050_ReadRaw(&device, &raw) == mapped[error]);
        assert(memcmp(&before_raw, &raw, sizeof(raw)) == 0);
    }
    fail_at = 0U;
    regs[0x3A] = 1U;
    assert(MPU6050_DataReady(&device, &ready) == MPU6050_OK && ready);
    assert(MPU6050_DataReady(&device, &ready) == MPU6050_OK && !ready);

    set_word(0x43, 164);
    set_word(0x45, -328);
    set_word(0x47, 0);
    assert(MPU6050_CalibrateGyro(&device, 10U) == MPU6050_OK);
    near(device.gyro_bias_dps[0], 10.0f);
    near(device.gyro_bias_dps[1], -20.0f);
    assert(MPU6050_Read(&device, &data) == MPU6050_OK);
    near(data.gyro_dps[0], 0.0f);
    near(data.gyro_dps[1], 0.0f);
    produce_samples = false;
    assert(MPU6050_CalibrateGyro(&device, 2U) == MPU6050_ERROR_TIMEOUT);
    near(device.gyro_bias_dps[0], 10.0f);
    produce_samples = true;
    failure = MCAL_I2C_ERROR;
    fail_at = calls + 3U; /* Error on calibration burst, after ready polling. */
    assert(MPU6050_CalibrateGyro(&device, 2U) == MPU6050_ERROR_I2C);
    near(device.gyro_bias_dps[1], -20.0f);
    assert(MPU6050_ClearGyroBias(&device) == MPU6050_OK);
    near(device.gyro_bias_dps[0], 0.0f);
    assert(MPU6050_CalibrateGyro(&device, 0U) == MPU6050_ERROR_ARGUMENT);
    assert(MPU6050_CalibrateGyro(&device, 4097U) == MPU6050_ERROR_ARGUMENT);
    assert(MPU6050_Read(NULL, &data) == MPU6050_ERROR_ARGUMENT);
    assert(MPU6050_Read(&device, NULL) == MPU6050_ERROR_ARGUMENT);
    config.dlpf = (MPU6050_Dlpf)7;
    assert(MPU6050_Init(&device, &config) == MPU6050_ERROR_ARGUMENT);
    puts("PASS: init/fault injection, identity, burst decode, all scales, errors, calibration");
    return 0;
}

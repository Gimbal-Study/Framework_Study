
// author: sgHyeon
// date: 2026/08/29
// version: 1

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "systick.h"
#include "mpu6050.h"

// raw data 읽기 및 정리
typedef struct
{
    int16_t accel[3];
    int16_t temperature_raw;
    int16_t gyro[3];

    float temperature_c;
} mpu6050_data_t;

static int16_t mpu6050_decode_i16(const uint8_t *data)
{
    return (int16_t)(((uint16_t)data[0] << 8U) | ((uint16_t)data[1]));
}

bool mpu6050_read_raw(mpu6050_data_t *output)
{
    uint8_t rx_data[14];

    if (output == NULL)
    {
        return false;
    }

    /* 0x3B부터 14바이트 I2C burst read */
    if (mcal_i2c_read((0x3B, rx_data, sizeof(rx_data))) == false)
    {
        return false;
    }

    output->accel[0] = mpu6050_decode_i16(&rx_data[0]);
    output->accel[1] = mpu6050_decode_i16(&rx_data[2]);
    output->accel[2] = mpu6050_decode_i16(&rx_data[4]);

    output->temperature_raw = mpu6050_decode_i16(&rx_data[6]);
    output->temperature_c = (float)output->temperature_raw / 340.0f + 36.53f;

    output->gyro[0] = mpu6050_decode_i16(&rx_data[8]);
    output->gyro[1] = mpu6050_decode_i16(&rx_data[10]);
    output->gyro[2] = mpu6050_decode_i16(&rx_data[12]);

    return true;
}

// 갑자기 변경되는 데이터 정류
typedef struct
{
    int32_t gyro_bias[3];
    int32_t accel_bias[3];
} mpu6050_calibration_t;

static mpu6050_calibration_t s_calibration;
static bool s_calibration_valid;

static mpu6050_data_t s_mpu6050;

bool mpu6050_init(void)
{
    uint8_t reg = 0;
    /********************
    [1] MPU-6050 기본 초기화
    ********************/
    // Initialize the MPU-6050 sensor
    // This function should configure the necessary registers and settings for the
    // sensor to operate correctly. The actual implementation will depend on the
    // specific requirements of your application and the I2C communication setup.
    mcal_i2c_read(1, (0x68 or 0x69) << 1U, read, 0x75, &reg); // who am i
    // 읽은 데이터가 0x68확인
    if (reg != 0x68)
        return false;

    mcal_i2c_write(1, (0x68 or 0x69) << 1U, write, 0x6B, 0x80); // reset

    delay_ms(100);
    mcal_i2c_write(1, (0x68 or 0x69) << 1U, write, 0x6B, 0x01); // PLL X axis gyroscope ref

    delay_ms(100);
    // MPU-6050 Sampling Rate
    // Sample rate = gyroscpeOutput / (1 + SMPLT_DIV)
    // 8kHz when the DLPF is disable
    mcal_i2c_write(1, (0x68 or 0x69) << 1U, write, 0x19, 0x04);

    // set DLPF
    mcal_i2c_write(1, (0x68 or 0x69) << 1U, write, 0x1A, 0x03);

    // Register 27 - GYRO_CONFIG 필터설정
    mcal_i2c_write(1, (0x68 or 0x69) << 1U, write, 0x1B, 0x03);

    // Register 28 - ACCEL_CONFIG 필터설정
    // 초기 테스트 성공 후 자세한 값을 확인하려면 0x00
    // 셀프테스트 기능도 여기에
    mcal_i2c_write(1, (0x68 or 0x69) << 1U, write, 0x1C, 0x08);

    // calibration / offset
    // 0x3B~0x40: Accelerometer
    // 0x41~0x42: Temp
    // 0x43~0x48: Gyro
    mpu6050_calibrate(10); // 임의의 값

    /*************
     * [2] DMP 초기화
     *************/

    dmpInitialize();

    // dmp/fifo reset
    mcal_i2c_write(1, (0x68 or 0x69) << 1U, write, 0x6A, 0x0C);

    systick_run(200);

    do
    {
        if (systick_check_timeout())
            return false;

        mcal_i2c_read(1, (0x68 or 0x69) << 1U, read, 0x6A, &reg);
    } while ((reg & 0x0CU) == 0U);

    // dmp/fifo en
    mcal_i2c_write(1, (0x68 or 0x69) << 1U, write, 0x6A, 0xC0);

    /**********
    [3] Runtime
    **********/

    // GPT는 인터럽트 사용 추천

    // 0x37 Active-high, push-pull, latch, any-read clear INT_PIN_CFG = 0x30;
    // 0x20: 인터럽트 상태 유지 0x10: 센서 데이터 읽을 때 인터럽트 해제 가능

    // 0x38 INT_ENABLE = 0x01;

    // INT_STATUS — 주소 0x3A(read only)
    /*
    MPU6050 DATA_RDY
        ↓
    MPU INT 핀
        ↓
    STM32 EXTI IRQ Handler
        ↓
    volatile data_ready = true
        ↓
    메인 루프 또는 제어 태스크
        ↓
    INT_STATUS 확인 및 센서 14바이트 버스트 읽기
        ↓
    자세 계산 / PID 제어
    */
}

/*
1. 센서가 정지해 있는지 전제
2. raw 가속도·자이로 데이터를 여러 번 읽음
3. 각 축 평균 계산
4. gyro bias 계산
5. accel bias 계산
6. s_calibration 변수에 저장
7. calibration valid 설정
8. 성공/실패 반환
*/
bool mpu6050_calibrate(uint16_t sample_count)
{
    int64_t accel_sum[3] = {0};
    int64_t gyro_sum[3] = {0};

    if (sample_count == 0U)
    {
        return false;
    }

    for (uint16_t sample = 0U; sample < sample_count; sample++)
    {
        if (!mpu6050_read_raw(&s_mpu6050))
        {
            return false;
        }

        for (uint8_t axis = 0U; axis < 3U; axis++)
        {
            accel_sum[axis] += s_mpu6050.accel[axis];
            gyro_sum[axis] += s_mpu6050.gyro[axis];
        }
    }

    for (uint8_t axis = 0U; axis < 3U; axis++)
    {
        s_calibration.accel_bias[axis] = (int32_t)(accel_sum[axis] / sample_count);
        s_calibration.gyro_bias[axis] = (int32_t)(gyro_sum[axis] / sample_count);
    }

    s_calibration_valid = true;

    return s_calibration_valid;
}




//author: sgHyeon
//date: 2026/08/24
//version: 1


#ifndef MCAL_I2C_H
#define MCAL_I2C_H

#ifdef __cplusplus
extern "C" {
#endif


// 함수 초안을 미리 올려놨습니다. 회의 때 구체적으로 짜봅시다.
typedef enum {
    MCAL_I2C_OK = 0,
    MCAL_I2C_ERROR,
    MCAL_I2C_BUSY,
    MCAL_I2C_TIMEOUT    
} mcal_i2c_status_t;

// 필요 매개 변수: channel(i2cx: x에 해당하는 장치, 모드, Freq
bool mcal_i2c_init(uint8_t i2c_instance, uint8_t mode, uint32_t Freq);
mcal_i2c_status_t mcal_i2c_write(uint8_t i2c_instance, uint8_t dev_addr, uint8_t reg_addr, const uint8_t *data, uint16_t len, uint16_t timeout);
mcal_i2c_status_t mcal_i2c_read(uint8_t i2c_instance, uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len, uint16_t timeout);

#ifdef __cplusplus
}
#endif

#endif /* MCAL_I2C_H */

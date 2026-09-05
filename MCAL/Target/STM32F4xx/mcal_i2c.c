#include "mcal_i2c.h"
#include "stm32f411xe.h"
#include <common.h>
#include "mcal_macro.h"



bool mcal_i2c_init(uint8_t channel, uint8_t mode, uint32 Freq)
{
    if(/* channel == ??? ||*/ Freq == 0)
    {
        return false;
    }

    // 2. 하드웨어 주변장치 클록 인가 (I2C1, GPIOB)
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN; //  (0x1UL << 21U)
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN; // (0x1UL << 1U)

    // 3. GPIO 핀 설정 (PB6 = SCL, PB7 = SDA)
    // MODER: PB6, PB7을 대체 기능 모드(Alternate Function, 10b)로 설정
    GPIOB->MODER &= ~((0x3U << (6 * 2)) | (0x3U << (7 * 2)));
    GPIOB->MODER |= ((0x2U << (6 * 2)) | (0x3U << (7 * 2)));

    // OTYPER: I2C 필수 조건인 Open-Drain(1) 설정
    GPIOB->OTYPER |= (0x1U << 6) | (0x1U << 7);

    // PUPDR: MCU 내부 풀업
    GPIOB->PUPDR &= ~((0x3U << (6 * 2)) | (0x3U << (7 * 2)));
    GPIOB->PUPDR |= ((0x1U << (6 * 2)) | (0x1U << (7 * 2)));

    // AFR[0]: PB6, PB7에 AF4(I2C1 기능 번호) 부여
    GPIOB->AFR[0] &= ~((0xFU << (6 * 4)) | (0xFU << (7 * 4)));
    GPIOB->AFR[0] |= ((0x4U << (6 * 4)) | (0x4U << (7 * 4)));

    // I2C 하드웨어 타이밍 설정
    // 레지스터 설정 전 반드시 I2C 모듈을 잠시 꺼두어야 함
    I2C1->CR1 &= ~I2C_CR1_PE; // (0x1UL << 0U)

    // I2C에 APB1 클록 주파수(MHz)를 알려줌
    // 해당 프로젝트에서는 STM32에 공급되는 주파수가 96MHz. 이것의 절반.
    I2C1->CR2 = 48; //
}

mcal_i2c_status_t mcal_i2c_write(uint8_t channel, uint8_t addr_len, uint16_t dev_addr, uint8_t is_reg, uint16_t reg_addr, const uint8_t *data, uint16_t len, uint16_t timeout)
{
    // 데이터의 주소가 NULL인지, 데이터의 길이가 0인지, dev_addr가 7bit를 넘어가는지 확인
    if((data == NULL) || (len == 0U) || (dev_addr > 0x7fU))
    {
        return MCAL_I2C_ERROR;
    }

    I2C_TypeDef * i2c_instance;

    switch(channel)
    {
        case 1:
        i2c_instance = I2C1;
        break;
        case 2:
        i2c_instance = I2C2;
        break;
        case 3:
        i2c_instance = I2C3;
        break;
        default:

        i2c_instance->SR2 |= I2C_SR2_MSL | ;
        
        return MCAL_I2C_ERROR;
    }
}

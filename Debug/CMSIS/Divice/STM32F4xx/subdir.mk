################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../CMSIS/Divice/STM32F4xx/system_stm32f4xx.c 

OBJS += \
./CMSIS/Divice/STM32F4xx/system_stm32f4xx.o 

C_DEPS += \
./CMSIS/Divice/STM32F4xx/system_stm32f4xx.d 


# Each subdirectory must supply rules for building sources it contributes
CMSIS/Divice/STM32F4xx/%.o CMSIS/Divice/STM32F4xx/%.su CMSIS/Divice/STM32F4xx/%.cyclo: ../CMSIS/Divice/STM32F4xx/%.c CMSIS/Divice/STM32F4xx/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DNUCLEO_F411RE -DSTM32 -DSTM32F4 -DSTM32F411RETx -c -I../Inc -O3 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-CMSIS-2f-Divice-2f-STM32F4xx

clean-CMSIS-2f-Divice-2f-STM32F4xx:
	-$(RM) ./CMSIS/Divice/STM32F4xx/system_stm32f4xx.cyclo ./CMSIS/Divice/STM32F4xx/system_stm32f4xx.d ./CMSIS/Divice/STM32F4xx/system_stm32f4xx.o ./CMSIS/Divice/STM32F4xx/system_stm32f4xx.su

.PHONY: clean-CMSIS-2f-Divice-2f-STM32F4xx


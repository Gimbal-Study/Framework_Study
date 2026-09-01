################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/main.c 

OBJS += \
./Core/main.o 

C_DEPS += \
./Core/main.d 


# Each subdirectory must supply rules for building sources it contributes
Core/%.o Core/%.su Core/%.cyclo: ../Core/%.c Core/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g -DDEBUG -DNUCLEO_F411RE -DSTM32 -DSTM32F4 -DSTM32F411RETx -c -I../Inc -I"D:/sg_private/ST_Study/test/CMSIS/Core" -I"D:/sg_private/ST_Study/test/CMSIS/Device/STM32F4xx" -I"D:/sg_private/ST_Study/test/Common" -I"D:/sg_private/ST_Study/test/Config" -I"D:/sg_private/ST_Study/test/Core/System" -I"D:/sg_private/ST_Study/test/MCAL" -I"D:/sg_private/ST_Study/test/Test" -I"D:/sg_private/ST_Study/test/Board/NUCLEO_F411RE" -Og -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core

clean-Core:
	-$(RM) ./Core/main.cyclo ./Core/main.d ./Core/main.o ./Core/main.su

.PHONY: clean-Core


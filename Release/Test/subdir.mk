################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Test/gpio_test.c 

OBJS += \
./Test/gpio_test.o 

C_DEPS += \
./Test/gpio_test.d 


# Each subdirectory must supply rules for building sources it contributes
Test/%.o Test/%.su Test/%.cyclo: ../Test/%.c Test/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DNUCLEO_F411RE -DSTM32 -DSTM32F4 -DSTM32F411RETx -c -I../Inc -I"D:/sg_private/ST_Study/test/CMSIS/Device/STM32F4xx" -I"D:/sg_private/ST_Study/test/CMSIS/Core" -I"D:/sg_private/ST_Study/test/Common" -I"D:/sg_private/ST_Study/test/Config" -I"D:/sg_private/ST_Study/test/Core/System" -I"D:/sg_private/ST_Study/test/MCAL" -I"D:/sg_private/ST_Study/test/Test" -I"D:/sg_private/ST_Study/test/Board/NUCLEO_F411RE" -O1 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Test

clean-Test:
	-$(RM) ./Test/gpio_test.cyclo ./Test/gpio_test.d ./Test/gpio_test.o ./Test/gpio_test.su

.PHONY: clean-Test


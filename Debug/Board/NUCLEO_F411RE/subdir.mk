################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Board/NUCLEO_F411RE/board_init.c 

OBJS += \
./Board/NUCLEO_F411RE/board_init.o 

C_DEPS += \
./Board/NUCLEO_F411RE/board_init.d 


# Each subdirectory must supply rules for building sources it contributes
Board/NUCLEO_F411RE/%.o Board/NUCLEO_F411RE/%.su Board/NUCLEO_F411RE/%.cyclo: ../Board/NUCLEO_F411RE/%.c Board/NUCLEO_F411RE/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g -DDEBUG -DNUCLEO_F411RE -DSTM32 -DSTM32F4 -DSTM32F411RETx -c -I../Inc -I"D:/sg_private/ST_Study/test/CMSIS/Core" -I"D:/sg_private/ST_Study/test/CMSIS/Device/STM32F4xx" -I"D:/sg_private/ST_Study/test/Common" -I"D:/sg_private/ST_Study/test/Config" -I"D:/sg_private/ST_Study/test/Core/System" -I"D:/sg_private/ST_Study/test/MCAL" -I"D:/sg_private/ST_Study/test/Test" -I"D:/sg_private/ST_Study/test/Board/NUCLEO_F411RE" -Og -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Board-2f-NUCLEO_F411RE

clean-Board-2f-NUCLEO_F411RE:
	-$(RM) ./Board/NUCLEO_F411RE/board_init.cyclo ./Board/NUCLEO_F411RE/board_init.d ./Board/NUCLEO_F411RE/board_init.o ./Board/NUCLEO_F411RE/board_init.su

.PHONY: clean-Board-2f-NUCLEO_F411RE


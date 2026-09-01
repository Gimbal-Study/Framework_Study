################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_SRCS += \
../Board/NUCLEO_F411RE/Startup/startup_stm32f411retx.s 

OBJS += \
./Board/NUCLEO_F411RE/Startup/startup_stm32f411retx.o 

S_DEPS += \
./Board/NUCLEO_F411RE/Startup/startup_stm32f411retx.d 


# Each subdirectory must supply rules for building sources it contributes
Board/NUCLEO_F411RE/Startup/%.o: ../Board/NUCLEO_F411RE/Startup/%.s Board/NUCLEO_F411RE/Startup/subdir.mk
	arm-none-eabi-gcc -mcpu=cortex-m4 -g -DDEBUG -c -I"D:/sg_private/ST_Study/test/Board/NUCLEO_F411RE/Startup" -x assembler-with-cpp -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@" "$<"

clean: clean-Board-2f-NUCLEO_F411RE-2f-Startup

clean-Board-2f-NUCLEO_F411RE-2f-Startup:
	-$(RM) ./Board/NUCLEO_F411RE/Startup/startup_stm32f411retx.d ./Board/NUCLEO_F411RE/Startup/startup_stm32f411retx.o

.PHONY: clean-Board-2f-NUCLEO_F411RE-2f-Startup


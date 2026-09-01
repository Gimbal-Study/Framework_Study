################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/System/clock.c \
../Core/System/syscalls.c \
../Core/System/sysmem.c \
../Core/System/systick.c 

OBJS += \
./Core/System/clock.o \
./Core/System/syscalls.o \
./Core/System/sysmem.o \
./Core/System/systick.o 

C_DEPS += \
./Core/System/clock.d \
./Core/System/syscalls.d \
./Core/System/sysmem.d \
./Core/System/systick.d 


# Each subdirectory must supply rules for building sources it contributes
Core/System/%.o Core/System/%.su Core/System/%.cyclo: ../Core/System/%.c Core/System/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DNUCLEO_F411RE -DSTM32 -DSTM32F4 -DSTM32F411RETx -c -I../Inc -I"D:/sg_private/ST_Study/test/CMSIS/Device/STM32F4xx" -I"D:/sg_private/ST_Study/test/CMSIS/Core" -I"D:/sg_private/ST_Study/test/Common" -I"D:/sg_private/ST_Study/test/Config" -I"D:/sg_private/ST_Study/test/Core/System" -I"D:/sg_private/ST_Study/test/MCAL" -I"D:/sg_private/ST_Study/test/Test" -I"D:/sg_private/ST_Study/test/Board/NUCLEO_F411RE" -O1 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-System

clean-Core-2f-System:
	-$(RM) ./Core/System/clock.cyclo ./Core/System/clock.d ./Core/System/clock.o ./Core/System/clock.su ./Core/System/syscalls.cyclo ./Core/System/syscalls.d ./Core/System/syscalls.o ./Core/System/syscalls.su ./Core/System/sysmem.cyclo ./Core/System/sysmem.d ./Core/System/sysmem.o ./Core/System/sysmem.su ./Core/System/systick.cyclo ./Core/System/systick.d ./Core/System/systick.o ./Core/System/systick.su

.PHONY: clean-Core-2f-System


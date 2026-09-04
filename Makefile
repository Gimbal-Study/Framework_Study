PROJECT := FrameWork
BUILD ?= Debug

CROSS_COMPILE ?= arm-none-eabi-
CC := $(CROSS_COMPILE)gcc
SIZE := $(CROSS_COMPILE)size
OBJDUMP := $(CROSS_COMPILE)objdump
OPENOCD ?= openocd

LINKER_SCRIPT := Board/NUCLEO_F411RE/STM32F411RETX_FLASH.ld

C_SOURCES := \
	Board/NUCLEO_F411RE/board_init.c \
	CMSIS/Device/STM32F4xx/system_stm32f4xx.c \
	Core/System/clock.c \
	Core/System/syscalls.c \
	Core/System/sysmem.c \
	Core/System/systick.c \
	Core/main.c \
	MCAL/Target/mcal_gpio.c \
	Test/gpio_test.c

ASM_SOURCES := \
	Board/NUCLEO_F411RE/Startup/startup_stm32f411retx.s

INCLUDES := \
	-IInc \
	-ICMSIS/Core \
	-ICMSIS/Device/STM32F4xx \
	-ICommon \
	-IConfig \
	-ICore/System \
	-IMCAL \
	-ITest \
	-IBoard/NUCLEO_F411RE

DEFINES := \
	-DDEBUG \
	-DNUCLEO_F411RE \
	-DSTM32 \
	-DSTM32F4 \
	-DSTM32F411RETx

CPU_FLAGS := \
	-mcpu=cortex-m4 \
	-mfpu=fpv4-sp-d16 \
	-mfloat-abi=hard \
	-mthumb

COMMON_FLAGS := \
	$(CPU_FLAGS) \
	-ffunction-sections \
	-fdata-sections \
	-Wall

ifeq ($(BUILD),Release)
OPT_FLAGS := -O3
else
OPT_FLAGS := -Og -g
endif

CFLAGS := \
	-std=gnu11 \
	$(COMMON_FLAGS) \
	$(OPT_FLAGS) \
	-fstack-usage \
	-MMD \
	-MP \
	--specs=nano.specs

ASFLAGS := \
	$(COMMON_FLAGS) \
	$(OPT_FLAGS) \
	-x assembler-with-cpp \
	-MMD \
	-MP \
	--specs=nano.specs

LDFLAGS := \
	$(CPU_FLAGS) \
	-T$(LINKER_SCRIPT) \
	--specs=nosys.specs \
	--specs=nano.specs \
	-Wl,-Map=$(BUILD)/$(PROJECT).map \
	-Wl,--gc-sections \
	-static

LDLIBS := \
	-Wl,--start-group \
	-lc \
	-lm \
	-Wl,--end-group

C_OBJECTS := $(addprefix $(BUILD)/,$(C_SOURCES:.c=.o))
ASM_OBJECTS := $(addprefix $(BUILD)/,$(ASM_SOURCES:.s=.o))
OBJECTS := $(C_OBJECTS) $(ASM_OBJECTS)
DEPS := $(OBJECTS:.o=.d)

ELF := $(BUILD)/$(PROJECT).elf
MAP := $(BUILD)/$(PROJECT).map
LIST := $(BUILD)/$(PROJECT).list

ifeq ($(OS),Windows_NT)
define make-output-dir
	@if not exist "$(subst /,\,$(patsubst %/,%,$(dir $@)))" mkdir "$(subst /,\,$(patsubst %/,%,$(dir $@)))"
endef
define remove-build-dir
	@if exist "$(subst /,\,$(BUILD))" rmdir /S /Q "$(subst /,\,$(BUILD))"
endef
else
define make-output-dir
	@mkdir -p "$(dir $@)"
endef
define remove-build-dir
	@rm -rf "$(BUILD)"
endef
endif

.DEFAULT_GOAL := all

.PHONY: all clean run flash size

all: $(ELF) $(LIST) size

$(BUILD)/%.o: %.c Makefile
	$(make-output-dir)
	$(CC) $(DEFINES) $(INCLUDES) $(CFLAGS) -c "$<" -o "$@"

$(BUILD)/%.o: %.s Makefile
	$(make-output-dir)
	$(CC) $(DEFINES) $(INCLUDES) $(ASFLAGS) -c "$<" -o "$@"

$(ELF): $(OBJECTS) $(LINKER_SCRIPT) Makefile
	$(make-output-dir)
	$(CC) $(OBJECTS) $(LDFLAGS) $(LDLIBS) -o "$@"

$(LIST): $(ELF)
	$(OBJDUMP) -h -S "$<" > "$@"

size: $(ELF)
	$(SIZE) "$<"

flash run: $(ELF)
	$(OPENOCD) -f interface/stlink.cfg -f target/stm32f4x.cfg -c "program $(ELF) verify reset exit"

clean:
	$(remove-build-dir)

-include $(DEPS)

# ============================================================
# Tool information
# ============================================================
TOOL_DIR       = C:\arm-gnu-toolchain-15.2.rel1-mingw-w64-i686-arm-none-eabi
VERSION        = 15.2.1
TARGET         = arm-none-eabi


# ============================================================
# Project name & Link script
# ============================================================
OUT_FILE_NAME  = rom_0x08000000
LDS_FILE_NAME  = rom_0x08000000.lds


# ============================================================
# Compiler Options
# STM32F411RE = Cortex-M4 + FPU
# ============================================================
CFLAGS  = -mcpu=cortex-m4
CFLAGS += -mthumb
CFLAGS += -mfpu=fpv4-sp-d16
CFLAGS += -mfloat-abi=hard
CFLAGS += -DSTM32F411xE
CFLAGS += -std=gnu99
CFLAGS += -O3
CFLAGS += -Wall
CFLAGS += -g
CFLAGS += -fno-builtin
CFLAGS += -funsigned-char
CFLAGS += -fno-strict-aliasing
CFLAGS += -fno-common


# ============================================================
# Linker Options
# ============================================================
LDFLAGS  = -mcpu=cortex-m4
LDFLAGS += -mthumb
LDFLAGS += -mfpu=fpv4-sp-d16
LDFLAGS += -mfloat-abi=hard
LDFLAGS += --specs=nano.specs
LDFLAGS += --specs=nosys.specs
LDFLAGS += -u _printf_float
LDFLAGS += -u _scanf_float
LDFLAGS += -nostartfiles
LDFLAGS += -ffreestanding
LDFLAGS += -Wl,-Map=$(OUT_FILE_NAME).map
LDFLAGS += -Wl,--cref
LDFLAGS += -Wl,-EL
LDFLAGS += -T $(LDS_FILE_NAME)


# ============================================================
# Output Files
# ============================================================
OUT_BIN_FILE   = $(OUT_FILE_NAME).bin
OUT_ELF_FILE   = $(OUT_FILE_NAME).elf
OUT_MAP_FILE   = $(OUT_FILE_NAME).map


# ============================================================
# Tool setting
# ============================================================
AS      = "$(TOOL_DIR)/bin/$(TARGET)-as"
CC      = "$(TOOL_DIR)/bin/$(TARGET)-gcc"
LD      = "$(TOOL_DIR)/bin/$(TARGET)-ld"
OBJCOPY = "$(TOOL_DIR)/bin/$(TARGET)-objcopy"
OBJDUMP = "$(TOOL_DIR)/bin/$(TARGET)-objdump"


# ============================================================
# Source Files
# ============================================================
CSRC = \
    Core/main.c \
    Core/System/system_stm32f4xx.c \
    Core/System/clock.c \
    MCAL/Target/STM32F4xx/stm32_uart.c \
    MCAL/Target/STM32F4xx/stm32_queue.c \
    Test/uart_test.c \
    Core/System/systick.c

ASRC = \
    Board/NUCLEO_F411RE/crt0.s


# ============================================================
# Object Files
# ============================================================
COBJS = $(CSRC:.c=.o)
AOBJS = $(ASRC:.s=.o)

OBJS = $(COBJS) $(AOBJS)


# ============================================================
# Library / Include
# ============================================================
C_DIR   = $(TOOL_DIR)/$(TARGET)
GCC_DIR = $(TOOL_DIR)/lib/gcc/$(TARGET)/$(VERSION)

LIB_OPTION = \
    -L "$(C_DIR)/lib/thumb2" \
    -L "$(GCC_DIR)/thumb2" \
    -lc \
    -lgcc

INCLUDE = \
    -nostdinc \
    -I. \
    -I Board/NUCLEO_F411RE \
    -I MCAL \
    -I Core/System \
    -I CMSIS/Core \
    -I Test \
    -I CMSIS/Device/STM32F4xx \
    -I CMSIS/Device/STM32F4xx \
    -I Core/System \
    -I "$(C_DIR)/include" \
    -I "$(GCC_DIR)/include"


# ============================================================
# Default Target
# ============================================================
.PHONY: all clean run

all: $(OUT_BIN_FILE)


# ============================================================
# C compile
# ============================================================
%.o: %.c
	$(CC) $(INCLUDE) $(CFLAGS) -c $< -o $@


# ============================================================
# Assembly compile
# ============================================================
%.o: %.s
	$(CC) $(INCLUDE) $(CFLAGS) -c $< -o $@


# ============================================================
# ELF Link
# ============================================================
$(OUT_ELF_FILE): $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) $(LIB_OPTION) -o $@


# ============================================================
# BIN / DUMP
# ============================================================
$(OUT_BIN_FILE): $(OUT_ELF_FILE)
	$(OBJCOPY) $(OUT_ELF_FILE) $(OUT_BIN_FILE) -O binary
	$(OBJDUMP) -x -D $(OUT_ELF_FILE) > __dump.txt
	$(OBJDUMP) -x -D -S $(OUT_ELF_FILE) > __dump_all.txt


# ============================================================
# Clean
# ============================================================
clean:
	rm -f $(OUT_BIN_FILE)
	rm -f $(OUT_ELF_FILE)
	rm -f $(OUT_MAP_FILE)
	rm -f $(OBJS)
	rm -f __dump.txt
	rm -f __dump_all.txt


# ============================================================
# Flash
# ============================================================
run: $(OUT_ELF_FILE)
	STM32_Programmer_CLI.exe -c port=SWD -w ./$(OUT_ELF_FILE) -v -rst -q
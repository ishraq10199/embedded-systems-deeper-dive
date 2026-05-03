# Default build type is debug
BUILD_TYPE ?= debug

# ANSI Colors
ESC 		:= $(shell printf '\033')
RED     := $(ESC)[0;31m
GREEN   := $(ESC)[0;32m
YELLOW  := $(ESC)[0;33m
BLUE   	:= $(ESC)[0;34m
RESET   := $(ESC)[0m

# Target file name
TARGET = $(notdir $(CURDIR))

# CPU architecture
CPU = -mcpu=cortex-m4
# FPU architecture v4, single precision only, with 16 double-word registers
FPU = -mfpu=fpv4-sp-d16
# Use FPU for fp calculations and pass fp values in FPU registers
FLOAT_ABI = -mfloat-abi=hard

# MCU - combined flags
MCU = $(CPU) -mthumb $(FPU) $(FLOAT_ABI)

# Toolchain
CC = arm-none-eabi-gcc
LD = arm-none-eabi-gcc
OCPY = arm-none-eabi-objcopy

LINKER_SCRIPT = ./linker_script.ld

CFLAGS =	$(MCU) -std=gnu11 -Wall \
					-ffunction-sections -fdata-sections

ifeq ($(BUILD_TYPE), debug)
    CFLAGS += -g3 -DDEBUG -O0
else
    CFLAGS += -Os
endif

INCLUDES =	-I./inc \
						-I$(SHARED_DIR)/inc \
						-I$(BOOT_DIR)/inc \
						-I$(COMMON_DIR)/inc \
						-I../_chip_headers/CMSIS/Include \
						-I../_chip_headers/CMSIS/Device/ST/STM32F4xx/Include

DEFS = -DSTM32 -DSTM32F4 -DSTM32F411RETx -DSTM32F411xE

# 0xFF is the reset value of flash, so we pad it with this byte
# 0x4000 is the space we reserved for the bootrom/bootloader
# We pad the rest of the space, upto where the approm starts
OCPYFLAGS = --gap-fill=0xFF \
						--pad-to=0x08004000

all: $(TARGET).bin $(TARGET).elf
		$(info $(GREEN)[SUCCESS]$(RESET) Build complete! - Output binary:)
		@du -h $<

##########

include shared/shared.mk
include boot/boot.mk

##########

$(TARGET).elf:	$(BOOT_DIR)/$(BOOT_TARGET).elf
		cp $(BOOT_DIR)/$(BOOT_TARGET).elf ./$(TARGET).elf

$(TARGET).bin:	$(BOOT_DIR)/$(BOOT_TARGET).bin
		cp $(BOOT_DIR)/$(BOOT_TARGET).bin ./$(TARGET).bin

clean:
		$(info $(BLUE)[INFO]$(RESET) Cleaning build files)
		rm -f obj/*.o $(TARGET).bin $(TARGET).elf
		rm -f $(COMMON_OBJ_DIR)/*.o
		rm -f $(BOOT_OBJ_DIR)/*.o $(BOOT_DIR)/$(BOOT_TARGET).bin  $(BOOT_DIR)/$(BOOT_TARGET).elf $(BOOT_DIR)/$(BOOT_TARGET).map

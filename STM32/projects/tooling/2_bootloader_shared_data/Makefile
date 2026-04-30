# Default build type is debug
BUILD_TYPE ?= debug

# ANSI Colors
ESC := $(shell printf '\033')
RED     := $(ESC)[0;31m
GREEN   := $(ESC)[0;32m
YELLOW  := $(ESC)[0;33m
BLUE   := $(ESC)[0;34m
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
						-I../chip_headers/CMSIS/Include \
						-I../chip_headers/CMSIS/Device/ST/STM32F4xx/Include

DEFS = -DSTM32 -DSTM32F4 -DSTM32F411RETx -DSTM32F411xE

OBJS =	obj/startup.o \
				obj/main.o \
				obj/syscalls.o \
				obj/sysmem.o

# 0xFF is the reset value of flash, so we pad it with this byte
# 0x4000 is the space we reserved for the bootrom/bootloader
# We pad the rest of the space, upto where the approm starts
OCPYFLAGS = --gap-fill=0xFF \
						--pad-to=0x08004000

all: $(TARGET).bin
		$(info $(GREEN)[SUCCESS]$(RESET) Build complete! - Binary ouput:)
		@du -h $^

##########

include shared/shared.mk
include boot/boot.mk
include app/app.mk

##########

TARGET_DEPS = $(BOOT_DIR)/$(BOOT_TARGET).bin $(APP_DIR)/$(APP_TARGET).bin

$(TARGET).bin:	$(TARGET_DEPS)
		$(info $(BLUE)[INFO]$(RESET) Concatenating bootloader and app binaries)
		cat $^ > $@

clean:
		$(info $(BLUE)[INFO]$(RESET) Cleaning build files)
		rm -f obj/*.o $(TARGET).bin
		rm -f $(COMMON_OBJ_DIR)/*.o
		rm -f $(BOOT_OBJ_DIR)/*.o $(BOOT_DIR)/$(BOOT_TARGET).bin  $(BOOT_DIR)/$(BOOT_TARGET).elf $(BOOT_DIR)/$(BOOT_TARGET).map
		rm -f $(APP_OBJ_DIR)/*.o $(APP_DIR)/$(APP_TARGET).bin  $(APP_DIR)/$(APP_TARGET).elf $(APP_DIR)/$(APP_TARGET).map

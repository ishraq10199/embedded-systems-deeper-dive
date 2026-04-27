# Default build type is debug
BUILD_TYPE ?= debug

# Target file name
TARGET = 0_minimal_blink_with_cmsis_headers

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

LINKER_SCRIPT = ./linker_script.ld

CFLAGS =	$(MCU) -std=gnu11 -Wall \
					-ffunction-sections -fdata-sections

ifeq ($(BUILD_TYPE), debug)
    CFLAGS += -g3 -DDEBUG -O0
else
    CFLAGS += -Os
endif

LDFLAGS =	$(MCU) -T$(LINKER_SCRIPT) -nostdlib \
					-Wl,--gc-sections \
					-Wl,-Map=$(TARGET).map

INCLUDES = 	-I../chip_headers/CMSIS/Include \
						-I../chip_headers/CMSIS/Device/ST/STM32F4xx/Include

DEFS = -DSTM32 -DSTM32F4 -DSTM32F411RETx -DSTM32F411xE
# Object files
OBJS =	obj/startup.o \
				obj/main.o \
				obj/syscalls.o \
				obj/sysmem.o \

all: $(TARGET).elf

# Compile rule
# $@ 	=> `file.o` (the target of the rule)
# $<	=> `file.c` (the first pre-requisite of the rule, only 1 here)
obj/%.o:	src/%.c
		$(CC) $(CFLAGS) $(DEFS) $(INCLUDES) -c -o $@ $<

$(TARGET).elf:	$(OBJS)
		$(LD) $(LDFLAGS) -o $@ $(OBJS)
						
clean:
	rm -f obj/*.o $(TARGET).elf $(TARGET).map
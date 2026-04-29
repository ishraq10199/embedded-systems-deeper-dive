PROJECT_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))/..))
SHARED_DIR := $(abspath $(PROJECT_DIR)/shared)
BOOT_DIR := $(abspath $(PROJECT_DIR)/boot)
BOOT_OBJ_DIR := $(abspath $(BOOT_DIR)/obj)
BOOT_TARGET := bootloader

include $(SHARED_DIR)/shared.mk

BOOT_OBJS := $(SHARED_OBJS) $(BOOT_OBJ_DIR)/bootloader.o

BOOT_LINKER_SCRIPT := $(BOOT_DIR)/boot.ld

BOOT_LDFLAGS := $(MCU) -T$(BOOT_LINKER_SCRIPT) -nostdlib \
								-Wl,--gc-sections \
								-Wl,-Map=$(BOOT_TARGET).map


$(warning $(BOOT_OBJ_DIR))

$(BOOT_OBJ_DIR)/%.o:	$(BOOT_DIR)/src/%.c
		$(CC) $(CFLAGS) $(DEFS) $(INCLUDES) -c -o $@ $<

$(BOOT_DIR)/$(BOOT_TARGET).elf:	$(BOOT_OBJS)
		$(LD) $(BOOT_LDFLAGS) -o $@ $(BOOT_OBJS)

# ONLY PAD THE BOOTLOADER TO 0x4000 with 0xFF
$(BOOT_DIR)/$(BOOT_TARGET).bin:	$(BOOT_DIR)/$(BOOT_TARGET).elf
		$(OCPY) $(OCPYFLAGS) -O binary $< $@
# 		$(OCPY) -O binary $< $@

PROJECT_DIR := .
COMMON_DIR := $(PROJECT_DIR)/common
SHARED_DIR := $(PROJECT_DIR)/shared
BOOT_DIR := $(PROJECT_DIR)/boot
BOOT_OBJ_DIR := $(BOOT_DIR)/obj
BOOT_TARGET := bootloader

include $(COMMON_DIR)/common.mk
include $(SHARED_DIR)/shared.mk

BOOT_OBJS := $(COMMON_OBJS) $(SHARED_OBJ_DIR)/shared.o $(BOOT_OBJ_DIR)/bootloader.o

BOOT_LINKER_SCRIPT := $(BOOT_DIR)/boot.ld

BOOT_LDFLAGS := $(MCU) -T$(BOOT_LINKER_SCRIPT) -nostdlib \
								-Wl,--gc-sections \
								-Wl,-Map=$(BOOT_DIR)/$(BOOT_TARGET).map

$(BOOT_OBJ_DIR)/%.o:	$(BOOT_DIR)/src/%.c
		$(info $(BLUE)[INFO]$(RESET) Building object: $@)
		$(CC) $(CFLAGS) $(DEFS) $(INCLUDES) -c -o $@ $<

$(BOOT_DIR)/$(BOOT_TARGET).elf:	$(BOOT_OBJS)
		$(info $(BLUE)[INFO]$(RESET) Building executable: $@)
		$(LD) $(BOOT_LDFLAGS) -o $@ $(BOOT_OBJS)

# ONLY PAD THE BOOTLOADER TO 0x4000 with 0xFF
$(BOOT_DIR)/$(BOOT_TARGET).bin:	$(BOOT_DIR)/$(BOOT_TARGET).elf
		$(info $(BLUE)[INFO]$(RESET) Converting executable file to binary with padding: boot)
		$(OCPY) $(OCPYFLAGS) -O binary $< $@

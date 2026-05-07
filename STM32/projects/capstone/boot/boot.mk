PROJECT_DIR := .
COMMON_DIR := $(PROJECT_DIR)/common
SHARED_DIR := $(PROJECT_DIR)/shared
BOOT_DIR := $(PROJECT_DIR)/boot
BOOT_OBJ_DIR := $(BOOT_DIR)/obj
BOOT_TARGET := bootloader

include $(COMMON_DIR)/common.mk
include $(SHARED_DIR)/shared.mk

BOOT_OBJS := 	$(COMMON_OBJS) $(SHARED_OBJ_DIR)/shared.o \
							$(BOOT_OBJ_DIR)/firmware.o \
							$(BOOT_OBJ_DIR)/bootloader.o

BOOT_LINKER_SCRIPT := $(BOOT_DIR)/boot.ld

# Before, we were using the `-nostdlib` flag, which made us not able to use the C Standard Library
# Therefore, using `memcpy`, `printf`, and other useful functions was not possible
# Simply omitting this flag makes Newlib available to us
BOOT_LDFLAGS := $(MCU) -T$(BOOT_LINKER_SCRIPT) \
								-Wl,--gc-sections \
								-Wl,-Map=$(BOOT_DIR)/$(BOOT_TARGET).map

# We use the nano version of libc and libm, which are less performant,
# But are built with embedded platforms in mind. It consumes less resources.
# Also, nosys gives us empty specs for the syscalls required for libc
# We would be overriding them with our implementations in `syscalls.c`
# For float formatting, (in printf and scanf), we need to opt-in.
# This is done with `-u _printf_float` and `-u _scanf_float`
BOOT_LDFLAGS += 	\
								-static --specs=nano.specs --specs=nosys.specs

# By default, the GNU linker processes libraries in a single left-to-right pass.
# A symbol must be referenced before the library that defines it is seen.
# If library A needs a symbol from library B, and B also needs something from A,
# a single pass will leave one side unresolved.
#
# --start-group / --end-group instructs the linker to repeatedly re-scan all 
# libraries inside the group until no new undefined symbols are resolved.
BOOT_LDFLAGS +=	\
								-Wl,--start-group -lc -lm -Wl,--end-group

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

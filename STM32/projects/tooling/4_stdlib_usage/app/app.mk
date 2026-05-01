PROJECT_DIR := .
COMMON_DIR := $(PROJECT_DIR)/common
SHARED_DIR := $(PROJECT_DIR)/shared
APP_DIR := $(PROJECT_DIR)/app
APP_OBJ_DIR := $(APP_DIR)/obj
APP_TARGET := app

include $(COMMON_DIR)/common.mk
include $(SHARED_DIR)/shared.mk

APP_OBJS := $(COMMON_OBJS) $(SHARED_OBJ_DIR)/shared.o $(APP_OBJ_DIR)/app.o

APP_LINKER_SCRIPT := $(APP_DIR)/app.ld

# Before, we were using the `-nostdlib` flag, which made us not able to use the C Standard Library
# Therefore, using `memcpy`, `printf`, and other useful functions was not possible
# Simply omitting this flag makes Newlib available to us
APP_LDFLAGS := $(MCU) -T$(APP_LINKER_SCRIPT) \
								-Wl,--gc-sections \
								-Wl,-Map=$(APP_DIR)/$(APP_TARGET).map

# We use the nano version of libc and libm, which are less performant,
# But are built with embedded platforms in mind. It consumes less resources.
# Also, nosys gives us empty specs for the syscalls required for libc
# We would be overriding them with our implementations in `syscalls.c`
# For float formatting, (in printf and scanf), we need to opt-in.
# This is done with `-u _printf_float` and `-u _scanf_float`
APP_LDFLAGS += 	\
								-static --specs=nano.specs --specs=nosys.specs

# By default, the GNU linker processes libraries in a single left-to-right pass.
# A symbol must be referenced before the library that defines it is seen.
# If library A needs a symbol from library B, and B also needs something from A,
# a single pass will leave one side unresolved.
#
# --start-group / --end-group instructs the linker to repeatedly re-scan all 
# libraries inside the group until no new undefined symbols are resolved.
APP_LDFLAGS +=	\
								-Wl,--start-group -lc -lm -Wl,--end-group

$(APP_OBJ_DIR)/%.o:	$(APP_DIR)/src/%.c
		$(info $(BLUE)[INFO]$(RESET) Building object: $@)
		$(CC) $(CFLAGS) $(DEFS) $(INCLUDES) -c -o $@ $<

$(APP_DIR)/$(APP_TARGET).elf:	$(APP_OBJS)
		$(info $(BLUE)[INFO]$(RESET) Building executable: $@)
		$(LD) $(APP_LDFLAGS) -o $@ $(APP_OBJS)

$(APP_DIR)/$(APP_TARGET).bin:	$(APP_DIR)/$(APP_TARGET).elf
		$(info $(BLUE)[INFO]$(RESET) Converting executable file to binary: app)
		$(OCPY) -O binary $< $@

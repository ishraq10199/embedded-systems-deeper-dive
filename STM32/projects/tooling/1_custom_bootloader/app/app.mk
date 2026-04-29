PROJECT_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))/..))
SHARED_DIR := $(abspath $(PROJECT_DIR)/shared)
APP_DIR := $(abspath $(PROJECT_DIR)/app)
APP_OBJ_DIR := $(abspath $(APP_DIR)/obj)
APP_TARGET := app

include $(SHARED_DIR)/shared.mk

APP_OBJS := $(SHARED_OBJS) $(APP_OBJ_DIR)/app.o

APP_LINKER_SCRIPT := $(APP_DIR)/app.ld

APP_LDFLAGS := $(MCU) -T$(APP_LINKER_SCRIPT) -nostdlib \
								-Wl,--gc-sections \
								-Wl,-Map=$(APP_TARGET).map


$(warning $(APP_OBJ_DIR))

$(APP_OBJ_DIR)/%.o:	$(APP_DIR)/src/%.c
		$(CC) $(CFLAGS) $(DEFS) $(INCLUDES) -c -o $@ $<

$(APP_DIR)/$(APP_TARGET).elf:	$(APP_OBJS)
		$(LD) $(APP_LDFLAGS) -o $@ $(APP_OBJS)

$(APP_DIR)/$(APP_TARGET).bin:	$(APP_DIR)/$(APP_TARGET).elf
		$(OCPY) -O binary $< $@

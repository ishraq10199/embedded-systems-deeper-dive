PROJECT_DIR := .
SHARED_DIR := $(PROJECT_DIR)/shared
APP_DIR := $(PROJECT_DIR)/app
APP_OBJ_DIR := $(APP_DIR)/obj
APP_TARGET := app

include $(SHARED_DIR)/shared.mk

APP_OBJS := $(SHARED_OBJS) $(APP_OBJ_DIR)/app.o

APP_LINKER_SCRIPT := $(APP_DIR)/app.ld

APP_LDFLAGS := $(MCU) -T$(APP_LINKER_SCRIPT) -nostdlib \
								-Wl,--gc-sections \
								-Wl,-Map=$(APP_DIR)/$(APP_TARGET).map

$(APP_OBJ_DIR)/%.o:	$(APP_DIR)/src/%.c
		$(info $(BLUE)[INFO]$(RESET) Building object: $@)
		$(CC) $(CFLAGS) $(DEFS) $(INCLUDES) -c -o $@ $<

$(APP_DIR)/$(APP_TARGET).elf:	$(APP_OBJS)
		$(info $(BLUE)[INFO]$(RESET) Building executable: $@)
		$(LD) $(APP_LDFLAGS) -o $@ $(APP_OBJS)

$(APP_DIR)/$(APP_TARGET).bin:	$(APP_DIR)/$(APP_TARGET).elf
		$(info $(BLUE)[INFO]$(RESET) Converting executable file to binary: app)
		$(OCPY) -O binary $< $@

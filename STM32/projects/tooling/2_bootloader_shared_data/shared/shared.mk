PROJECT_DIR := .
SHARED_DIR := $(PROJECT_DIR)/shared
SHARED_OBJ_DIR := $(SHARED_DIR)/obj
SHARED_TARGET := shared

SHARED_LDFLAGS := $(MCU) -nostdlib \
									-Wl,--gc-sections \
									-Wl,-Map=$(SHARED_DIR)/$(SHARED_TARGET).map

$(SHARED_OBJ_DIR)/%.o:	$(SHARED_DIR)/src/%.c
		$(info $(BLUE)[INFO]$(RESET) Building object: $@)
		$(CC) $(CFLAGS) $(DEFS) $(INCLUDES) -c -o $@ $<

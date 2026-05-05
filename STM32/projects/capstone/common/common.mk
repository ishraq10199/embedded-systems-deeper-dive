PROJECT_DIR := .
COMMON_DIR := $(PROJECT_DIR)/common
COMMON_OBJ_DIR := $(COMMON_DIR)/obj

COMMON_SRCS := 	$(COMMON_DIR)/src/startup.c \
								$(COMMON_DIR)/src/syscalls.c \
								$(COMMON_DIR)/src/uart.c \
								$(COMMON_DIR)/src/flash.c



COMMON_OBJS := 	$(COMMON_OBJ_DIR)/startup.o \
								$(COMMON_OBJ_DIR)/syscalls.o \
								$(COMMON_OBJ_DIR)/uart.o \
								$(COMMON_OBJ_DIR)/flash.o

$(COMMON_OBJ_DIR)/%.o:	$(COMMON_DIR)/src/%.c
		$(info $(BLUE)[INFO]$(RESET) Building object: $@)
		$(CC) $(CFLAGS) $(DEFS) $(INCLUDES) -c -o $@ $<

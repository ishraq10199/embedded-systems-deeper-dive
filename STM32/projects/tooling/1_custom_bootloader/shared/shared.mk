PROJECT_DIR := .
SHARED_DIR := $(PROJECT_DIR)/shared
SHARED_OBJ_DIR := $(SHARED_DIR)/obj

SHARED_SRCS := 	$(SHARED_DIR)/src/startup.c \
								$(SHARED_DIR)/src/syscalls.c \
								$(SHARED_DIR)/src/sysmem.c


SHARED_OBJS := 	$(SHARED_OBJ_DIR)/startup.o \
								$(SHARED_OBJ_DIR)/syscalls.o \
								$(SHARED_OBJ_DIR)/sysmem.o


$(SHARED_OBJ_DIR)/%.o:	$(SHARED_DIR)/src/%.c
		$(info $(BLUE)[INFO]$(RESET) Building object: $@)
		$(CC) $(CFLAGS) $(DEFS) $(INCLUDES) -c -o $@ $<

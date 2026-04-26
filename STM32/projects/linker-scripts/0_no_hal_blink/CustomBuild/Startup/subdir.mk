# Add inputs and outputs from these tool invocations to the build variables 
S_SRCS += \
../Startup/startup.c

OBJS += \
./Startup/startup.o 

S_DEPS += \
./Startup/startup.d 


# Each subdirectory must supply rules for building sources it contributes
# Startup/%.o: ../Startup/%.s Startup/subdir.mk
#	arm-none-eabi-gcc -mcpu=cortex-m4 -c -x assembler-with-cpp -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@" "$<"

# Use our own startup.c file
Startup/%.o: ../Startup/%.c Startup/subdir.mk
	arm-none-eabi-gcc -mcpu=cortex-m4 -std=gnu11 -c -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@" "$<"

clean: clean-Startup

clean-Startup:
	-$(RM) ./Startup/startup.d ./Startup/startup.o

.PHONY: clean-Startup


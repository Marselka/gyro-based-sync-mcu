################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include Drivers/STM32F4xx_HAL_Driver/Src/subdir.mk
-include Core/Startup/subdir.mk
-include Core/Src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
gyro_based_sync.elf \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
gyro_based_sync.list \

OBJCOPY_BIN += \
gyro_based_sync.bin \


# All Target
all: gyro_based_sync.elf secondary-outputs

# Tool invocations
gyro_based_sync.elf: $(OBJS) $(USER_OBJS) /home/ma/STM32CubeIDE/gyro_based_sync/gyro_based_sync/STM32F407VGTX_FLASH.ld
	arm-none-eabi-gcc -o "gyro_based_sync.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m4 -T"/home/ma/STM32CubeIDE/gyro_based_sync/gyro_based_sync/STM32F407VGTX_FLASH.ld" --specs=nosys.specs -Wl,-Map="gyro_based_sync.map" -Wl,--gc-sections -static --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

gyro_based_sync.list: $(EXECUTABLES)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "gyro_based_sync.list"
	@echo 'Finished building: $@'
	@echo ' '

gyro_based_sync.bin: $(EXECUTABLES)
	arm-none-eabi-objcopy  -O binary $(EXECUTABLES) "gyro_based_sync.bin"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) *
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST) $(OBJCOPY_BIN)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets

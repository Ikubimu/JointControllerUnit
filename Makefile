# Makefile for STM32F103C8T6 (Blue Pill)
# Using ARM GCC Toolchain

TARGET = JointControllerUnit

# Toolchain paths
TOOLCHAIN_PATH = /opt/st/stm32cubeclt_1.19.0/GNU-tools-for-STM32/bin
PREFIX = arm-none-eabi-

CC = $(TOOLCHAIN_PATH)/$(PREFIX)gcc
AS = $(TOOLCHAIN_PATH)/$(PREFIX)gcc
OC = $(TOOLCHAIN_PATH)/$(PREFIX)objcopy
OD = $(TOOLCHAIN_PATH)/$(PREFIX)objdump
OS = $(TOOLCHAIN_PATH)/$(PREFIX)size

# Directories
PROJECT_ROOT = .
BUILD_DIR = $(PROJECT_ROOT)/build
SRC_DIR = $(PROJECT_ROOT)/Core/Src
DRIVERS_DIR = $(PROJECT_ROOT)/Drivers
CMSIS_DIR = $(DRIVERS_DIR)/CMSIS
HAL_DIR = $(DRIVERS_DIR)/STM32F1xx_HAL_Driver
DEVICE_DIR = $(CMSIS_DIR)/Device/ST/STM32F1xx

# MCU defines for STM32F103C8T6
DEFS = -DUSE_HAL_DRIVER -DSTM32F103xB

# Include paths
INCLUDES = \
	-I$(DEVICE_DIR)/Include \
	-I$(CMSIS_DIR)/Include \
	-I$(CMSIS_DIR)/Include/arm \
	-I$(HAL_DIR)/Inc \
	-I$(SRC_DIR)/../Inc

# Optimization level
OPT = -Og

# Compiler flags for Cortex-M3
CFLAGS = $(OPT) -Wall -fdata-sections -ffunction-sections \
	--specs=nano.specs --specs=nosys.specs \
	-mcpu=cortex-m3 -mthumb \
	-fno-delete-null-pointer-checks -fno-strict-aliasing \
	$(DEFS) $(INCLUDES)

ASFLAGS = $(OPT) -Wall -fdata-sections -ffunction-sections \
	--specs=nano.specs --specs=nosys.specs \
	-mcpu=cortex-m3 -mthumb \
	-x assembler-with-cpp $(DEFS) $(INCLUDES)

LDFLAGS = -T$(PROJECT_ROOT)/STM32F103C8T6_FLASH.ld \
	-Wl,--gc-sections -Wl,-Map=$(BUILD_DIR)/$(TARGET).map \
	-mcpu=cortex-m3 -mthumb \
	--specs=nano.specs --specs=nosys.specs \
	-Wl,--start-group -lc -lm -lnosys -Wl,--end-group

# Source files
C_SOURCES = \
	$(SRC_DIR)/main.c \
	$(SRC_DIR)/stm32f1xx_it.c \
	$(SRC_DIR)/stm32f1xx_hal_msp.c \
	$(SRC_DIR)/system_stm32f1xx.c \
	$(HAL_DIR)/Src/stm32f1xx_hal.c \
	$(HAL_DIR)/Src/stm32f1xx_hal_cortex.c \
	$(HAL_DIR)/Src/stm32f1xx_hal_rcc.c \
	$(HAL_DIR)/Src/stm32f1xx_hal_rcc_ex.c \
	$(HAL_DIR)/Src/stm32f1xx_hal_gpio.c \
	$(HAL_DIR)/Src/stm32f1xx_hal_gpio_ex.c \
	$(HAL_DIR)/Src/stm32f1xx_hal_exti.c \
	$(HAL_DIR)/Src/stm32f1xx_hal_dma.c \
	$(HAL_DIR)/Src/stm32f1xx_hal_pwr.c \
	$(HAL_DIR)/Src/stm32f1xx_hal_flash.c \
	$(HAL_DIR)/Src/stm32f1xx_hal_flash_ex.c \
	$(HAL_DIR)/Src/stm32f1xx_hal_adc.c \
	$(HAL_DIR)/Src/stm32f1xx_hal_adc_ex.c \
	$(HAL_DIR)/Src/stm32f1xx_hal_can.c \
	$(HAL_DIR)/Src/stm32f1xx_hal_tim.c \
	$(HAL_DIR)/Src/stm32f1xx_hal_tim_ex.c

ASM_SOURCES = \
	$(DEVICE_DIR)/Source/Templates/gcc/startup_stm32f103xb.s

# Object files (flattened to single directory)
OBJECTS = $(patsubst $(PROJECT_ROOT)/%,$(BUILD_DIR)/%,$(C_SOURCES:%.c=%.o))
OBJECTS += $(patsubst $(PROJECT_ROOT)/%,$(BUILD_DIR)/%,$(ASM_SOURCES:%.s=%.o))

# Pattern rules
$(BUILD_DIR)/%.o: $(PROJECT_ROOT)/%.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	@echo "Compiling $<"
	$(CC) -c $(CFLAGS) -o $@ $<

$(BUILD_DIR)/%.o: $(PROJECT_ROOT)/%.s | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	@echo "Assembling $<"
	$(AS) -c $(ASFLAGS) -o $@ $<

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS)
	@echo "Linking $@"
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	@echo "Creating binary..."
	$(OC) -O binary $@ $(BUILD_DIR)/$(TARGET).bin
	@echo "Creating hex..."
	$(OC) -O ihex $@ $(BUILD_DIR)/$(TARGET).hex
	@echo "Creating listing..."
	$(OD) -h -S $@ > $(BUILD_DIR)/$(TARGET).lst
	@echo "Size:"
	$(OS) $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

.PHONY: all clean

all: $(BUILD_DIR)/$(TARGET).elf
	@echo "Build complete!"

clean:
	rm -rf $(BUILD_DIR)
	@echo "Clean complete!"

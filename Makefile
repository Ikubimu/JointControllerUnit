# Makefile for STM32F103C8T6 (Blue Pill)
# Using ARM GCC Toolchain

TARGET = JointControllerUnit

# Toolchain paths
TOOLCHAIN_PATH = /opt/st/stm32cubeclt_1.19.0/GNU-tools-for-STM32/bin
PREFIX = arm-none-eabi-

CC = $(TOOLCHAIN_PATH)/$(PREFIX)gcc
CXX = $(TOOLCHAIN_PATH)/$(PREFIX)g++
LD = $(TOOLCHAIN_PATH)/$(PREFIX)g++
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
FREERTOS_DIR = $(PROJECT_ROOT)/FreeRTOS

# MCU defines for STM32F103C8T6
DEFS = -DUSE_HAL_DRIVER -DSTM32F103xB

# Include paths
INCLUDES = \
	-I$(DEVICE_DIR)/Include \
	-I$(CMSIS_DIR)/Include \
	-I$(CMSIS_DIR)/Include/arm \
	-I$(HAL_DIR)/Inc \
	-I$(SRC_DIR)/../Inc \
	-I$(SRC_DIR)/../Inc/LOW \
	-I$(SRC_DIR)/../Inc/StateMachine \
	-I$(FREERTOS_DIR) \
	-I$(FREERTOS_DIR)/Source/include \
	-I$(FREERTOS_DIR)/Source/portable/GCC/ARM_CM3

# Optimization level
OPT = -Og

# Common flags
COMMON_FLAGS = $(OPT) -Wall -fdata-sections -ffunction-sections \
	--specs=nano.specs --specs=nosys.specs \
	-mcpu=cortex-m3 -mthumb \
	-fno-delete-null-pointer-checks -fno-strict-aliasing \
	$(DEFS) $(INCLUDES)

# Compiler flags
CFLAGS = $(COMMON_FLAGS)
CXXFLAGS = $(COMMON_FLAGS) -std=c++17 -fno-exceptions -fno-rtti -fno-use-cxa-atexit

ASFLAGS = $(OPT) -Wall -fdata-sections -ffunction-sections \
	--specs=nano.specs --specs=nosys.specs \
	-mcpu=cortex-m3 -mthumb \
	-x assembler-with-cpp $(DEFS) $(INCLUDES)

LDFLAGS = -T$(PROJECT_ROOT)/STM32F103C8T6_FLASH.ld \
	-Wl,--gc-sections -Wl,-Map=$(BUILD_DIR)/$(TARGET).map \
	-mcpu=cortex-m3 -mthumb \
	--specs=nano.specs --specs=nosys.specs \
	-u _printf_float \
	-Wl,--start-group -lc -lm -lstdc++ -lnosys -Wl,--end-group

# C source files
C_SOURCES = \
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
	$(HAL_DIR)/Src/stm32f1xx_hal_tim_ex.c \
	$(HAL_DIR)/Src/stm32f1xx_hal_uart.c \
	$(FREERTOS_DIR)/Source/tasks.c \
	$(FREERTOS_DIR)/Source/queue.c \
	$(FREERTOS_DIR)/Source/list.c \
	$(FREERTOS_DIR)/Source/timers.c \
	$(FREERTOS_DIR)/Source/event_groups.c \
	$(FREERTOS_DIR)/Source/stream_buffer.c \
	$(FREERTOS_DIR)/Source/croutine.c \
	$(FREERTOS_DIR)/Source/portable/GCC/ARM_CM3/port.c \
	$(FREERTOS_DIR)/Source/portable/MemMang/heap_4.c

# C++ source files
CPP_SOURCES = \
	$(SRC_DIR)/main.cpp \
	$(SRC_DIR)/JointController.cpp \
	$(SRC_DIR)/StateMachine/State.cpp \
	$(SRC_DIR)/StateMachine/StateMachine.cpp \
	$(SRC_DIR)/LOW/DigitalOutput.cpp \
	$(SRC_DIR)/LOW/PWM.cpp \
	$(SRC_DIR)/LOW/ADC.cpp \
	$(SRC_DIR)/LOW/CAN.cpp \
	$(SRC_DIR)/LOW/Peripherals.cpp \
	$(SRC_DIR)/CommunicationHandler.cpp \
	$(SRC_DIR)/Motor.cpp

ASM_SOURCES = \
	$(DEVICE_DIR)/Source/Templates/gcc/startup_stm32f103xb.s

# Object files (flattened to single directory)
OBJECTS = $(patsubst $(PROJECT_ROOT)/%,$(BUILD_DIR)/%,$(C_SOURCES:%.c=%.o))
OBJECTS += $(patsubst $(PROJECT_ROOT)/%,$(BUILD_DIR)/%,$(CPP_SOURCES:%.cpp=%.o))
OBJECTS += $(patsubst $(PROJECT_ROOT)/%,$(BUILD_DIR)/%,$(ASM_SOURCES:%.s=%.o))

# Pattern rules
$(BUILD_DIR)/%.o: $(PROJECT_ROOT)/%.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	@echo "Compiling $<"
	$(CC) -c $(CFLAGS) -o $@ $<

$(BUILD_DIR)/%.o: $(PROJECT_ROOT)/%.cpp | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	@echo "Compiling $<"
	$(CXX) -c $(CXXFLAGS) -o $@ $<

$(BUILD_DIR)/%.o: $(PROJECT_ROOT)/%.s | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	@echo "Assembling $<"
	$(AS) -c $(ASFLAGS) -o $@ $<

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS)
	@echo "Linking $@"
	$(LD) $(OBJECTS) $(LDFLAGS) -o $@
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

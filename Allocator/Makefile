TARGET = nrf52840_bare_metal
CPU = cortex-m4
LINKER_SCRIPT = ./src/nrf52840_xxaa.ld
TOOLCHAIN_BASE = ~/arm-gnu-toolchain-15.3.rel1-x86_64-arm-none-eabi/bin

CC = $(TOOLCHAIN_BASE)/arm-none-eabi-gcc
OBJCOPY = $(TOOLCHAIN_BASE)/arm-none-eabi-objcopy
SIZE = $(TOOLCHAIN_BASE)/arm-none-eabi-size

# Define source and build directories
SRC_DIR = src
BUILD_DIR = build

CFLAGS = -mcpu=$(CPU) -mthumb -mabi=aapcs -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Wall -Wextra -Wpedantic -O0 -g
CFLAGS += --specs=nano.specs -specs=nosys.specs

LDFLAGS = -T$(LINKER_SCRIPT) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map
LDFLAGS += -lc -lnosys

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

.PHONY: all clean

all: $(BUILD_DIR)/$(TARGET).bin size

$(BUILD_DIR)/$(TARGET).bin: $(BUILD_DIR)/$(TARGET).elf
	$(OBJCOPY) -O binary $< $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $@

size: $(BUILD_DIR)/$(TARGET).elf
	$(SIZE) $<

clean:
	rm -rf $(BUILD_DIR)

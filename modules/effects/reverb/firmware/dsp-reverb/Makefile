# Project Name
PROJECT = dsp-reverb

# Directories
BUILD_DIR = cmake-build-debug
TOOLCHAIN_FILE = arm-none-eabi-toolchain.cmake

# Commands
CMAKE = cmake
MAKE = make
RM = rm -rf

# Default target
all: build

# Create build directory and generate cmake files
cmake:
	@echo "Generating CMake files..."
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && $(CMAKE) .. -DCMAKE_TOOLCHAIN_FILE=../$(TOOLCHAIN_FILE) -G "Unix Makefiles"

# Build the project
build: cmake
	@echo "Building project..."
	@cd $(BUILD_DIR) && $(MAKE)

# Clean build directory
clean:
	@echo "Cleaning build directory..."
	@$(RM) $(BUILD_DIR)

# Flash the device using ST-Link
flash: build
	@echo "Flashing device..."
	@st-flash write $(BUILD_DIR)/$(PROJECT).elf.bin 0x8000000

# Optional: add verify version
flash-verify: build
	@echo "Flashing device with verification..."
	@st-flash --verify write $(BUILD_DIR)/$(PROJECT).elf.bin 0x8000000

# Optional: add erase command
erase:
	@echo "Erasing device..."
	@st-flash erase

# Debug using OpenOCD
debug: build
	@echo "Starting OpenOCD debug session..."
	@openocd -f interface/stlink.cfg -f target/stm32g4x.cfg

# Start GDB session
gdb: build
	@echo "Starting GDB session..."
	@arm-none-eabi-gdb $(BUILD_DIR)/$(PROJECT).elf

# Show size of compiled binary
size: build
	@arm-none-eabi-size $(BUILD_DIR)/$(PROJECT).elf

# Help target
help:
	@echo "Available targets:"
	@echo "  make          - Generate CMake files and build project"
	@echo "  make build    - Build the project"
	@echo "  make clean    - Remove build directory"
	@echo "  make flash    - Flash binary to device using ST-Link"
	@echo "  make debug    - Start OpenOCD debug session"
	@echo "  make gdb      - Start GDB session"
	@echo "  make size     - Show binary size"
	@echo "  make help     - Show this help message"

.PHONY: all cmake build clean flash debug gdb size help
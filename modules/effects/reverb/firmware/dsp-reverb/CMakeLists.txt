cmake_minimum_required(VERSION 3.20)

# Specify toolchain file before project declaration
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/arm-none-eabi-toolchain.cmake)

# Disable compiler checks
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# Move project() before enable_language
project(dsp-reverb C CXX ASM)

# Enable ASM after project declaration
enable_language(ASM)

# Specify STM32 specific definitions
add_definitions(
        -DUSE_HAL_DRIVER
        -DSTM32G474xx
)

# Include directories
include_directories(
        Core/Inc
        Drivers/STM32G4xx_HAL_Driver/Inc
        Drivers/STM32G4xx_HAL_Driver/Inc/Legacy
        Drivers/CMSIS/Device/ST/STM32G4xx/Include
        Drivers/CMSIS/Include
)

# Source files
file(GLOB_RECURSE SOURCES
        "Core/Src/*.c"
        "Core/Startup/*.s"
        "Drivers/STM32G4xx_HAL_Driver/Src/*.c"
)

# Create executable
add_executable(${PROJECT_NAME}.elf ${SOURCES})

# Linker script
set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/STM32G474RBTx_FLASH.ld)

# Updated linker options
target_link_options(${PROJECT_NAME}.elf PRIVATE
        -T${LINKER_SCRIPT}
        -Wl,-Map=${PROJECT_NAME}.map
        -Wl,--gc-sections
        -Wl,--start-group
        -lc
        -lm
        -lstdc++
        -lsupc++
        -Wl,--end-group
        --specs=nosys.specs
        -static
        -Wl,--cref
        -Wl,--no-warn-mismatch
        -Wl,--no-undefined
)

# Print size after build
add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
        COMMAND arm-none-eabi-size $<TARGET_FILE:${PROJECT_NAME}.elf>
)

# Create HEX and BIN files
add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
        COMMAND arm-none-eabi-objcopy -O ihex $<TARGET_FILE:${PROJECT_NAME}.elf> ${PROJECT_NAME}.hex
        COMMAND arm-none-eabi-objcopy -O binary $<TARGET_FILE:${PROJECT_NAME}.elf> ${PROJECT_NAME}.bin
)
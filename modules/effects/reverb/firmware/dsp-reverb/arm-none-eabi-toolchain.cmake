set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR ARM)

# Specify the cross compiler
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)

# Search for programs only in host directories
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# STM32 specific flags
set(CPU_FLAGS "-mthumb -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard")
set(COMMON_FLAGS "-ffunction-sections -fdata-sections -fno-common -fmessage-length=0")

# Remove all specs flags from here
set(CMAKE_C_FLAGS "${CPU_FLAGS} ${COMMON_FLAGS}" CACHE STRING "C compiler flags")
set(CMAKE_CXX_FLAGS "${CPU_FLAGS} ${COMMON_FLAGS}" CACHE STRING "C++ compiler flags")
set(CMAKE_ASM_FLAGS "${CPU_FLAGS}" CACHE STRING "ASM compiler flags")

# Set executable suffix
set(CMAKE_EXECUTABLE_SUFFIX_C ".elf")
set(CMAKE_EXECUTABLE_SUFFIX_CXX ".elf")

# Basic linker flags without specs
set(CMAKE_EXE_LINKER_FLAGS "${CPU_FLAGS} -Wl,--gc-sections" CACHE STRING "Linker flags")
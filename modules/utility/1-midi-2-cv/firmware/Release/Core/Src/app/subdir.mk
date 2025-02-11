################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/app/buffer.c \
../Core/Src/app/mcp4728.c \
../Core/Src/app/midi.c \
../Core/Src/app/midi_handler.c 

OBJS += \
./Core/Src/app/buffer.o \
./Core/Src/app/mcp4728.o \
./Core/Src/app/midi.o \
./Core/Src/app/midi_handler.o 

C_DEPS += \
./Core/Src/app/buffer.d \
./Core/Src/app/mcp4728.d \
./Core/Src/app/midi.d \
./Core/Src/app/midi_handler.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/app/%.o Core/Src/app/%.su Core/Src/app/%.cyclo: ../Core/Src/app/%.c Core/Src/app/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-app

clean-Core-2f-Src-2f-app:
	-$(RM) ./Core/Src/app/buffer.cyclo ./Core/Src/app/buffer.d ./Core/Src/app/buffer.o ./Core/Src/app/buffer.su ./Core/Src/app/mcp4728.cyclo ./Core/Src/app/mcp4728.d ./Core/Src/app/mcp4728.o ./Core/Src/app/mcp4728.su ./Core/Src/app/midi.cyclo ./Core/Src/app/midi.d ./Core/Src/app/midi.o ./Core/Src/app/midi.su ./Core/Src/app/midi_handler.cyclo ./Core/Src/app/midi_handler.d ./Core/Src/app/midi_handler.o ./Core/Src/app/midi_handler.su

.PHONY: clean-Core-2f-Src-2f-app


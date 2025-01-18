################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/app/command_queue.c \
../Core/Src/app/encoder.c \
../Core/Src/app/main_app.c \
../Core/Src/app/mcp4822.c \
../Core/Src/app/sequencer.c \
../Core/Src/app/ui.c 

OBJS += \
./Core/Src/app/command_queue.o \
./Core/Src/app/encoder.o \
./Core/Src/app/main_app.o \
./Core/Src/app/mcp4822.o \
./Core/Src/app/sequencer.o \
./Core/Src/app/ui.o 

C_DEPS += \
./Core/Src/app/command_queue.d \
./Core/Src/app/encoder.d \
./Core/Src/app/main_app.d \
./Core/Src/app/mcp4822.d \
./Core/Src/app/sequencer.d \
./Core/Src/app/ui.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/app/%.o Core/Src/app/%.su Core/Src/app/%.cyclo: ../Core/Src/app/%.c Core/Src/app/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-app

clean-Core-2f-Src-2f-app:
	-$(RM) ./Core/Src/app/command_queue.cyclo ./Core/Src/app/command_queue.d ./Core/Src/app/command_queue.o ./Core/Src/app/command_queue.su ./Core/Src/app/encoder.cyclo ./Core/Src/app/encoder.d ./Core/Src/app/encoder.o ./Core/Src/app/encoder.su ./Core/Src/app/main_app.cyclo ./Core/Src/app/main_app.d ./Core/Src/app/main_app.o ./Core/Src/app/main_app.su ./Core/Src/app/mcp4822.cyclo ./Core/Src/app/mcp4822.d ./Core/Src/app/mcp4822.o ./Core/Src/app/mcp4822.su ./Core/Src/app/sequencer.cyclo ./Core/Src/app/sequencer.d ./Core/Src/app/sequencer.o ./Core/Src/app/sequencer.su ./Core/Src/app/ui.cyclo ./Core/Src/app/ui.d ./Core/Src/app/ui.o ./Core/Src/app/ui.su

.PHONY: clean-Core-2f-Src-2f-app


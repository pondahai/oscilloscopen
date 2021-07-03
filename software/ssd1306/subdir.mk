################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../ssd1306/ssd1306.c \
../ssd1306/ssd1306_fonts.c \
../ssd1306/ssd1306_tests.c 

OBJS += \
./ssd1306/ssd1306.o \
./ssd1306/ssd1306_fonts.o \
./ssd1306/ssd1306_tests.o 

C_DEPS += \
./ssd1306/ssd1306.d \
./ssd1306/ssd1306_fonts.d \
./ssd1306/ssd1306_tests.d 


# Each subdirectory must supply rules for building sources it contributes
ssd1306/%.o: ../ssd1306/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -mfloat-abi=soft '-D__weak=__attribute__((weak))' '-D__packed="__attribute__((__packed__))"' -DUSE_HAL_DRIVER -DSTM32F103xB -DSSD1306_USE_I2C -DSTM32F1 -I"/Users/pondahai/Dropbox/oscilloscopen/v1/Inc" -I"/Users/pondahai/Dropbox/oscilloscopen/v1/Drivers/STM32F1xx_HAL_Driver/Inc" -I"/Users/pondahai/Dropbox/oscilloscopen/v1/Drivers/STM32F1xx_HAL_Driver/Inc/Legacy" -I"/Users/pondahai/Dropbox/oscilloscopen/v1/Drivers/CMSIS/Device/ST/STM32F1xx/Include" -I"/Users/pondahai/Dropbox/oscilloscopen/v1/Drivers/CMSIS/Include" -I"/Users/pondahai/Dropbox/oscilloscopen/v1/ssd1306"  -Og -g3 -Wall -fmessage-length=0 -ffunction-sections -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



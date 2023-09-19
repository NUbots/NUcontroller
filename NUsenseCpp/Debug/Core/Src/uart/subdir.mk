################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Core/Src/uart/Port.cpp \
../Core/Src/uart/RS485.cpp 

OBJS += \
./Core/Src/uart/Port.o \
./Core/Src/uart/RS485.o 

CPP_DEPS += \
./Core/Src/uart/Port.d \
./Core/Src/uart/RS485.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/uart/%.o Core/Src/uart/%.su Core/Src/uart/%.cyclo: ../Core/Src/uart/%.cpp Core/Src/uart/subdir.mk
	arm-none-eabi-g++ "$<" -mcpu=cortex-m7 -std=gnu++14 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H753xx -c -I../Core/Inc -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-use-cxa-atexit -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-uart

clean-Core-2f-Src-2f-uart:
	-$(RM) ./Core/Src/uart/Port.cyclo ./Core/Src/uart/Port.d ./Core/Src/uart/Port.o ./Core/Src/uart/Port.su ./Core/Src/uart/RS485.cyclo ./Core/Src/uart/RS485.d ./Core/Src/uart/RS485.o ./Core/Src/uart/RS485.su

.PHONY: clean-Core-2f-Src-2f-uart


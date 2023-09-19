################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Core/Src/platform/NUsense/Convert.cpp \
../Core/Src/platform/NUsense/NUgus.cpp 

OBJS += \
./Core/Src/platform/NUsense/Convert.o \
./Core/Src/platform/NUsense/NUgus.o 

CPP_DEPS += \
./Core/Src/platform/NUsense/Convert.d \
./Core/Src/platform/NUsense/NUgus.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/platform/NUsense/%.o Core/Src/platform/NUsense/%.su Core/Src/platform/NUsense/%.cyclo: ../Core/Src/platform/NUsense/%.cpp Core/Src/platform/NUsense/subdir.mk
	arm-none-eabi-g++ "$<" -mcpu=cortex-m7 -std=gnu++14 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H753xx -c -I../Core/Inc -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-use-cxa-atexit -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-platform-2f-NUsense

clean-Core-2f-Src-2f-platform-2f-NUsense:
	-$(RM) ./Core/Src/platform/NUsense/Convert.cyclo ./Core/Src/platform/NUsense/Convert.d ./Core/Src/platform/NUsense/Convert.o ./Core/Src/platform/NUsense/Convert.su ./Core/Src/platform/NUsense/NUgus.cyclo ./Core/Src/platform/NUsense/NUgus.d ./Core/Src/platform/NUsense/NUgus.o ./Core/Src/platform/NUsense/NUgus.su

.PHONY: clean-Core-2f-Src-2f-platform-2f-NUsense


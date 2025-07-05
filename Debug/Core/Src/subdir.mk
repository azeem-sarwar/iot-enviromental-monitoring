################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/bme680_interface.c \
../Core/Src/bme68x.c \
../Core/Src/command_interface.c \
../Core/Src/lora_interface.c \
../Core/Src/lr_fhss_mac.c \
../Core/Src/main.c \
../Core/Src/stm32g0xx_hal_msp.c \
../Core/Src/stm32g0xx_it.c \
../Core/Src/sx126x.c \
../Core/Src/sx126x_driver_version.c \
../Core/Src/sx126x_hal.c \
../Core/Src/sx126x_lr_fhss.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32g0xx.c \
../Core/Src/usart2_test.c \
../Core/Src/usart4_test.c 

OBJS += \
./Core/Src/bme680_interface.o \
./Core/Src/bme68x.o \
./Core/Src/command_interface.o \
./Core/Src/lora_interface.o \
./Core/Src/lr_fhss_mac.o \
./Core/Src/main.o \
./Core/Src/stm32g0xx_hal_msp.o \
./Core/Src/stm32g0xx_it.o \
./Core/Src/sx126x.o \
./Core/Src/sx126x_driver_version.o \
./Core/Src/sx126x_hal.o \
./Core/Src/sx126x_lr_fhss.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32g0xx.o \
./Core/Src/usart2_test.o \
./Core/Src/usart4_test.o 

C_DEPS += \
./Core/Src/bme680_interface.d \
./Core/Src/bme68x.d \
./Core/Src/command_interface.d \
./Core/Src/lora_interface.d \
./Core/Src/lr_fhss_mac.d \
./Core/Src/main.d \
./Core/Src/stm32g0xx_hal_msp.d \
./Core/Src/stm32g0xx_it.d \
./Core/Src/sx126x.d \
./Core/Src/sx126x_driver_version.d \
./Core/Src/sx126x_hal.d \
./Core/Src/sx126x_lr_fhss.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32g0xx.d \
./Core/Src/usart2_test.d \
./Core/Src/usart4_test.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G071xx -c -I../Core/Inc -I../Drivers/STM32G0xx_HAL_Driver/Inc -I../Drivers/STM32G0xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G0xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/bme680_interface.cyclo ./Core/Src/bme680_interface.d ./Core/Src/bme680_interface.o ./Core/Src/bme680_interface.su ./Core/Src/bme68x.cyclo ./Core/Src/bme68x.d ./Core/Src/bme68x.o ./Core/Src/bme68x.su ./Core/Src/command_interface.cyclo ./Core/Src/command_interface.d ./Core/Src/command_interface.o ./Core/Src/command_interface.su ./Core/Src/lora_interface.cyclo ./Core/Src/lora_interface.d ./Core/Src/lora_interface.o ./Core/Src/lora_interface.su ./Core/Src/lr_fhss_mac.cyclo ./Core/Src/lr_fhss_mac.d ./Core/Src/lr_fhss_mac.o ./Core/Src/lr_fhss_mac.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/stm32g0xx_hal_msp.cyclo ./Core/Src/stm32g0xx_hal_msp.d ./Core/Src/stm32g0xx_hal_msp.o ./Core/Src/stm32g0xx_hal_msp.su ./Core/Src/stm32g0xx_it.cyclo ./Core/Src/stm32g0xx_it.d ./Core/Src/stm32g0xx_it.o ./Core/Src/stm32g0xx_it.su ./Core/Src/sx126x.cyclo ./Core/Src/sx126x.d ./Core/Src/sx126x.o ./Core/Src/sx126x.su ./Core/Src/sx126x_driver_version.cyclo ./Core/Src/sx126x_driver_version.d ./Core/Src/sx126x_driver_version.o ./Core/Src/sx126x_driver_version.su ./Core/Src/sx126x_hal.cyclo ./Core/Src/sx126x_hal.d ./Core/Src/sx126x_hal.o ./Core/Src/sx126x_hal.su ./Core/Src/sx126x_lr_fhss.cyclo ./Core/Src/sx126x_lr_fhss.d ./Core/Src/sx126x_lr_fhss.o ./Core/Src/sx126x_lr_fhss.su ./Core/Src/syscalls.cyclo ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.cyclo ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_stm32g0xx.cyclo ./Core/Src/system_stm32g0xx.d ./Core/Src/system_stm32g0xx.o ./Core/Src/system_stm32g0xx.su ./Core/Src/usart2_test.cyclo ./Core/Src/usart2_test.d ./Core/Src/usart2_test.o ./Core/Src/usart2_test.su ./Core/Src/usart4_test.cyclo ./Core/Src/usart4_test.d ./Core/Src/usart4_test.o ./Core/Src/usart4_test.su

.PHONY: clean-Core-2f-Src


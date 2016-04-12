################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/BellManager.c \
../src/Campanaro.c \
../src/ToolManager.c 

OBJS += \
./src/BellManager.o \
./src/Campanaro.o \
./src/ToolManager.o 

C_DEPS += \
./src/BellManager.d \
./src/Campanaro.d \
./src/ToolManager.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-linux-gnueabi-gcc-4.7 -I"/home/casa/Campanaro/Campanaro/src/SocketGlue" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



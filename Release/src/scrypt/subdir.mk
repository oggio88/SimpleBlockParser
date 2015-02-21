################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/scrypt/pbkdf2.cpp \
../src/scrypt/scrypt.cpp 

OBJS += \
./src/scrypt/pbkdf2.o \
./src/scrypt/scrypt.o 

CPP_DEPS += \
./src/scrypt/pbkdf2.d \
./src/scrypt/scrypt.d 


# Each subdirectory must supply rules for building sources it contributes
src/scrypt/%.o: ../src/scrypt/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++11 -DBITCOIN -I"/home/walter/c_workspace/SimpleBlockParser/src" -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



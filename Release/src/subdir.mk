################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/callback.cpp \
../src/opcodes.cpp \
../src/option.cpp \
../src/parser.cpp \
../src/rmd160.cpp \
../src/sha256.cpp \
../src/util.cpp 

OBJS += \
./src/callback.o \
./src/opcodes.o \
./src/option.o \
./src/parser.o \
./src/rmd160.o \
./src/sha256.o \
./src/util.o 

CPP_DEPS += \
./src/callback.d \
./src/opcodes.d \
./src/option.d \
./src/parser.d \
./src/rmd160.d \
./src/sha256.d \
./src/util.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++11 -DBITCOIN -I"/home/walter/c_workspace/SimpleBlockParser/src" -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



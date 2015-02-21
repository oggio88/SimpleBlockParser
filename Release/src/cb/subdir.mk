################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/cb/allBalances.cpp \
../src/cb/closure.cpp \
../src/cb/dumpTX.cpp \
../src/cb/help.cpp \
../src/cb/pristine.cpp \
../src/cb/rawdump.cpp \
../src/cb/rewards.cpp \
../src/cb/simpleStats.cpp \
../src/cb/sql.cpp \
../src/cb/taint.cpp \
../src/cb/transactions.cpp 

OBJS += \
./src/cb/allBalances.o \
./src/cb/closure.o \
./src/cb/dumpTX.o \
./src/cb/help.o \
./src/cb/pristine.o \
./src/cb/rawdump.o \
./src/cb/rewards.o \
./src/cb/simpleStats.o \
./src/cb/sql.o \
./src/cb/taint.o \
./src/cb/transactions.o 

CPP_DEPS += \
./src/cb/allBalances.d \
./src/cb/closure.d \
./src/cb/dumpTX.d \
./src/cb/help.d \
./src/cb/pristine.d \
./src/cb/rawdump.d \
./src/cb/rewards.d \
./src/cb/simpleStats.d \
./src/cb/sql.d \
./src/cb/taint.d \
./src/cb/transactions.d 


# Each subdirectory must supply rules for building sources it contributes
src/cb/%.o: ../src/cb/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++11 -DBITCOIN -I"/home/walter/c_workspace/SimpleBlockParser/src" -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



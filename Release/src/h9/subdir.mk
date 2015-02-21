################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/h9/aes_helper.c \
../src/h9/blake.c \
../src/h9/bmw.c \
../src/h9/cubehash.c \
../src/h9/echo.c \
../src/h9/groestl.c \
../src/h9/jh.c \
../src/h9/keccak.c \
../src/h9/luffa.c \
../src/h9/shavite.c \
../src/h9/simd.c \
../src/h9/skein.c 

OBJS += \
./src/h9/aes_helper.o \
./src/h9/blake.o \
./src/h9/bmw.o \
./src/h9/cubehash.o \
./src/h9/echo.o \
./src/h9/groestl.o \
./src/h9/jh.o \
./src/h9/keccak.o \
./src/h9/luffa.o \
./src/h9/shavite.o \
./src/h9/simd.o \
./src/h9/skein.o 

C_DEPS += \
./src/h9/aes_helper.d \
./src/h9/blake.d \
./src/h9/bmw.d \
./src/h9/cubehash.d \
./src/h9/echo.d \
./src/h9/groestl.d \
./src/h9/jh.d \
./src/h9/keccak.d \
./src/h9/luffa.d \
./src/h9/shavite.d \
./src/h9/simd.d \
./src/h9/skein.d 


# Each subdirectory must supply rules for building sources it contributes
src/h9/%.o: ../src/h9/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -DBITCOIN -I"/home/walter/c_workspace/SimpleBlockParser/src" -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



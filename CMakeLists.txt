cmake_minimum_required(VERSION 2.6)
project(SimpleBlockParser)

set(SOURCES
src/cb/sql.cpp
src/cb/rewards.cpp
src/cb/help.cpp
src/cb/allBalances.cpp
src/cb/simpleStats.cpp
src/cb/dumpTX.cpp
src/cb/rawdump.cpp
src/cb/taint.cpp
src/cb/pristine.cpp
src/cb/transactions.cpp
src/cb/closure.cpp
src/sha256.cpp
src/rmd160.cpp
src/opcodes.cpp
src/scrypt/scrypt.cpp
src/scrypt/pbkdf2.cpp
src/h9/blake.c
src/h9/groestl.c
src/h9/luffa.c
src/h9/cubehash.c
src/h9/skein.c
src/h9/aes_helper.c
src/h9/shavite.c
src/h9/bmw.c
src/h9/simd.c
src/h9/keccak.c
src/h9/jh.c
src/h9/echo.c
src/option.cpp
src/callback.cpp
src/util.cpp
src/parser.cpp
)



add_executable(sbp ${SOURCES})

install(TARGETS sbp RUNTIME DESTINATION bin)

include_directories(src)

add_definitions( -DBITCOIN )
list( APPEND CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")


TARGET_LINK_LIBRARIES(sbp crypto dl)

cmake_minimum_required(VERSION 3.26)
project(Crypto_Demo)

set(CMAKE_CXX_STANDARD 11)

set(OPENSSL_HEAD "/opt/homebrew/Cellar/openssl@3/3.2.0_1")

include_directories("${OPENSSL_HEAD}/include")

# print.
message(STATUS "head path : ${OPENSSL_HEAD}/include/")

link_directories(${OPENSSL_HEAD}/lib/)

link_libraries(ssl crypto z)

add_executable(Crypto_Demo
        main.cpp
        TestEvp.cpp
        TestEvp.h
        TestZlib.cpp
        TestZlib.h
        FlowManager.cpp
        FlowManager.h
        handle_string/Proguard.cpp
        handle_string/Proguard.h
        ZZConstant.h
        handle_string/OnlyEncryptString.cpp
        handle_string/OnlyEncryptString.h)



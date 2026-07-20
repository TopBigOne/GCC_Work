cmake_minimum_required(VERSION 4.0)
project(use_uint8)

set(CMAKE_CXX_STANDARD 20)

add_executable(use_uint8
        main.cpp
        src/01_basics.cpp
        src/02_pointer_arith.cpp
        src/03_protocol_parse.cpp
        src/04_bitwise_ops.cpp
        src/05_hexdump.cpp
        src/06_checksum.cpp
)

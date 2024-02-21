cmake_minimum_required(VERSION 3.26)
project(test_enum)

set(CMAKE_CXX_STANDARD 11)

add_executable(test_enum main.cpp
        ValueType.cpp
        ValueType.h)

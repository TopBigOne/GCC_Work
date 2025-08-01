cmake_minimum_required(VERSION 3.31)
project(std_array)

set(CMAKE_CXX_STANDARD 17)

add_executable(std_array main.cpp
        TestArray.cpp
        TestArray.h)

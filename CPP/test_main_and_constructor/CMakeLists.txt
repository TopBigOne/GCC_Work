cmake_minimum_required(VERSION 3.26)
project(test_main_and_constructor)

set(CMAKE_CXX_STANDARD 11)

add_executable(test_main_and_constructor main.cpp
        What.cpp
        What.h)

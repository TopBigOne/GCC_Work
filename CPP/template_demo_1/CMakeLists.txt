cmake_minimum_required(VERSION 3.26)
project(template_demo_1)

set(CMAKE_CXX_STANDARD 11)

add_executable(template_demo_1 main.cpp
        TestMaxElement.cpp
        TestMaxElement.h)

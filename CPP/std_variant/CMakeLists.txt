cmake_minimum_required(VERSION 4.0)
project(std_variant)

set(CMAKE_CXX_STANDARD 20)

add_executable(std_variant main.cpp
        VariantDemo.cpp
        VariantDemo.h)

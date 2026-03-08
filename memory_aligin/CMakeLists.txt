cmake_minimum_required(VERSION 3.22)
project(memory_aligin)

set(CMAKE_CXX_STANDARD 11)

add_executable(memory_aligin main.cpp
        test_struct.h
        test_struct.c)

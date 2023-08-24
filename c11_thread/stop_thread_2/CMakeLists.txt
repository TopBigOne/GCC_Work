cmake_minimum_required(VERSION 3.26)
project(stop_thread_2)

set(CMAKE_CXX_STANDARD 11)

add_executable(stop_thread_2 main.cpp
        ThreadExtension.cpp
        ThreadExtension.h)

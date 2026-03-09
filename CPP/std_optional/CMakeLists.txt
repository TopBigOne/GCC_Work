cmake_minimum_required(VERSION 4.0)
project(std_optional)

set(CMAKE_CXX_STANDARD 20)

add_executable(std_optional main.cpp
        OptionalDemo.cpp
        OptionalDemo.h
        ExpensiveResource.h
        ResourceHolder.h)

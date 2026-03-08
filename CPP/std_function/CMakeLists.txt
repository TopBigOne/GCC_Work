cmake_minimum_required(VERSION 4.0)
project(std_function)

set(CMAKE_CXX_STANDARD 20)

add_executable(std_function main.cpp
        Calculator.cpp
        Calculator.h
        EventSystem.h
        StrategyPatternSort.h)

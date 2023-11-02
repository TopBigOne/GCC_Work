cmake_minimum_required(VERSION 3.26)
project(map)

set(CMAKE_CXX_STANDARD 11)

add_executable(map main.cpp
        case_kv/CaseKV.cpp
        case_kv/CaseKV.h)

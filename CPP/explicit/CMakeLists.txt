cmake_minimum_required(VERSION 3.26)
project(explicit)

set(CMAKE_CXX_STANDARD 17)

add_executable(explicit main.cpp
        NormalClass.cpp
        NormalClass.h
        ExplicitClass.cpp
        ExplicitClass.h)

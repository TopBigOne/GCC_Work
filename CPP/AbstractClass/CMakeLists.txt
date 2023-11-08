cmake_minimum_required(VERSION 3.26)
project(AbstractClass)

set(CMAKE_CXX_STANDARD 11)

add_executable(AbstractClass main.cpp
        AbstractClass.cpp
        AbstractClass.h
        ConcreteClass.cpp
        ConcreteClass.h)

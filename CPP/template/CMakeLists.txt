cmake_minimum_required(VERSION 3.22)
project(template)

set(CMAKE_CXX_STANDARD 11)

add_executable(template main.cpp add.h MyStack.h TemplateSpecialization.h)

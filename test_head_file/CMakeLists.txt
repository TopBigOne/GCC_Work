cmake_minimum_required(VERSION 3.22)
project(test_head_file C)

set(CMAKE_C_STANDARD 99)

add_executable(test_head_file main.c my_print.c my_print.h)

cmake_minimum_required(VERSION 3.22)
project(without_head_file C)

set(CMAKE_C_STANDARD 99)

add_executable(without_head_file main.c test_2.c)

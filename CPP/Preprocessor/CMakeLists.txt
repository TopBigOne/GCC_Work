cmake_minimum_required(VERSION 3.22)
project(Preprocessor C)

set(CMAKE_C_STANDARD 99)

add_executable(Preprocessor main.c ifdef.c ifdef.h names.h  doubincl.h doubincl.c)

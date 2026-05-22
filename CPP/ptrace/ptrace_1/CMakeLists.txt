cmake_minimum_required(VERSION 3.26)
project(ptrace_1)

set(CMAKE_CXX_STANDARD 11)
# debug 模式
SET(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_FLAGS_DEBUG "-O0")             # 调试包不优化
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG ")   # release包优化

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build)


add_executable(ptrace_1 main.cpp)


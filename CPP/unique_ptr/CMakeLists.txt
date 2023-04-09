cmake_minimum_required(VERSION 3.22)
project(unique_ptr)

set(CMAKE_CXX_STANDARD 14)

add_executable(unique_ptr main.cpp test_shared_ptr.cpp test_unique_ptr.cpp test_weak_ptr.cpp)

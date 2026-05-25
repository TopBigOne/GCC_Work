cmake_minimum_required(VERSION 4.0)
project(std_exception_ptr)

set(CMAKE_CXX_STANDARD 20)

add_executable(std_exception_ptr
        main.cpp
        demo/demo_basic.cpp
        demo/demo_thread.cpp
        demo/demo_aggregator.cpp
        demo/demo_chain.cpp
)

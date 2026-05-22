cmake_minimum_required(VERSION 4.0)
project(std_string_view)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)   # 生成 compile_commands.json 供 clangd 使用

add_executable(std_string_view
    main.cpp
    demos/DemoBasic.cpp
    demos/DemoOperations.cpp
    demos/DemoPerf.cpp
    demos/DemoLifetime.cpp
    demos/DemoPractical.cpp
)

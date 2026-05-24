cmake_minimum_required(VERSION 3.16)
project(LearnRAII)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(LearnRAII
    main.cpp
    resource_guard/resource_guard.cpp
    factory_resource/factory_resource.cpp
    proxy_resource/proxy_resource.cpp
    crtp_policy/crtp_policy.cpp
    scope_guard/scope_guard.cpp
)

# 根目录：main.cpp 用 "subdir/header.h" 方式引用
# 各子目录：.cpp 文件用 "header.h" 引用同目录头文件
target_include_directories(LearnRAII PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

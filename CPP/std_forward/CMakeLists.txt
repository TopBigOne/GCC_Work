cmake_minimum_required(VERSION 3.14)
project(std_forward
    VERSION 1.0.0
    DESCRIPTION "std::forward 完美转发学习演示"
    LANGUAGES CXX
)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 编译选项
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -pedantic)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(-g -O0)
    else()
        add_compile_options(-O3)
    endif()
endif()

# 定义源文件
set(SOURCES
    main.cpp
    demo_object.cpp
    forward_demos.cpp
)

# 定义头文件
set(HEADERS
    demo_object.h
    forward_demos.h
)

# 创建可执行文件
add_executable(std_forward_demo ${SOURCES} ${HEADERS})

# 为可执行文件设置属性
set_target_properties(std_forward_demo PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# 包含当前目录作为头文件搜索路径
target_include_directories(std_forward_demo PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# 输出构建信息
message(STATUS "Building std::forward demo with C++${CMAKE_CXX_STANDARD}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Source files: ${SOURCES}")

# 如果是 Debug 模式，启用更多诊断信息
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(NOT MSVC)
        target_compile_options(std_forward_demo PRIVATE -fsanitize=address -fno-omit-frame-pointer)
        target_link_options(std_forward_demo PRIVATE -fsanitize=address)
    endif()
endif()

# 安装规则（可选）
install(TARGETS std_forward_demo
    RUNTIME DESTINATION bin
)

# 可选：添加自定义目标以便快速清理
add_custom_target(clean-all
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}
    COMMENT "Cleaning all build files"
)

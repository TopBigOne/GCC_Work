cmake_minimum_required(VERSION 3.10)
project(std_move LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 项目描述
set(PROJECT_DESCRIPTION "Complete std::move demonstration project based on comprehensive README.md")

# 头文件目录
include_directories(.)

# 源文件列表
set(SOURCES
    main.cpp
    MyObject.cpp
    MoveDemo.cpp
    BestPracticesAndTraps.cpp
    performance_analysis.cpp
)

# 头文件列表
set(HEADERS
    MyObject.h
    MoveDemo.h
)

# 创建可执行文件
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# 编译器特定选项
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Wno-pessimizing-move  # 忽略陷阱演示中的故意警告
    )
endif()

# 调试模式特定选项
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(${PROJECT_NAME} PRIVATE -g -DDEBUG)
    message(STATUS "Debug mode enabled")
endif()

# 发布模式特定选项
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(${PROJECT_NAME} PRIVATE -O2 -DNDEBUG)
    message(STATUS "Release mode enabled")
endif()

# 输出项目信息
message(STATUS "Project: ${PROJECT_NAME}")
message(STATUS "Description: ${PROJECT_DESCRIPTION}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")

# 安装规则（可选）
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)

# 添加自定义目标来清理构建
add_custom_target(clean-all
    COMMAND ${CMAKE_BUILD_TOOL} clean
    COMMAND ${CMAKE_COMMAND} -E remove_directory CMakeFiles
    COMMAND ${CMAKE_COMMAND} -E remove CMakeCache.txt cmake_install.cmake Makefile
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Cleaning all build files"
)

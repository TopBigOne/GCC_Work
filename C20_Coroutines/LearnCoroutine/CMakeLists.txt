cmake_minimum_required(VERSION 3.20)
project(LearnCoroutine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-Wall -Wextra)

# ── 所有模块头文件（header-only 实现，显式列出供 CLion 索引和跳转）──────────
set(MODULE_HEADERS
    module0_prerequisites/prerequisites.h

    module1_basics/basics.h

    module2_promise/promise.h

    module3_generator/generator.h

    module4_awaitable/awaitable.h

    module5_task/task.h

    module6_realworld/scheduler.h
    module6_realworld/event.h
    module6_realworld/channel.h
    module6_realworld/realworld.h
)

add_executable(LearnCoroutine
    main.cpp
    ${MODULE_HEADERS}
)

# 根目录作为 include 路径，使 #include "moduleX_xxx/xxx.h" 生效
target_include_directories(LearnCoroutine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

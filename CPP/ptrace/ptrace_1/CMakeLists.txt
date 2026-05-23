cmake_minimum_required(VERSION 3.20)
project(ptrace_demos LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_FLAGS_DEBUG   "-O0 -g")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")


# 输出 compile_commands.json 供 clangd 使用
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build)

# ── core 层（RAII + Facade + Command） ──────────────────────────
set(CORE_SRCS
    core/RegisterAccess.cpp
    core/MemoryAccess.cpp
    core/ProcessCtrl.cpp
    core/Breakpoint.cpp
    core/Debugger.cpp
)

# ── demos 层（Template Method） ─────────────────────────────────
set(DEMO_SRCS
    demos/DemoTraceMe.cpp
    demos/DemoRegisters.cpp
    demos/DemoMemory.cpp
    demos/DemoSingleStep.cpp
    demos/DemoAntiDebug.cpp
)

add_executable(ptrace_demos main.cpp ${CORE_SRCS} ${DEMO_SRCS})



# 以项目根目录为 include 根路径
# 头文件引用方式：#include "core/Debugger.h"
target_include_directories(ptrace_demos PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# task_for_pid 需要 com.apple.security.cs.debugger 权限
# configure 时在 build 目录生成 entitlements，不需要单独的文件
file(WRITE ${CMAKE_BINARY_DIR}/debug.entitlements
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>
<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">
<plist version=\"1.0\">
<dict>
    <key>com.apple.security.cs.debugger</key>
    <true/>
</dict>
</plist>")

add_custom_command(TARGET ptrace_demos POST_BUILD
    COMMAND codesign -s - --entitlements ${CMAKE_BINARY_DIR}/debug.entitlements --force $<TARGET_FILE:ptrace_demos>
    COMMENT "Re-signing ptrace_demos with debug entitlements"
)

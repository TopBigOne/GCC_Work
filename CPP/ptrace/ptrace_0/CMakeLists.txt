cmake_minimum_required(VERSION 3.26)
project(ptrace_0)

set(CMAKE_CXX_STANDARD 11)

add_executable(ptrace_0 main.cpp)

add_custom_command(TARGET ptrace_0 POST_BUILD
    COMMAND codesign -s - --entitlements ${CMAKE_SOURCE_DIR}/debug.entitlements --force $<TARGET_FILE:ptrace_0>
    COMMENT "Re-signing with debug entitlements"
)

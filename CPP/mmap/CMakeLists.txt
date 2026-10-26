# 3.16 = Ubuntu 20.04 自带的版本；项目用到的特性（CONFIGURE_DEPENDS 等）3.12 起就有
cmake_minimum_required(VERSION 3.16)
project(mmap_learn CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 自动收集 src/ 下所有 .cpp；
# 新增 stage 时无需改这里（CONFIGURE_DEPENDS 让增删文件后自动重新配置）
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)
add_executable(mmap_learn ${SOURCES})

# shm_open / shm_unlink（stage4）在哪个库里因系统而异：
#   macOS、glibc >= 2.34（Ubuntu 22.04+）：在 libc 中，无需额外链接
#   glibc <  2.34（Ubuntu 20.04 等）    ：在 librt 中，需要 -lrt
# 这里在配置阶段实测一次，只在 libc 里找不到时才链接 rt
include(CheckCXXSymbolExists)
check_cxx_symbol_exists(shm_open "sys/mman.h" HAVE_SHM_OPEN_IN_LIBC)
if(NOT HAVE_SHM_OPEN_IN_LIBC)
    find_library(RT_LIBRARY rt)
    if(NOT RT_LIBRARY)
        message(FATAL_ERROR "shm_open 不在 libc 中，也找不到 librt")
    endif()
    target_link_libraries(mmap_learn PRIVATE ${RT_LIBRARY})
endif()

# clean 时顺带删除程序运行生成的临时目录 mmap_tmp/（见 src/mmap_wrapper.hpp 的 kWorkDir）。
# mmap_tmp/ 建在「运行程序时的工作目录」下，常见的有两处，都登记上：
#   构建目录：CLion 默认在这里运行程序
#   项目根目录：在项目根目录下用终端运行，或 CLion 工作目录设为 $ProjectFileDir$
# ADDITIONAL_CLEAN_FILES 需要 CMake 3.15+，可以登记目录，clean 时连同目录一起删除
set_property(TARGET mmap_learn APPEND PROPERTY ADDITIONAL_CLEAN_FILES
    ${CMAKE_BINARY_DIR}/mmap_tmp
    ${CMAKE_SOURCE_DIR}/mmap_tmp
)

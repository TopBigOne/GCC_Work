cmake_minimum_required(VERSION 3.31)
project(Test_sdl2_Env)

set(CMAKE_CXX_STANDARD 11)

set(SDL_DIR /opt/homebrew/Cellar/sdl2/2.32.8/)
include_directories(${SDL_DIR}/include/)
link_directories(${SDL_DIR}/lib/)




add_executable(Test_sdl2_Env main.cpp)
# 将可执行文件链接到 SDL2 的动态库（SDL2）和入口库（SDL2main）
target_link_libraries(Test_sdl2_Env
        SDL2    # Linux 通常只需链接 SDL2
       SDL2main #  Windows/macOS 上处理程序入口点的辅助库。
)

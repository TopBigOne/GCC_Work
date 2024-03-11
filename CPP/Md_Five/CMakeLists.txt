cmake_minimum_required(VERSION 3.26)
project(Md_Five)

set(CMAKE_CXX_STANDARD 11)

set(OPENSSL_HEAD "/opt/homebrew/Cellar/openssl@3/3.2.0_1")

include_directories("${OPENSSL_HEAD}/include")

link_directories(${OPENSSL_HEAD}/lib/)

link_libraries(ssl crypto z)

add_executable(Md_Five main.cpp)

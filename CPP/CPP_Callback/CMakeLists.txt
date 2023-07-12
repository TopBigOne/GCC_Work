cmake_minimum_required(VERSION 3.22)
project(CPP_Callback)

set(CMAKE_CXX_STANDARD 14)

add_executable(CPP_Callback main.cpp Callback.cpp Callback.h Caller.cpp Caller.h Receiver.cpp Receiver.h)

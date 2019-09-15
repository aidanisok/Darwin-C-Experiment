cmake_minimum_required(VERSION 3.14)
project(IDA3)

set(CMAKE_CXX_STANDARD 14)

add_executable(IDA3 main.cpp Utilities.h Utilities.cpp Virtualization.cpp Virtualization.h AppParams.h)
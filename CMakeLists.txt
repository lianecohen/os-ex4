cmake_minimum_required(VERSION 3.25)
project(ex4os)

set(CMAKE_CXX_STANDARD 11)

add_executable(ex4os PhysicalMemory.cpp SimpleTest.cpp VirtualMemory.cpp)

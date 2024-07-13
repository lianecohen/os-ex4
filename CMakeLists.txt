cmake_minimum_required(VERSION 3.16)
project(os-ex4)

set(CMAKE_CXX_STANDARD 14)

include_directories(os-ex4)

add_executable(os-ex4
        PhysicalMemory.cpp
        PhysicalMemory.h
        VirtualMemory.h
        SimpleTest.cpp
        )

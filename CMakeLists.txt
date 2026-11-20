cmake_minimum_required(VERSION 4.3)
project(1d_heat)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(1d_heat
        heat.h
        input.cpp
        main.cpp
        output.cpp)

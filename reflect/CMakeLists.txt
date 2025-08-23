cmake_minimum_required(VERSION 3.31)
project(reflect)

set(CMAKE_CXX_STANDARD 23)

# C++ 反射系统演示
add_executable(reflect main.cpp reflect.h
        )

add_executable(example example.cpp reflect.h)


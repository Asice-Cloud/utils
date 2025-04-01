cmake_minimum_required(VERSION 3.30)
project(nw)

set(CMAKE_CXX_STANDARD 23)

add_executable(router src/main.cpp src/router.h src/router.cpp)

add_executable(debug_test src/debug.hpp test/debug_test.cpp)
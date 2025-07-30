cmake_minimum_required(VERSION 3.28)
project(nw)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(GCC_COVERAGE_COMPILE_FLAGS "-fcoroutines")

file(GLOB_RECURSE router_files src/router/*.cpp)
file(GLOB_RECURSE router_headers src/router/*.h)

file(GLOB_RECURSE core_files src/core/*.cpp)
file(GLOB_RECURSE core_headers src/core/*.h)

file(GLOB_RECURSE util_files src/util/*.cpp)
file(GLOB_RECURSE util_headers src/util/*.h)

add_executable(nw src/main.cpp
        ${router_files} ${router_headers}
        ${core_files} ${core_headers}
        ${util_files} ${util_headers}
)

#target_include_directories(nw PRIVATE ${CMAKE_SOURCE_DIR}/include)

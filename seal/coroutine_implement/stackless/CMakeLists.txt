cmake_minimum_required(VERSION 3.10)
project(corcpp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB EXAMPLE_SOURCES examples/*.cpp)

foreach(EXAMPLE_SOURCE ${EXAMPLE_SOURCES})
    get_filename_component(EXAMPLE_NAME ${EXAMPLE_SOURCE} NAME_WE)
    add_executable(${EXAMPLE_NAME} ${EXAMPLE_SOURCE} coroutine.h)
    target_include_directories(${EXAMPLE_NAME} PUBLIC .)
endforeach(EXAMPLE_SOURCE)

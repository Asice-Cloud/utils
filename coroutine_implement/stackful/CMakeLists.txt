cmake_minimum_required(VERSION 3.15)

project(coroutine LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -fexperimental-library")
endif()

# # enable address sanitizer
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
# set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=address")

file(GLOB SOURCES ./*.cpp)
# add cpp format

add_executable(coroutine ${SOURCES})

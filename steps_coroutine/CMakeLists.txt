cmake_minimum_required(VERSION 3.30)
project(steps_coroutine)

set(CMAKE_CXX_STANDARD 26)


file(GLOB SOURCES ./*.cpp)
foreach (SOURCES_FILE ${SOURCES})
    get_filename_component(EXE_NAME ${SOURCES_FILE} NAME_WE)
    add_executable(${EXE_NAME} ${SOURCES_FILE})
endforeach (SOURCES_FILE)
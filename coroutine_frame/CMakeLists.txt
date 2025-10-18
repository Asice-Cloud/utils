cmake_minimum_required(VERSION 4.0)
project(task_do)

set(CMAKE_CXX_STANDARD 23)

find_package(Threads REQUIRED)

# Main quick test
add_executable(task_do main.cpp
        core/task.h
        core/executor.h
        core/executor.cpp)
target_link_libraries(task_do Threads::Threads)

# Basic Demo (formerly main.cpp)
add_executable(basic_demo
        examples/basic_demo.cpp
        core/task.h
        core/executor.h
        core/executor.cpp)
target_link_libraries(basic_demo Threads::Threads)

# HTTP Server Example
add_executable(http_server 
        examples/http_server.cpp
        core/task.h
        core/executor.h
        core/executor.cpp)
target_link_libraries(http_server Threads::Threads)

# Advanced Features Demo
add_executable(advanced_features
        examples/advanced_features.cpp
        core/task.h
        core/executor.h
        core/executor.cpp)
target_link_libraries(advanced_features Threads::Threads)

# Core Features Test Suite
add_executable(core_features_test
        examples/core_features_test.cpp
        core/task.h
        core/executor.h
        core/executor.cpp)
target_link_libraries(core_features_test Threads::Threads)

# WebSocket Server Example
find_package(OpenSSL REQUIRED)
add_executable(websocket_server
        examples/websocket_server.cpp
        core/task.h
        core/executor.h
        core/executor.cpp)
target_link_libraries(websocket_server Threads::Threads OpenSSL::SSL OpenSSL::Crypto)

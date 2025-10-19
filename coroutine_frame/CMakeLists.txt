cmake_minimum_required(VERSION 4.0)
project(task_do)

# Set C++23 globally
set(CMAKE_CXX_STANDARD 23)

find_package(Threads REQUIRED)

# Core library sources (all .cpp implementations)
set(CORE_SOURCES
    core/executor.cpp
    core/io_epoll.cpp
)

# Main quick test
add_executable(task_do main.cpp ${CORE_SOURCES})
target_link_libraries(task_do Threads::Threads)


# HTTP Server Example
add_executable(http_server examples/http_server.cpp ${CORE_SOURCES})
target_link_libraries(http_server Threads::Threads)


# WebSocket Server Example
find_package(OpenSSL REQUIRED)
add_executable(websocket_server examples/websocket_server.cpp ${CORE_SOURCES})
target_link_libraries(websocket_server Threads::Threads OpenSSL::SSL OpenSSL::Crypto)


# Structured Concurrency Example
add_executable(structured_concurrency examples/structured_concurrency.cpp ${CORE_SOURCES})
target_link_libraries(structured_concurrency Threads::Threads)

# Coroutine Stress Test Example
add_executable(coroutine_stress_test examples/coroutine_stress_test.cpp ${CORE_SOURCES})
target_link_libraries(coroutine_stress_test Threads::Threads)
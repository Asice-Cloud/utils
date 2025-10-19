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
target_compile_features(task_do PUBLIC cxx_std_23)
target_compile_options(task_do PUBLIC -fcoroutines)


# HTTP Server Example
add_executable(http_server examples/http_server.cpp ${CORE_SOURCES})
target_link_libraries(http_server Threads::Threads)
target_compile_features(http_server PUBLIC cxx_std_23)
target_compile_options(http_server PUBLIC -fcoroutines)


# WebSocket Server Example
find_package(OpenSSL REQUIRED)
add_executable(websocket_server examples/websocket_server.cpp ${CORE_SOURCES})
target_link_libraries(websocket_server Threads::Threads OpenSSL::SSL OpenSSL::Crypto)
target_compile_features(websocket_server PUBLIC cxx_std_23)
target_compile_options(websocket_server PUBLIC -fcoroutines)

# Structured Concurrency Example
add_executable(structured_concurrency examples/structured_concurrency.cpp ${CORE_SOURCES})
target_link_libraries(structured_concurrency Threads::Threads)
target_compile_features(structured_concurrency PUBLIC cxx_std_23)
target_compile_options(structured_concurrency PUBLIC -fcoroutines)


# Coroutine Stress Test Example
add_executable(coroutine_stress_test examples/coroutine_stress_test.cpp ${CORE_SOURCES})
target_link_libraries(coroutine_stress_test Threads::Threads)
target_compile_features(coroutine_stress_test PUBLIC cxx_std_23)
target_compile_options(coroutine_stress_test PUBLIC -fcoroutines)
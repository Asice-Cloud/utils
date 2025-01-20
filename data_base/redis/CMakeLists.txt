cmake_minimum_required(VERSION 3.10)
project(RedisTest)

set(CMAKE_CXX_STANDARD 23)

# 查找 redis++ 和 hiredis 库
find_package(PkgConfig REQUIRED)
pkg_check_modules(REDIS_PLUS_PLUS REQUIRED redis++)
pkg_check_modules(HIREDIS REQUIRED hiredis)

# 添加可执行文件
add_executable(redis_test redis_test.cpp)

# 链接 redis++ 和 hiredis 库
target_include_directories(redis_test PRIVATE ${REDIS_PLUS_PLUS_INCLUDE_DIRS} ${HIREDIS_INCLUDE_DIRS})
target_link_libraries(redis_test ${REDIS_PLUS_PLUS_LIBRARIES} ${HIREDIS_LIBRARIES})
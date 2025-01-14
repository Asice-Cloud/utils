cmake_minimum_required(VERSION 3.28)
project(MySQLExample)

set(CMAKE_CXX_STANDARD 20)

# Set the path to the MySQL package
include_directories(/usr/local/include/mysql)
include_directories(/usr/local/include)
add_executable(MySQLExample 1_connect.cpp)
target_link_libraries(MySQLExample mysqlclient)
target_link_libraries(MySQLExample simdjson)
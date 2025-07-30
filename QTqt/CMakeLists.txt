cmake_minimum_required(VERSION 3.31)
project(QTqt)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS
        Core
        Widgets
       )

add_executable(QTqt main.cpp)
target_link_libraries(QTqt
        Qt::Core
        Qt::Widgets
)


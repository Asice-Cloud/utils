cmake_minimum_required(VERSION 3.31)
project(QTqt)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(Qt6_DIR "/home/asice-cloud/Qt/6.9.1/gcc_64/lib/cmake/Qt6")

find_package(Qt6 REQUIRED COMPONENTS
        Core
        Widgets
       )

add_executable(QTqt main.cpp)
target_link_libraries(QTqt
        Qt::Core
        Qt::Widgets
)


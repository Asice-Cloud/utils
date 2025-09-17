cmake_minimum_required(VERSION 3.31)
project(Maze)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS
        Core
        Widgets
       )


file(GLOB QT_SOURCES
        "*.cpp"
        "*.h"
)

add_executable(Maze1
        ${QT_SOURCES}
)

qt_add_resources(QTqt_resources resources.qrc)
target_link_libraries(Maze1
        Qt::Core
        Qt::Widgets
)
target_sources(Maze1 PRIVATE resources.qrc)

cmake_minimum_required(VERSION 3.25)
set(CMAKE_CXX_STANDARD 23)
project(async)

find_package(Boost 1.83 REQUIRED COMPONENTS system)

#add_definitions(-DASIO_HAS_CO_AWAIT)
add_executable(async11 async11.cpp)
add_executable(async20 async20.cpp)
target_link_libraries(async11 Boost::system)
target_link_libraries(async20 Boost::system)

find_package(RTTR)
add_executable(rttr rttrexample.cpp)
target_link_libraries(rttr RTTR::Core)
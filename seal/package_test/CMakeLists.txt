cmake_minimum_required(VERSION 3.25)
set(CMAKE_CXX_STANDARD 23)
project(pt)

find_package(Boost REQUIRED COMPONENTS system)

#add_definitions(-DASIO_HAS_CO_AWAIT)
add_executable(async11 async11.cpp)
add_executable(async20 async20.cpp)
target_link_libraries(async11 Boost::system)
target_link_libraries(async20 Boost::system)

find_package(RTTR)
add_executable(rttr rttrexample.cpp)
target_link_libraries(rttr RTTR::Core)

add_executable(echo1 echo_server.cpp)
add_executable(echo2 echo_server20.cpp)

find_package(TBB REQUIRED)

add_executable(tbbb tbbtbb.cpp)
target_link_libraries(tbbb TBB::tbb)

cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 23)

project(template)

foreach(source ${SOURCES})
  get_filename_component(Name ${source} NAME_WE)
add_executable(${Name} ${source})
endforeach()
  

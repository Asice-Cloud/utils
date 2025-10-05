cmake_minimum_required(VERSION 3.28)

project(bugmaker LANGUAGES C CXX)

# 导入 PkgConfig 库
find_package(PkgConfig REQUIRED)
pkg_check_modules(readline REQUIRED IMPORTED_TARGET readline)

include(CTest)

add_subdirectory("src")
add_subdirectory("tools")


if(BUILD_TESTING)
# === Third-party libraries ===
# 请将 Catch2 源码放到 third_party/Catch2 目录下
if(EXISTS "${CMAKE_SOURCE_DIR}/third_party/Catch2/CMakeLists.txt")
  add_subdirectory(third_party/Catch2)
endif()
  add_subdirectory("test")
endif()





cmake_minimum_required(VERSION 3.28)

project(moderncpp)

set(CMAKE_CXX_STANDARD 23)

# Set the output directory for executables
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Set RPATH to use the origin and prioritize /usr/lib/x86_64-linux-gnu
set(CMAKE_BUILD_RPATH_USE_ORIGIN TRUE)
set(CMAKE_BUILD_RPATH "$ORIGIN/../lib;/usr/lib/x86_64-linux-gnu")

# Collect all .cpp files in the src directory
file(GLOB SOURCES "*.cpp")
find_package(fmt CONFIG REQUIRED)
find_package(RTTR CONFIG REQUIRED Core)
find_package(OpenSSL REQUIRED)

# Loop through each .cpp file and create an executable target
foreach (SOURCE_FILE ${SOURCES})
    get_filename_component(EXE_NAME ${SOURCE_FILE} NAME_WE)

    add_executable(${EXE_NAME} ${SOURCE_FILE})
    target_link_libraries(${EXE_NAME} PRIVATE fmt::fmt RTTR::Core OpenSSL::SSL OpenSSL::Crypto)
endforeach ()
cmake_minimum_required(VERSION 3.28)

project(moderncpp)

set(CMAKE_CXX_STANDARD 23)

# Set the output directory for executables
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Collect all .cpp files in the src directory
file(GLOB SOURCES "*.cpp")
find_package(fmt CONFIG REQUIRED)
find_package(SDL2 REQUIRED)
find_package(RTTR CONFIG REQUIRED Core)
add_compile_options("-Wall" "-Wextra" "-Wpedantic" "-Werror" "-Wno-unused-variable" "-Wno-unused-parameter" "-Wno-unused-function" "-Wno-unused-but-set-variable" "-Wno-unused-private-field" "-Wno-unused-local-typedefs")
# Loop through each .cpp file and create an executable target
foreach (SOURCE_FILE ${SOURCES})
    get_filename_component(EXE_NAME ${SOURCE_FILE} NAME_WE)

    add_executable(${EXE_NAME} ${SOURCE_FILE}
    )
    target_link_libraries(${EXE_NAME} PRIVATE fmt::fmt SDL2::SDL2
            SDL2::SDL2main
            RTTR::Core
    )
endforeach ()
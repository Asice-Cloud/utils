# CMake configuration for test directory
# Add test targets here if needed

add_executable(test_sdb test.cpp)
target_link_libraries(test_sdb PRIVATE sdb::libsdb Catch2::Catch2WithMain)

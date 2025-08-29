add_executable(sdb sdb.cpp)

target_link_libraries(sdb PRIVATE sdb::libsdb PkgConfig::readline)

# Add include directory for libsdb headers
target_include_directories(sdb PRIVATE ${CMAKE_SOURCE_DIR}/include)

include(GNUInstallDirs)
install(TARGETS sdb
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  )

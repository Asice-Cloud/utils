add_library(libsdb libsdb.cpp)
add_library(sdb::libsdb ALIAS libsdb)

set_target_properties(libsdb PROPERTIES
    CXX_STANDARD 20
    OUTPUT_NAME sdb
)

target_compile_features(libsdb PUBLIC cxx_std_20)

target_include_directories(libsdb
    PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)


include(GNUInstallDirs)
install(TARGETS libsdb
    EXPORT libsdb-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(EXPORT libsdb-targets
    FILE libsdb-targets.cmake
    NAMESPACE sdb::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/libsdb
)

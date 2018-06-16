add_subdirectory(escaping)

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/config_cmake.h.in ${CMAKE_CURRENT_BINARY_DIR}/config_cmake.h)

add_library(skm-odbc SHARED
    attr.cpp
    config.cpp
    connection.cpp
    diagnostics.cpp
    environment.cpp
    handles.cpp
    info.cpp
    odbc.cpp
    result_set.cpp
    statement.cpp
    type_parser.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set_target_properties(skm-odbc
        PROPERTIES PREFIX ""
               OUTPUT_NAME "skm-odbc"
    )
else()
    set_target_properties(skm-odbc PROPERTIES OUTPUT_NAME "skmodbc")
endif()

target_include_directories (skm-odbc PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

if (NOT WIN32)
    target_include_directories (skm-odbc PRIVATE ${ODBC_INCLUDE_DIRECTORIES})
endif()

target_link_libraries(skm-odbc skm-odbc-escaping)

if(USE_SSL)
    target_link_libraries(skm-odbc ${Poco_NetSSL_LIBRARY} ${Poco_Util_LIBRARY})
endif()

target_link_libraries(skm-odbc
    ${Poco_Net_LIBRARY}
    ${Poco_Foundation_LIBRARY}
    ${ODBC_LIBRARIES}
)

if(APPLE)
    target_link_libraries(skm-odbc ${LTDL_LIBRARY})
endif()

if (WIN32)
  target_link_libraries(skm-odbc ws2_32 Iphlpapi libcmt msvcrt msvcmrt)
endif()

if (NOT WIN32)
    install(TARGETS skm-odbc LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/odbc)
    install(FILES unixodbc.ini DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/skm-odbc)
    install(FILES odbc.ini DESTINATION ${CMAKE_INSTALL_DOCDIR}/examples)
endif()

if(SKM_ODBC_TEST)
    add_subdirectory(ut)
endif()

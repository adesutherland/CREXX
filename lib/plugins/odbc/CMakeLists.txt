cmake_minimum_required(VERSION 3.24)
project(odbc_plugin C)

set(CMAKE_C_STANDARD 99)
include(${CMAKE_SOURCE_DIR}/rxpa/RXPluginFunction.cmake)

if(ENABLE_ODBC)
    if(WIN32)
        find_library(ODBC32_LIBRARY odbc32)
        find_library(ODBCCP32_LIBRARY odbccp32)
        if(NOT ODBC32_LIBRARY OR NOT ODBCCP32_LIBRARY)
            message(FATAL_ERROR "ODBC libraries not found on Windows")
        endif()
        set(ODBC_LIBS ${ODBC32_LIBRARY} ${ODBCCP32_LIBRARY})
    else()
        find_path(ODBC_INCLUDE_DIR sql.h
                PATHS /usr/include /usr/local/include /opt/homebrew/include)
        find_library(ODBC_LIBRARY odbc
                PATHS /usr/lib /usr/local/lib /opt/homebrew/lib)
        if(NOT ODBC_INCLUDE_DIR OR NOT ODBC_LIBRARY)
            message(FATAL_ERROR
                    "ODBC headers (sql.h) or library (libodbc) not found; install unixODBC")
        endif()
        set(ODBC_LIBS ${ODBC_LIBRARY})
    endif()

    add_static_plugin_target(_odbc odbc.c)
    add_dynamic_plugin_target(_odbc odbc.c)
    target_include_directories(_odbc PRIVATE ${ODBC_INCLUDE_DIR})
    target_include_directories(_odbc_static PRIVATE ${ODBC_INCLUDE_DIR})
    target_link_libraries(_odbc PRIVATE ${ODBC_LIBS} Threads::Threads)
    target_link_libraries(_odbc_static PRIVATE ${ODBC_LIBS} Threads::Threads)

    add_custom_command(
            COMMAND $<TARGET_FILE:rxc>
                    -i "${CMAKE_BINARY_DIR}/bin\;${CMAKE_CURRENT_BINARY_DIR}"
                    -o odbc_test "${CMAKE_CURRENT_SOURCE_DIR}/odbc_test"
            COMMAND $<TARGET_FILE:rxas> odbc_test
            DEPENDS rxas rxc library compiler_exit_bin _odbc
                    ${CMAKE_CURRENT_SOURCE_DIR}/odbc_test.crexx
            OUTPUT odbc_test.rxbin
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    add_custom_target(odbc_test ALL DEPENDS odbc_test.rxbin)
endif()

if(BUILD_TESTING)
    add_dynamic_plugin_target(_odbc_mock PROVIDER_ID rx_odbc
            odbc.c tests/mock_odbc.c)
    target_compile_definitions(_odbc_mock PRIVATE CREXX_ODBC_MOCK=1)
    target_include_directories(_odbc_mock PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/tests)
    target_link_libraries(_odbc_mock PRIVATE Threads::Threads)
    set_target_properties(_odbc_mock PROPERTIES
            LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/mock"
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/mock")

    set(_odbc_mock_runtime_base
            "${CMAKE_CURRENT_BINARY_DIR}/odbc_mock_prepared")
    add_custom_command(
            OUTPUT "${_odbc_mock_runtime_base}.rxbin"
            BYPRODUCTS "${_odbc_mock_runtime_base}.rxas"
            COMMAND $<TARGET_FILE:rxc> -n
                    -i "${CMAKE_BINARY_DIR}/bin\;${CMAKE_CURRENT_BINARY_DIR}/mock"
                    -o "${_odbc_mock_runtime_base}"
                    "${CMAKE_CURRENT_SOURCE_DIR}/tests/odbc_mock_prepared.crexx"
            COMMAND $<TARGET_FILE:rxas> -n
                    -o "${_odbc_mock_runtime_base}"
                    "${_odbc_mock_runtime_base}"
            DEPENDS rxc rxas compiler_exit_bin library _odbc_mock
                    "${CMAKE_CURRENT_SOURCE_DIR}/tests/odbc_mock_prepared.crexx")
    add_custom_target(odbc_mock_prepared_runtime ALL
            DEPENDS "${_odbc_mock_runtime_base}.rxbin")
    add_test(NAME odbc_mock_prepared_rxbvm
            COMMAND $<TARGET_FILE:rxbvm>
                    --provider-path "${CMAKE_CURRENT_BINARY_DIR}/mock"
                    "${_odbc_mock_runtime_base}"
                    "${CMAKE_BINARY_DIR}/bin/library")
    set_tests_properties(odbc_mock_prepared_rxbvm PROPERTIES
            REQUIRED_FILES "${_odbc_mock_runtime_base}.rxbin"
            PASS_REGULAR_EXPRESSION "ODBC_MOCK_RUNTIME_OK"
            LABELS "runtime;rxbvm;plugins;rxpa;odbc")
    if(TARGET rxtvm)
        add_test(NAME odbc_mock_prepared_rxtvm
                COMMAND $<TARGET_FILE:rxtvm>
                        --provider-path "${CMAKE_CURRENT_BINARY_DIR}/mock"
                        "${_odbc_mock_runtime_base}"
                        "${CMAKE_BINARY_DIR}/bin/library")
        set_tests_properties(odbc_mock_prepared_rxtvm PROPERTIES
                REQUIRED_FILES "${_odbc_mock_runtime_base}.rxbin"
                PASS_REGULAR_EXPRESSION "ODBC_MOCK_RUNTIME_OK"
                LABELS "runtime;rxtvm;plugins;rxpa;odbc")
    endif()

    if(ENABLE_ODBC AND NOT WIN32)
        find_library(CREXX_SQLITE_ODBC_DRIVER
                NAMES sqlite3odbc
                PATHS /usr/lib /usr/local/lib /opt/homebrew/lib
                PATH_SUFFIXES odbc
                DOC "SQLite ODBC driver used by the real-driver regression tests")
        if(CREXX_SQLITE_ODBC_DRIVER)
            set(_odbc_sqlite_ini
                    "${CMAKE_CURRENT_BINARY_DIR}/odbc_sqlite.ini")
            configure_file(tests/odbc_sqlite.ini.in
                    "${_odbc_sqlite_ini}" @ONLY)

            set(_odbc_sqlite_runtime_base
                    "${CMAKE_CURRENT_BINARY_DIR}/odbc_sqlite_prepared")
            add_custom_command(
                    OUTPUT "${_odbc_sqlite_runtime_base}.rxbin"
                    BYPRODUCTS "${_odbc_sqlite_runtime_base}.rxas"
                    COMMAND $<TARGET_FILE:rxc> -n
                            -i "${CMAKE_BINARY_DIR}/bin\;${CMAKE_CURRENT_BINARY_DIR}"
                            -o "${_odbc_sqlite_runtime_base}"
                            "${CMAKE_CURRENT_SOURCE_DIR}/tests/odbc_sqlite_prepared.crexx"
                    COMMAND $<TARGET_FILE:rxas> -n
                            -o "${_odbc_sqlite_runtime_base}"
                            "${_odbc_sqlite_runtime_base}"
                    DEPENDS rxc rxas compiler_exit_bin library _odbc
                            "${CMAKE_CURRENT_SOURCE_DIR}/tests/odbc_sqlite_prepared.crexx")
            add_custom_target(odbc_sqlite_prepared_runtime ALL
                    DEPENDS "${_odbc_sqlite_runtime_base}.rxbin")

            add_test(NAME odbc_sqlite_prepared_rxbvm
                    COMMAND $<TARGET_FILE:rxbvm>
                            "${_odbc_sqlite_runtime_base}"
                            "$<TARGET_FILE_DIR:_odbc>/rx_odbc"
                            "${CMAKE_BINARY_DIR}/bin/library")
            set_tests_properties(odbc_sqlite_prepared_rxbvm PROPERTIES
                    REQUIRED_FILES "${_odbc_sqlite_runtime_base}.rxbin"
                    ENVIRONMENT "ODBCINI=${_odbc_sqlite_ini}"
                    PASS_REGULAR_EXPRESSION "ODBC_SQLITE_RUNTIME_OK"
                    LABELS "runtime;rxbvm;plugins;rxpa;odbc;real-driver")
            if(TARGET rxtvm)
                add_test(NAME odbc_sqlite_prepared_rxtvm
                        COMMAND $<TARGET_FILE:rxtvm>
                                "${_odbc_sqlite_runtime_base}"
                                "$<TARGET_FILE_DIR:_odbc>/rx_odbc"
                                "${CMAKE_BINARY_DIR}/bin/library")
                set_tests_properties(odbc_sqlite_prepared_rxtvm PROPERTIES
                        REQUIRED_FILES "${_odbc_sqlite_runtime_base}.rxbin"
                        ENVIRONMENT "ODBCINI=${_odbc_sqlite_ini}"
                        PASS_REGULAR_EXPRESSION "ODBC_SQLITE_RUNTIME_OK"
                        LABELS "runtime;rxtvm;plugins;rxpa;odbc;real-driver")
            endif()
        else()
            message(STATUS
                    "SQLite ODBC driver not found; real-driver regression tests skipped")
        endif()
    endif()
endif()

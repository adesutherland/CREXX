cmake_minimum_required(VERSION 3.24)

project(plugins NONE)

find_package(Threads REQUIRED)
include(${CMAKE_SOURCE_DIR}/rxpa/RXPluginFunction.cmake)

add_dynamic_plugin_target(_rxpa_session_failure
        ${CMAKE_SOURCE_DIR}/interpreter/tests/rxpa_session_failure_plugin.c)

#
# These are the plugins that are built for Windows, macOS and Linux
#
add_subdirectory(arrays)
add_subdirectory(float)
add_subdirectory(stats)
add_subdirectory(stack)
add_subdirectory(cipher)
add_subdirectory(hash)
add_subdirectory(strings)
add_subdirectory(getpi)
add_subdirectory(keyaccess)
add_subdirectory(llist)
add_subdirectory(treemap)
add_subdirectory(id)
add_subdirectory(fs)
add_subdirectory(platform)
add_subdirectory(recv390)
add_subdirectory(map)
add_subdirectory(rxtcp)

option(CREXX_BUILD_LEGACY_SOCKET_PLUGIN
       "Build the deprecated OpenSSL-backed dynamic socket plugin"
       OFF)
if(CREXX_BUILD_LEGACY_SOCKET_PLUGIN)
    add_subdirectory(socket)
else()
    message(STATUS "Legacy OpenSSL socket plugin is disabled; use VM core sockets/rxsocket instead.")
endif()

#
# These are the plugins that are additionally built for Linux
#
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  # add_subdirectory(map)
#  add_subdirectory(console) probably misses xwindows libs
endif()

#
# These are the plugins that are additionally built for macOS
#
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
#  add_subdirectory(console) probably misses xwindows libs
endif()

#
# These are the plugins that are additionally built for Windows (X64)
#
if(WIN32)
	add_subdirectory(rxml)
#	add_subdirectory(regex)
#	add_subdirectory(pegparser)
	add_subdirectory(fileio)
	add_subdirectory(matrix)
#	add_subdirectory(rxtcp)
#	add_subdirectory(rxshm)
#	add_subdirectory(pick)
#       add_subdirectory(pipe)
#       add_subdirectory(socket)
#       add_subdirectory(process)
#       add_subdirectory(subtask)
#	add_subdirectory(vpool)
        # add_subdirectory(map)
endif()

#
# These plugins are conditionally built, because they require external libraries
#

# specify -DENABLE_GTK=ON on the cmake commandline to build with GTK GUI support
if(ENABLE_GTK)
    add_subdirectory(gui)
    message(STATUS "Building with GTK Gui support enabled.")
else()
    message(STATUS "GTK Gui support is disabled, skipping subdirectory.")
endif()

# speficy -DENABLE_ODBC=ON on the cmake commandline to build with ODBC support
if(ENABLE_ODBC)
    add_subdirectory(odbc)
    message(STATUS "Building with ODBC support enabled.")
else()
    if(BUILD_TESTING)
        add_subdirectory(odbc)
        message(STATUS "ODBC support is disabled; building the mock-driver qualification target only.")
    else()
        message(STATUS "ODBC support is disabled, skipping subdirectory.")
    endif()
endif()

# The E3 concurrency harness is declared by the interpreter before the bundled
# plugins exist.  Attach the production static forms here, then exercise every
# qualified dynamic plugin through two simultaneous VM contexts.
configure_linker_for_static_lib(test_rxpa_concurrency _cipher)
configure_linker_for_static_lib(test_rxpa_concurrency _hash)
configure_linker_for_static_lib(test_rxpa_concurrency _stack)
configure_linker_for_static_lib(test_rxpa_concurrency id)
configure_linker_for_static_lib(test_rxpa_concurrency float)
configure_linker_for_static_lib(test_rxpa_concurrency stats)
configure_linker_for_static_lib(test_rxpa_concurrency fs)
configure_linker_for_static_lib(test_rxpa_concurrency _platform
        PROVIDER_ID rxplatform)
target_link_libraries(test_rxpa_concurrency rxpashim)
add_dependencies(test_rxpa_concurrency
        _cipher _cipher_static
        _hash _hash_static
        _stack _stack_static
        _strings _getpi
        id id_static)
add_dependencies(test_rxpa_concurrency float float_static)
add_dependencies(test_rxpa_concurrency
        stats stats_static
        fs fs_static
        _platform _platform_static)
add_dependencies(test_rxpa_concurrency _rxpa_session_failure)

add_test(NAME rxpa_bundled_cipher_concurrency
        COMMAND test_rxpa_concurrency bundled
                "$<TARGET_FILE_DIR:_cipher>" "$<TARGET_FILE_NAME:_cipher>" cipher)
add_test(NAME rxpa_bundled_hash_concurrency
        COMMAND test_rxpa_concurrency bundled
                "$<TARGET_FILE_DIR:_hash>" "$<TARGET_FILE_NAME:_hash>" hash)
add_test(NAME rxpa_bundled_stack_concurrency
        COMMAND test_rxpa_concurrency bundled
                "$<TARGET_FILE_DIR:_stack>" "$<TARGET_FILE_NAME:_stack>" stack)
add_test(NAME rxpa_bundled_strings_concurrency
        COMMAND test_rxpa_concurrency bundled
                "$<TARGET_FILE_DIR:_strings>" "$<TARGET_FILE_NAME:_strings>" strings)
add_test(NAME rxpa_bundled_getpi_concurrency
        COMMAND test_rxpa_concurrency bundled
                "$<TARGET_FILE_DIR:_getpi>" "$<TARGET_FILE_NAME:_getpi>" getpi)
add_test(NAME rxpa_bundled_id_concurrency
        COMMAND test_rxpa_concurrency bundled
                "$<TARGET_FILE_DIR:id>" "$<TARGET_FILE_NAME:id>" id)
add_test(NAME rxpa_bundled_float_concurrency
        COMMAND test_rxpa_concurrency bundled
                "$<TARGET_FILE_DIR:float>" "$<TARGET_FILE_NAME:float>" float)
add_test(NAME rxpa_bundled_stats_concurrency
        COMMAND test_rxpa_concurrency bundled
                "$<TARGET_FILE_DIR:stats>" "$<TARGET_FILE_NAME:stats>" stats)
add_test(NAME rxpa_bundled_fs_concurrency
        COMMAND test_rxpa_concurrency bundled
                "$<TARGET_FILE_DIR:fs>" "$<TARGET_FILE_NAME:fs>" fs)
add_test(NAME rxpa_bundled_platform_concurrency
        COMMAND test_rxpa_concurrency bundled
                "$<TARGET_FILE_DIR:_platform>" "$<TARGET_FILE_NAME:_platform>" platform)
add_test(NAME rxpa_dynamic_session_factory_failure_rollback
        COMMAND test_rxpa_concurrency session-load-failure
                "$<TARGET_FILE_DIR:_rxpa_session_failure>"
                "$<TARGET_FILE_NAME:_rxpa_session_failure>")
set_tests_properties(
        rxpa_bundled_cipher_concurrency
        rxpa_bundled_hash_concurrency
        rxpa_bundled_stack_concurrency
        rxpa_bundled_strings_concurrency
        rxpa_bundled_getpi_concurrency
        rxpa_bundled_id_concurrency
        rxpa_bundled_float_concurrency
        rxpa_bundled_stats_concurrency
        rxpa_bundled_fs_concurrency
        rxpa_bundled_platform_concurrency
        rxpa_dynamic_session_factory_failure_rollback
        PROPERTIES LABELS "interpreter;worker;plugins;rxpa;bundled")

if(TARGET _odbc_mock)
    add_dependencies(test_rxpa_odbc _odbc_mock)
    add_dependencies(test_rxpa_odbc_old_host _odbc_mock)
    add_test(NAME rxpa_odbc_mock_prepared_sessions
            COMMAND test_rxpa_odbc "$<TARGET_FILE_DIR:_odbc_mock>")
    set_tests_properties(rxpa_odbc_mock_prepared_sessions PROPERTIES
            LABELS "interpreter;worker;plugins;rxpa;odbc")
    add_test(NAME rxpa_odbc_old_host_default_session
            COMMAND test_rxpa_odbc_old_host "$<TARGET_FILE_DIR:_odbc_mock>")
    set_tests_properties(rxpa_odbc_old_host_default_session PROPERTIES
            LABELS "interpreter;plugins;rxpa;odbc;compatibility")
endif()

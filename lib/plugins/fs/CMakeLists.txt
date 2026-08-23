cmake_minimum_required(VERSION 3.24)
project(rxfs_plugin C)

set(CMAKE_C_STANDARD 99)

include(${CMAKE_SOURCE_DIR}/cmake/CrexxTestModes.cmake)
include(${CMAKE_SOURCE_DIR}/rxpa/RXPluginFunction.cmake)

add_dynamic_plugin_target(fs rxfs.c)
add_static_plugin_target(fs rxfs.c)
add_rxpa_provider_package(fs)

add_dependencies(rxc fs)
add_dependencies(rxvm fs)
add_dependencies(rxbvm fs)
if(TARGET rxtvm)
    add_dependencies(rxtvm fs)
endif()

if(BUILD_TESTING)
    add_custom_target(rxfs_test ALL)
    set(_rxfs_runners rxbvm)
    if(TARGET rxtvm)
        list(APPEND _rxfs_runners rxtvm)
    endif()
    foreach(_rxfs_runner IN LISTS _rxfs_runners)
        crexx_add_rexx_opt_matrix(
                PASS_TEST_NAME_ARGUMENT
                NAME rxfs_${_rxfs_runner}
                SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/rxfs_test.crexx
                TARGET_GROUP rxfs_test
                RUNNER ${_rxfs_runner}
                IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin
                DEPENDS library fs fs_static
                RUNTIME_ARGS ${CMAKE_BINARY_DIR}/bin/library
                TEST_LABELS "library;plugins;rxpa;filesystem"
                REQUIRED_OUTPUT_REGEX "PASS: rxfs public contract"
                FORBIDDEN_OUTPUT_REGEX "FAIL:|PANIC:")
    endforeach()
endif()

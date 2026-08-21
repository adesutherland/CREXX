cmake_minimum_required(VERSION 3.24)
project(rxid_plugin C)

set(CMAKE_C_STANDARD 99)

include(${CMAKE_SOURCE_DIR}/cmake/CrexxTestModes.cmake)
include(${CMAKE_SOURCE_DIR}/rxpa/RXPluginFunction.cmake)

add_dynamic_plugin_target(id id.c)
target_link_libraries(id PRIVATE Threads::Threads)

add_static_plugin_target(id id.c)
target_link_libraries(id_static PRIVATE Threads::Threads)

add_rxpa_provider_package(id)

if(BUILD_TESTING)
    add_custom_target(rxid_test ALL)
    set(_rxid_runners rxbvm)
    if(TARGET rxtvm)
        list(APPEND _rxid_runners rxtvm)
    endif()

    foreach(_rxid_runner IN LISTS _rxid_runners)
        crexx_add_rexx_opt_matrix(
                NAME rxid_${_rxid_runner}
                SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/rxid_test.crexx
                TARGET_GROUP rxid_test
                RUNNER ${_rxid_runner}
                IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin
                DEPENDS library id id_static
                RUNTIME_ARGS ${CMAKE_BINARY_DIR}/bin/library
                TEST_LABELS "library;plugins;rxpa;id;optional"
                REQUIRED_OUTPUT_REGEX "PASS: rxid public contract"
                FORBIDDEN_OUTPUT_REGEX "FAIL:|PANIC:")
    endforeach()
endif()

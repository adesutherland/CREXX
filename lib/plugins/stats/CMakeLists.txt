cmake_minimum_required(VERSION 3.24)
project(rxstats_plugin C)

set(CMAKE_C_STANDARD 99)

include(${CMAKE_SOURCE_DIR}/cmake/CrexxTestModes.cmake)
include(${CMAKE_SOURCE_DIR}/rxpa/RXPluginFunction.cmake)

add_dynamic_plugin_target(stats rxstats.c)
target_link_libraries(stats PRIVATE ${CREXX_MATH_LIBRARY})

add_static_plugin_target(stats rxstats.c)
target_link_libraries(stats_static PRIVATE ${CREXX_MATH_LIBRARY})

add_rxpa_provider_package(stats)

add_dependencies(rxc stats)
add_dependencies(rxvm stats)
add_dependencies(rxbvm stats)
if(TARGET rxtvm)
    add_dependencies(rxtvm stats)
endif()

if(BUILD_TESTING)
    add_custom_target(rxstats_test ALL)
    set(_rxstats_runners rxbvm)
    if(TARGET rxtvm)
        list(APPEND _rxstats_runners rxtvm)
    endif()

    foreach(_rxstats_runner IN LISTS _rxstats_runners)
        crexx_add_rexx_opt_matrix(
                NAME rxstats_${_rxstats_runner}
                SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/rxstats_test.crexx
                TARGET_GROUP rxstats_test
                RUNNER ${_rxstats_runner}
                IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin
                DEPENDS library stats stats_static
                RUNTIME_ARGS ${CMAKE_BINARY_DIR}/bin/library
                TEST_LABELS "library;plugins;rxpa;math;stats;transitional"
                REQUIRED_OUTPUT_REGEX "PASS: rxstats transitional boxed-array contract"
                FORBIDDEN_OUTPUT_REGEX "FAIL:|PANIC:")
    endforeach()
endif()

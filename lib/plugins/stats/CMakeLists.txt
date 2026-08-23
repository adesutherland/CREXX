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
    add_test(
            NAME rxstats_packedint_rejected
            COMMAND ${CMAKE_COMMAND}
                    -DRXC=$<TARGET_FILE:rxc>
                    -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rxstats_packedint_rejected.crexx
                    -DIMPORT_PATH=${CMAKE_BINARY_DIR}/bin
                    -DOUTPUT_BASE=${CMAKE_CURRENT_BINARY_DIR}/rxstats_packedint_rejected
                    -P ${CMAKE_CURRENT_SOURCE_DIR}/check_packedint_rejected.cmake)
    set_tests_properties(rxstats_packedint_rejected PROPERTIES
            LABELS "library;plugins;rxpa;math;stats;packed;negative")
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
                DEPENDS library rxfnsg float float_static stats stats_static
                RUNTIME_ARGS ${CMAKE_BINARY_DIR}/bin/library
                        ${CMAKE_BINARY_DIR}/bin/rxfnsg
                TEST_LABELS "library;plugins;rxpa;math;stats;packed"
                REQUIRED_OUTPUT_REGEX "PASS: rxstats packed-float contract"
                FORBIDDEN_OUTPUT_REGEX "FAIL:|PANIC:")
    endforeach()
endif()

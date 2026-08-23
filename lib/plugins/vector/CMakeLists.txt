cmake_minimum_required(VERSION 3.24)
project(rxvector_plugin C)

set(CMAKE_C_STANDARD 99)

include(${CMAKE_SOURCE_DIR}/cmake/CrexxTestModes.cmake)
include(${CMAKE_SOURCE_DIR}/rxpa/RXPluginFunction.cmake)

add_dynamic_plugin_target(vector rxvector.c)
target_link_libraries(vector PRIVATE ${CREXX_MATH_LIBRARY})

add_static_plugin_target(vector rxvector.c)
target_link_libraries(vector_static PRIVATE ${CREXX_MATH_LIBRARY})

add_rxpa_provider_package(vector)

add_dependencies(rxc vector)
add_dependencies(rxvm vector)
add_dependencies(rxbvm vector)
if(TARGET rxtvm)
    add_dependencies(rxtvm vector)
endif()

if(BUILD_TESTING)
    add_custom_target(rxvector_test ALL)
    set(_rxvector_runners rxbvm)
    if(TARGET rxtvm)
        list(APPEND _rxvector_runners rxtvm)
    endif()

    foreach(_rxvector_runner IN LISTS _rxvector_runners)
        crexx_add_rexx_opt_matrix(
                NAME rxvector_${_rxvector_runner}
                SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/rxvector_test.crexx
                TARGET_GROUP rxvector_test
                RUNNER ${_rxvector_runner}
                IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin
                DEPENDS library rxfnsg float float_static vector vector_static
                RUNTIME_ARGS ${CMAKE_BINARY_DIR}/bin/library
                        ${CMAKE_BINARY_DIR}/bin/rxfnsg
                TEST_LABELS "library;plugins;rxpa;vector;packed;exact"
                REQUIRED_OUTPUT_REGEX "PASS: rxvector exact packed contract"
                FORBIDDEN_OUTPUT_REGEX "FAIL:|PANIC:")
    endforeach()
endif()

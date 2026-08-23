cmake_minimum_required(VERSION 3.24)
project(rxplatform_plugin C)

set(CMAKE_C_STANDARD 99)

include(${CMAKE_SOURCE_DIR}/cmake/CrexxTestModes.cmake)
include(${CMAKE_SOURCE_DIR}/rxpa/RXPluginFunction.cmake)

# The core platform target already owns the natural CMake name.
add_dynamic_plugin_target(_platform PROVIDER_ID rxplatform rxplatform.c)
add_static_plugin_target(_platform PROVIDER_ID rxplatform rxplatform.c)
add_rxpa_provider_package(_platform PROVIDER_ID rxplatform)

if(BUILD_TESTING)
    add_custom_target(rxplatform_test ALL)
    set(_rxplatform_runners rxbvm)
    if(TARGET rxtvm)
        list(APPEND _rxplatform_runners rxtvm)
    endif()
    foreach(_rxplatform_runner IN LISTS _rxplatform_runners)
        crexx_add_rexx_opt_matrix(
                NAME rxplatform_${_rxplatform_runner}
                SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/rxplatform_test.crexx
                TARGET_GROUP rxplatform_test
                RUNNER ${_rxplatform_runner}
                IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin
                DEPENDS library _platform _platform_static
                RUNTIME_ARGS ${CMAKE_BINARY_DIR}/bin/library
                TEST_LABELS "library;plugins;rxpa;platform;optional"
                REQUIRED_OUTPUT_REGEX "PASS: rxplatform public contract"
                FORBIDDEN_OUTPUT_REGEX "FAIL:|PANIC:")
    endforeach()
endif()

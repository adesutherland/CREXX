cmake_minimum_required(VERSION 3.24)

project(rxfloat_plugin C)

set(CMAKE_C_STANDARD 99)

# Remove exact artifacts published by superseded pre-release provider names.
# Otherwise an incremental build can discover rx_float before rxfloat and
# preserve the retired provider ID in newly compiled RXBIN metadata.
file(REMOVE
        "${CMAKE_BINARY_DIR}/bin/rx_float.rxplugin"
        "${CMAKE_BINARY_DIR}/bin/rx_float_static${CMAKE_STATIC_LIBRARY_SUFFIX}"
        "${CMAKE_BINARY_DIR}/bin/rx_rxmath_legacy.rxplugin"
        "${CMAKE_BINARY_DIR}/bin/rx_rxmath_legacy_static${CMAKE_STATIC_LIBRARY_SUFFIX}"
        "${CMAKE_BINARY_DIR}/bin/providers/rx_float.rxplugin"
        "${CMAKE_BINARY_DIR}/bin/providers/rx_float${CMAKE_STATIC_LIBRARY_SUFFIX}"
        "${CMAKE_BINARY_DIR}/bin/providers/rx_float_static${CMAKE_STATIC_LIBRARY_SUFFIX}"
        "${CMAKE_BINARY_DIR}/bin/providers/rx_rxmath_legacy.rxplugin"
        "${CMAKE_BINARY_DIR}/bin/providers/rx_rxmath_legacy${CMAKE_STATIC_LIBRARY_SUFFIX}"
        "${CMAKE_BINARY_DIR}/bin/providers/rx_rxmath_legacy_static${CMAKE_STATIC_LIBRARY_SUFFIX}")

include(${CMAKE_SOURCE_DIR}/cmake/CrexxTestModes.cmake)
include(${CMAKE_SOURCE_DIR}/rxpa/RXPluginFunction.cmake)

add_dynamic_plugin_target(float rxfloat.c)
target_link_libraries(float PRIVATE ${CREXX_MATH_LIBRARY})

add_static_plugin_target(float rxfloat.c)
target_link_libraries(float_static PRIVATE ${CREXX_MATH_LIBRARY})

add_rxpa_provider_package(float)

# The canonical provider and archive names are rxfloat.  Preserve the
# historical artifacts as declaration-time compatibility inputs for existing
# source/build invocations; their embedded provider identity remains rxfloat.
add_custom_command(TARGET float POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "$<TARGET_FILE:float>"
                "${CMAKE_BINARY_DIR}/bin/rx_rxmath.rxplugin")
add_custom_command(TARGET float_static POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "$<TARGET_FILE:float_static>"
                "${CMAKE_BINARY_DIR}/bin/rx_rxmath_static${CMAKE_STATIC_LIBRARY_SUFFIX}")
add_custom_target(_rxmath DEPENDS float)
add_custom_target(_rxmath_static DEPENDS float_static)

# Targeted compiler/VM builds must publish declarations and the trusted
# provider artifact used by automatic dependency discovery.
add_dependencies(rxc float)
add_dependencies(rxvm float)
add_dependencies(rxbvm float)
if(TARGET rxtvm)
    add_dependencies(rxtvm float)
endif()

if(BUILD_TESTING)
    add_custom_target(rxfloat_test ALL)
    set(_rxfloat_runners rxbvm)
    if(TARGET rxtvm)
        list(APPEND _rxfloat_runners rxtvm)
    endif()

    foreach(_rxfloat_runner IN LISTS _rxfloat_runners)
        crexx_add_rexx_opt_matrix(
                NAME rxfloat_${_rxfloat_runner}
                SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/rxfloat_test.crexx
                TARGET_GROUP rxfloat_test
                RUNNER ${_rxfloat_runner}
                IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin
                DEPENDS library float float_static
                RUNTIME_ARGS ${CMAKE_BINARY_DIR}/bin/library
                TEST_LABELS "library;plugins;rxpa;math;float"
                REQUIRED_OUTPUT_REGEX "PASS: rxfloat scalar and compatibility contracts"
                FORBIDDEN_OUTPUT_REGEX "FAIL:|PANIC:")
    endforeach()
endif()

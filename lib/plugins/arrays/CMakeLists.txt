cmake_minimum_required(VERSION 3.24)
project(arrays_plugin C)

set(CMAKE_C_STANDARD 99)

include(${CMAKE_SOURCE_DIR}/cmake/CrexxTestModes.cmake)

# Including RXPA Build System
include(${CMAKE_SOURCE_DIR}/rxpa/RXPluginFunction.cmake)

# Create dynamic plugin module
add_dynamic_plugin_target(_arrays arrays.c)

add_custom_target(arrays_test ALL)
crexx_add_rexx_opt_matrix(
        NAME arrays_test
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/arrays_test.rexx
        TARGET_GROUP arrays_test
        IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin ${CMAKE_CURRENT_BINARY_DIR}
        DEPENDS library _arrays
        RUNTIME_ARGS rx_arrays ${CMAKE_BINARY_DIR}/bin/library
)

enable_testing()

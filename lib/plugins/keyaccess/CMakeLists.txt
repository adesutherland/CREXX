cmake_minimum_required(VERSION 3.24)
project(keyaccess_plugin C)

set(CMAKE_C_STANDARD 90)

include(${CMAKE_SOURCE_DIR}/cmake/CrexxTestModes.cmake)

# Including RXPA Build System
include(${CMAKE_SOURCE_DIR}/rxpa/RXPluginFunction.cmake)

# Create dynamic plugin module
add_dynamic_plugin_target(_keyaccess keyaccess.c)

# Create static plugin module
add_static_plugin_target(_keyaccess keyaccess.c)

add_custom_target(keyaccess_test ALL)
crexx_add_rexx_opt_matrix(
        NAME keyaccess_test
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/keyaccess_test.rexx
        TARGET_GROUP keyaccess_test
        IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin ${CMAKE_CURRENT_BINARY_DIR}
        DEPENDS library _keyaccess
        RUNTIME_ARGS rx_keyaccess ${CMAKE_BINARY_DIR}/bin/library
)

enable_testing()

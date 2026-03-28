cmake_minimum_required(VERSION 3.24)
project(getpi_plugin C)

set(CMAKE_C_STANDARD 90)

include(${CMAKE_SOURCE_DIR}/cmake/CrexxTestModes.cmake)

# Including RXPA Build System
include(${CMAKE_SOURCE_DIR}/rxpa/RXPluginFunction.cmake)

# Create dynamic plugin module
add_dynamic_plugin_target(_getpi getpi.c)

add_custom_target(getpi_test ALL)
crexx_add_rexx_opt_matrix(
        NAME getpi_test
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/getpi_test.rexx
        TARGET_GROUP getpi_test
        IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin ${CMAKE_CURRENT_BINARY_DIR}
        DEPENDS library _getpi
        RUNTIME_ARGS rx_getpi ${CMAKE_BINARY_DIR}/bin/library
)

enable_testing()

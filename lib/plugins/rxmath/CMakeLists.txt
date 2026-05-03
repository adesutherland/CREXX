cmake_minimum_required(VERSION 3.24)
project(rxmath_plugin C)

set(CMAKE_C_STANDARD 90)

include(${CMAKE_SOURCE_DIR}/cmake/CrexxTestModes.cmake)

# Including RXPA Build System
include(${CMAKE_SOURCE_DIR}/rxpa/RXPluginFunction.cmake)

# Create dynamic plugin module
add_dynamic_plugin_target(_rxmath rxmath.c)

# Create static plugin library
add_static_plugin_target(_rxmath rxmath.c)

add_custom_target(math_test ALL)
crexx_add_rexx_opt_matrix(
        NAME math_test
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/math_test.rexx
        TARGET_GROUP math_test
        IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin ${CMAKE_CURRENT_BINARY_DIR}
        DEPENDS library _rxmath
        RUNTIME_ARGS rx_rxmath ${CMAKE_BINARY_DIR}/bin/library
)

enable_testing()

cmake_minimum_required(VERSION 3.24)
project(stack_plugin C)

set(CMAKE_C_STANDARD 90)

include(${CMAKE_SOURCE_DIR}/cmake/CrexxTestModes.cmake)

# Including RXPA Build System
include(${CMAKE_SOURCE_DIR}/rxpa/RXPluginFunction.cmake)

# Create dynamic plugin module
add_dynamic_plugin_target(_stack stack.c)

add_custom_target(stack_test ALL)
crexx_add_rexx_opt_matrix(
        NAME stack_test
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/stack_test.rexx
        TARGET_GROUP stack_test
        IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin ${CMAKE_CURRENT_BINARY_DIR}
        DEPENDS library _stack
        RUNTIME_ARGS rx_stack ${CMAKE_BINARY_DIR}/bin/library
)

enable_testing()

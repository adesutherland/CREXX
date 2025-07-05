cmake_minimum_required(VERSION 3.24)
project(matrix_plugin C)

set(CMAKE_C_STANDARD 99)

# Including RXPA Build System
include(${CMAKE_SOURCE_DIR}/rxpa/RXPluginFunction.cmake)

# Create dynamic plugin module
add_dynamic_plugin_target(_matrix matrix.c)

# Build Test Rexx
add_custom_command(
        COMMAND ${CMAKE_BINARY_DIR}/compiler/rxc
        -i \"${CMAKE_BINARY_DIR}/lib/rxfnsb\;${CMAKE_CURRENT_BINARY_DIR}\"
        -o matrix_test \"${CMAKE_CURRENT_SOURCE_DIR}/matrix_test\" &&
        ${CMAKE_BINARY_DIR}/assembler/rxas matrix_test

        DEPENDS rxas rxc library _matrix ${CMAKE_CURRENT_SOURCE_DIR}/matrix_test.rexx
        OUTPUT matrix_test.rxbin
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
add_custom_target(matrix_test ALL
        DEPENDS matrix_test.rxbin
        )

# Enable testing functionality
enable_testing()

# Basic test to run matrix_test.rxbin with the matrix plugin, full test sample
add_test(NAME matrix_test
        COMMAND ${CMAKE_BINARY_DIR}/interpreter/rxvm matrix_test rx_matrix ${CMAKE_BINARY_DIR}/lib/rxfnsb/library
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )
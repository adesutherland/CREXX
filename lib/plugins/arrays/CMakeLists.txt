cmake_minimum_required(VERSION 3.24)
project(arrays_plugin C)

set(CMAKE_C_STANDARD 90)

# Including RXPA Build System
include(${CMAKE_SOURCE_DIR}/rxpa/RXPluginFunction.cmake)

# Create dynamic plugin module
add_dynamic_plugin_target(_arrays arrays.c)

# Build Test Rexx
add_custom_command(
        COMMAND ${CMAKE_BINARY_DIR}/compiler/rxc
        -i \"${CMAKE_BINARY_DIR}/lib/rxfns\;${CMAKE_CURRENT_BINARY_DIR}\"
        -o arrays_test \"${CMAKE_CURRENT_SOURCE_DIR}/arrays_test\" &&
        ${CMAKE_BINARY_DIR}/assembler/rxas arrays_test

        DEPENDS rxas rxc library _arrays ${CMAKE_CURRENT_SOURCE_DIR}/arrays_test.rexx
        OUTPUT arrays_test.rxbin
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
add_custom_target(arrays_test ALL
        DEPENDS arrays_test.rxbin
        )

# Enable testing functionality
enable_testing()

# Basic test to run arrays_test.rxbin with the arrays plugin
add_test(NAME arrays_test
        COMMAND ${CMAKE_BINARY_DIR}/interpreter/rxvm arrays_test rx_arrays ${CMAKE_BINARY_DIR}/lib/rxfns/library
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )

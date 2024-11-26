cmake_minimum_required(VERSION 3.24)
project(stack_plugin C)

set(CMAKE_C_STANDARD 90)

# Including RXPA Build System
include(${CMAKE_SOURCE_DIR}/rxpa/RXPluginFunction.cmake)

# Create dynamic plugin module
add_dynamic_plugin_target(_stack stack.c)

# Build Test Rexx
add_custom_command(
        COMMAND ${CMAKE_BINARY_DIR}/compiler/rxc
        -i \"${CMAKE_BINARY_DIR}/lib/rxfns\;${CMAKE_CURRENT_BINARY_DIR}\"
        -o stack_test \"${CMAKE_CURRENT_SOURCE_DIR}/stack_test\" &&
        ${CMAKE_BINARY_DIR}/assembler/rxas stack_test

        DEPENDS rxas rxc library _stack ${CMAKE_CURRENT_SOURCE_DIR}/stack_test.rexx
        OUTPUT stack_test.rxbin
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
add_custom_target(stack_test ALL
        DEPENDS stack_test.rxbin
        )

# Enable testing functionality
enable_testing()

# Basic test to run stack_test.rxbin with the stack plugin
add_test(NAME stack_test
        COMMAND ${CMAKE_BINARY_DIR}/interpreter/rxvm stack_test rx_stack ${CMAKE_BINARY_DIR}/lib/rxfns/library
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )
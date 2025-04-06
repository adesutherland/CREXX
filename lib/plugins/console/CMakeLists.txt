cmake_minimum_required(VERSION 3.24)
project(console_plugin C)

set(CMAKE_C_STANDARD 99)

# Including RXPA Build System
include(${CMAKE_SOURCE_DIR}/rxpa/RXPluginFunction.cmake)

# Create dynamic plugin module
add_dynamic_plugin_target(_console console.c)

# Build Test Rexx
add_custom_command(
        COMMAND ${CMAKE_BINARY_DIR}/compiler/rxc
        -i \"${CMAKE_BINARY_DIR}/lib/rxfnsb\;${CMAKE_CURRENT_BINARY_DIR}\"
        -o console_test \"${CMAKE_CURRENT_SOURCE_DIR}/console_test\" &&
        ${CMAKE_BINARY_DIR}/assembler/rxas console_test

        DEPENDS rxas rxc library _console ${CMAKE_CURRENT_SOURCE_DIR}/console_test.rexx
        OUTPUT console_test.rxbin
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
add_custom_target(console_test ALL
        DEPENDS console_test.rxbin
        )

# Enable testing functionality
enable_testing()

# Basic test to run console_test.rxbin with the console plugin
add_test(NAME console_test
        COMMAND ${CMAKE_BINARY_DIR}/interpreter/rxvm console_test rx_console ${CMAKE_BINARY_DIR}/lib/rxfnsb/library
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )
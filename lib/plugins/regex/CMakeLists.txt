cmake_minimum_required(VERSION 3.24)
project(regex_plugin C)

set(CMAKE_C_STANDARD 90)

# Including RXPA Build System
include(${CMAKE_SOURCE_DIR}/rxpa/RXPluginFunction.cmake)

# Create dynamic plugin module
add_dynamic_plugin_target(_regex regex.c)

# Add regex library linkage
target_link_libraries(_regex PRIVATE regex)

# Build Test Rexx
add_custom_command(
        COMMAND ${CMAKE_BINARY_DIR}/compiler/rxc
        -i \"${CMAKE_BINARY_DIR}/lib/rxfnsb\;${CMAKE_CURRENT_BINARY_DIR}\"
        -o regex_test \"${CMAKE_CURRENT_SOURCE_DIR}/regex_test\" &&
        ${CMAKE_BINARY_DIR}/assembler/rxas regex_test

        DEPENDS rxas rxc library _regex ${CMAKE_CURRENT_SOURCE_DIR}/regex_test.rexx
        OUTPUT regex_test.rxbin
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
add_custom_target(regex_test ALL
        DEPENDS regex_test.rxbin
        )

# Enable testing functionality
enable_testing()

# Basic test to run regex_test.rxbin with the regex plugin
add_test(NAME regex_test
        COMMAND ${CMAKE_BINARY_DIR}/interpreter/rxvm regex_test rx_regex ${CMAKE_BINARY_DIR}/lib/rxfnsb/library
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )
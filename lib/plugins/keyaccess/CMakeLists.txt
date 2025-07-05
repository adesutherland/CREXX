cmake_minimum_required(VERSION 3.24)
project(keyaccess_plugin C)

set(CMAKE_C_STANDARD 90)

# Including RXPA Build System
include(${CMAKE_SOURCE_DIR}/rxpa/RXPluginFunction.cmake)

# Create dynamic plugin module
add_dynamic_plugin_target(_keyaccess keyaccess.c)

# Build Test Rexx
add_custom_command(
        COMMAND ${CMAKE_BINARY_DIR}/compiler/rxc
        -i \"${CMAKE_BINARY_DIR}/lib/rxfnsb\;${CMAKE_CURRENT_BINARY_DIR}\"
        -o keyaccess_test \"${CMAKE_CURRENT_SOURCE_DIR}/keyaccess_test\" &&
        ${CMAKE_BINARY_DIR}/assembler/rxas keyaccess_test

        DEPENDS rxas rxc library _keyaccess ${CMAKE_CURRENT_SOURCE_DIR}/keyaccess_test.rexx
        OUTPUT keyaccess_test.rxbin
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
add_custom_target(keyaccess_test ALL
        DEPENDS keyaccess_test.rxbin
        )

# Enable testing functionality
enable_testing()

# Basic test to run keyaccess_test.rxbin with the keyaccess plugin
add_test(NAME keyaccess_test
        COMMAND ${CMAKE_BINARY_DIR}/interpreter/rxvm keyaccess_test rx_keyaccess ${CMAKE_BINARY_DIR}/lib/rxfnsb/library
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )
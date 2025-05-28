cmake_minimum_required(VERSION 3.24)
project(precomp_plugin C)

set(CMAKE_C_STANDARD 99)

# Including RXPA Build System
include(${CMAKE_SOURCE_DIR}/rxpa/RXPluginFunction.cmake)

# Create dynamic plugin module
add_dynamic_plugin_target(_precomp precomp.c)

# Build test2 Rexx
add_custom_command(
        COMMAND ${CMAKE_BINARY_DIR}/compiler/rxc
        -i \"${CMAKE_BINARY_DIR}/lib/rxfnsb\;${CMAKE_CURRENT_BINARY_DIR}\"
        -o rxpp \"${CMAKE_CURRENT_SOURCE_DIR}/rxpp\" &&
        ${CMAKE_BINARY_DIR}/assembler/rxas rxpp

        DEPENDS rxas rxc library _precomp ${CMAKE_CURRENT_SOURCE_DIR}/rxpp.rexx
        OUTPUT rxpp.rxbin
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
add_custom_target(rxpp ALL
        DEPENDS rxpp.rxbin
        )

# Enable test2ing functionality
enable_testing()

# Basic test to run rxpp.rxbin with the precomp plugin
add_test(NAME rxpp
        COMMAND ${CMAKE_BINARY_DIR}/interpreter/rxvm rxpp rx_precomp ${CMAKE_BINARY_DIR}/lib/rxfnsb/library
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )
cmake_minimum_required(VERSION 3.24)
project(rxml_plugin C)

set(CMAKE_C_STANDARD 90)

# Including RXPA Build System
include(${CMAKE_SOURCE_DIR}/rxpa/RXPluginFunction.cmake)

# Create dynamic plugin module
add_dynamic_plugin_target(_rxml rxml.c)

# Build test2 Rexx
add_custom_command(
        COMMAND ${CMAKE_BINARY_DIR}/compiler/rxc
        -i \"${CMAKE_BINARY_DIR}/lib/rxfnsb\;${CMAKE_CURRENT_BINARY_DIR}\"
        -o rxml_test \"${CMAKE_CURRENT_SOURCE_DIR}/rxml_test\" &&
        ${CMAKE_BINARY_DIR}/assembler/rxas rxml_test

        DEPENDS rxas rxc library _rxml ${CMAKE_CURRENT_SOURCE_DIR}/rxml_test.rexx
        OUTPUT rxml_test.rxbin
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
add_custom_target(rxml_test ALL
        DEPENDS rxml_test.rxbin
        )

# Enable test2ing functionality
enable_testing()

# Basic test to run rxml_test.rxbin with the rxml plugin
add_test(NAME rxml_test
        COMMAND ${CMAKE_BINARY_DIR}/interpreter/rxvm rxml_test rx_rxml ${CMAKE_BINARY_DIR}/lib/rxfnsb/library
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )
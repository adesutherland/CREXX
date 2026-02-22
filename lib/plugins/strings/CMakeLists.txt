cmake_minimum_required(VERSION 3.24)
project(strings_plugin C)

set(CMAKE_C_STANDARD 99)

# Including RXPA Build System
include(${CMAKE_SOURCE_DIR}/rxpa/RXPluginFunction.cmake)

# Create dynamic plugin module
add_dynamic_plugin_target(_strings strings.c)

# Build Test Rexx
add_custom_command(
        COMMAND $<TARGET_FILE:rxc>
        -i \"${CMAKE_BINARY_DIR}/bin\;${CMAKE_CURRENT_BINARY_DIR}\"
        -o strings_test \"${CMAKE_CURRENT_SOURCE_DIR}/strings_test\" &&
        $<TARGET_FILE:rxas> strings_test

        DEPENDS rxas rxc library _strings ${CMAKE_CURRENT_SOURCE_DIR}/strings_test.rexx
        OUTPUT strings_test.rxbin
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
add_custom_target(strings_test ALL
        DEPENDS strings_test.rxbin
)

# Enable testing functionality
enable_testing()

# Basic test to run strings_test.rxbin with the strings plugin
add_test(NAME strings_test
        COMMAND $<TARGET_FILE:rxvm> strings_test rx_strings ${CMAKE_BINARY_DIR}/bin/library
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )

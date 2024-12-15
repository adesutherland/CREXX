cmake_minimum_required(VERSION 3.24)
project(system_plugin C)

set(CMAKE_C_STANDARD 90)

# Including RXPA Build System
include(${CMAKE_SOURCE_DIR}/rxpa/RXPluginFunction.cmake)

# Create dynamic plugin module
add_dynamic_plugin_target(_system system.c)

# Build Test Rexx
add_custom_command(
        COMMAND ${CMAKE_BINARY_DIR}/compiler/rxc
            -i \"${CMAKE_BINARY_DIR}/lib/rxfnsb\;${CMAKE_CURRENT_BINARY_DIR}\"
            -o system_test \"${CMAKE_CURRENT_SOURCE_DIR}/system_test\" &&
        ${CMAKE_BINARY_DIR}/assembler/rxas system_test

        DEPENDS rxas rxc library _system ${CMAKE_CURRENT_SOURCE_DIR}/system_test.rexx
        OUTPUT system_test.rxbin
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
add_custom_target(system_test ALL
        DEPENDS system_test.rxbin
)

# Enable testing functionality
enable_testing()

# Basic test to run system_test.rxbin with the system plugin
add_test(NAME system_test
        COMMAND ${CMAKE_BINARY_DIR}/interpreter/rxvm system_test rx_system ${CMAKE_BINARY_DIR}/lib/rxfnsb/library
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

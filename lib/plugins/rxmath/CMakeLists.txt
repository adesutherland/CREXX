cmake_minimum_required(VERSION 3.24)
project(rxmath_plugin C)

set(CMAKE_C_STANDARD 90)

# Including RXPA Build System
include(${CMAKE_SOURCE_DIR}/rxpa/RXPluginFunction.cmake)

# Create dynamic plugin module
add_dynamic_plugin_target(_rxmath rxmath.c)

# Build Test Rexx
add_custom_command(
        COMMAND ${CMAKE_BINARY_DIR}/compiler/rxc
        -i \"${CMAKE_BINARY_DIR}/lib/rxfnsb\;${CMAKE_CURRENT_BINARY_DIR}\"
        -o math_test \"${CMAKE_CURRENT_SOURCE_DIR}/math_test\" &&
        ${CMAKE_BINARY_DIR}/assembler/rxas math_test

        DEPENDS rxas rxc library _rxmath ${CMAKE_CURRENT_SOURCE_DIR}/math_test.rexx
        OUTPUT math_test.rxbin
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
add_custom_target(math_test ALL
        DEPENDS math_test.rxbin
        )

# Enable testing functionality
enable_testing()

# Basic test to run math_test.rxbin with the rxmath plugin
add_test(NAME math_test
        COMMAND ${CMAKE_BINARY_DIR}/interpreter/rxvm math_test rx_rxmath ${CMAKE_BINARY_DIR}/lib/rxfnsb/library
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )

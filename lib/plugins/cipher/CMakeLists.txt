cmake_minimum_required(VERSION 3.24)
project(cipher_plugin C)

set(CMAKE_C_STANDARD 90)

# Including RXPA Build System
include(${CMAKE_SOURCE_DIR}/rxpa/RXPluginFunction.cmake)

# Create dynamic plugin module
add_dynamic_plugin_target(_cipher cipher.c)

# Build Test Rexx
add_custom_command(
        COMMAND ${CMAKE_BINARY_DIR}/compiler/rxc
        -i \"${CMAKE_BINARY_DIR}/lib/rxfns\;${CMAKE_CURRENT_BINARY_DIR}\"
        -o cipher_test \"${CMAKE_CURRENT_SOURCE_DIR}/cipher_test\" &&
        ${CMAKE_BINARY_DIR}/assembler/rxas cipher_test

        DEPENDS rxas rxc library _cipher ${CMAKE_CURRENT_SOURCE_DIR}/cipher_test.rexx
        OUTPUT cipher_test.rxbin
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
add_custom_target(cipher_test ALL
        DEPENDS cipher_test.rxbin
        )

# Enable testing functionality
enable_testing()

# Basic test to run cipher_test.rxbin with the cipher plugin
add_test(NAME cipher_test
        COMMAND ${CMAKE_BINARY_DIR}/interpreter/rxvm cipher_test rx_cipher ${CMAKE_BINARY_DIR}/lib/rxfns/library
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )
cmake_minimum_required(VERSION 3.24)
project(rxuuid_plugin C)

set(CMAKE_C_STANDARD 90)

# Including RXPA Build System
include(${CMAKE_SOURCE_DIR}/rxpa/RXPluginFunction.cmake)

# Create dynamic plugin module
add_dynamic_plugin_target(_rxuuid rxuuid.c)

# Build Test Rexx
add_custom_command(
        COMMAND ${CMAKE_BINARY_DIR}/compiler/rxc
        -i \"${CMAKE_BINARY_DIR}/lib/rxfnsb\;${CMAKE_CURRENT_BINARY_DIR}\"
        -o uuid_test \"${CMAKE_CURRENT_SOURCE_DIR}/uuid_test\" &&
        ${CMAKE_BINARY_DIR}/assembler/rxas uuid_test

        DEPENDS rxas rxc library _rxuuid ${CMAKE_CURRENT_SOURCE_DIR}/uuid_test.rexx
        OUTPUT uuid_test.rxbin
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
add_custom_target(uuid_test ALL
        DEPENDS uuid_test.rxbin
        )

# Enable testing functionality
enable_testing()

# Basic test to run uuid_test.rxbin with the rxuuid plugin
add_test(NAME uuid_test
        COMMAND ${CMAKE_BINARY_DIR}/interpreter/rxvm uuid_test rx_rxuuid ${CMAKE_BINARY_DIR}/lib/rxfnsb/library
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )

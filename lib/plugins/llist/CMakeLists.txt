cmake_minimum_required(VERSION 3.24)
project(llist_plugin C)

set(CMAKE_C_STANDARD 90)

# Including RXPA Build System
include(${CMAKE_SOURCE_DIR}/rxpa/RXPluginFunction.cmake)

# Create dynamic plugin module
add_dynamic_plugin_target(_llist llist.c)

# Create the static link module
add_static_plugin_target(_llist llist.c)

# Build Test Rexx
add_custom_command(
        COMMAND ${CMAKE_BINARY_DIR}/compiler/rxc
        -i \"${CMAKE_BINARY_DIR}/lib/rxfnsb\;${CMAKE_CURRENT_BINARY_DIR}\"
        -o llist_test \"${CMAKE_CURRENT_SOURCE_DIR}/llist_test\" &&
        ${CMAKE_BINARY_DIR}/assembler/rxas llist_test

        DEPENDS rxas rxc library _llist ${CMAKE_CURRENT_SOURCE_DIR}/llist_test.rexx
        OUTPUT llist_test.rxbin
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
add_custom_target(llist_test ALL
        DEPENDS llist_test.rxbin
        )

# Enable testing functionality
enable_testing()

# Basic test to run llist_test.rxbin with the llist plugin
add_test(NAME llist_test
        COMMAND ${CMAKE_BINARY_DIR}/interpreter/rxvm llist_test rx_llist ${CMAKE_BINARY_DIR}/lib/rxfnsb/library
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )

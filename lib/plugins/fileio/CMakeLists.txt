cmake_minimum_required(VERSION 3.24)
project(fileio_plugin C)

set(CMAKE_C_STANDARD 90)

# Including RXPA Build System
include(${CMAKE_SOURCE_DIR}/rxpa/RXPluginFunction.cmake)

# Create dynamic plugin module
add_dynamic_plugin_target(_fileio fileio.c)

# Build Test Rexx
add_custom_command(
        COMMAND ${CMAKE_BINARY_DIR}/compiler/rxc
        -i \"${CMAKE_BINARY_DIR}/lib/rxfnsb\;${CMAKE_CURRENT_BINARY_DIR}\"
        -o fileio_test \"${CMAKE_CURRENT_SOURCE_DIR}/fileio_test\" &&
        ${CMAKE_BINARY_DIR}/assembler/rxas fileio_test

        DEPENDS rxas rxc library _fileio ${CMAKE_CURRENT_SOURCE_DIR}/fileio_test.rexx
        OUTPUT fileio_test.rxbin
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
add_custom_target(fileio_test ALL
        DEPENDS fileio_test.rxbin
        )

# Enable testing functionality
enable_testing()

# Basic test to run fileio_test.rxbin with the fileio plugin
add_test(NAME fileio_test
        COMMAND ${CMAKE_BINARY_DIR}/interpreter/rxvm fileio_test rx_fileio ${CMAKE_BINARY_DIR}/lib/rxfnsb/library
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )

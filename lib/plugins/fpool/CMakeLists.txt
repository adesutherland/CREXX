cmake_minimum_required(VERSION 3.24)
project(fpool_plugin C)

set(CMAKE_C_STANDARD 90)

# Including RXPA Build System
include(${CMAKE_SOURCE_DIR}/rxpa/RXPluginFunction.cmake)

# Create dynamic plugin module
add_dynamic_plugin_target(_fpool fpool.c)

# Build Test Rexx
add_custom_command(
        COMMAND $<TARGET_FILE:rxc>
            -i \"${CMAKE_BINARY_DIR}/bin\;${CMAKE_CURRENT_BINARY_DIR}\"
            -o fpool_test \"${CMAKE_CURRENT_SOURCE_DIR}/fpool_test\" &&
        $<TARGET_FILE:rxas> fpool_test

        DEPENDS rxas rxc library _fpool ${CMAKE_CURRENT_SOURCE_DIR}/fpool_test.rexx
        OUTPUT fpool_test.rxbin
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
add_custom_target(fpool_test ALL
        DEPENDS fpool_test.rxbin
)

# Enable testing functionality
enable_testing()

# Basic test to run fpool_test.rxbin with the fpool plugin
add_test(NAME fpool_test
        COMMAND $<TARGET_FILE:rxvm> fpool_test rx_fpool ${CMAKE_BINARY_DIR}/bin/library
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

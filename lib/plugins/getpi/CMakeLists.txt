cmake_minimum_required(VERSION 3.24)
project(getpi_plugin C)

set(CMAKE_C_STANDARD 90)

# Including RXPA Build System
include(${CMAKE_SOURCE_DIR}/rxpa/RXPluginFunction.cmake)

# Create dynamic plugin module
add_dynamic_plugin_target(_getpi getpi.c)

# Build Test Rexx
add_custom_command(
        COMMAND $<TARGET_FILE:rxc>
            -i \"${CMAKE_BINARY_DIR}/bin\;${CMAKE_CURRENT_BINARY_DIR}\"
            -o getpi_test \"${CMAKE_CURRENT_SOURCE_DIR}/getpi_test\" &&
        $<TARGET_FILE:rxas> getpi_test

        DEPENDS rxas rxc library _getpi ${CMAKE_CURRENT_SOURCE_DIR}/getpi_test.rexx
        OUTPUT getpi_test.rxbin
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
add_custom_target(getpi_test ALL
        DEPENDS getpi_test.rxbin
)

# Enable testing functionality
enable_testing()

# Basic test to run getpi_test.rxbin with the getpi plugin
add_test(NAME getpi_test
        COMMAND $<TARGET_FILE:rxvm> getpi_test rx_getpi ${CMAKE_BINARY_DIR}/bin/library
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

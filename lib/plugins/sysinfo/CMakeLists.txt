cmake_minimum_required(VERSION 3.24)
project(sysinfo_plugin C)

set(CMAKE_C_STANDARD 90)

# Including RXPA Build System
include(${CMAKE_SOURCE_DIR}/rxpa/RXPluginFunction.cmake)

# Create dynamic plugin module
add_dynamic_plugin_target(_sysinfo sysinfo.c)

# Build Test Rexx
add_custom_command(
        COMMAND ${CMAKE_BINARY_DIR}/compiler/rxc
            -i \"${CMAKE_BINARY_DIR}/lib/rxfns\;${CMAKE_CURRENT_BINARY_DIR}\"
            -o sysinfo_test \"${CMAKE_CURRENT_SOURCE_DIR}/sysinfo_test\" &&
        ${CMAKE_BINARY_DIR}/assembler/rxas sysinfo_test

        DEPENDS rxas rxc library _sysinfo ${CMAKE_CURRENT_SOURCE_DIR}/sysinfo_test.rexx
        OUTPUT sysinfo_test.rxbin
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
add_custom_target(sysinfo_test ALL
        DEPENDS sysinfo_test.rxbin
)

# Enable testing functionality
enable_testing()

# Basic test to run sysinfo_test.rxbin with the sysinfo plugin
add_test(NAME sysinfo_test
        COMMAND ${CMAKE_BINARY_DIR}/interpreter/rxvm sysinfo_test rx_sysinfo ${CMAKE_BINARY_DIR}/lib/rxfns/library
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

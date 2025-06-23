cmake_minimum_required(VERSION 3.24)
project(rxpatest C)

set(CMAKE_C_STANDARD 90)

# Including RXPlugin Build System
include(${CMAKE_SOURCE_DIR}/rxpa/RXPluginFunction.cmake)

# Create dynamic plugin 1
add_dynamic_plugin_target(_plugin1 plugin1.c)
add_dependencies(_plugin1 rxpa library rxvm rxc) # The test (not plugin) depends on rxpa, library, rxvm, and rxc

# Create dynamic plugin 2
add_dynamic_plugin_target(_plugin2 plugin2.c)
add_dependencies(_plugin2 rxpa library rxvm rxc) # The test (not plugin) depends on rxpa, library, rxvm, and rxc

# Build Dynamic Test Rexx
add_custom_command(
        COMMAND ${CMAKE_BINARY_DIR}/compiler/rxc -i \"${CMAKE_BINARY_DIR}/lib/rxfnsb\;${CMAKE_CURRENT_BINARY_DIR}\" -o rxpa_mult_test \"${CMAKE_CURRENT_SOURCE_DIR}/rxpa_mult_test\" && ${CMAKE_BINARY_DIR}/assembler/rxas rxpa_mult_test
        DEPENDS rxas rxc library _plugin1 _plugin2 ${CMAKE_CURRENT_SOURCE_DIR}/rxpa_mult_test.rexx
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/rxpa_mult_test.rxbin ${CMAKE_CURRENT_BINARY_DIR}/rxpa_mult_test.rxas
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
add_custom_target(rxpa_mult_test ALL
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/rxpa_mult_test.rxbin
)

# Enable testing functionality
enable_testing()

# Multiplugin test
add_test(NAME rxpa_mult_test
        COMMAND ${CMAKE_BINARY_DIR}/interpreter/rxvm rxpa_mult_test rx_plugin1 rx_plugin2 ${CMAKE_BINARY_DIR}/lib/rxfnsb/library
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# Set the property to check for expected output
set_tests_properties(rxpa_mult_test PROPERTIES
        PASS_REGULAR_EXPRESSION "plugin1[^\\1]*plugin2"
)

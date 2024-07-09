cmake_minimum_required(VERSION 3.5)
project(rxdes C)

set(CMAKE_C_STANDARD 90)

# Including RXPlugin Build System
include(${CMAKE_SOURCE_DIR}/rxpa/RXPluginFunction.cmake)

# Create module
add_plugin_target(des rxdes.c desbase.c desbase.h)

# Test for desbase.c
add_executable(desbase_test desbase_test.c desbase.c desbase.h)

# Enable testing functionality
enable_testing()

# Add desbase_test as a test
add_test(NAME DESBaseTest COMMAND desbase_test)

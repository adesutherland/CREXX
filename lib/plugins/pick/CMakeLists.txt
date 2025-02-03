cmake_minimum_required(VERSION 3.24)
project(pick_plugin C)

set(CMAKE_C_STANDARD 99)

# Find GTK using pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

# Include GTK headers
include_directories(${GTK3_INCLUDE_DIRS})
link_directories(${GTK3_LIBRARY_DIRS})

# Add compile options
add_compile_options(${GTK3_CFLAGS_OTHER})

# Including RXPA Build System
include(${CMAKE_SOURCE_DIR}/rxpa/RXPluginFunction.cmake)

# Create dynamic plugin module
add_dynamic_plugin_target(_pick pick.c)

# Link GTK3 to the pick plugin
target_link_libraries(_pick PRIVATE ${GTK3_LIBRARIES})

# Build Test Rexx
add_custom_command(
        COMMAND ${CMAKE_BINARY_DIR}/compiler/rxc
        -i \"${CMAKE_BINARY_DIR}/lib/rxfnsb\;${CMAKE_CURRENT_BINARY_DIR}\"
        -o pick_test \"${CMAKE_CURRENT_SOURCE_DIR}/pick_test\" &&
        ${CMAKE_BINARY_DIR}/assembler/rxas pick_test

        DEPENDS rxas rxc library _pick ${CMAKE_CURRENT_SOURCE_DIR}/pick_test.rexx
        OUTPUT pick_test.rxbin
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
add_custom_target(pick_test ALL
        DEPENDS pick_test.rxbin
        )

# Enable testing functionality
enable_testing()

# Basic test to run pick_test.rxbin with the pick plugin
add_test(NAME pick_test
        COMMAND ${CMAKE_BINARY_DIR}/interpreter/rxvm pick_test rx_pick ${CMAKE_BINARY_DIR}/lib/rxfnsb/library
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )

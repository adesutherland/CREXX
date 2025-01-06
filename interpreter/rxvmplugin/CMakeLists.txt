cmake_minimum_required(VERSION 3.24)

project(RXVMPlugin C)

set(CMAKE_C_STANDARD 90)

# Add subdirectories first if needed
add_subdirectory(rxvmplugins)

# Add the rxvmplugin support library
add_library(rxvmplugin rxvmplugin_framework.c rxvmplugin.h rxvmplugin_framework.h)
target_include_directories(rxvmplugin PRIVATE ..)

# Enable testing
enable_testing()

# Decimal RXAS Tests
add_custom_command(
        OUTPUT decimaltests.rxbin
        COMMAND ${CMAKE_BINARY_DIR}/assembler/rxas -o decimaltests ${CMAKE_CURRENT_SOURCE_DIR}/decimaltests
        DEPENDS decimaltests.rxas rxas rxvm mc_decimal_dynamic db_decimal_manual
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "cREXX decimal Plugin Tests ..."
)

# Create a custom target that depends on the custom command's output
add_custom_target(run_decimaltests ALL DEPENDS decimaltests.rxbin)

# Add the test to be executed after the custom command
add_test(
        NAME rxasdecimaltests
        COMMAND ${CMAKE_BINARY_DIR}/interpreter/rxvm decimaltests
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

add_test(
        NAME rxasdecimaltests-mc
        COMMAND ${CMAKE_BINARY_DIR}/interpreter/rxvm -p ./rxvmplugins/mc_decimal/rxvm_mc_decimal decimaltests
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)



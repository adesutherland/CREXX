cmake_minimum_required(VERSION 3.10)

project(DecimalPlugins C)

set(CMAKE_C_STANDARD 90)

# Including Decimal Build System
include(${CMAKE_SOURCE_DIR}/decimal/DecPluginFunction.cmake)

# Add the decimal support library
add_library(decplugin decplugin.c decplugin.h decplugin_framework.h)
target_include_directories(decplugin PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../interpreter)

# Add the decnumber dynamic plugin library
add_dynamic_decplugin_target(mc_decimal_dynamic mc_decimal.c decplugin.h)
add_dependencies(mc_decimal_dynamic decplugin rxvm) # Note: For build safety we want rxvm to be built first
target_include_directories(mc_decimal_dynamic PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(mc_decimal_dynamic PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../decnumber)
target_include_directories(mc_decimal_dynamic PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../interpreter)
target_link_libraries(mc_decimal_dynamic decnumber)
add_dependencies(mc_decimal_dynamic decnumber)

# Add the decnumber static plugin library
add_static_decplugin_target(mc_decimal mc_decimal.c decplugin.h)
add_dependencies(mc_decimal decplugin rxvm) # Note: For build safety we want rxvm to be built first
target_include_directories(mc_decimal PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(mc_decimal PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../decnumber)
target_include_directories(mc_decimal PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../interpreter)
target_link_libraries(mc_decimal decnumber)
add_dependencies(mc_decimal decnumber)

# Add the static test1 executable
add_executable(mc_decimal_test mc_decimal_test.c)
target_include_directories(mc_decimal_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../interpreter)
# Ensure the linker includes the plugin
configure_linker_for_static_decplugin(mc_decimal_test mc_decimal)
target_link_libraries(mc_decimal_test decplugin mc_decimal)

# Add the decnumber dynamic plugin library
add_dynamic_decplugin_target(mc_decimal_dyn mc_decimal.c decplugin.h)
# SET BUILD_DLL to build a DLL
target_compile_definitions(mc_decimal_dyn PRIVATE BUILD_DLL)
add_dependencies(mc_decimal_dyn decplugin rxvm) # Note: For build safety we want rxvm to be built first
target_include_directories(mc_decimal_dyn PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(mc_decimal_dyn PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../decnumber)
target_include_directories(mc_decimal_dyn PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../interpreter)
target_link_libraries(mc_decimal_dyn decnumber)
add_dependencies(mc_decimal_dyn decnumber)

# Add the dynamic test1 executable
add_executable(mc_decimal_dyn_test mc_decimal_test.c)
# Add a DYNAMIC define to pass to the compile
target_compile_definitions(mc_decimal_dyn_test PRIVATE DYNAMIC)
target_include_directories(mc_decimal_dyn_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../interpreter)
target_link_libraries(mc_decimal_dyn_test decplugin mc_decimal)

# Enable Testing
enable_testing()
# Add the first test - 32 digits, with arguments 1, 2 and check that the output are:
# ADD 3
# SUB -1
# MUL 2
# DIV 0.5
add_test(mc_decimal_test1 mc_decimal_test 32 1 2)
set_tests_properties(mc_decimal_test1 PROPERTIES PASS_REGULAR_EXPRESSION
        "ADD:3:.*SUB:-1:.*MUL:2:.*DIV:0\\.5:")

# Add the second test - 32 digits, with 32 digit arguments and check output
add_test(mc_decimal_test2 mc_decimal_test 32 10000000000000000000000000000000 20000000000000000000000000000000)
set_tests_properties(mc_decimal_test2 PROPERTIES PASS_REGULAR_EXPRESSION
        "ADD:3E\\+31:.*SUB:-1E\\+31:.*MUL:2E\\+62:.*DIV:0\\.5:")

# Add the third test - 100 significant digits, with 100 digit results
add_test(mc_decimal_test3 mc_decimal_test 100
        12345678901234567890123456789012345678901234567890
        98765432109876543210987654321098765432109876543210)
# Set the PASS_REGULAR_EXPRESSION with the new patterns
# Note that this test checks the first few most significant digits and the last few least significant digits - good enough
# I tested the result with a calculator and it is correct - http://www.apfloat.org/calculator/
set_tests_properties(mc_decimal_test3 PROPERTIES PASS_REGULAR_EXPRESSION
        "ADD:1\\.1111[0-9]*1111E\\+50:.*SUB:-8\\.6419[0-9]*7532E\\+49:.*MUL:1\\.2193[0-9]*5269E\\+99:.*DIV:0\\.1249[0-9]*8958:")

add_test(mc_decimal_dyn_test1 mc_decimal_dyn_test 32 1 2)
set_tests_properties(mc_decimal_dyn_test1 PROPERTIES PASS_REGULAR_EXPRESSION
        "ADD:3:.*SUB:-1:.*MUL:2:.*DIV:0\\.5:")

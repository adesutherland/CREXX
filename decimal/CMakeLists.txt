cmake_minimum_required(VERSION 3.10)

project(DecimalPlugins C)

set(CMAKE_C_STANDARD 90)

add_library(icudecimal icudecimal.c decplugin.h)
target_include_directories(icudecimal PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(icudecimal PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../decnumber)
target_include_directories(icudecimal PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../interpreter)
target_link_libraries(icudecimal decnumber)
add_dependencies(icudecimal decnumber)

# Add the test1 executable
add_executable(icudecimal_test icudecimal_test.c)
target_include_directories(icudecimal_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../interpreter)
target_link_libraries(icudecimal_test icudecimal)

# Enable Testing
enable_testing()
# Add the first test - 32 digits, with arguments 1, 2 and check that the output are:
# ADD 3
# SUB -1
# MUL 2
# DIV 0.5
add_test(icudecimal_test1 icudecimal_test 32 1 2)
set_tests_properties(icudecimal_test1 PROPERTIES PASS_REGULAR_EXPRESSION
        "ADD:3:.*SUB:-1:.*MUL:2:.*DIV:0\\.5:")

# Add the second test - 32 digits, with 32 digit arguments and check output
add_test(icudecimal_test2 icudecimal_test 32 10000000000000000000000000000000 20000000000000000000000000000000)
set_tests_properties(icudecimal_test2 PROPERTIES PASS_REGULAR_EXPRESSION
        "ADD:3E\\+31:.*SUB:-1E\\+31:.*MUL:2E\\+62:.*DIV:0\\.5:")

# Add the third test - 100 significant digits, with 100 digit results
add_test(icudecimal_test3 icudecimal_test 100
        12345678901234567890123456789012345678901234567890
        98765432109876543210987654321098765432109876543210)
# Set the PASS_REGULAR_EXPRESSION with the new patterns
# Note that this test checks the first few most significant digits and the last few least significant digits - good enough
# I tested the result with a calculator and it is correct - http://www.apfloat.org/calculator/
set_tests_properties(icudecimal_test3 PROPERTIES PASS_REGULAR_EXPRESSION
        "ADD:1\\.1111[0-9]*1111E\\+50:.*SUB:-8\\.6419[0-9]*7532E\\+49:.*MUL:1\\.2193[0-9]*5269E\\+99:.*DIV:0\\.1249[0-9]*8958:")


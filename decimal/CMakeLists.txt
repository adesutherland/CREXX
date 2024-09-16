cmake_minimum_required(VERSION 3.10)

project(DecimalPlugins C)

set(CMAKE_C_STANDARD 90)

add_library(icudecimal icudecimal.c decplugin.h)
target_include_directories(icudecimal PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(icudecimal PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../icu)
target_include_directories(icudecimal PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../interpreter)
target_link_libraries(icudecimal icu)
add_dependencies(icudecimal icu)

# Add the test1 executable
add_executable(icudecimal_test icudecimal_test.c)
target_include_directories(icudecimal_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../interpreter)
target_link_libraries(icudecimal_test icudecimal)

# Enable Testing
enable_testing()
# Add the first test - 32 digits, with arguments 1, 2 and check that the output are:
# ADD 3
# SUB 1
# MUL 2
# DIV 0.5
add_test(icudecimal_test1 icudecimal_test 32 1 2)
set_tests_properties(icudecimal_test1 PROPERTIES PASS_REGULAR_EXPRESSION "ADD 3")
set_tests_properties(icudecimal_test1 PROPERTIES PASS_REGULAR_EXPRESSION "SUB 1")
set_tests_properties(icudecimal_test1 PROPERTIES PASS_REGULAR_EXPRESSION "MUL 2")
set_tests_properties(icudecimal_test1 PROPERTIES PASS_REGULAR_EXPRESSION "DIV 0.5")

# Add the second test - 32 digits, with 32 digit arguments and check output
# ADD 30000000000000000000000000000000
# SUB -10000000000000000000000000000000
# MUL 2.0000000000000000000000000000000E+62
# DIV 0.5
add_test(icudecimal_test2 icudecimal_test 32 10000000000000000000000000000000 20000000000000000000000000000000)
set_tests_properties(icudecimal_test2 PROPERTIES PASS_REGULAR_EXPRESSION "ADD 30000000000000000000000000000000")
set_tests_properties(icudecimal_test2 PROPERTIES PASS_REGULAR_EXPRESSION "SUB -10000000000000000000000000000000")
set_tests_properties(icudecimal_test2 PROPERTIES PASS_REGULAR_EXPRESSION "MUL 2.0000000000000000000000000000000E+62")
set_tests_properties(icudecimal_test2 PROPERTIES PASS_REGULAR_EXPRESSION "DIV 0.5")

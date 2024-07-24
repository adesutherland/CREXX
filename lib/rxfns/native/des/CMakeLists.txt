cmake_minimum_required(VERSION 3.5)
project(rxdes C)

set(CMAKE_C_STANDARD 90)

# Including RXPlugin Build System
include(${CMAKE_SOURCE_DIR}/rxpa/RXPluginFunction.cmake)

# Create module (dynamic)
add_dynamic_plugin_target(des rxdes.c desbase.c desbase.h)

# Test for desbase.c
add_executable(desbase_test desbase_test.c desbase.c desbase.h)

# Rexx Test
add_custom_target(DESRexxTest ALL
        BYPRODUCTS RxDesTst.rxbin
        COMMAND ${CMAKE_BINARY_DIR}/compiler/rxc -i \"${CMAKE_BINARY_DIR}/lib/rxfns\;${CMAKE_CURRENT_BINARY_DIR}\" -o RxDesTst ${CMAKE_CURRENT_SOURCE_DIR}/RxDesTst
        COMMAND ${CMAKE_BINARY_DIR}/assembler/rxas RxDesTst
        DEPENDS rxas rxc library RxDesTst.rexx
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# Rexx Error Test
add_custom_target(DESRexxInvalidTest ALL
        BYPRODUCTS RxDesTstInvalid.rxbin
        COMMAND ${CMAKE_BINARY_DIR}/compiler/rxc -i \"${CMAKE_BINARY_DIR}/lib/rxfns\;${CMAKE_CURRENT_BINARY_DIR}\" -o RxDesTstInvalid ${CMAKE_CURRENT_SOURCE_DIR}/RxDesTstInvalid
        COMMAND ${CMAKE_BINARY_DIR}/assembler/rxas RxDesTstInvalid
        DEPENDS rxas rxc library RxDesTst.rexx
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# Enable testing functionality
enable_testing()

# Add desbase_test as a test
add_test(NAME DESBaseTest COMMAND desbase_test)

# Test with Rexx
add_test(NAME DESRexxTest
        COMMAND ${CMAKE_BINARY_DIR}/interpreter/rxvm RxDesTst rxdes ${CMAKE_BINARY_DIR}/lib/rxfns/library
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# Test with Rexx with invalid input
add_test(NAME DESRexxInvalidTest
        COMMAND ${CMAKE_BINARY_DIR}/interpreter/rxvm RxDesTstInvalid rxdes ${CMAKE_BINARY_DIR}/lib/rxfns/library
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
# Check for "Signal 10" in the output
set_tests_properties(DESRexxInvalidTest PROPERTIES PASS_REGULAR_EXPRESSION ".*Signal 10.*")

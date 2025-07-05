cmake_minimum_required(VERSION 3.24)
project(sigtesttool C)
set(CMAKE_C_STANDARD 99) # enables inline keyword

# Set the binary tool binary output directory to /tools
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tools)
if(WIN32)
    # On Windows, we use sigtest-windows.c
    add_executable(signal_tester sigtest-windows.c)
else()
    # On POSIX systems, we use sigtest-posix.c
    add_executable(signal_tester sigtest-posix.c)
endif()

# Build the rxas test
add_custom_command(
        OUTPUT intr_test.rxbin
        COMMAND ${CMAKE_BINARY_DIR}/assembler/rxas -o intr_test ${CMAKE_CURRENT_SOURCE_DIR}/intr_test
        DEPENDS intr_test.rxas rxas
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "cREXX RXAS intr_test - Native Signal Test Harness ..."
)

# Create a custom target that depends on the custom command's output
add_custom_target(intr_test ALL DEPENDS intr_test.rxbin rxvm signal_tester)

# Add the native signal tests
add_test(
        NAME QUIT_SIGNAL
        COMMAND signal_tester QUIT "QUIT Signal" 5 ${CMAKE_BINARY_DIR}/interpreter/rxvm intr_test
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
add_test(
        NAME TERM_SIGNAL
        COMMAND signal_tester TERM "TERM Signal" 5 ${CMAKE_BINARY_DIR}/interpreter/rxvm intr_test
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
add_test(
        NAME INT_SIGNAL
        COMMAND signal_tester INT "POSIX_INT Signal" 5 ${CMAKE_BINARY_DIR}/interpreter/rxvm intr_test
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

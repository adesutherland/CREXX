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
        COMMAND $<TARGET_FILE:rxas> -o intr_test ${CMAKE_CURRENT_SOURCE_DIR}/intr_test
        DEPENDS intr_test.rxas rxas
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "cREXX RXAS intr_test - Native Signal Test Harness ..."
)

# Create a custom target that depends on the custom command's output
add_custom_target(intr_test ALL DEPENDS intr_test.rxbin rxvm rxbvm signal_tester)

function(crexx_add_native_signal_test SIGNAL_NAME SIGNAL_DESC)
    add_test(
            NAME ${SIGNAL_NAME}
            COMMAND signal_tester ${SIGNAL_NAME} "${SIGNAL_DESC}" 5 $<TARGET_FILE:rxvm> intr_test
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
    add_test(
            NAME ${SIGNAL_NAME}_BYTECODE
            COMMAND signal_tester ${SIGNAL_NAME} "${SIGNAL_DESC}" 5 $<TARGET_FILE:rxbvm> intr_test
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
endfunction()

crexx_add_native_signal_test(QUIT "QUIT Signal")
crexx_add_native_signal_test(TERM "TERM Signal")

# On Windows the harness uses GenerateConsoleCtrlEvent() to emulate native interrupts.
# CTRL_C_EVENT cannot be targeted to a specific process group, so in CI it can hit the
# harness/runner console rather than just the child under test. Keep TERM/QUIT coverage
# on Windows and only run the INT test on POSIX where the delivery semantics are reliable.
if(WIN32)
    message(STATUS "Skipping INT_SIGNAL native signal test on Windows")
else()
    crexx_add_native_signal_test(INT "POSIX_INT Signal")
endif()

set(_native_signal_tests QUIT TERM QUIT_BYTECODE TERM_BYTECODE)
if(NOT WIN32)
    list(APPEND _native_signal_tests INT INT_BYTECODE)
endif()

set_tests_properties(${_native_signal_tests} PROPERTIES
        LABELS "native_signal"
        RUN_SERIAL TRUE
        TIMEOUT 15
)

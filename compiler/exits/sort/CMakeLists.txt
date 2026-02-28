cmake_minimum_required(VERSION 3.24)

# Build Sort.rxbin
# Needs token.rxbin for class metadata.
set(LIBRPATH ${CMAKE_BINARY_DIR}/bin/library)

add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Sort.rxbin
        COMMAND ${CMAKE_COMMAND} -E env RXCP_DISABLE_EXIT=1 $<TARGET_FILE:rxc> -x -n -i "\"${CMAKE_CURRENT_SOURCE_DIR}/..;${CMAKE_CURRENT_BINARY_DIR}/..\"" -o Sort ${CMAKE_CURRENT_SOURCE_DIR}/Sort.rexx
        COMMAND $<TARGET_FILE:rxas> -o Sort Sort
        DEPENDS rxc rxas library ${CMAKE_CURRENT_SOURCE_DIR}/Sort.rexx ${CMAKE_CURRENT_BINARY_DIR}/../token.rxbin
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Building Sort.rxbin"
)
add_custom_target(exit_sort_bin DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/Sort.rxbin)

# Test the sort exit
add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/test_sort.rxas
        COMMAND ${CMAKE_COMMAND} -E env RXCP_EXIT_MODULE=rxcexits $<TARGET_FILE:rxc> -i ${CMAKE_BINARY_DIR}/bin -o test_sort ${CMAKE_CURRENT_SOURCE_DIR}/test_sort.rexx
        DEPENDS rxc ${CMAKE_CURRENT_SOURCE_DIR}/test_sort.rexx compiler_exit_bin
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Compiling test_sort.rexx with exit"
)
execute_process(
        COMMAND cmd /c "echo Debug breakpoint reached & pause"
)
add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/test_sort.rxbin
        COMMAND $<TARGET_FILE:rxas> -o test_sort test_sort
        DEPENDS rxas ${CMAKE_CURRENT_BINARY_DIR}/test_sort.rxas
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Assembling test_sort.rxas"
)
add_custom_target(test_sort_bin ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/test_sort.rxbin)

add_test(NAME test_sort COMMAND rxvm test_sort.rxbin ${LIBRPATH} WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

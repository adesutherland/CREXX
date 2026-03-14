cmake_minimum_required(VERSION 3.24)

# Build fsay.rxbin
# Needs token.rxbin for class metadata.
set(LIBRPATH ${CMAKE_BINARY_DIR}/bin/library)

add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Fsay.rxbin
        COMMAND ${CMAKE_COMMAND} -E env RXCP_DISABLE_EXIT=1 $<TARGET_FILE:rxc> -x -n -i "\"${CMAKE_CURRENT_SOURCE_DIR}/..;${CMAKE_CURRENT_BINARY_DIR}/..\"" -o Fsay ${CMAKE_CURRENT_SOURCE_DIR}/Fsay.rexx
        COMMAND $<TARGET_FILE:rxas> -o Fsay Fsay
        DEPENDS rxc rxas library ${CMAKE_CURRENT_SOURCE_DIR}/Fsay.rexx ${CMAKE_CURRENT_BINARY_DIR}/../token.rxbin
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Building Fsay.rxbin"
)
add_custom_target(exit_fsay_bin DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/Fsay.rxbin)

# Test the sort exit
add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/test_fsay.rxas
        COMMAND ${CMAKE_COMMAND} -E env RXCP_EXIT_MODULE=rxcexits $<TARGET_FILE:rxc> -i ${CMAKE_BINARY_DIR}/bin -o test_fsay ${CMAKE_CURRENT_SOURCE_DIR}/test_fsay.rexx
        DEPENDS rxc ${CMAKE_CURRENT_SOURCE_DIR}/test_fsay.rexx compiler_exit_bin
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Compiling test_fsay.rexx with exit"
)
execute_process(
        COMMAND cmd /c "echo Debug breakpoint reached & pause"
)
add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/test_fsay.rxbin
        COMMAND $<TARGET_FILE:rxas> -o test_fsay test_fsay
        DEPENDS rxas ${CMAKE_CURRENT_BINARY_DIR}/test_fsay.rxas
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Assembling test_fsay.rxas"
)
add_custom_target(test_fsay_bin ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/test_fsay.rxbin)

add_test(NAME test_fsay COMMAND rxvm test_fsay.rxbin ${LIBRPATH} WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

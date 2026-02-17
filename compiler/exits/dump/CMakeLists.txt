cmake_minimum_required(VERSION 3.24)

# Build Dump.rxbin
# Needs token.rxbin for class metadata.
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Dump.rxbin
    COMMAND ${CMAKE_COMMAND} -E env RXCP_DISABLE_EXIT=1 $<TARGET_FILE:rxc> -n -i ${CMAKE_CURRENT_SOURCE_DIR}/.. -o Dump ${CMAKE_CURRENT_SOURCE_DIR}/Dump.rexx
    COMMAND $<TARGET_FILE:rxas> -o Dump.rxbin Dump.rxas
    DEPENDS rxc rxas ${CMAKE_CURRENT_SOURCE_DIR}/Dump.rexx ${CMAKE_CURRENT_BINARY_DIR}/../token.rxbin
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Building Dump.rxbin"
)
add_custom_target(exit_dump_bin DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/Dump.rxbin)

# Test the dump exit
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/test_dump.rxas
    COMMAND $<TARGET_FILE:rxc> -o test_dump ${CMAKE_CURRENT_SOURCE_DIR}/test_dump.rexx
    DEPENDS rxc ${CMAKE_CURRENT_SOURCE_DIR}/test_dump.rexx compiler_exit_bin
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Compiling test_dump.rexx with exit"
)

add_custom_command(
    OUTPUT test_dump.rxbin
    COMMAND $<TARGET_FILE:rxas> -o test_dump.rxbin test_dump.rxas
    DEPENDS rxas test_dump.rxas
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Assembling test_dump.rxas"
)

add_custom_target(test_dump_bin ALL DEPENDS test_dump.rxbin)

add_test(NAME test_dump COMMAND rxvm test_dump.rxbin WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# Build Repro.rxbin (Reproduction Harness)
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Repro.rxbin
    COMMAND ${CMAKE_COMMAND} -E env RXCP_DISABLE_EXIT=1 $<TARGET_FILE:rxc> -n -i ${CMAKE_CURRENT_SOURCE_DIR}/.. -o Repro ${CMAKE_CURRENT_SOURCE_DIR}/Repro.rexx
    COMMAND $<TARGET_FILE:rxas> -o Repro.rxbin Repro.rxas
    DEPENDS rxc rxas ${CMAKE_CURRENT_SOURCE_DIR}/Repro.rexx
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Building Repro.rxbin"
)
add_custom_target(repro_rxbin DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/Repro.rxbin)

# Build repro_issue (Reproduction Harness Executable)
add_executable(repro_issue repro_issue.c)
target_link_libraries(repro_issue PRIVATE rxvml rxvmplugin platform rxpa)
target_include_directories(repro_issue PRIVATE
    ${CMAKE_SOURCE_DIR}/interpreter
    ${CMAKE_SOURCE_DIR}/inc
    ${CMAKE_SOURCE_DIR}/binutils/include
)
add_dependencies(repro_issue repro_rxbin)

# Test repro_issue
add_test(NAME repro_issue COMMAND repro_issue WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

cmake_minimum_required(VERSION 3.24)

# Build substr.rxbin
# Needs token.rxbin for class metadata.
set(LIBRPATH ${CMAKE_BINARY_DIR}/bin/library)

add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Substr.rxbin
        COMMAND ${CMAKE_COMMAND} -E env RXCP_DISABLE_EXIT=1 $<TARGET_FILE:rxc> -x -i ${CREXX_EXIT_IMPORT_PATH} -o Substr ${CMAKE_CURRENT_SOURCE_DIR}/Substr.crexx
        COMMAND $<TARGET_FILE:rxas> -o Substr Substr
        DEPENDS rxc rxas library ${CMAKE_CURRENT_SOURCE_DIR}/Substr.crexx ${CMAKE_CURRENT_BINARY_DIR}/../token.rxbin
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Building Substr.rxbin"
)
add_custom_target(exit_substr_bin DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/Substr.rxbin)

crexx_add_compiler_exit_runtime_test(
        test_substr
        test_substr.crexx
        RUN_SUFFIX ${LIBRPATH}
)

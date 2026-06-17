cmake_minimum_required(VERSION 3.24)

set(LIBRPATH ${CMAKE_BINARY_DIR}/bin/library)

add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/RexxScript.rxbin
        COMMAND ${CMAKE_COMMAND} -E env RXCP_DISABLE_EXIT=1 $<TARGET_FILE:rxc> -x -i ${CREXX_EXIT_IMPORT_PATH} -o RexxScript ${CMAKE_CURRENT_SOURCE_DIR}/RexxScript.crexx
        COMMAND $<TARGET_FILE:rxas> -o RexxScript RexxScript
        DEPENDS rxc rxas library ${CMAKE_CURRENT_SOURCE_DIR}/RexxScript.crexx ${CMAKE_CURRENT_BINARY_DIR}/../token.rxbin
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Building RexxScript.rxbin"
)
add_custom_target(exit_rexxscript_bin DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/RexxScript.rxbin)

crexx_add_compiler_exit_runtime_test(
        test_rexxscript
        test_rexxscript.crexx
        RUN_SUFFIX ${LIBRPATH}
)

crexx_add_compiler_exit_runtime_test(
        test_rexxscript_direct
        test_rexxscript_direct.crexx
        EXTRA_DEPENDS exit_test_support_bin
        RUN_SUFFIX ${CMAKE_BINARY_DIR}/bin/exittestsupport.rxbin ${CMAKE_BINARY_DIR}/bin/rxcexits.rxbin ${LIBRPATH}
)

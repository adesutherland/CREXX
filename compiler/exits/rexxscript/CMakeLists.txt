cmake_minimum_required(VERSION 3.24)

set(LIBRPATH ${CMAKE_BINARY_DIR}/bin/library)
set(CLASSLIBPATH ${CMAKE_BINARY_DIR}/bin/classlib)
set(CLASSLIB_RXBIN ${CLASSLIBPATH}.rxbin)
set(TREEMAPPATH ${CMAKE_BINARY_DIR}/bin/rx_treemap)
set(RXFNSCPATH ${CMAKE_BINARY_DIR}/bin/rxfnsc)
set(RXFNSC_RXBIN ${RXFNSCPATH}.rxbin)
set(REXXSCRIPTPATH ${CMAKE_BINARY_DIR}/bin/rexxscript)
set(REXXSCRIPT_RXBIN ${REXXSCRIPTPATH}.rxbin)

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
        EXTRA_DEPENDS _treemap ${CLASSLIB_RXBIN} ${RXFNSC_RXBIN} ${REXXSCRIPT_RXBIN}
        RUN_SUFFIX ${TREEMAPPATH} ${LIBRPATH} ${CLASSLIBPATH} ${RXFNSCPATH} ${REXXSCRIPTPATH}
)

crexx_add_compiler_exit_runtime_test(
        test_rexxscript_direct
        test_rexxscript_direct.crexx
        EXTRA_DEPENDS exit_test_support_bin _treemap ${CLASSLIB_RXBIN} ${RXFNSC_RXBIN} ${REXXSCRIPT_RXBIN}
        RUN_SUFFIX ${CMAKE_BINARY_DIR}/bin/exittestsupport.rxbin ${CMAKE_BINARY_DIR}/bin/rxcexits.rxbin ${TREEMAPPATH} ${LIBRPATH} ${CLASSLIBPATH} ${RXFNSCPATH} ${REXXSCRIPTPATH}
)

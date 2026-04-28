cmake_minimum_required(VERSION 3.24)

set(LIBRPATH ${CMAKE_BINARY_DIR}/bin/library)

add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Signal.rxbin
        COMMAND ${CMAKE_COMMAND} -E env RXCP_DISABLE_EXIT=1 $<TARGET_FILE:rxc> -x -i ${CREXX_EXIT_IMPORT_PATH} -o Signal ${CMAKE_CURRENT_SOURCE_DIR}/Signal.rexx
        COMMAND $<TARGET_FILE:rxas> -o Signal Signal
        DEPENDS rxc rxas library ${CMAKE_CURRENT_SOURCE_DIR}/Signal.rexx ${CMAKE_CURRENT_BINARY_DIR}/../token.rxbin
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Building Signal.rxbin"
)
add_custom_target(exit_signal_bin DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/Signal.rxbin)

crexx_add_compiler_exit_runtime_test(
        test_signal
        test_signal.rexx
        RUN_SUFFIX ${LIBRPATH}
)

crexx_add_compiler_exit_runtime_test(
        test_signal_direct
        test_signal_direct.rexx
        EXTRA_DEPENDS ${CMAKE_BINARY_DIR}/bin/exittestsupport.rxbin
        RUN_SUFFIX ${CMAKE_BINARY_DIR}/bin/exittestsupport.rxbin ${CMAKE_BINARY_DIR}/bin/rxcexits.rxbin ${LIBRPATH}
)

set(SIGNAL_COMPILE_FAIL_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/signal_compile_fail.cmake")
file(WRITE "${SIGNAL_COMPILE_FAIL_SCRIPT}" [=[
execute_process(
        COMMAND "${CMAKE_COMMAND}" -E env RXCP_EXIT_MODULE=rxcexits "${RXC}" -i "${BINDIR}" -o "${OUTPUT_BASE}" "${SOURCE}"
        WORKING_DIRECTORY "${WORKDIR}"
        RESULT_VARIABLE result
        OUTPUT_VARIABLE out
        ERROR_VARIABLE err
)

set(combined "${out}${err}")
if(result EQUAL 0)
    message(FATAL_ERROR "Expected compiler failure for ${SOURCE}, but rxc succeeded.\n${combined}")
endif()

if(NOT combined MATCHES "${EXPECTED_REGEX}")
    message(FATAL_ERROR "Expected diagnostic regex '${EXPECTED_REGEX}' for ${SOURCE}.\n${combined}")
endif()
]=])

function(crexx_add_signal_compile_fail_test TARGET_NAME SOURCE_FILE EXPECTED_REGEX)
    add_test(
            NAME ${TARGET_NAME}
            COMMAND ${CMAKE_COMMAND}
                    -DCMAKE_COMMAND=${CMAKE_COMMAND}
                    -DRXC=$<TARGET_FILE:rxc>
                    -DBINDIR=${CMAKE_BINARY_DIR}/bin
                    -DWORKDIR=${CMAKE_CURRENT_BINARY_DIR}
                    -DOUTPUT_BASE=${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}
                    -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE}
                    -DEXPECTED_REGEX=${EXPECTED_REGEX}
                    -P ${SIGNAL_COMPILE_FAIL_SCRIPT}
    )
endfunction()

crexx_add_signal_compile_fail_test(
        test_signal_bad_arg_diagnostic
        test_signal_bad_arg.rexx
        "Error in test_signal_bad_arg\\.rexx @ [0-9]+:[0-9]+ - #SIGNAL_HANDLER_BAD_SIGNATURE, \"handle_other\", \"handle_other\""
)

crexx_add_signal_compile_fail_test(
        test_signal_bad_return_diagnostic
        test_signal_bad_return.rexx
        "Error in test_signal_bad_return\\.rexx @ [0-9]+:[0-9]+ - #SIGNAL_HANDLER_BAD_SIGNATURE, \"handle_other\", \"handle_other\""
)

crexx_add_signal_compile_fail_test(
        test_signal_no_arg_diagnostic
        test_signal_no_arg.rexx
        "Error in test_signal_no_arg\\.rexx @ [0-9]+:[0-9]+ - #SIGNAL_HANDLER_BAD_SIGNATURE, \"handle_other\", \"handle_other\""
)

crexx_add_signal_compile_fail_test(
        test_signal_missing_handler_diagnostic
        test_signal_missing_handler.rexx
        "Error in test_signal_missing_handler\\.rexx @ [0-9]+:[0-9]+ - #SIGNAL_HANDLER_NOT_FOUND, \"missing_handler\", \"missing_handler\""
)

cmake_minimum_required(VERSION 3.24)

set(LIBRPATH ${CMAKE_BINARY_DIR}/bin/library)

add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Trace.rxbin
        COMMAND ${CMAKE_COMMAND} -E env RXCP_DISABLE_EXIT=1 $<TARGET_FILE:rxc> -x -i ${CREXX_EXIT_IMPORT_PATH} -o Trace ${CMAKE_CURRENT_SOURCE_DIR}/Trace.crexx
        COMMAND $<TARGET_FILE:rxas> -o Trace Trace
        DEPENDS rxc rxas library ${CMAKE_CURRENT_SOURCE_DIR}/Trace.crexx ${CMAKE_CURRENT_BINARY_DIR}/../token.rxbin
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Building Trace.rxbin"
)
add_custom_target(exit_trace_bin DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/Trace.rxbin)

crexx_add_compiler_exit_runtime_test(
        test_trace_exit
        test_trace_exit.crexx
        PASS_REGULAR_EXPRESSION "[ ]*5 \\*-\\*[ ]+say .*TRACE REXX ACTIVE"
        RUN_SUFFIX ${LIBRPATH}
)

crexx_add_compiler_exit_runtime_test(
        test_trace_negated_string
        test_trace_negated_string.crexx
        PASS_REGULAR_EXPRESSION "[ ]*6 \\*-\\*[ ]+return \\\\\\\\flag"
        RUN_SUFFIX ${LIBRPATH}
)

crexx_add_compiler_exit_runtime_test(
        test_trace_results
        test_trace_results.crexx
        PASS_REGULAR_EXPRESSION ">>>[ ]+\"42\""
        RUN_SUFFIX ${LIBRPATH}
)

crexx_add_compiler_exit_runtime_test(
        test_trace_llm
        test_trace_llm.crexx
        PASS_REGULAR_EXPRESSION "\"format\":\"llm\".*\"source\":\"[ ]*return"
        RUN_SUFFIX ${LIBRPATH}
)

crexx_add_compiler_exit_runtime_test(
        test_trace_direct
        test_trace_direct.crexx
        EXTRA_DEPENDS exit_test_support_bin
        RUN_SUFFIX ${CMAKE_BINARY_DIR}/bin/exittestsupport.rxbin ${CMAKE_BINARY_DIR}/bin/rxcexits.rxbin ${LIBRPATH}
)

set(TRACE_CAPTURE_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/trace_capture.cmake")
file(WRITE "${TRACE_CAPTURE_SCRIPT}" [=[
file(MAKE_DIRECTORY "${WORKDIR}")
if(DEFINED TRACE_FILE AND NOT "${TRACE_FILE}" STREQUAL "")
    file(REMOVE "${WORKDIR}/${TRACE_FILE}")
endif()

if(DEFINED ENV_CREXX_TRACE AND NOT "${ENV_CREXX_TRACE}" STREQUAL "")
    set(ENV{CREXX_TRACE} "${ENV_CREXX_TRACE}")
endif()

if(DEFINED ENV_CREXX_TRACE_TO AND NOT "${ENV_CREXX_TRACE_TO}" STREQUAL "")
    set(ENV{CREXX_TRACE_TO} "${ENV_CREXX_TRACE_TO}")
endif()

execute_process(
        COMMAND "${RXVM}" "${RXBIN}" "${LIBRPATH}"
        WORKING_DIRECTORY "${WORKDIR}"
        RESULT_VARIABLE result
        OUTPUT_VARIABLE out
        ERROR_VARIABLE err
)

if(NOT result EQUAL 0)
    message(FATAL_ERROR "rxvm failed with ${result}\nstdout:\n${out}\nstderr:\n${err}")
endif()

if(DEFINED EXPECT_STDOUT_REGEX AND NOT "${EXPECT_STDOUT_REGEX}" STREQUAL "" AND NOT out MATCHES "${EXPECT_STDOUT_REGEX}")
    message(FATAL_ERROR "stdout did not match '${EXPECT_STDOUT_REGEX}'\nstdout:\n${out}\nstderr:\n${err}")
endif()

if(DEFINED REJECT_STDOUT_REGEX AND NOT "${REJECT_STDOUT_REGEX}" STREQUAL "" AND out MATCHES "${REJECT_STDOUT_REGEX}")
    message(FATAL_ERROR "stdout unexpectedly matched '${REJECT_STDOUT_REGEX}'\nstdout:\n${out}\nstderr:\n${err}")
endif()

if(DEFINED EXPECT_STDERR_REGEX AND NOT "${EXPECT_STDERR_REGEX}" STREQUAL "" AND NOT err MATCHES "${EXPECT_STDERR_REGEX}")
    message(FATAL_ERROR "stderr did not match '${EXPECT_STDERR_REGEX}'\nstdout:\n${out}\nstderr:\n${err}")
endif()

if(DEFINED REJECT_STDERR_REGEX AND NOT "${REJECT_STDERR_REGEX}" STREQUAL "" AND err MATCHES "${REJECT_STDERR_REGEX}")
    message(FATAL_ERROR "stderr unexpectedly matched '${REJECT_STDERR_REGEX}'\nstdout:\n${out}\nstderr:\n${err}")
endif()

if(DEFINED TRACE_FILE AND NOT "${TRACE_FILE}" STREQUAL "")
    set(trace_path "${WORKDIR}/${TRACE_FILE}")
    if(NOT EXISTS "${trace_path}")
        message(FATAL_ERROR "expected trace file ${trace_path} was not created\nstdout:\n${out}\nstderr:\n${err}")
    endif()
    file(READ "${trace_path}" trace_content)
    if(DEFINED EXPECT_FILE_REGEX AND NOT "${EXPECT_FILE_REGEX}" STREQUAL "" AND NOT trace_content MATCHES "${EXPECT_FILE_REGEX}")
        message(FATAL_ERROR "trace file did not match '${EXPECT_FILE_REGEX}'\nfile:\n${trace_content}\nstdout:\n${out}\nstderr:\n${err}")
    endif()
endif()
]=])

function(crexx_add_trace_capture_test TARGET_NAME SOURCE_FILE)
    set(options)
    set(oneValueArgs EXPECT_STDOUT_REGEX REJECT_STDOUT_REGEX EXPECT_STDERR_REGEX REJECT_STDERR_REGEX TRACE_FILE EXPECT_FILE_REGEX ENV_CREXX_TRACE ENV_CREXX_TRACE_TO)
    cmake_parse_arguments(TRACE_CAPTURE "${options}" "${oneValueArgs}" "" ${ARGN})

    set(_outputs)
    foreach(_mode IN ITEMS noopt opt)
        set(_base ${TARGET_NAME}_${_mode})
        set(_rxc_flags)
        set(_rxas_flags)
        if(_mode STREQUAL "noopt")
            list(APPEND _rxc_flags -n)
            list(APPEND _rxas_flags -n)
        endif()

        set(_output ${CMAKE_CURRENT_BINARY_DIR}/${_base}.rxbin)
        add_custom_command(
                OUTPUT ${_output}
                COMMAND ${CMAKE_COMMAND} -E env RXCP_EXIT_MODULE=rxcexits $<TARGET_FILE:rxc> -i ${CMAKE_BINARY_DIR}/bin ${_rxc_flags} -o ${_base} ${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE}
                COMMAND $<TARGET_FILE:rxas> ${_rxas_flags} -o ${_base} ${_base}
                DEPENDS rxc rxas ${CMAKE_BINARY_DIR}/bin/rxcexits.rxbin ${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE}
                WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                COMMENT "Compiling ${SOURCE_FILE} with exit (${_mode})"
        )
        list(APPEND _outputs ${_output})

        add_test(
                NAME ${TARGET_NAME}_${_mode}
                COMMAND ${CMAKE_COMMAND}
                        -DRXVM=$<TARGET_FILE:rxvm>
                        -DRXBIN=${_output}
                        -DLIBRPATH=${LIBRPATH}
                        -DWORKDIR=${CMAKE_CURRENT_BINARY_DIR}/${_base}_work
                        -DENV_CREXX_TRACE=${TRACE_CAPTURE_ENV_CREXX_TRACE}
                        -DENV_CREXX_TRACE_TO=${TRACE_CAPTURE_ENV_CREXX_TRACE_TO}
                        -DEXPECT_STDOUT_REGEX=${TRACE_CAPTURE_EXPECT_STDOUT_REGEX}
                        -DREJECT_STDOUT_REGEX=${TRACE_CAPTURE_REJECT_STDOUT_REGEX}
                        -DEXPECT_STDERR_REGEX=${TRACE_CAPTURE_EXPECT_STDERR_REGEX}
                        -DREJECT_STDERR_REGEX=${TRACE_CAPTURE_REJECT_STDERR_REGEX}
                        -DTRACE_FILE=${TRACE_CAPTURE_TRACE_FILE}
                        -DEXPECT_FILE_REGEX=${TRACE_CAPTURE_EXPECT_FILE_REGEX}
                        -P ${TRACE_CAPTURE_SCRIPT}
        )
    endforeach()

    add_custom_target(${TARGET_NAME}_bin ALL DEPENDS ${_outputs})
    add_dependencies(${TARGET_NAME}_bin compiler_exit_bin rxvm)
endfunction()

crexx_add_trace_capture_test(
        test_trace_stderr
        test_trace_stderr.crexx
        EXPECT_STDERR_REGEX "[ ]*6 \\*-\\*[ ]+return"
        REJECT_STDOUT_REGEX "return"
)

crexx_add_trace_capture_test(
        test_trace_file
        test_trace_file.crexx
        TRACE_FILE "trace-output.txt"
        EXPECT_FILE_REGEX "\"format\":\"llm\".*\"source\":\"[ ]*return"
        REJECT_STDOUT_REGEX "return"
        REJECT_STDERR_REGEX "return"
)

crexx_add_trace_capture_test(
        test_trace_normal_quiet
        test_trace_normal_quiet.crexx
        EXPECT_STDOUT_REGEX "TRACE NORMAL QUIET"
        REJECT_STDOUT_REGEX "\\*-\\*"
)

crexx_add_trace_capture_test(
        test_trace_normal_command
        test_trace_normal_command.crexx
        EXPECT_STDOUT_REGEX "\\+\\+\\+[ ]+RC=-3 NOSUCH whatever"
)

crexx_add_trace_capture_test(
        test_trace_env
        test_trace_env.crexx
        ENV_CREXX_TRACE "results"
        ENV_CREXX_TRACE_TO "stderr"
        EXPECT_STDOUT_REGEX "TRACE ENV DONE 42"
        EXPECT_STDERR_REGEX ">>>[ ]+\"42\""
        REJECT_STDOUT_REGEX "\\*-\\*"
)

crexx_add_trace_capture_test(
        test_trace_env_invalid
        test_trace_env.crexx
        ENV_CREXX_TRACE "ras"
        ENV_CREXX_TRACE_TO "stderr"
        EXPECT_STDOUT_REGEX "TRACE ENV DONE 42"
        EXPECT_STDERR_REGEX "\\+\\+\\+[ ]+TRACE ENV invalid CREXX_TRACE value.*ras"
        REJECT_STDERR_REGEX ">>>"
)

crexx_add_trace_capture_test(
        test_trace_env_ignored
        test_trace_env_ignored.crexx
        ENV_CREXX_TRACE "results"
        ENV_CREXX_TRACE_TO "stderr"
        EXPECT_STDOUT_REGEX "TRACE ENV IGNORED 42"
        REJECT_STDERR_REGEX ">>>"
)

set(TRACE_COMPILE_FAIL_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/trace_compile_fail.cmake")
file(WRITE "${TRACE_COMPILE_FAIL_SCRIPT}" [=[
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

function(crexx_add_trace_compile_fail_test TARGET_NAME SOURCE_FILE EXPECTED_REGEX)
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
                    -P ${TRACE_COMPILE_FAIL_SCRIPT}
    )
endfunction()

crexx_add_trace_compile_fail_test(
        test_trace_unknown_option_diagnostic
        test_trace_unknown_option.crexx
        "#TRACE_UNKNOWN_OPTION, \"potato\""
)

crexx_add_trace_compile_fail_test(
        test_trace_bad_abbreviation_diagnostic
        test_trace_bad_abbreviation.crexx
        "#TRACE_UNKNOWN_OPTION, \"ras\""
)

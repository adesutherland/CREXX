foreach(required RXC RXAS RXVM BINDIR SOURCE BUILD_DIR NAME EXPECTED_STATUS)
    if(NOT DEFINED ${required})
        message(FATAL_ERROR "Missing ${required}")
    endif()
endforeach()

set(rxc_flags)
if(NOOPT)
    list(APPEND rxc_flags -n)
endif()
execute_process(
        COMMAND "${RXC}" ${rxc_flags} -i "${BINDIR}" -o "${NAME}" "${SOURCE}"
        WORKING_DIRECTORY "${BUILD_DIR}"
        OUTPUT_VARIABLE output ERROR_VARIABLE error RESULT_VARIABLE result)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "RETURN status rxc failed: ${output}${error}")
endif()
execute_process(
        COMMAND "${RXAS}" -o "${NAME}.rxbin" "${NAME}"
        WORKING_DIRECTORY "${BUILD_DIR}"
        OUTPUT_VARIABLE output ERROR_VARIABLE error RESULT_VARIABLE result)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "RETURN status rxas failed: ${output}${error}")
endif()
set(run_args)
if(DEFINED ARG_ONE)
    list(APPEND run_args -a "${ARG_ONE}")
    if(DEFINED ARG_TWO)
        list(APPEND run_args "${ARG_TWO}")
    endif()
endif()
set(expected_output "")
if(DEFINED EXPECTED_FILE)
    file(READ "${EXPECTED_FILE}" expected_output)
endif()
execute_process(
        COMMAND "${RXVM}" "${NAME}.rxbin"
                "${BINDIR}/library" "${BINDIR}/classlib" "${BINDIR}/rxfnsc"
                ${run_args}
        WORKING_DIRECTORY "${BUILD_DIR}"
        OUTPUT_VARIABLE output ERROR_VARIABLE error RESULT_VARIABLE result)
if(NOT result EQUAL EXPECTED_STATUS OR NOT output STREQUAL expected_output)
    message(FATAL_ERROR "RETURN status expected ${EXPECTED_STATUS} and [${expected_output}]; got ${result}: ${output}${error}")
endif()

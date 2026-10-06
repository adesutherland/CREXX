foreach(required RXC RXAS RXLINK RXVM BINDIR SOURCE BUILD_DIR)
    if(NOT DEFINED ${required})
        message(FATAL_ERROR "Missing ${required}")
    endif()
endforeach()

if(DEFINED NAME)
    set(base "${NAME}")
else()
    set(base levelc_arg_linked)
endif()
set(rxc_flags)
if(NOOPT)
    list(APPEND rxc_flags -n)
endif()
execute_process(
        COMMAND "${RXC}" ${rxc_flags} -i "${BINDIR}" -o "${base}" "${SOURCE}"
        WORKING_DIRECTORY "${BUILD_DIR}"
        OUTPUT_VARIABLE output ERROR_VARIABLE error RESULT_VARIABLE result)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "rxc failed: ${output}${error}")
endif()
execute_process(
        COMMAND "${RXAS}" -o "${base}.rxbin" "${base}"
        WORKING_DIRECTORY "${BUILD_DIR}"
        OUTPUT_VARIABLE output ERROR_VARIABLE error RESULT_VARIABLE result)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "rxas failed: ${output}${error}")
endif()
execute_process(
        COMMAND "${RXLINK}" -o "${base}_image.rxbin"
                "${base}.rxbin" "${BINDIR}/library.rxbin"
                "${BINDIR}/classlib.rxbin" "${BINDIR}/rxfnsc.rxbin"
        WORKING_DIRECTORY "${BUILD_DIR}"
        OUTPUT_VARIABLE output ERROR_VARIABLE error RESULT_VARIABLE result)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "rxlink failed: ${output}${error}")
endif()
execute_process(
        COMMAND "${RXVM}" "${base}_image.rxbin"
        WORKING_DIRECTORY "${BUILD_DIR}"
        OUTPUT_VARIABLE output ERROR_VARIABLE error RESULT_VARIABLE result)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "rxvm failed: ${output}${error}")
endif()
string(REPLACE "\r\n" "\n" output "${output}")
if(DEFINED EXPECTED_FILE)
    file(READ "${EXPECTED_FILE}" expected_output)
else()
    set(expected_output "parse=mn|op|\nliteral=AB|CD|\nposition=AB|CDEF|\ncapture=X|B:TAIL||\n")
endif()
if(NOT output STREQUAL expected_output)
    message(FATAL_ERROR "Unexpected linked Level C output: ${output}")
endif()

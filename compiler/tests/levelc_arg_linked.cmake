foreach(required RXC RXAS RXLINK RXVM BINDIR SOURCE BUILD_DIR)
    if(NOT DEFINED ${required})
        message(FATAL_ERROR "Missing ${required}")
    endif()
endforeach()

set(base levelc_arg_linked)
execute_process(
        COMMAND "${RXC}" -i "${BINDIR}" -o "${base}" "${SOURCE}"
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
if(NOT output STREQUAL "parse=mn|op|\nliteral=AB|CD|\nposition=AB|CDEF|\ncapture=X|B:TAIL||\n")
    message(FATAL_ERROR "Unexpected linked ARG output: ${output}")
endif()

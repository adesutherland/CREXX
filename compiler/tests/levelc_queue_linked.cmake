foreach(required RXC RXAS RXLINK RXVM BINDIR NAME SOURCE EXPECTED BUILD_DIR)
    if(NOT DEFINED ${required})
        message(FATAL_ERROR "Missing ${required}")
    endif()
endforeach()

if(NOOPT)
    set(base "${NAME}_noopt")
    set(rxc_flags -n)
else()
    set(base "${NAME}")
    set(rxc_flags)
endif()
if(DEFINED INPUT AND NOT INPUT STREQUAL "")
    set(input_args INPUT_FILE "${INPUT}")
else()
    set(input_args)
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
        WORKING_DIRECTORY "${BUILD_DIR}" ${input_args}
        OUTPUT_VARIABLE output ERROR_VARIABLE error RESULT_VARIABLE result)
if(NOT result EQUAL 0 OR NOT error STREQUAL "")
    message(FATAL_ERROR "rxvm failed (${result}): ${output}${error}")
endif()
file(READ "${EXPECTED}" expected)
string(REPLACE "\r\n" "\n" output "${output}")
if(NOT output STREQUAL expected)
    message(FATAL_ERROR "Unexpected Level C queue output: ${output}")
endif()

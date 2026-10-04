foreach(required RXC RXAS RXVM BINDIR SOURCE BUILD_DIR NAME)
    if(NOT DEFINED ${required})
        message(FATAL_ERROR "Missing ${required}")
    endif()
endforeach()

set(noopt_flag)
if(NOOPT)
    set(noopt_flag -n)
endif()
execute_process(
        COMMAND "${RXC}" -i "${BINDIR}" ${noopt_flag} -o "${NAME}" "${SOURCE}"
        WORKING_DIRECTORY "${BUILD_DIR}"
        OUTPUT_VARIABLE compile_output ERROR_VARIABLE compile_error
        RESULT_VARIABLE result)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "rxc failed: ${compile_output}${compile_error}")
endif()
execute_process(
        COMMAND "${RXAS}" -o "${NAME}.rxbin" "${NAME}"
        WORKING_DIRECTORY "${BUILD_DIR}"
        OUTPUT_VARIABLE assemble_output ERROR_VARIABLE assemble_error
        RESULT_VARIABLE result)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "rxas failed: ${assemble_output}${assemble_error}")
endif()
execute_process(
        COMMAND "${RXVM}" "${NAME}.rxbin" "${BINDIR}/library"
                "${BINDIR}/classlib" "${BINDIR}/rxfnsc" -a "blue green" "red"
        WORKING_DIRECTORY "${BUILD_DIR}"
        OUTPUT_VARIABLE actual ERROR_VARIABLE runtime_error
        RESULT_VARIABLE result)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "rxvm failed: ${actual}${runtime_error}")
endif()
string(REPLACE "\r\n" "\n" actual "${actual}")
if(NOT actual STREQUAL "BLUE GREEN\nRED\nBLUE GREEN\n")
    message(FATAL_ERROR "Unexpected main ARG output: ${actual}")
endif()

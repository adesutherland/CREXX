foreach(required RXC RXAS RXVM BINDIR BUILD_DIR SOURCE NAME EXPECTED_DIAGNOSTIC)
    if(NOT DEFINED ${required})
        message(FATAL_ERROR "Missing ${required}")
    endif()
endforeach()

execute_process(
    COMMAND "${RXC}" -i "${BINDIR}" -o "${NAME}" "${SOURCE}"
    WORKING_DIRECTORY "${BUILD_DIR}"
    OUTPUT_VARIABLE rxc_out ERROR_VARIABLE rxc_err RESULT_VARIABLE rxc_result)
if(NOT rxc_result EQUAL 0)
    message(FATAL_ERROR "rxc failed: ${rxc_out}${rxc_err}")
endif()

execute_process(
    COMMAND "${RXAS}" -o "${NAME}.rxbin" "${NAME}"
    WORKING_DIRECTORY "${BUILD_DIR}"
    OUTPUT_VARIABLE rxas_out ERROR_VARIABLE rxas_err RESULT_VARIABLE rxas_result)
if(NOT rxas_result EQUAL 0)
    message(FATAL_ERROR "rxas failed: ${rxas_out}${rxas_err}")
endif()

execute_process(
    COMMAND "${RXVM}" "${NAME}.rxbin"
            "${BINDIR}/library" "${BINDIR}/classlib" "${BINDIR}/rxfnsc"
    WORKING_DIRECTORY "${BUILD_DIR}"
    OUTPUT_VARIABLE rxvm_out ERROR_VARIABLE rxvm_err RESULT_VARIABLE rxvm_result)
if(rxvm_result EQUAL 0)
    message(FATAL_ERROR "Invalid Level C program unexpectedly succeeded: ${rxvm_out}${rxvm_err}")
endif()
if(NOT "${rxvm_out}${rxvm_err}" MATCHES "${EXPECTED_DIAGNOSTIC}")
    message(FATAL_ERROR "Missing ${EXPECTED_DIAGNOSTIC}: ${rxvm_out}${rxvm_err}")
endif()

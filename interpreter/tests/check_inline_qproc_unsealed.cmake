if(NOT DEFINED RUNNER OR NOT DEFINED RXBIN)
    message(FATAL_ERROR "RUNNER and RXBIN are required")
endif()
execute_process(COMMAND "${RUNNER}" "${RXBIN}"
        RESULT_VARIABLE result OUTPUT_VARIABLE stdout ERROR_VARIABLE stderr)
if(result EQUAL 0 OR NOT "${stderr}" MATCHES "Invalid or unsealed qualified procedure")
    message(FATAL_ERROR
            "Expected checked unsealed QPR1 rejection; exit=${result}\n"
            "stdout:\n${stdout}\nstderr:\n${stderr}")
endif()

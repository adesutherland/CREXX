execute_process(
        COMMAND "${RXDAS}" "${INPUT_RXBIN}"
        OUTPUT_VARIABLE stdout
        ERROR_VARIABLE stderr
        RESULT_VARIABLE result)

if(result EQUAL 0)
    message(FATAL_ERROR "RXDAS accepted a corrupted RXBIN 007 graph")
endif()

string(FIND "${stderr}" "semantic graph is invalid" diagnostic_position)
if(diagnostic_position EQUAL -1)
    message(FATAL_ERROR
            "RXDAS did not report the graph validation error.\nstdout:\n${stdout}\nstderr:\n${stderr}")
endif()

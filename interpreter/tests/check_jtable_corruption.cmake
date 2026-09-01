cmake_minimum_required(VERSION 3.24)

foreach(_required IN ITEMS RUNNER RXBIN)
    if(NOT DEFINED ${_required} OR "${${_required}}" STREQUAL "")
        message(FATAL_ERROR "Missing required variable ${_required}")
    endif()
endforeach()

execute_process(
        COMMAND "${RUNNER}" "${RXBIN}"
        RESULT_VARIABLE _result
        OUTPUT_VARIABLE _stdout
        ERROR_VARIABLE _stderr)

set(_output "${_stdout}${_stderr}")
if(NOT _output MATCHES "RXBIN_CORRUPTION")
    message(FATAL_ERROR
            "Expected RXBIN_CORRUPTION from ${RUNNER} ${RXBIN}\n"
            "exit: ${_result}\n"
            "output:\n${_output}")
endif()

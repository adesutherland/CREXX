foreach(_required RXVM PROGRAM EXPECTED)
    if(NOT DEFINED ${_required} OR "${${_required}}" STREQUAL "")
        message(FATAL_ERROR "RunExample.cmake requires ${_required}")
    endif()
endforeach()

set(_command "${RXVM}" "${PROGRAM}")
foreach(_module MODULE1 MODULE2 MODULE3)
    if(DEFINED ${_module} AND NOT "${${_module}}" STREQUAL "")
        list(APPEND _command "${${_module}}")
    endif()
endforeach()

execute_process(
        COMMAND ${_command}
        RESULT_VARIABLE _result
        OUTPUT_VARIABLE _actual
        ERROR_VARIABLE _error)

if(NOT _result EQUAL 0)
    message(FATAL_ERROR "Example returned ${_result}:\n${_actual}${_error}")
endif()

if(NOT _error STREQUAL "")
    message(FATAL_ERROR "Example wrote unexpected stderr:\n${_error}")
endif()

file(READ "${EXPECTED}" _expected)
string(REPLACE "\r\n" "\n" _actual "${_actual}")
string(REPLACE "\r\n" "\n" _expected "${_expected}")

if(NOT _actual STREQUAL _expected)
    message(FATAL_ERROR
            "Example output mismatch.\nExpected:\n${_expected}\nActual:\n${_actual}")
endif()

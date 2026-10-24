cmake_minimum_required(VERSION 3.24)

file(REMOVE_RECURSE "${WORK}")
file(MAKE_DIRECTORY "${WORK}")

if(NOT DEFINED OK_INPUT OR NOT DEFINED ERROR_INPUT)
    message(FATAL_ERROR "OK_INPUT and ERROR_INPUT are required")
endif()

set(ok_output "${WORK}/diagnostic_ok.crexx")

execute_process(
    COMMAND "${CMAKE_COMMAND}" -E env
            "CREXX_DIAGNOSTICS=raw"
            "${RXPP_BIN}" -I "${OK_INPUT}" -O "${ok_output}" -M "${MACLIB_FILE}"
    WORKING_DIRECTORY "${WORK}"
    RESULT_VARIABLE ok_result
    OUTPUT_VARIABLE ok_output_text
    ERROR_VARIABLE ok_error_text
    ENCODING UTF-8)
set(ok_full "${ok_output_text}${ok_error_text}")
if(NOT ok_result EQUAL 0)
    message(FATAL_ERROR "INFO/WARN directives failed with ${ok_result}:\n${ok_full}")
endif()
if(NOT ok_full MATCHES "RXPP_INFO")
    message(FATAL_ERROR "##INFO did not emit RXPP_INFO:\n${ok_full}")
endif()
if(NOT ok_full MATCHES "RXPP_WARNING")
    message(FATAL_ERROR "##WARN did not emit RXPP_WARNING:\n${ok_full}")
endif()
file(READ "${ok_output}" generated)
if(NOT generated MATCHES "say \"continued\"")
    message(FATAL_ERROR "INFO/WARN did not allow preprocessing to continue:\n${generated}")
endif()
if(generated MATCHES "##INFO" OR generated MATCHES "##WARN")
    message(FATAL_ERROR "INFO/WARN directives leaked into generated CREXX:\n${generated}")
endif()

set(error_output "${WORK}/diagnostic_error.crexx")

execute_process(
    COMMAND "${CMAKE_COMMAND}" -E env
            "CREXX_DIAGNOSTICS=raw"
            "${RXPP_BIN}" -I "${ERROR_INPUT}" -O "${error_output}" -M "${MACLIB_FILE}"
    WORKING_DIRECTORY "${WORK}"
    RESULT_VARIABLE error_result
    OUTPUT_VARIABLE error_output_text
    ERROR_VARIABLE error_error_text
    ENCODING UTF-8)
set(error_full "${error_output_text}${error_error_text}")
if(NOT error_result EQUAL 8)
    message(FATAL_ERROR "##ERROR returned ${error_result}, expected 8:\n${error_full}")
endif()
if(NOT error_full MATCHES "RXPP_ERROR")
    message(FATAL_ERROR "##ERROR did not emit RXPP_ERROR:\n${error_full}")
endif()
if(NOT error_full MATCHES "This configuration is unsupported")
    message(FATAL_ERROR "##ERROR did not preserve its message:\n${error_full}")
endif()

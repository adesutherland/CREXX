cmake_minimum_required(VERSION 3.24)

if(NOT DEFINED RXPP_BIN OR NOT DEFINED SOURCE_ROOT OR NOT DEFINED WORK)
    message(FATAL_ERROR "RXPP_BIN, SOURCE_ROOT and WORK are required")
endif()

file(MAKE_DIRECTORY "${WORK}")
set(MACLIB_FILE "${SOURCE_ROOT}/preprocessor/maclib.rexx")

execute_process(
    COMMAND "${CMAKE_COMMAND}" -E env "CREXX_DIAGNOSTICS=raw"
            "${RXPP_BIN}"
            -I "${SOURCE_ROOT}/preprocessor/tests/testx_mcall_depth.rxpp"
            -O "${WORK}/over_limit.crexx"
            -M "${MACLIB_FILE}"
    RESULT_VARIABLE over_limit_result
    OUTPUT_VARIABLE over_limit_stdout
    ERROR_VARIABLE over_limit_stderr)
set(over_limit_output "${over_limit_stdout}${over_limit_stderr}")
if(NOT over_limit_result EQUAL 8)
    message(FATAL_ERROR
            "17th .mcall returned ${over_limit_result}, expected 8:\n${over_limit_output}")
endif()
if(NOT over_limit_output MATCHES "RXPP_MCALL_DEPTH macro=\"##loop\" depth=\"17\" limit=\"16\"")
    message(FATAL_ERROR "Missing depth-limit diagnostic:\n${over_limit_output}")
endif()
if(EXISTS "${WORK}/over_limit.crexx")
    message(FATAL_ERROR "RXPP wrote generated CREXX after the .mcall depth error")
endif()
message(STATUS "Expected depth-17 .mcall error detected; over-limit check passed.")

execute_process(
    COMMAND "${RXPP_BIN}"
            -I "${SOURCE_ROOT}/preprocessor/tests/testx_mcall_depth_ok.rxpp"
            -O "${WORK}/at_limit.crexx"
            -M "${MACLIB_FILE}"
    RESULT_VARIABLE at_limit_result
    OUTPUT_VARIABLE at_limit_stdout
    ERROR_VARIABLE at_limit_stderr)
if(NOT at_limit_result EQUAL 0)
    message(FATAL_ERROR
            "16 nested .mcall expansions returned ${at_limit_result}:\n${at_limit_stdout}${at_limit_stderr}")
endif()
file(READ "${WORK}/at_limit.crexx" at_limit_source)
if(NOT at_limit_source MATCHES "say \"completed at the nesting limit\"")
    message(FATAL_ERROR "The depth-16 macro did not complete:\n${at_limit_source}")
endif()
message(STATUS "Sixteen nested .mcall expansions completed successfully.")

if(NOT DEFINED RXPP_BIN)
    message(FATAL_ERROR "RXPP_BIN is required")
endif()
if(NOT DEFINED RXC_BIN)
    message(FATAL_ERROR "RXC_BIN is required")
endif()
if(NOT DEFINED INPUT_FILE)
    message(FATAL_ERROR "INPUT_FILE is required")
endif()
if(NOT DEFINED OUTPUT_FILE)
    message(FATAL_ERROR "OUTPUT_FILE is required")
endif()
if(NOT DEFINED MACLIB_FILE)
    message(FATAL_ERROR "MACLIB_FILE is required")
endif()

execute_process(
    COMMAND "${RXPP_BIN}" rxprecomp -I "${INPUT_FILE}" -o "${OUTPUT_FILE}" -m "${MACLIB_FILE}"
    RESULT_VARIABLE rxpp_res
    OUTPUT_VARIABLE rxpp_out
    ERROR_VARIABLE rxpp_err
)
if(NOT rxpp_res EQUAL 0)
    message(FATAL_ERROR "variadic forwarding rxpp failed with ${rxpp_res}:\n${rxpp_out}${rxpp_err}")
endif()

file(READ "${OUTPUT_FILE}" generated)
if(NOT generated MATCHES "say \"FORWARDED alpha beta gamma\"")
    message(FATAL_ERROR "single-shot forwarding did not preserve all ordinary arguments:\n${generated}")
endif()
if(NOT generated MATCHES "say \"demo: one two three\"")
    message(FATAL_ERROR "forwarding after a fixed parameter did not preserve the tail:\n${generated}")
endif()
if(NOT generated MATCHES "beta gamma")
    message(FATAL_ERROR "quoted forwarding argument was lost:\n${generated}")
endif()
if(NOT generated MATCHES "beta,gamma")
    message(FATAL_ERROR "comma inside quoted forwarding argument was lost:\n${generated}")
endif()

set(output_base "${OUTPUT_FILE}.compiled")
execute_process(
    COMMAND "${RXC_BIN}" --diagnostics raw -o "${output_base}" "${OUTPUT_FILE}"
    RESULT_VARIABLE rxc_res
    OUTPUT_VARIABLE rxc_out
    ERROR_VARIABLE rxc_err
)
if(NOT rxc_res EQUAL 0)
    message(FATAL_ERROR "rxc failed to compile variadic forwarding output with ${rxc_res}:\n${rxc_out}${rxc_err}\nGenerated:\n${generated}")
endif()

set(bad_input "${OUTPUT_FILE}.bad.rxpp")
set(bad_output "${OUTPUT_FILE}.bad.crexx")
file(WRITE "${bad_input}" [=[options levelb
##define bad(...args tail) {say "bad"}
]=])
execute_process(
    COMMAND "${CMAKE_COMMAND}" -E env CREXX_DIAGNOSTICS=raw
            "${RXPP_BIN}" rxprecomp -I "${bad_input}" -o "${bad_output}" -m "${MACLIB_FILE}"
    RESULT_VARIABLE bad_res
    OUTPUT_VARIABLE bad_out
    ERROR_VARIABLE bad_err
)
if(bad_res EQUAL 0)
    message(FATAL_ERROR "non-terminal forwarding marker was accepted:\n${bad_out}${bad_err}")
endif()
if(NOT "${bad_out}${bad_err}" MATCHES "RXPP_MACRO_FORWARDING_POSITION")
    message(FATAL_ERROR "non-terminal forwarding marker did not produce its diagnostic:\n${bad_out}${bad_err}")
endif()

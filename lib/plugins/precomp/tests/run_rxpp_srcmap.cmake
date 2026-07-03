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
    message(FATAL_ERROR "rxpp failed with ${rxpp_res}:\n${rxpp_out}${rxpp_err}")
endif()

file(READ "${OUTPUT_FILE}" out_content)
string(FIND "${out_content}" "options levelb srcmap" options_pos)
if(options_pos EQUAL -1)
    message(FATAL_ERROR "rxpp did not emit options levelb srcmap:\n${out_content}")
endif()
string(FIND "${out_content}" "say \"Hello@@example.com\"" escaped_at_pos)
if(escaped_at_pos EQUAL -1)
    message(FATAL_ERROR "rxpp did not escape literal @ in srcmap mode:\n${out_content}")
endif()
string(FIND "${out_content}" "say @5+9{3*3@}" square_span_pos)
if(square_span_pos EQUAL -1)
    message(FATAL_ERROR "rxpp did not emit source-map span for SQUARE macro:\n${out_content}")
endif()
string(FIND "${out_content}" "@1+23{say \"value\" 16@}" trace_span_pos)
if(trace_span_pos EQUAL -1)
    message(FATAL_ERROR "rxpp did not emit source-map span for TRACEVALUE macro:\n${out_content}")
endif()

set(output_base "${OUTPUT_FILE}.compiled")
execute_process(
    COMMAND "${RXC_BIN}" --diagnostics raw -o "${output_base}" "${OUTPUT_FILE}"
    RESULT_VARIABLE rxc_res
    OUTPUT_VARIABLE rxc_out
    ERROR_VARIABLE rxc_err
)
if(NOT rxc_res EQUAL 0)
    message(FATAL_ERROR "rxc failed to compile rxpp srcmap output with ${rxc_res}:\n${rxc_out}${rxc_err}\nGenerated:\n${out_content}")
endif()

file(READ "${output_base}.rxas" rxas_content)
string(FIND "${rxas_content}" "say \"Hello@example.com\"" compiled_at_pos)
if(compiled_at_pos EQUAL -1)
    message(FATAL_ERROR "rxc did not strip escaped @ from rxpp output:\n${rxas_content}")
endif()

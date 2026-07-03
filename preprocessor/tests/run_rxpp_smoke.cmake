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
    RESULT_VARIABLE res
    OUTPUT_VARIABLE rxpp_out
    ERROR_VARIABLE rxpp_err
)
if(NOT res EQUAL 0)
    message(FATAL_ERROR "rxpp failed with ${res}:\n${rxpp_out}${rxpp_err}")
endif()

file(READ "${OUTPUT_FILE}" out_content)
if(NOT out_content MATCHES "say \"Hello\"")
    message(FATAL_ERROR "Missing say \"Hello\" in output")
endif()
if(NOT out_content MATCHES "say \"Hello@@example.com\"")
    message(FATAL_ERROR "Missing escaped literal @ case in automatic srcmap output")
endif()
string(FIND "${out_content}" "options levelb srcmap" srcmap_options_pos)
if(srcmap_options_pos EQUAL -1)
    message(FATAL_ERROR "rxpp did not emit srcmap options by default:\n${out_content}")
endif()
string(FIND "${out_content}" "say @5+9{@12+1{3@}*@12+1{3@}@}" marker_pos)
if(marker_pos EQUAL -1)
    message(FATAL_ERROR "rxpp did not emit nested source-map spans by default:\n${out_content}")
endif()
string(FIND "${out_content}" "@1+23{say @12+7{\"value\"@} @21+2{16@}@}" trace_marker_pos)
if(trace_marker_pos EQUAL -1)
    message(FATAL_ERROR "Missing expanded TRACEVALUE macro with nested spans in output:\n${out_content}")
endif()

set(output_base "${OUTPUT_FILE}.compiled")
execute_process(
    COMMAND "${RXC_BIN}" --diagnostics raw -o "${output_base}" "${OUTPUT_FILE}"
    RESULT_VARIABLE rxc_res
    OUTPUT_VARIABLE rxc_out
    ERROR_VARIABLE rxc_err
)
if(NOT rxc_res EQUAL 0)
    message(FATAL_ERROR "rxc failed to compile automatic srcmap rxpp output with ${rxc_res}:\n${rxc_out}${rxc_err}\nGenerated:\n${out_content}")
endif()

file(READ "${output_base}.rxas" rxas_content)
string(FIND "${rxas_content}" "say \"Hello@example.com\"" compiled_at_pos)
if(compiled_at_pos EQUAL -1)
    message(FATAL_ERROR "rxc did not strip escaped @ from automatic srcmap rxpp output:\n${rxas_content}")
endif()
string(FIND "${rxas_content}" "say 3*3" compiled_square_pos)
if(compiled_square_pos EQUAL -1)
    message(FATAL_ERROR "rxc did not strip nested source-map spans from automatic srcmap rxpp output:\n${rxas_content}")
endif()

set(nosrcmap_input "${OUTPUT_FILE}.nosrcmap.rxpp")
set(nosrcmap_output "${OUTPUT_FILE}.nosrcmap.crexx")
file(WRITE "${nosrcmap_input}" [=[##CFLAG nosrcmap
say "Hello@example.com"
say SQUARE(3)
]=])

execute_process(
    COMMAND "${RXPP_BIN}" rxprecomp -I "${nosrcmap_input}" -o "${nosrcmap_output}" -m "${MACLIB_FILE}"
    RESULT_VARIABLE nosrcmap_res
    OUTPUT_VARIABLE nosrcmap_rxpp_out
    ERROR_VARIABLE nosrcmap_rxpp_err
)
if(NOT nosrcmap_res EQUAL 0)
    message(FATAL_ERROR "rxpp nosrcmap run failed with ${nosrcmap_res}:\n${nosrcmap_rxpp_out}${nosrcmap_rxpp_err}")
endif()

file(READ "${nosrcmap_output}" nosrcmap_content)
string(FIND "${nosrcmap_content}" "options levelb srcmap" nosrcmap_options_pos)
if(NOT nosrcmap_options_pos EQUAL -1)
    message(FATAL_ERROR "rxpp emitted srcmap options despite ##CFLAG nosrcmap:\n${nosrcmap_content}")
endif()
string(FIND "${nosrcmap_content}" "say \"Hello@example.com\"" nosrcmap_at_pos)
if(nosrcmap_at_pos EQUAL -1)
    message(FATAL_ERROR "rxpp did not preserve literal @ in nosrcmap output:\n${nosrcmap_content}")
endif()
string(FIND "${nosrcmap_content}" "@5+9{" nosrcmap_marker_pos)
if(NOT nosrcmap_marker_pos EQUAL -1)
    message(FATAL_ERROR "rxpp emitted source-map spans despite ##CFLAG nosrcmap:\n${nosrcmap_content}")
endif()

set(nosrcmap_base "${OUTPUT_FILE}.nosrcmap.compiled")
execute_process(
    COMMAND "${RXC_BIN}" --diagnostics raw -o "${nosrcmap_base}" "${nosrcmap_output}"
    RESULT_VARIABLE nosrcmap_rxc_res
    OUTPUT_VARIABLE nosrcmap_rxc_out
    ERROR_VARIABLE nosrcmap_rxc_err
)
if(NOT nosrcmap_rxc_res EQUAL 0)
    message(FATAL_ERROR "rxc failed to compile nosrcmap rxpp output with ${nosrcmap_rxc_res}:\n${nosrcmap_rxc_out}${nosrcmap_rxc_err}\nGenerated:\n${nosrcmap_content}")
endif()

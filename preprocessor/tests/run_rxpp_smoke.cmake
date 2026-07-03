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
if(NOT out_content MATCHES "say \"Hello@example.com\"")
    message(FATAL_ERROR "Missing literal @ compatibility case in output")
endif()
if(NOT out_content MATCHES "say 3\\*3")
    message(FATAL_ERROR "Missing say 3*3 in output")
endif()
if(NOT out_content MATCHES "say \"value\" 16")
    message(FATAL_ERROR "Missing expanded TRACEVALUE macro in output")
endif()
string(FIND "${out_content}" "options levelb srcmap" srcmap_options_pos)
if(NOT srcmap_options_pos EQUAL -1)
    message(FATAL_ERROR "rxpp emitted srcmap options without ##CFLAG srcmap:\n${out_content}")
endif()
string(FIND "${out_content}" "@@" escaped_at_pos)
if(NOT escaped_at_pos EQUAL -1)
    message(FATAL_ERROR "rxpp escaped literal @ in no-srcmap mode:\n${out_content}")
endif()
string(FIND "${out_content}" "@5+9{" marker_pos)
if(NOT marker_pos EQUAL -1)
    message(FATAL_ERROR "rxpp emitted source-map spans in no-srcmap mode:\n${out_content}")
endif()

set(output_base "${OUTPUT_FILE}.compiled")
execute_process(
    COMMAND "${RXC_BIN}" --diagnostics raw -o "${output_base}" "${OUTPUT_FILE}"
    RESULT_VARIABLE rxc_res
    OUTPUT_VARIABLE rxc_out
    ERROR_VARIABLE rxc_err
)
if(NOT rxc_res EQUAL 0)
    message(FATAL_ERROR "rxc failed to compile no-srcmap rxpp output with ${rxc_res}:\n${rxc_out}${rxc_err}\nGenerated:\n${out_content}")
endif()

file(READ "${output_base}.rxas" rxas_content)
string(FIND "${rxas_content}" "say \"Hello@example.com\"" compiled_at_pos)
if(compiled_at_pos EQUAL -1)
    message(FATAL_ERROR "rxc did not preserve literal @ in no-srcmap rxpp output:\n${rxas_content}")
endif()

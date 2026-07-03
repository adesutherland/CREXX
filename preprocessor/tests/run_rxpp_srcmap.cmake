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
string(FIND "${out_content}" "${INPUT_FILE}" input_file_marker_pos)
if(input_file_marker_pos EQUAL -1)
    message(FATAL_ERROR "rxpp did not emit source-map file marker for input path:\n${out_content}")
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
string(FIND "${out_content}" "@1+12{say \"scripted\"@}" script_span_pos)
if(script_span_pos EQUAL -1)
    message(FATAL_ERROR "rxpp did not map script-macro output back to the invocation:\n${out_content}")
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
string(FIND "${rxas_content}" "${INPUT_FILE}" srcstep_file_pos)
if(srcstep_file_pos EQUAL -1)
    message(FATAL_ERROR "rxc did not preserve rxpp source file in srcmap source-step metadata:\n${rxas_content}")
endif()

set(include_file "${OUTPUT_FILE}.included.rxpp")
set(include_input "${OUTPUT_FILE}.include_input.rxpp")
set(include_output "${OUTPUT_FILE}.include_output.crexx")
file(WRITE "${include_file}" [=[answer = SQUARE(totl + 1)
]=])
file(WRITE "${include_input}" "##CFLAG srcmap\n##include ${include_file}\n")

execute_process(
    COMMAND "${RXPP_BIN}" rxprecomp -I "${include_input}" -o "${include_output}" -m "${MACLIB_FILE}"
    RESULT_VARIABLE include_rxpp_res
    OUTPUT_VARIABLE include_rxpp_out
    ERROR_VARIABLE include_rxpp_err
)
if(NOT include_rxpp_res EQUAL 0)
    message(FATAL_ERROR "rxpp failed for include source-map case with ${include_rxpp_res}:\n${include_rxpp_out}${include_rxpp_err}")
endif()

file(READ "${include_output}" include_content)
string(FIND "${include_content}" "${include_file}" include_marker_pos)
if(include_marker_pos EQUAL -1)
    message(FATAL_ERROR "rxpp did not switch source-map file origin for included lines:\n${include_content}")
endif()

execute_process(
    COMMAND "${RXC_BIN}" --diagnostics raw "${include_output}"
    RESULT_VARIABLE include_rxc_res
    OUTPUT_VARIABLE include_rxc_out
    ERROR_VARIABLE include_rxc_err
)
if(include_rxc_res EQUAL 0)
    message(FATAL_ERROR "rxc unexpectedly compiled include source-map diagnostic case:\n${include_content}")
endif()
if(NOT include_rxc_err MATCHES "included\\.rxpp @ 1:")
    message(FATAL_ERROR "included-file diagnostic was not remapped to included source:\n${include_rxc_err}\nGenerated:\n${include_content}")
endif()
if(NOT include_rxc_err MATCHES "totl")
    message(FATAL_ERROR "included-file diagnostic did not include included source text:\n${include_rxc_err}")
endif()

set(double_input "${OUTPUT_FILE}.already_srcmap.rxpp")
set(double_output "${OUTPUT_FILE}.already_srcmap.out.crexx")
file(WRITE "${double_input}" [=[/* RXPP */
options levelb srcmap
say "already generated"
]=])

execute_process(
    COMMAND "${RXPP_BIN}" rxprecomp -I "${double_input}" -o "${double_output}" -m "${MACLIB_FILE}"
    RESULT_VARIABLE double_res
    OUTPUT_VARIABLE double_out
    ERROR_VARIABLE double_err
)
if(double_res EQUAL 0)
    message(FATAL_ERROR "rxpp unexpectedly accepted already source-mapped generated input")
endif()
if(NOT "${double_out}${double_err}" MATCHES "already contains options srcmap")
    message(FATAL_ERROR "rxpp double-processing guard did not explain the source-map input:\n${double_out}${double_err}")
endif()

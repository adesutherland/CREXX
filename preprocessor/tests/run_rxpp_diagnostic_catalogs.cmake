cmake_minimum_required(VERSION 3.24)

file(MAKE_DIRECTORY "${WORK}")

set(source_list "${WORK}/rxpp_diagnostic_catalog_sources.txt")
file(WRITE "${source_list}" "${SOURCE_ROOT}/preprocessor/rxpp.crexx\n")

set(tool_base "${WORK}/check_diagnostic_catalogs")
execute_process(
    COMMAND "${RXC_BIN}" -i "${BIN_DIR}" -o "${tool_base}" "${SOURCE_ROOT}/tools/check_diagnostic_catalogs.crexx"
    WORKING_DIRECTORY "${WORK}"
    RESULT_VARIABLE compile_res
    OUTPUT_VARIABLE compile_out
    ERROR_VARIABLE compile_err)
if(NOT compile_res EQUAL 0)
    message(FATAL_ERROR "rxpp diagnostic catalogue checker compile failed with ${compile_res}:\n${compile_out}${compile_err}")
endif()

execute_process(
    COMMAND "${RXAS_BIN}" -o "${tool_base}.rxbin" "${tool_base}"
    WORKING_DIRECTORY "${WORK}"
    RESULT_VARIABLE assemble_res
    OUTPUT_VARIABLE assemble_out
    ERROR_VARIABLE assemble_err)
if(NOT assemble_res EQUAL 0)
    message(FATAL_ERROR "rxpp diagnostic catalogue checker assembly failed with ${assemble_res}:\n${assemble_out}${assemble_err}")
endif()

execute_process(
    COMMAND "${RXVM_BIN}" "${BIN_DIR}/library.rxbin" "${tool_base}.rxbin" -a
            "${SOURCE_ROOT}/messages/diagnostics.en_GB.msg"
            "${source_list}"
            "2"
            "${SOURCE_ROOT}/messages/diagnostics.de_DE.msg"
            "${SOURCE_ROOT}/messages/diagnostics.nl_NL.msg"
            "1"
            "${SOURCE_ROOT}/messages/diagnostics.en_US.msg"
    WORKING_DIRECTORY "${WORK}"
    RESULT_VARIABLE check_res
    OUTPUT_VARIABLE check_out
    ERROR_VARIABLE check_err)
if(NOT check_res EQUAL 0)
    message(FATAL_ERROR "rxpp diagnostic catalogue checker failed with ${check_res}:\n${check_out}${check_err}")
endif()

message(STATUS "${check_out}")

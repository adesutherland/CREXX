if(NOT DEFINED RXPP_BIN)
    message(FATAL_ERROR "RXPP_BIN is required")
endif()
if(NOT DEFINED RXPP_SH_BIN)
    message(FATAL_ERROR "RXPP_SH_BIN is required")
endif()
if(NOT DEFINED PARSER_TESTER_BIN)
    message(FATAL_ERROR "PARSER_TESTER_BIN is required")
endif()
if(NOT DEFINED MACLIB_FILE)
    message(FATAL_ERROR "MACLIB_FILE is required")
endif()
if(NOT DEFINED DUMP_AST_FILE)
    message(FATAL_ERROR "DUMP_AST_FILE is required")
endif()
if(NOT DEFINED WORK)
    message(FATAL_ERROR "WORK is required")
endif()

if(NOT DEFINED RXPP_SH_USE_ENV)
    set(RXPP_SH_USE_ENV ON)
endif()

file(MAKE_DIRECTORY "${WORK}")
set(input_file "${WORK}/rxpp_sh_srcmap.rxpp")
file(WRITE "${input_file}" "say SQUARE(totl + 1)\n")

if(RXPP_SH_USE_ENV)
    execute_process(
        COMMAND "${CMAKE_COMMAND}" -E env
            "RXPP_SH_RXPP=${RXPP_BIN}"
            "RXPP_SH_MACLIB=${MACLIB_FILE}"
            "${PARSER_TESTER_BIN}" -q -p "${RXPP_SH_BIN}" -s "${input_file}" "${DUMP_AST_FILE}"
        WORKING_DIRECTORY "${WORK}"
        OUTPUT_VARIABLE parser_out
        ERROR_VARIABLE parser_err
        RESULT_VARIABLE parser_res
    )
else()
    execute_process(
        COMMAND "${PARSER_TESTER_BIN}" -q -p "${RXPP_SH_BIN}" -s "${input_file}" "${DUMP_AST_FILE}"
        WORKING_DIRECTORY "${WORK}"
        OUTPUT_VARIABLE parser_out
        ERROR_VARIABLE parser_err
        RESULT_VARIABLE parser_res
    )
endif()

set(parser_combined "${parser_out}${parser_err}")
if(NOT parser_res EQUAL 0)
    message(FATAL_ERROR "rxpp-sh parser_tester failed with ${parser_res}:\n${parser_combined}")
endif()
if(NOT parser_combined MATCHES "Node: type=120 \\(SYNTAX_ERROR\\), pos=11, len=8")
    message(FATAL_ERROR "rxpp-sh did not map the compiler diagnostic onto the original RXPP span:\n${parser_combined}")
endif()
if(NOT parser_combined MATCHES "BAD_CONVERSION")
    message(FATAL_ERROR "rxpp-sh did not preserve the compiler diagnostic message:\n${parser_combined}")
endif()
if(NOT parser_combined MATCHES "Node: type=[0-9]+ \\(LEXER_IDENTIFIER\\), pos=11, len=4, sev=51, text=\"totl\"")
    message(FATAL_ERROR "rxpp-sh did not overlay the mapped compiler diagnostic onto the authored RXPP token:\n${parser_combined}")
endif()

set(line_input_file "${WORK}/rxpp_sh_line_srcmap.rxpp")
file(WRITE "${line_input_file}" "totl =\nsay 'after'\n")

if(RXPP_SH_USE_ENV)
    execute_process(
        COMMAND "${CMAKE_COMMAND}" -E env
            "RXPP_SH_RXPP=${RXPP_BIN}"
            "RXPP_SH_MACLIB=${MACLIB_FILE}"
            "${PARSER_TESTER_BIN}" -q -p "${RXPP_SH_BIN}" -s "${line_input_file}" "${DUMP_AST_FILE}"
        WORKING_DIRECTORY "${WORK}"
        OUTPUT_VARIABLE line_parser_out
        ERROR_VARIABLE line_parser_err
        RESULT_VARIABLE line_parser_res
    )
else()
    execute_process(
        COMMAND "${PARSER_TESTER_BIN}" -q -p "${RXPP_SH_BIN}" -s "${line_input_file}" "${DUMP_AST_FILE}"
        WORKING_DIRECTORY "${WORK}"
        OUTPUT_VARIABLE line_parser_out
        ERROR_VARIABLE line_parser_err
        RESULT_VARIABLE line_parser_res
    )
endif()

set(line_parser_combined "${line_parser_out}${line_parser_err}")
if(NOT line_parser_res EQUAL 0)
    message(FATAL_ERROR "rxpp-sh line-marker parser_tester failed with ${line_parser_res}:\n${line_parser_combined}")
endif()
if(NOT line_parser_combined MATCHES "Node: type=120 \\(SYNTAX_ERROR\\), pos=0, len=6")
    message(FATAL_ERROR "rxpp-sh did not map a line-marker compiler diagnostic onto the authored RXPP line:\n${line_parser_combined}")
endif()
if(NOT line_parser_combined MATCHES "Node: type=[0-9]+ \\(LEXER_IDENTIFIER\\), pos=0, len=4, sev=51, text=\"totl\"")
    message(FATAL_ERROR "rxpp-sh did not overlay the line-marker compiler diagnostic onto the authored RXPP token:\n${line_parser_combined}")
endif()

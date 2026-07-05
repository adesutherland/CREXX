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

file(MAKE_DIRECTORY "${WORK}")
set(input_file "${WORK}/rxpp_sh_tokens.rxpp")
file(WRITE "${input_file}" "/* RXPP token demo.\n * continuation line\n */\n##set prefix 123\n##define SQUARE(x) {(x) * (x)}\ntotal = 1\nsay SQUARE(total + 1)\nsay CUBE(total)\nsay {prefix}\n")

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

set(parser_combined "${parser_out}${parser_err}")
if(NOT parser_res EQUAL 0)
    message(FATAL_ERROR "rxpp-sh parser_tester failed with ${parser_res}:\n${parser_combined}")
endif()
if(NOT parser_combined MATCHES "LEXER_PREPROCESSOR\\), pos=[0-9]+, len=5, sev=48, text=\"##set\"")
    message(FATAL_ERROR "rxpp-sh did not emit the ##set directive as a preprocessor token:\n${parser_combined}")
endif()
if(NOT parser_combined MATCHES "LEXER_COMMENT\\), pos=[0-9]+, len=[0-9]+, sev=48, text=\" [*] continuation line\"")
    message(FATAL_ERROR "rxpp-sh did not emit the continuation line in a block comment as a comment token:\n${parser_combined}")
endif()
if(NOT parser_combined MATCHES "LEXER_MACRO_CONSTANT\\), pos=[0-9]+, len=6, sev=48, text=\"prefix\"")
    message(FATAL_ERROR "rxpp-sh did not emit the ##set variable as a macro constant:\n${parser_combined}")
endif()
if(NOT parser_combined MATCHES "LEXER_PREPROCESSOR\\), pos=[0-9]+, len=8, sev=48, text=\"##define\"")
    message(FATAL_ERROR "rxpp-sh did not emit ##define as a preprocessor token:\n${parser_combined}")
endif()
if(NOT parser_combined MATCHES "LEXER_MACRO_IDENTIFIER\\), pos=[0-9]+, len=6, sev=48, text=\"SQUARE\"")
    message(FATAL_ERROR "rxpp-sh did not emit the macro definition/call as a macro identifier:\n${parser_combined}")
endif()
if(NOT parser_combined MATCHES "LEXER_MACRO_IDENTIFIER\\), pos=[0-9]+, len=4, sev=48, text=\"CUBE\"")
    message(FATAL_ERROR "rxpp-sh did not emit a maclib macro call as a macro identifier:\n${parser_combined}")
endif()
if(NOT parser_combined MATCHES "LEXER_MACRO_VARIABLE\\), pos=[0-9]+, len=8, sev=48, text=\"[{]prefix[}]\"")
    message(FATAL_ERROR "rxpp-sh did not emit {prefix} as a macro variable:\n${parser_combined}")
endif()

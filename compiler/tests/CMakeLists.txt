cmake_minimum_required(VERSION 3.24)

project(CompilerTests C)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

set(CMAKE_C_STANDARD 90)
set(CMAKE_C_STANDARD_REQUIRED ON)

# The test driver itself must be compiled as strict C90
add_executable(crexx_test_driver src/test_driver.c)

add_executable(mutate_rxbin_feature mutate_rxbin_feature.c)

# Ensure the driver is built when tests are built
add_dependencies(crexx_test_driver rxc)

if (ENABLE_PARSER_MODE)
    add_executable(crexx_fake_editor src/fake_editor.c)
    target_link_libraries(crexx_fake_editor DSLSH::editor)

    set(RXC_HELP_SYNTAXHIGHLIGHT_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/rxc_help_syntaxhighlight.cmake")
    file(GENERATE OUTPUT "${RXC_HELP_SYNTAXHIGHLIGHT_SCRIPT}" CONTENT
"execute_process(
    COMMAND \"$<TARGET_FILE:rxc>\" -h
    WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"
    OUTPUT_VARIABLE out
    ERROR_VARIABLE err
    RESULT_VARIABLE res
)
if(NOT res EQUAL 0)
  message(FATAL_ERROR \"rxc -h failed with exit code \${res}.\\nSTDOUT:\\n\${out}\\nSTDERR:\\n\${err}\")
endif()
if(NOT out MATCHES \"--syntaxhighlight\")
  message(FATAL_ERROR \"rxc help does not advertise --syntaxhighlight.\\nSTDOUT:\\n\${out}\\nSTDERR:\\n\${err}\")
endif()
if(out MATCHES \"--parser\")
  message(FATAL_ERROR \"rxc help still advertises --parser instead of --syntaxhighlight.\\nSTDOUT:\\n\${out}\\nSTDERR:\\n\${err}\")
endif()
execute_process(
    COMMAND \"$<TARGET_FILE:rxc>\" --help
    WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"
    OUTPUT_VARIABLE long_out
    ERROR_VARIABLE long_err
    RESULT_VARIABLE long_res
)
if(NOT long_res EQUAL 0)
  message(FATAL_ERROR \"rxc --help failed with exit code \${long_res}.\\nSTDOUT:\\n\${long_out}\\nSTDERR:\\n\${long_err}\")
endif()
if(NOT long_out MATCHES \"--syntaxhighlight\")
  message(FATAL_ERROR \"rxc --help does not advertise --syntaxhighlight.\\nSTDOUT:\\n\${long_out}\\nSTDERR:\\n\${long_err}\")
endif()
if(long_out MATCHES \"--parser\")
  message(FATAL_ERROR \"rxc --help still advertises --parser instead of --syntaxhighlight.\\nSTDOUT:\\n\${long_out}\\nSTDERR:\\n\${long_err}\")
endif()
")
    add_test(NAME rxc_help_syntaxhighlight
             COMMAND ${CMAKE_COMMAND} -P "${RXC_HELP_SYNTAXHIGHLIGHT_SCRIPT}"
             WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    set_tests_properties(rxc_help_syntaxhighlight PROPERTIES LABELS "compiler;syntax_highlighting")

    function(add_crexx_syntax_highlight_test NAME SOURCE_FILE)
        set(options)
        set(oneValueArgs)
        set(multiValueArgs MUST_MATCH MUST_NOT_MATCH)
        cmake_parse_arguments(HL "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

        set(SCRIPT_FILE "${CMAKE_CURRENT_BINARY_DIR}/${NAME}.cmake")
        set(SCRIPT_CONTENT "")

        string(APPEND SCRIPT_CONTENT
               "execute_process(\n"
               "    COMMAND \"$<TARGET_FILE:parser_tester>\" -q -p \"$<TARGET_FILE:rxc>\" -s \"${SOURCE_FILE}\" \"${CMAKE_CURRENT_SOURCE_DIR}/highlighting/dump_ast.txt\"\n"
               "    WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
               "    OUTPUT_VARIABLE out\n"
               "    ERROR_VARIABLE err\n"
               "    RESULT_VARIABLE res\n"
               ")\n")
        string(APPEND SCRIPT_CONTENT
               "if(NOT res EQUAL 0)\n"
               "  message(FATAL_ERROR \"parser_tester failed with exit code \${res}\\nSTDERR:\\n\${err}\\nSTDOUT:\\n\${out}\")\n"
               "endif()\n")

        foreach(_pattern IN LISTS HL_MUST_MATCH)
            string(APPEND SCRIPT_CONTENT
                   "if(NOT out MATCHES [=[${_pattern}]=])\n"
                   "  message(FATAL_ERROR \"Missing expected syntax-highlighting assertion.\\nSTDOUT:\\n\${out}\\nSTDERR:\\n\${err}\")\n"
                   "endif()\n")
        endforeach()

        foreach(_pattern IN LISTS HL_MUST_NOT_MATCH)
            string(APPEND SCRIPT_CONTENT
                   "if(out MATCHES [=[${_pattern}]=])\n"
                   "  message(FATAL_ERROR \"Found forbidden syntax-highlighting pattern.\\nSTDOUT:\\n\${out}\\nSTDERR:\\n\${err}\")\n"
                   "endif()\n")
        endforeach()

        file(GENERATE OUTPUT "${SCRIPT_FILE}" CONTENT "${SCRIPT_CONTENT}")

        add_test(NAME ${NAME}
                 COMMAND ${CMAKE_COMMAND} -P "${SCRIPT_FILE}"
                 WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
        set_tests_properties(${NAME} PROPERTIES
                LABELS "compiler;syntax_highlighting;stage0"
                RESOURCE_LOCK syntax_highlighting_parser)
    endfunction()
endif()

enable_testing()
include(${CMAKE_SOURCE_DIR}/cmake/CrexxLinkedRuntime.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/CrexxSmokeTests.cmake)

add_test(NAME rxc_help_no_exe_import COMMAND $<TARGET_FILE:rxc> -h)
set_tests_properties(rxc_help_no_exe_import PROPERTIES
        PASS_REGULAR_EXPRESSION "--no-exe-import"
        LABELS "compiler;imports")

add_executable(test_rxcp_super_combiner test_rxcp_super_combiner.c)
target_link_libraries(test_rxcp_super_combiner PRIVATE rxclib)
target_include_directories(test_rxcp_super_combiner PRIVATE ${CMAKE_SOURCE_DIR}/compiler)
add_test(NAME rxcp_super_combiner COMMAND test_rxcp_super_combiner)
set_tests_properties(rxcp_super_combiner PROPERTIES LABELS "compiler;performance;nr09")

# Compiler flags for optimization levels
# Based on rxcpmain.c: -n is for No Optimising, and it's on by default.
set(COMPILER_NOOPT_FLAG "-n")
set(COMPILER_OPT_FLAG "") # Default is optimized

# Macro to add a compiler test
# NAME: Name of the test
# SOURCE_FILE: Path to the Rexx source file (relative to this directory)
macro(add_crexx_compiler_test NAME SOURCE_FILE)
    # Test 1 (No-Opt)
    add_test(NAME ${NAME}_noopt
             COMMAND crexx_test_driver
             ${CMAKE_CURRENT_SOURCE_DIR}/golden/noopt/${NAME}.rxas
             ${CMAKE_CURRENT_BINARY_DIR}/${NAME}_noopt.rxas
             $<TARGET_FILE:rxc> -i ${CMAKE_BINARY_DIR}/bin ${COMPILER_NOOPT_FLAG} -o ${CMAKE_CURRENT_BINARY_DIR}/${NAME}_noopt ${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE})

    # Test 2 (Opt)
    add_test(NAME ${NAME}_opt
             COMMAND crexx_test_driver
             ${CMAKE_CURRENT_SOURCE_DIR}/golden/opt/${NAME}.rxas
             ${CMAKE_CURRENT_BINARY_DIR}/${NAME}_opt.rxas
             $<TARGET_FILE:rxc> -i ${CMAKE_BINARY_DIR}/bin ${COMPILER_OPT_FLAG} -o ${CMAKE_CURRENT_BINARY_DIR}/${NAME}_opt ${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE})
endmacro()

# Macro to add a compiler warning test
macro(add_crexx_compiler_warning_test NAME SOURCE_FILE GOLDEN_FILE)
    add_test(NAME ${NAME}_warn
             COMMAND crexx_test_driver --warnings
             ${CMAKE_CURRENT_SOURCE_DIR}/golden/warnings/${GOLDEN_FILE}
             ${CMAKE_CURRENT_BINARY_DIR}/${NAME}_warn.txt
             $<TARGET_FILE:rxc> ${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE})
endmacro()

# Macro to add a compiler failure test
macro(add_crexx_compiler_fail_test NAME SOURCE_FILE GOLDEN_FILE)
    add_test(NAME ${NAME}_fail
             COMMAND crexx_test_driver --expect-fail
             ${CMAKE_CURRENT_SOURCE_DIR}/golden/errors/${GOLDEN_FILE}
             ${CMAKE_CURRENT_BINARY_DIR}/${NAME}_fail.txt
             $<TARGET_FILE:rxc> ${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE})
endmacro()

# Macro to add a compiler test with one dependency
# NAME: Name of the test
# SOURCE_FILE: Path to the Rexx source file
# DEP_FILE: Path to the dependency file
macro(add_crexx_compiler_dep_test NAME SOURCE_FILE DEP_FILE)
    # Test 1 (No-Opt)
    add_test(NAME ${NAME}_noopt
             COMMAND crexx_test_driver --copy ${CMAKE_CURRENT_SOURCE_DIR}/${DEP_FILE}
             ${CMAKE_CURRENT_SOURCE_DIR}/golden/noopt/${NAME}.rxas
             ${CMAKE_CURRENT_BINARY_DIR}/${NAME}_noopt.rxas
             $<TARGET_FILE:rxc> ${COMPILER_NOOPT_FLAG} -o ${CMAKE_CURRENT_BINARY_DIR}/${NAME}_noopt ${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE})

    # Test 2 (Opt)
    add_test(NAME ${NAME}_opt
             COMMAND crexx_test_driver --copy ${CMAKE_CURRENT_SOURCE_DIR}/${DEP_FILE}
             ${CMAKE_CURRENT_SOURCE_DIR}/golden/opt/${NAME}.rxas
             ${CMAKE_CURRENT_BINARY_DIR}/${NAME}_opt.rxas
             $<TARGET_FILE:rxc> ${COMPILER_OPT_FLAG} -o ${CMAKE_CURRENT_BINARY_DIR}/${NAME}_opt ${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE})
endmacro()

# Macro to add a parsing test
macro(add_crexx_parsing_test NAME SOURCE_FILE)
    add_test(NAME ${NAME}_parse
             COMMAND crexx_test_driver --ast
             ${CMAKE_CURRENT_SOURCE_DIR}/golden/parsing/${NAME}.txt
             ${CMAKE_CURRENT_BINARY_DIR}/${NAME}_ast.txt
             $<TARGET_FILE:rxc> -dp ${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE})
endmacro()

set(RXAS_DIAG_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/rxas_invalid_diagnostic.cmake")
set(RXAS_DIAG_CONTENT "")
set(RXAS_DIAG_DOLLAR "$")
string(APPEND RXAS_DIAG_CONTENT
       "file(WRITE \"${CMAKE_CURRENT_BINARY_DIR}/rxas_invalid_diagnostic.rxas\" \"main() .locals=1\\n   bogus r0\\n\")\n")
string(APPEND RXAS_DIAG_CONTENT
       "execute_process(\n"
       "  COMMAND \"$<TARGET_FILE:rxas>\" -o \"rxas_invalid_diagnostic\" \"${CMAKE_CURRENT_BINARY_DIR}/rxas_invalid_diagnostic.rxas\"\n"
       "  WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
       "  OUTPUT_VARIABLE out\n"
       "  ERROR_VARIABLE err\n"
       "  RESULT_VARIABLE res\n"
       ")\n")
string(APPEND RXAS_DIAG_CONTENT
       "set(full_out \"${RXAS_DIAG_DOLLAR}{out}${RXAS_DIAG_DOLLAR}{err}\")\n"
       "string(REPLACE \"\\r\\n\" \"\\n\" full_out \"${RXAS_DIAG_DOLLAR}{full_out}\")\n"
       "if(res EQUAL 0)\n"
       "  message(FATAL_ERROR \"Expected rxas to fail on invalid mnemonic, but it succeeded.\\nOutput:\\n${RXAS_DIAG_DOLLAR}{full_out}\")\n"
       "endif()\n"
       "if(NOT full_out MATCHES \"invalid instruction mnemonic\")\n"
       "  message(FATAL_ERROR \"Expected rxas diagnostic was not emitted.\\nOutput:\\n${RXAS_DIAG_DOLLAR}{full_out}\")\n"
       "endif()\n")
file(GENERATE OUTPUT "${RXAS_DIAG_SCRIPT}" CONTENT "${RXAS_DIAG_CONTENT}")

add_test(NAME rxas_invalid_diagnostic
         COMMAND ${CMAKE_COMMAND} -P "${RXAS_DIAG_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

set(RXAS_INVALID_UTF8_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/rxas_invalid_utf8_string.cmake")
set(RXAS_INVALID_UTF8_CONTENT "")
string(APPEND RXAS_INVALID_UTF8_CONTENT
       "file(WRITE \"${CMAKE_CURRENT_BINARY_DIR}/rxas_invalid_utf8_string.rxas\" \"main() .locals=1\\n   load r0,\\\"\\\\xff\\\"\\n\")\n")
string(APPEND RXAS_INVALID_UTF8_CONTENT
       "execute_process(\n"
       "  COMMAND \"$<TARGET_FILE:rxas>\" -o \"rxas_invalid_utf8_string\" \"${CMAKE_CURRENT_BINARY_DIR}/rxas_invalid_utf8_string.rxas\"\n"
       "  WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
       "  OUTPUT_VARIABLE out\n"
       "  ERROR_VARIABLE err\n"
       "  RESULT_VARIABLE res\n"
       ")\n")
string(APPEND RXAS_INVALID_UTF8_CONTENT
       "set(full_out \"${RXAS_DIAG_DOLLAR}{out}${RXAS_DIAG_DOLLAR}{err}\")\n"
       "string(REPLACE \"\\r\\n\" \"\\n\" full_out \"${RXAS_DIAG_DOLLAR}{full_out}\")\n"
       "if(res EQUAL 0)\n"
       "  message(FATAL_ERROR \"Expected rxas to fail on invalid UTF-8 string, but it succeeded.\\nOutput:\\n${RXAS_DIAG_DOLLAR}{full_out}\")\n"
       "endif()\n"
       "if(NOT full_out MATCHES \"string constant is not valid UTF-8\")\n"
       "  message(FATAL_ERROR \"Expected invalid UTF-8 diagnostic was not emitted.\\nOutput:\\n${RXAS_DIAG_DOLLAR}{full_out}\")\n"
       "endif()\n")
file(GENERATE OUTPUT "${RXAS_INVALID_UTF8_SCRIPT}" CONTENT "${RXAS_INVALID_UTF8_CONTENT}")

add_test(NAME rxas_invalid_utf8_string
         COMMAND ${CMAKE_COMMAND} -P "${RXAS_INVALID_UTF8_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

set(RXC_INVALID_UTF8_HEX_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/rxc_invalid_utf8_hex_literal.cmake")
set(RXC_INVALID_UTF8_HEX_CONTENT "")
string(APPEND RXC_INVALID_UTF8_HEX_CONTENT
       "file(WRITE \"${CMAKE_CURRENT_BINARY_DIR}/rxc_invalid_utf8_hex_literal.crexx\" \"options levelb\\n\\nmain: procedure\\n  say 'FFFFFF'x\\n\")\n")
string(APPEND RXC_INVALID_UTF8_HEX_CONTENT
       "execute_process(\n"
       "  COMMAND \"$<TARGET_FILE:rxc>\" -x -o \"rxc_invalid_utf8_hex_literal\" \"${CMAKE_CURRENT_BINARY_DIR}/rxc_invalid_utf8_hex_literal.crexx\"\n"
       "  WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
       "  OUTPUT_VARIABLE rxc_out\n"
       "  ERROR_VARIABLE rxc_err\n"
       "  RESULT_VARIABLE rxc_res\n"
       ")\n"
       "set(full_out \"${RXAS_DIAG_DOLLAR}{rxc_out}${RXAS_DIAG_DOLLAR}{rxc_err}\")\n")
string(APPEND RXC_INVALID_UTF8_HEX_CONTENT
       "string(REPLACE \"\\r\\n\" \"\\n\" full_out \"${RXAS_DIAG_DOLLAR}{full_out}\")\n"
       "if(rxc_res EQUAL 0)\n"
       "  message(FATAL_ERROR \"Expected rxc to reject invalid UTF-8 hex literal, but it succeeded.\\nOutput:\\n${RXAS_DIAG_DOLLAR}{full_out}\")\n"
       "endif()\n"
       "if(NOT full_out MATCHES \"CANNOT_CAST_BINARY\")\n"
       "  message(FATAL_ERROR \"Expected binary-to-text diagnostic was not emitted for source hex literal.\\nOutput:\\n${RXAS_DIAG_DOLLAR}{full_out}\")\n"
       "endif()\n")
file(GENERATE OUTPUT "${RXC_INVALID_UTF8_HEX_SCRIPT}" CONTENT "${RXC_INVALID_UTF8_HEX_CONTENT}")

add_test(NAME rxc_invalid_utf8_hex_literal
         COMMAND ${CMAKE_COMMAND} -P "${RXC_INVALID_UTF8_HEX_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

set(RXC_INVALID_UTF8_BINARY_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/rxc_invalid_utf8_binary_literal.cmake")
set(RXC_INVALID_UTF8_BINARY_CONTENT "")
string(APPEND RXC_INVALID_UTF8_BINARY_CONTENT
       "file(WRITE \"${CMAKE_CURRENT_BINARY_DIR}/rxc_invalid_utf8_binary_literal.crexx\" \"options levelb\\n\\nmain: procedure\\n  say '11111111'b\\n\")\n")
string(APPEND RXC_INVALID_UTF8_BINARY_CONTENT
       "execute_process(\n"
       "  COMMAND \"$<TARGET_FILE:rxc>\" -x -o \"rxc_invalid_utf8_binary_literal\" \"${CMAKE_CURRENT_BINARY_DIR}/rxc_invalid_utf8_binary_literal.crexx\"\n"
       "  WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
       "  OUTPUT_VARIABLE rxc_out\n"
       "  ERROR_VARIABLE rxc_err\n"
       "  RESULT_VARIABLE rxc_res\n"
       ")\n"
       "set(full_out \"${RXAS_DIAG_DOLLAR}{rxc_out}${RXAS_DIAG_DOLLAR}{rxc_err}\")\n")
string(APPEND RXC_INVALID_UTF8_BINARY_CONTENT
       "string(REPLACE \"\\r\\n\" \"\\n\" full_out \"${RXAS_DIAG_DOLLAR}{full_out}\")\n"
       "if(rxc_res EQUAL 0)\n"
       "  message(FATAL_ERROR \"Expected rxc to reject invalid UTF-8 binary literal, but it succeeded.\\nOutput:\\n${RXAS_DIAG_DOLLAR}{full_out}\")\n"
       "endif()\n"
       "if(NOT full_out MATCHES \"CANNOT_CAST_BINARY\")\n"
       "  message(FATAL_ERROR \"Expected binary-to-text diagnostic was not emitted for source binary literal.\\nOutput:\\n${RXAS_DIAG_DOLLAR}{full_out}\")\n"
       "endif()\n")
file(GENERATE OUTPUT "${RXC_INVALID_UTF8_BINARY_SCRIPT}" CONTENT "${RXC_INVALID_UTF8_BINARY_CONTENT}")

add_test(NAME rxc_invalid_utf8_binary_literal
         COMMAND ${CMAKE_COMMAND} -P "${RXC_INVALID_UTF8_BINARY_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

set(RXC_INVALID_BINARY_ASSIGN_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/rxc_invalid_binary_assign_literal.cmake")
set(RXC_INVALID_BINARY_ASSIGN_CONTENT "")
string(APPEND RXC_INVALID_BINARY_ASSIGN_CONTENT
       "file(WRITE \"${CMAKE_CURRENT_BINARY_DIR}/rxc_invalid_binary_assign_literal.crexx\" \"options levelb\\n\\nmain: procedure = .int\\n  x = 'ffff'x\\n  return 0\\n\")\n")
string(APPEND RXC_INVALID_BINARY_ASSIGN_CONTENT
       "execute_process(\n"
       "  COMMAND \"$<TARGET_FILE:rxc>\" -x -o \"rxc_invalid_binary_assign_literal\" \"${CMAKE_CURRENT_BINARY_DIR}/rxc_invalid_binary_assign_literal.crexx\"\n"
       "  WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
       "  OUTPUT_VARIABLE rxc_out\n"
       "  ERROR_VARIABLE rxc_err\n"
       "  RESULT_VARIABLE rxc_res\n"
       ")\n"
       "set(full_out \"${RXAS_DIAG_DOLLAR}{rxc_out}${RXAS_DIAG_DOLLAR}{rxc_err}\")\n"
       "string(REPLACE \"\\r\\n\" \"\\n\" full_out \"${RXAS_DIAG_DOLLAR}{full_out}\")\n"
       "if(rxc_res EQUAL 0)\n"
       "  message(FATAL_ERROR \"Expected rxc to reject untyped invalid UTF-8 byte literal assignment, but it succeeded.\\nOutput:\\n${RXAS_DIAG_DOLLAR}{full_out}\")\n"
       "endif()\n"
       "if(NOT full_out MATCHES \"CANNOT_CAST_BINARY\")\n"
       "  message(FATAL_ERROR \"Expected binary-to-text diagnostic was not emitted for untyped invalid byte literal assignment.\\nOutput:\\n${RXAS_DIAG_DOLLAR}{full_out}\")\n"
       "endif()\n")
file(GENERATE OUTPUT "${RXC_INVALID_BINARY_ASSIGN_SCRIPT}" CONTENT "${RXC_INVALID_BINARY_ASSIGN_CONTENT}")

add_test(NAME rxc_invalid_binary_assign_literal
         COMMAND ${CMAKE_COMMAND} -P "${RXC_INVALID_BINARY_ASSIGN_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

set(RXC_INVALID_BINARY_AS_STRING_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/rxc_invalid_binary_literal_as_string.cmake")
set(RXC_INVALID_BINARY_AS_STRING_CONTENT "")
string(APPEND RXC_INVALID_BINARY_AS_STRING_CONTENT
       "file(WRITE \"${CMAKE_CURRENT_BINARY_DIR}/rxc_invalid_binary_literal_as_string.crexx\" \"options levelb\\n\\nmain: procedure = .int\\n  x = 'ffff'x as .string\\n  say x\\n  return 0\\n\")\n")
string(APPEND RXC_INVALID_BINARY_AS_STRING_CONTENT
       "execute_process(\n"
       "  COMMAND \"$<TARGET_FILE:rxc>\" -x -o \"rxc_invalid_binary_literal_as_string\" \"${CMAKE_CURRENT_BINARY_DIR}/rxc_invalid_binary_literal_as_string.crexx\"\n"
       "  WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
       "  OUTPUT_VARIABLE rxc_out\n"
       "  ERROR_VARIABLE rxc_err\n"
       "  RESULT_VARIABLE rxc_res\n"
       ")\n"
       "set(full_out \"${RXAS_DIAG_DOLLAR}{rxc_out}${RXAS_DIAG_DOLLAR}{rxc_err}\")\n"
       "string(REPLACE \"\\r\\n\" \"\\n\" full_out \"${RXAS_DIAG_DOLLAR}{full_out}\")\n"
       "if(rxc_res EQUAL 0)\n"
       "  message(FATAL_ERROR \"Expected rxc to reject invalid byte literal cast to .string, but it succeeded.\\nOutput:\\n${RXAS_DIAG_DOLLAR}{full_out}\")\n"
       "endif()\n"
       "if(NOT full_out MATCHES \"CANNOT_CAST_BINARY\")\n"
       "  message(FATAL_ERROR \"Expected binary-to-text diagnostic was not emitted for invalid byte literal cast to .string.\\nOutput:\\n${RXAS_DIAG_DOLLAR}{full_out}\")\n"
       "endif()\n")
file(GENERATE OUTPUT "${RXC_INVALID_BINARY_AS_STRING_SCRIPT}" CONTENT "${RXC_INVALID_BINARY_AS_STRING_CONTENT}")

add_test(NAME rxc_invalid_binary_literal_as_string
         COMMAND ${CMAKE_COMMAND} -P "${RXC_INVALID_BINARY_AS_STRING_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

set(RXC_FUNCTION_LOOKUP_DIAG_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/rxc_function_lookup_diagnostic.cmake")
set(RXC_FUNCTION_LOOKUP_DIAG_CONTENT "")
string(APPEND RXC_FUNCTION_LOOKUP_DIAG_CONTENT
       "file(WRITE \"${CMAKE_CURRENT_BINARY_DIR}/rxc_function_lookup_diagnostic.crexx\" [=[options levelb\n\nmain: procedure = .int\n  box = .MethodBox()\n  if length(box.current()) <> 1 then return 1\n  return 0\n\nMethodBox: class\n  *: factory\n    return\n\n  current: method = .string\n    return \"x\"\n]=])\n")
string(APPEND RXC_FUNCTION_LOOKUP_DIAG_CONTENT
       "execute_process(\n"
       "  COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -n -x -o \"rxc_function_lookup_diagnostic\" \"${CMAKE_CURRENT_BINARY_DIR}/rxc_function_lookup_diagnostic.crexx\"\n"
       "  WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
       "  OUTPUT_VARIABLE rxc_out\n"
       "  ERROR_VARIABLE rxc_err\n"
       "  RESULT_VARIABLE rxc_res\n"
       ")\n"
       "set(full_out \"${RXAS_DIAG_DOLLAR}{rxc_out}${RXAS_DIAG_DOLLAR}{rxc_err}\")\n"
       "string(REPLACE \"\\r\\n\" \"\\n\" full_out \"${RXAS_DIAG_DOLLAR}{full_out}\")\n"
       "if(rxc_res EQUAL 0)\n"
       "  message(FATAL_ERROR \"Expected rxc to reject the unresolved length function.\\nOutput:\\n${RXAS_DIAG_DOLLAR}{full_out}\")\n"
       "endif()\n"
       "if(NOT full_out MATCHES [=[#FUNCTION_NOT_FOUND, \"length\"]=])\n"
       "  message(FATAL_ERROR \"Expected unresolved function diagnostic to point at the callee token.\\nOutput:\\n${RXAS_DIAG_DOLLAR}{full_out}\")\n"
       "endif()\n"
       "if(full_out MATCHES [=[#FUNCTION_NOT_FOUND, \"length[(]box[.]current]=])\n"
       "  message(FATAL_ERROR \"Unresolved function diagnostic still includes a truncated nested call span.\\nOutput:\\n${RXAS_DIAG_DOLLAR}{full_out}\")\n"
       "endif()\n")
file(GENERATE OUTPUT "${RXC_FUNCTION_LOOKUP_DIAG_SCRIPT}" CONTENT "${RXC_FUNCTION_LOOKUP_DIAG_CONTENT}")

add_test(NAME rxc_function_lookup_diagnostic
         COMMAND ${CMAKE_COMMAND} -P "${RXC_FUNCTION_LOOKUP_DIAG_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(rxc_function_lookup_diagnostic PROPERTIES LABELS "compiler;diagnostics")

set(RXC_INVALID_BINARY_TO_STRING_RUNTIME_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/rxc_invalid_binary_to_string_runtime.cmake")
set(RXC_INVALID_BINARY_TO_STRING_RUNTIME_CONTENT "")
string(APPEND RXC_INVALID_BINARY_TO_STRING_RUNTIME_CONTENT
       "file(WRITE \"${CMAKE_CURRENT_BINARY_DIR}/rxc_invalid_binary_to_string_runtime.crexx\" \"options levelb\\n\\nmain: procedure = .int\\n  payload = .binary\\n  text = \\\"\\\"\\n  payload = 'ffff'x\\n  text = payload as .string\\n  say text\\n  return 0\\n\")\n")
string(APPEND RXC_INVALID_BINARY_TO_STRING_RUNTIME_CONTENT
       "execute_process(\n"
       "  COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -n -o \"rxc_invalid_binary_to_string_runtime\" \"${CMAKE_CURRENT_BINARY_DIR}/rxc_invalid_binary_to_string_runtime.crexx\"\n"
       "  WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
       "  OUTPUT_VARIABLE rxc_out\n"
       "  ERROR_VARIABLE rxc_err\n"
       "  RESULT_VARIABLE rxc_res\n"
       ")\n"
       "set(full_out \"${RXAS_DIAG_DOLLAR}{rxc_out}${RXAS_DIAG_DOLLAR}{rxc_err}\")\n"
       "if(NOT rxc_res EQUAL 0)\n"
       "  message(FATAL_ERROR \"rxc failed for runtime invalid binary-to-string test.\\nOutput:\\n${RXAS_DIAG_DOLLAR}{full_out}\")\n"
       "endif()\n"
       "execute_process(\n"
       "  COMMAND \"$<TARGET_FILE:rxas>\" -o \"rxc_invalid_binary_to_string_runtime.rxbin\" \"rxc_invalid_binary_to_string_runtime\"\n"
       "  WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
       "  OUTPUT_VARIABLE rxas_out\n"
       "  ERROR_VARIABLE rxas_err\n"
       "  RESULT_VARIABLE rxas_res\n"
       ")\n"
       "set(full_out \"${RXAS_DIAG_DOLLAR}{full_out}${RXAS_DIAG_DOLLAR}{rxas_out}${RXAS_DIAG_DOLLAR}{rxas_err}\")\n"
       "if(NOT rxas_res EQUAL 0)\n"
       "  message(FATAL_ERROR \"rxas failed for runtime invalid binary-to-string test.\\nOutput:\\n${RXAS_DIAG_DOLLAR}{full_out}\")\n"
       "endif()\n"
       "execute_process(\n"
       "  COMMAND \"$<TARGET_FILE:rxvm>\" \"${CMAKE_BINARY_DIR}/bin/library.rxbin\" \"rxc_invalid_binary_to_string_runtime.rxbin\"\n"
       "  WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
       "  OUTPUT_VARIABLE rxvm_out\n"
       "  ERROR_VARIABLE rxvm_err\n"
       "  RESULT_VARIABLE rxvm_res\n"
       ")\n"
       "set(full_out \"${RXAS_DIAG_DOLLAR}{full_out}${RXAS_DIAG_DOLLAR}{rxvm_out}${RXAS_DIAG_DOLLAR}{rxvm_err}\")\n"
       "string(REPLACE \"\\r\\n\" \"\\n\" full_out \"${RXAS_DIAG_DOLLAR}{full_out}\")\n"
       "if(NOT full_out MATCHES \"UNICODE_ERROR\")\n"
       "  message(FATAL_ERROR \"Expected runtime binary-to-string conversion to raise UNICODE_ERROR.\\nOutput:\\n${RXAS_DIAG_DOLLAR}{full_out}\")\n"
       "endif()\n")
file(GENERATE OUTPUT "${RXC_INVALID_BINARY_TO_STRING_RUNTIME_SCRIPT}" CONTENT "${RXC_INVALID_BINARY_TO_STRING_RUNTIME_CONTENT}")

add_test(NAME rxc_invalid_binary_to_string_runtime
         COMMAND ${CMAKE_COMMAND} -P "${RXC_INVALID_BINARY_TO_STRING_RUNTIME_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

set(RXC_BINARY_LITERAL_LOAD_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/rxc_binary_literal_load.cmake")
set(RXC_BINARY_LITERAL_LOAD_CONTENT "")
string(APPEND RXC_BINARY_LITERAL_LOAD_CONTENT
       "execute_process(\n"
       "  COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -n -o \"binary_literal_load_rxas_check\" \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/binary_literal_load.crexx\"\n"
       "  WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
       "  OUTPUT_VARIABLE rxc_out\n"
       "  ERROR_VARIABLE rxc_err\n"
       "  RESULT_VARIABLE rxc_res\n"
       ")\n"
       "set(full_out \"${RXAS_DIAG_DOLLAR}{rxc_out}${RXAS_DIAG_DOLLAR}{rxc_err}\")\n"
       "string(REPLACE \"\\r\\n\" \"\\n\" full_out \"${RXAS_DIAG_DOLLAR}{full_out}\")\n"
       "if(NOT rxc_res EQUAL 0)\n"
       "  message(FATAL_ERROR \"rxc failed for binary literal load check.\\nOutput:\\n${RXAS_DIAG_DOLLAR}{full_out}\")\n"
       "endif()\n"
       "file(READ \"${CMAKE_CURRENT_BINARY_DIR}/binary_literal_load_rxas_check.rxas\" rxas_out)\n"
       "string(REPLACE \"\\r\\n\" \"\\n\" rxas_out \"${RXAS_DIAG_DOLLAR}{rxas_out}\")\n"
       "if(NOT rxas_out MATCHES [=[load r[0-9]+,0x00ff41]=])\n"
       "  message(FATAL_ERROR \"Binary hex literal did not lower to an RXAS binary load.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_out}\")\n"
       "endif()\n"
       "if(NOT rxas_out MATCHES [=[load r[0-9]+,0x414243]=])\n"
       "  message(FATAL_ERROR \"String assigned to .binary did not lower to an RXAS binary load.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_out}\")\n"
       "endif()\n"
       "if(NOT rxas_out MATCHES [=[load r[0-9]+,0xffff]=])\n"
       "  message(FATAL_ERROR \"Explicit binary cast literal did not lower to an RXAS binary load.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_out}\")\n"
       "endif()\n"
       "if(NOT rxas_out MATCHES [=[stobin r[0-9]+]=])\n"
       "  message(FATAL_ERROR \"String cast to .binary did not emit stobin.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_out}\")\n"
       "endif()\n"
       "if(NOT rxas_out MATCHES [=[bintos r[0-9]+]=])\n"
       "  message(FATAL_ERROR \"Binary cast to .string did not emit bintos.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_out}\")\n"
       "endif()\n"
       "execute_process(\n"
       "  COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"binary_literal_load_rxas_check_opt\" \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/binary_literal_load.crexx\"\n"
       "  WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
       "  OUTPUT_VARIABLE rxc_opt_out\n"
       "  ERROR_VARIABLE rxc_opt_err\n"
       "  RESULT_VARIABLE rxc_opt_res\n"
       ")\n"
       "set(full_opt_out \"${RXAS_DIAG_DOLLAR}{rxc_opt_out}${RXAS_DIAG_DOLLAR}{rxc_opt_err}\")\n"
       "if(NOT rxc_opt_res EQUAL 0)\n"
       "  message(FATAL_ERROR \"optimized rxc failed for binary literal load check.\\nOutput:\\n${RXAS_DIAG_DOLLAR}{full_opt_out}\")\n"
       "endif()\n"
       "file(READ \"${CMAKE_CURRENT_BINARY_DIR}/binary_literal_load_rxas_check_opt.rxas\" opt_rxas_out)\n"
       "string(REPLACE \"\\r\\n\" \"\\n\" opt_rxas_out \"${RXAS_DIAG_DOLLAR}{opt_rxas_out}\")\n"
       "if(NOT opt_rxas_out MATCHES [=[load r[0-9]+,0xceb1]=])\n"
       "  message(FATAL_ERROR \"Optimized string-to-binary constant cast did not fold to a binary load.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{opt_rxas_out}\")\n"
       "endif()\n"
       "if(NOT opt_rxas_out MATCHES [=[load r[0-9]+,\"AB\"]=])\n"
       "  message(FATAL_ERROR \"Optimized binary-to-string constant cast did not fold to a string load.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{opt_rxas_out}\")\n"
       "endif()\n"
       "if(opt_rxas_out MATCHES [=[0x3078]=])\n"
       "  message(FATAL_ERROR \"Optimized binary constants were re-encoded as ASCII hex text.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{opt_rxas_out}\")\n"
       "endif()\n")
file(GENERATE OUTPUT "${RXC_BINARY_LITERAL_LOAD_SCRIPT}" CONTENT "${RXC_BINARY_LITERAL_LOAD_CONTENT}")

add_test(NAME rxc_binary_literal_load
         COMMAND ${CMAKE_COMMAND} -P "${RXC_BINARY_LITERAL_LOAD_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

set(RXC_BINARY_MEMORY_SOURCE_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/rxc_binary_memory_source.cmake")
set(RXC_BINARY_MEMORY_SOURCE_CONTENT "")
string(APPEND RXC_BINARY_MEMORY_SOURCE_CONTENT
       "execute_process(\n"
       "  COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -n -o \"binary_memory_source_rxas_check\" \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/binary_memory_source.crexx\"\n"
       "  WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
       "  OUTPUT_VARIABLE rxc_out\n"
       "  ERROR_VARIABLE rxc_err\n"
       "  RESULT_VARIABLE rxc_res\n"
       ")\n"
       "set(full_out \"${RXAS_DIAG_DOLLAR}{rxc_out}${RXAS_DIAG_DOLLAR}{rxc_err}\")\n"
       "string(REPLACE \"\\r\\n\" \"\\n\" full_out \"${RXAS_DIAG_DOLLAR}{full_out}\")\n"
       "if(NOT rxc_res EQUAL 0)\n"
       "  message(FATAL_ERROR \"rxc failed for binary memory source check.\\nOutput:\\n${RXAS_DIAG_DOLLAR}{full_out}\")\n"
       "endif()\n"
       "file(READ \"${CMAKE_CURRENT_BINARY_DIR}/binary_memory_source_rxas_check.rxas\" rxas_out)\n"
       "string(REPLACE \"\\r\\n\" \"\\n\" rxas_out \"${RXAS_DIAG_DOLLAR}{rxas_out}\")\n"
       "foreach(_opcode blen bgetu32 bgetu8 bsetu16 bgetu16 bgetf64 bgeti64 bseti64 bgetf32 bsetf32 bresize bcopy bintos bgets bsets bcmpb bcmps stod dtos)\n"
       "  if(NOT rxas_out MATCHES \"${RXAS_DIAG_DOLLAR}{_opcode}\")\n"
       "    message(FATAL_ERROR \"Binary memory source did not emit ${RXAS_DIAG_DOLLAR}{_opcode}.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_out}\")\n"
       "  endif()\n"
       "endforeach()\n"
       "if(NOT rxas_out MATCHES [=[bgetu32 [^\\n]*,0x78563412fe7f0000000000000000f03f4f4b]=])\n"
       "  message(FATAL_ERROR \"Binary memory source did not read a fixed-width field directly from a binary constant.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_out}\")\n"
       "endif()\n"
       "foreach(_opcode bcheckrange getbinpos setbinpos bslice)\n"
       "  if(rxas_out MATCHES \"${RXAS_DIAG_DOLLAR}{_opcode}\")\n"
       "    message(FATAL_ERROR \"Binary memory source emitted legacy cursor/slice opcode ${RXAS_DIAG_DOLLAR}{_opcode}.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_out}\")\n"
       "  endif()\n"
       "endforeach()\n"
       "if(NOT rxas_out MATCHES [=[load r[0-9]+,4]=])\n"
       "  message(FATAL_ERROR \"Binary memory <sizeof..type> did not lower to a literal width load.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_out}\")\n"
       "endif()\n"
       "if(NOT rxas_out MATCHES [=[load r[0-9]+,8]=])\n"
       "  message(FATAL_ERROR \"Binary memory <sizeof..int> did not lower to an 8-byte literal width load.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_out}\")\n"
       "endif()\n")
file(GENERATE OUTPUT "${RXC_BINARY_MEMORY_SOURCE_SCRIPT}" CONTENT "${RXC_BINARY_MEMORY_SOURCE_CONTENT}")

add_test(NAME rxc_binary_memory_source
         COMMAND ${CMAKE_COMMAND} -P "${RXC_BINARY_MEMORY_SOURCE_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

set(RXC_BINARY_MEMORY_PACKED_LOOKUP_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/rxc_binary_memory_packed_lookup.cmake")
set(RXC_BINARY_MEMORY_PACKED_LOOKUP_CONTENT "")
string(APPEND RXC_BINARY_MEMORY_PACKED_LOOKUP_CONTENT
       "execute_process(\n"
       "  COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -n -o \"binary_memory_packed_lookup_rxas_check\" \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/binary_memory_packed_lookup.crexx\"\n"
       "  WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
       "  OUTPUT_VARIABLE rxc_out\n"
       "  ERROR_VARIABLE rxc_err\n"
       "  RESULT_VARIABLE rxc_res\n"
       ")\n"
       "set(full_out \"${RXAS_DIAG_DOLLAR}{rxc_out}${RXAS_DIAG_DOLLAR}{rxc_err}\")\n"
       "string(REPLACE \"\\r\\n\" \"\\n\" full_out \"${RXAS_DIAG_DOLLAR}{full_out}\")\n"
       "if(NOT rxc_res EQUAL 0)\n"
       "  message(FATAL_ERROR \"rxc failed for packed binary lookup check.\\nOutput:\\n${RXAS_DIAG_DOLLAR}{full_out}\")\n"
       "endif()\n"
       "file(READ \"${CMAKE_CURRENT_BINARY_DIR}/binary_memory_packed_lookup_rxas_check.rxas\" rxas_out)\n"
       "string(REPLACE \"\\r\\n\" \"\\n\" rxas_out \"${RXAS_DIAG_DOLLAR}{rxas_out}\")\n"
       "foreach(_opcode blen bgetu32 bgetu16 bcmpb)\n"
       "  if(NOT rxas_out MATCHES \"${RXAS_DIAG_DOLLAR}{_opcode}\")\n"
       "    message(FATAL_ERROR \"Packed binary lookup did not emit ${RXAS_DIAG_DOLLAR}{_opcode}.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_out}\")\n"
       "  endif()\n"
       "endforeach()\n"
       "foreach(_opcode bslice bintos)\n"
       "  if(rxas_out MATCHES \"${RXAS_DIAG_DOLLAR}{_opcode}\")\n"
       "    message(FATAL_ERROR \"Packed binary lookup materialized a key slice via ${RXAS_DIAG_DOLLAR}{_opcode}.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_out}\")\n"
       "  endif()\n"
       "endforeach()\n")
file(GENERATE OUTPUT "${RXC_BINARY_MEMORY_PACKED_LOOKUP_SCRIPT}" CONTENT "${RXC_BINARY_MEMORY_PACKED_LOOKUP_CONTENT}")

add_test(NAME rxc_binary_memory_packed_lookup
         COMMAND ${CMAKE_COMMAND} -P "${RXC_BINARY_MEMORY_PACKED_LOOKUP_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

set(RXC_CONSTANTS_LEVELB_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/rxc_constants_levelb.cmake")
set(RXC_CONSTANTS_LEVELB_CONTENT "")
foreach(_mode IN ITEMS noopt opt)
    if(_mode STREQUAL "noopt")
        set(_flag "-n")
    else()
        set(_flag "")
    endif()
    string(APPEND RXC_CONSTANTS_LEVELB_CONTENT
           "execute_process(\n"
           "  COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" ${_flag} -o \"constants_levelb_${_mode}\" \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/constants_levelb.crexx\"\n"
           "  WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
           "  OUTPUT_VARIABLE rxc_${_mode}_out\n"
           "  ERROR_VARIABLE rxc_${_mode}_err\n"
           "  RESULT_VARIABLE rxc_${_mode}_res\n"
           ")\n"
           "set(full_${_mode}_out \"${RXAS_DIAG_DOLLAR}{rxc_${_mode}_out}${RXAS_DIAG_DOLLAR}{rxc_${_mode}_err}\")\n"
           "string(REPLACE \"\\r\\n\" \"\\n\" full_${_mode}_out \"${RXAS_DIAG_DOLLAR}{full_${_mode}_out}\")\n"
           "if(NOT rxc_${_mode}_res EQUAL 0)\n"
           "  message(FATAL_ERROR \"rxc ${_mode} failed for constants_levelb.\\nOutput:\\n${RXAS_DIAG_DOLLAR}{full_${_mode}_out}\")\n"
           "endif()\n"
           "file(READ \"${CMAKE_CURRENT_BINARY_DIR}/constants_levelb_${_mode}.rxas\" rxas_${_mode}_out)\n"
           "string(REPLACE \"\\r\\n\" \"\\n\" rxas_${_mode}_out \"${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "if(NOT rxas_${_mode}_out MATCHES [=[settp r[0-9]+,196608]=])\n"
           "  message(FATAL_ERROR \"Named constants did not fold to an assembler immediate in ${_mode}.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "endif()\n"
           "if(NOT rxas_${_mode}_out MATCHES [=[(load r[0-9]+,196608|ret 196608)]=])\n"
           "  message(FATAL_ERROR \"Named constants did not fold to a normal integer immediate in ${_mode}.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "endif()\n")
endforeach()
file(GENERATE OUTPUT "${RXC_CONSTANTS_LEVELB_SCRIPT}" CONTENT "${RXC_CONSTANTS_LEVELB_CONTENT}")

add_test(NAME rxc_constants_levelb
         COMMAND ${CMAKE_COMMAND} -P "${RXC_CONSTANTS_LEVELB_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

set(RXC_REGISTER_FLAG_VIEWS_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/rxc_register_flag_views.cmake")
set(RXC_REGISTER_FLAG_VIEWS_CONTENT "")
foreach(_mode IN ITEMS noopt opt)
    if(_mode STREQUAL "noopt")
        set(_flag "-n")
    else()
        set(_flag "")
    endif()
    string(APPEND RXC_REGISTER_FLAG_VIEWS_CONTENT
           "execute_process(\n"
           "  COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" ${_flag} -o \"register_flag_views_${_mode}\" \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/class_attr_register_views.crexx\"\n"
           "  WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
           "  OUTPUT_VARIABLE rxc_${_mode}_out\n"
           "  ERROR_VARIABLE rxc_${_mode}_err\n"
           "  RESULT_VARIABLE rxc_${_mode}_res\n"
           ")\n"
           "set(full_${_mode}_out \"${RXAS_DIAG_DOLLAR}{rxc_${_mode}_out}${RXAS_DIAG_DOLLAR}{rxc_${_mode}_err}\")\n"
           "string(REPLACE \"\\r\\n\" \"\\n\" full_${_mode}_out \"${RXAS_DIAG_DOLLAR}{full_${_mode}_out}\")\n"
           "if(NOT rxc_${_mode}_res EQUAL 0)\n"
           "  message(FATAL_ERROR \"rxc ${_mode} failed for register flag views.\\nOutput:\\n${RXAS_DIAG_DOLLAR}{full_${_mode}_out}\")\n"
           "endif()\n"
           "file(READ \"${CMAKE_CURRENT_BINARY_DIR}/register_flag_views_${_mode}.rxas\" rxas_${_mode}_out)\n"
           "string(REPLACE \"\\r\\n\" \"\\n\" rxas_${_mode}_out \"${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "if(NOT rxas_${_mode}_out MATCHES [=[getandtp r[0-9]+,a1,65280]=])\n"
           "  message(FATAL_ERROR \"Compiler flag view did not lower to masked getandtp in ${_mode}.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "endif()\n"
           "if(NOT rxas_${_mode}_out MATCHES [=[bcopy r[0-9]+,(a1|r[0-9]+)]=])\n"
           "  message(FATAL_ERROR \"Binary register view did not lower through bcopy in ${_mode}.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "endif()\n"
           "if(rxas_${_mode}_out MATCHES [=[(^|\\n)[ \\t]*acopy[ \\t]]=])\n"
           "  message(FATAL_ERROR \"Register view codegen emitted acopy in ${_mode}.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "endif()\n"
           "if(NOT rxas_${_mode}_out MATCHES [=[settpmask a1,r[0-9]+,16711680]=])\n"
           "  message(FATAL_ERROR \"Library flag view write did not lower to settpmask in ${_mode}.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "endif()\n"
           "if(NOT rxas_${_mode}_out MATCHES [=[settpmask a1,r[0-9]+,2147418112]=])\n"
           "  message(FATAL_ERROR \"Public flag view write did not lower to source-writable settpmask in ${_mode}.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "endif()\n")
endforeach()
file(GENERATE OUTPUT "${RXC_REGISTER_FLAG_VIEWS_SCRIPT}" CONTENT "${RXC_REGISTER_FLAG_VIEWS_CONTENT}")

add_test(NAME rxc_register_flag_views
         COMMAND ${CMAKE_COMMAND} -P "${RXC_REGISTER_FLAG_VIEWS_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

set(RXC_SCOPED_REGISTER_REUSE_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/rxc_scoped_register_reuse.cmake")
set(RXC_SCOPED_REGISTER_REUSE_CONTENT "")
foreach(_mode IN ITEMS noopt)
    if(_mode STREQUAL "noopt")
        set(_flag "-n")
    else()
        set(_flag "")
    endif()
    string(APPEND RXC_SCOPED_REGISTER_REUSE_CONTENT
           "execute_process(\n"
           "  COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" ${_flag} -o \"scoped_register_reuse_${_mode}\" \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/scoped_register_reuse.crexx\"\n"
           "  WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
           "  OUTPUT_VARIABLE rxc_${_mode}_out\n"
           "  ERROR_VARIABLE rxc_${_mode}_err\n"
           "  RESULT_VARIABLE rxc_${_mode}_res\n"
           ")\n"
           "set(full_${_mode}_out \"${RXAS_DIAG_DOLLAR}{rxc_${_mode}_out}${RXAS_DIAG_DOLLAR}{rxc_${_mode}_err}\")\n"
           "string(REPLACE \"\\r\\n\" \"\\n\" full_${_mode}_out \"${RXAS_DIAG_DOLLAR}{full_${_mode}_out}\")\n"
           "if(NOT rxc_${_mode}_res EQUAL 0)\n"
           "  message(FATAL_ERROR \"rxc ${_mode} failed for scoped register reuse.\\nOutput:\\n${RXAS_DIAG_DOLLAR}{full_${_mode}_out}\")\n"
           "endif()\n"
           "file(READ \"${CMAKE_CURRENT_BINARY_DIR}/scoped_register_reuse_${_mode}.rxas\" rxas_${_mode}_out)\n"
           "string(REPLACE \"\\r\\n\" \"\\n\" rxas_${_mode}_out \"${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "string(REGEX MATCH [=[\\.meta \"scoped_register_reuse.main.left\"=\"b\" \"\\.int\" r([0-9]+)]=] left_match \"${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "if(NOT left_match)\n"
           "  message(FATAL_ERROR \"Left scoped local metadata missing in ${_mode}.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "endif()\n"
           "set(left_reg \"${RXAS_DIAG_DOLLAR}{CMAKE_MATCH_1}\")\n"
           "string(REGEX MATCH [=[\\.meta \"scoped_register_reuse.main.right\"=\"b\" \"\\.int\" r([0-9]+)]=] right_match \"${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "if(NOT right_match)\n"
           "  message(FATAL_ERROR \"Right scoped local metadata missing in ${_mode}.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "endif()\n"
           "set(right_reg \"${RXAS_DIAG_DOLLAR}{CMAKE_MATCH_1}\")\n"
           "if(NOT left_reg STREQUAL right_reg)\n"
           "  message(FATAL_ERROR \"Scoped scalar locals did not reuse a register in ${_mode}: left=r${RXAS_DIAG_DOLLAR}{left_reg}, right=r${RXAS_DIAG_DOLLAR}{right_reg}.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "endif()\n"
           "string(REGEX MATCH [=[\\.meta \"scoped_register_reuse.main.object_left\"=\"b\" \"\\.box\" r([0-9]+)]=] object_left_match \"${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "if(NOT object_left_match)\n"
           "  message(FATAL_ERROR \"Object-left scoped local metadata missing in ${_mode}.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "endif()\n"
           "set(object_left_reg \"${RXAS_DIAG_DOLLAR}{CMAKE_MATCH_1}\")\n"
           "string(REGEX MATCH [=[\\.meta \"scoped_register_reuse.main.object_right\"=\"b\" \"\\.box\" r([0-9]+)]=] object_right_match \"${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "if(NOT object_right_match)\n"
           "  message(FATAL_ERROR \"Object-right scoped local metadata missing in ${_mode}.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "endif()\n"
           "set(object_right_reg \"${RXAS_DIAG_DOLLAR}{CMAKE_MATCH_1}\")\n"
           "if(NOT object_left_reg STREQUAL object_right_reg)\n"
           "  message(FATAL_ERROR \"Scoped object locals did not reuse a register in ${_mode}: object_left=r${RXAS_DIAG_DOLLAR}{object_left_reg}, object_right=r${RXAS_DIAG_DOLLAR}{object_right_reg}.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "endif()\n"
           "string(REGEX MATCH [=[\\.meta \"scoped_register_reuse.main.reference_left\"=\"b\" \"reference \\.int\" r([0-9]+)]=] reference_left_match \"${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "if(NOT reference_left_match)\n"
           "  message(FATAL_ERROR \"Reference-left scoped local metadata missing in ${_mode}.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "endif()\n"
           "set(reference_left_reg \"${RXAS_DIAG_DOLLAR}{CMAKE_MATCH_1}\")\n"
           "string(REGEX MATCH [=[\\.meta \"scoped_register_reuse.main.reference_right\"=\"b\" \"reference \\.int\" r([0-9]+)]=] reference_right_match \"${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "if(NOT reference_right_match)\n"
           "  message(FATAL_ERROR \"Reference-right scoped local metadata missing in ${_mode}.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "endif()\n"
           "set(reference_right_reg \"${RXAS_DIAG_DOLLAR}{CMAKE_MATCH_1}\")\n"
           "if(NOT reference_left_reg STREQUAL reference_right_reg)\n"
           "  message(FATAL_ERROR \"Scoped reference locals did not reuse a register in ${_mode}: reference_left=r${RXAS_DIAG_DOLLAR}{reference_left_reg}, reference_right=r${RXAS_DIAG_DOLLAR}{reference_right_reg}.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "endif()\n"
           "string(REGEX MATCH [=[\\.meta \"scoped_register_reuse.main.binary_left\"=\"b\" \"\\.binary\" r([0-9]+)]=] binary_left_match \"${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "if(NOT binary_left_match)\n"
           "  message(FATAL_ERROR \"Binary-left scoped local metadata missing in ${_mode}.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "endif()\n"
           "set(binary_left_reg \"${RXAS_DIAG_DOLLAR}{CMAKE_MATCH_1}\")\n"
           "string(REGEX MATCH [=[\\.meta \"scoped_register_reuse.main.binary_right\"=\"b\" \"\\.binary\" r([0-9]+)]=] binary_right_match \"${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "if(NOT binary_right_match)\n"
           "  message(FATAL_ERROR \"Binary-right scoped local metadata missing in ${_mode}.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "endif()\n"
           "set(binary_right_reg \"${RXAS_DIAG_DOLLAR}{CMAKE_MATCH_1}\")\n"
           "if(NOT binary_left_reg STREQUAL binary_right_reg)\n"
           "  message(FATAL_ERROR \"Scoped binary locals did not reuse a register in ${_mode}: binary_left=r${RXAS_DIAG_DOLLAR}{binary_left_reg}, binary_right=r${RXAS_DIAG_DOLLAR}{binary_right_reg}.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "endif()\n"
           "string(REGEX MATCH [=[\\.meta \"scoped_register_reuse.main.array_left\"=\"b\" \"\\.int\\[2\\]\" r([0-9]+)]=] array_left_match \"${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "if(NOT array_left_match)\n"
           "  message(FATAL_ERROR \"Array-left scoped local metadata missing in ${_mode}.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "endif()\n"
           "set(array_left_reg \"${RXAS_DIAG_DOLLAR}{CMAKE_MATCH_1}\")\n"
           "string(REGEX MATCH [=[\\.meta \"scoped_register_reuse.main.array_right\"=\"b\" \"\\.int\\[2\\]\" r([0-9]+)]=] array_right_match \"${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "if(NOT array_right_match)\n"
           "  message(FATAL_ERROR \"Array-right scoped local metadata missing in ${_mode}.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "endif()\n"
           "set(array_right_reg \"${RXAS_DIAG_DOLLAR}{CMAKE_MATCH_1}\")\n"
           "if(NOT array_left_reg STREQUAL array_right_reg)\n"
           "  message(FATAL_ERROR \"Scoped array locals did not reuse a register in ${_mode}: array_left=r${RXAS_DIAG_DOLLAR}{array_left_reg}, array_right=r${RXAS_DIAG_DOLLAR}{array_right_reg}.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "endif()\n"
           "string(REGEX MATCHALL [=[\\.meta \"scoped_register_reuse.main.left\"]=] left_meta_entries \"${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "list(LENGTH left_meta_entries left_meta_count)\n"
           "if(NOT left_meta_count EQUAL 2)\n"
           "  message(FATAL_ERROR \"Expected definition and clear metadata records for left scoped local in ${_mode}.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "endif()\n")
endforeach()
string(APPEND RXC_SCOPED_REGISTER_REUSE_CONTENT
       "function(assert_scope_lifetime symbol reg expected)\n"
       "  set(definition \".meta \\\"scoped_register_reuse.main.${RXAS_DIAG_DOLLAR}{symbol}\\\"=\\\"b\\\"\")\n"
       "  set(clear \"\\n   .meta \\\"scoped_register_reuse.main.${RXAS_DIAG_DOLLAR}{symbol}\\\"\\n\")\n"
       "  string(FIND \"${RXAS_DIAG_DOLLAR}{rxas_noopt_out}\" \"${RXAS_DIAG_DOLLAR}{definition}\" scope_start)\n"
       "  string(FIND \"${RXAS_DIAG_DOLLAR}{rxas_noopt_out}\" \"${RXAS_DIAG_DOLLAR}{clear}\" scope_end)\n"
       "  if(scope_start EQUAL -1 OR scope_end EQUAL -1 OR NOT scope_end GREATER scope_start)\n"
       "    message(FATAL_ERROR \"Could not isolate scoped lifetime block for ${RXAS_DIAG_DOLLAR}{symbol}.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_noopt_out}\")\n"
       "  endif()\n"
       "  math(EXPR scope_length \"${RXAS_DIAG_DOLLAR}{scope_end} - ${RXAS_DIAG_DOLLAR}{scope_start}\")\n"
       "  string(SUBSTRING \"${RXAS_DIAG_DOLLAR}{rxas_noopt_out}\" ${RXAS_DIAG_DOLLAR}{scope_start} ${RXAS_DIAG_DOLLAR}{scope_length} scope_block)\n"
       "  string(FIND \"${RXAS_DIAG_DOLLAR}{scope_block}\" \"endlife r${RXAS_DIAG_DOLLAR}{reg}\" endlife_pos)\n"
       "  if(expected STREQUAL \"absent\" AND NOT endlife_pos EQUAL -1)\n"
       "    message(FATAL_ERROR \"Reference-lifetime-free scalar ${RXAS_DIAG_DOLLAR}{symbol} retained endlife.\\nBlock:\\n${RXAS_DIAG_DOLLAR}{scope_block}\")\n"
       "  elseif(expected STREQUAL \"present\" AND endlife_pos EQUAL -1)\n"
       "    message(FATAL_ERROR \"Reference-capable scoped local ${RXAS_DIAG_DOLLAR}{symbol} lost endlife.\\nBlock:\\n${RXAS_DIAG_DOLLAR}{scope_block}\")\n"
       "  endif()\n"
       "endfunction()\n"
       "assert_scope_lifetime(left ${RXAS_DIAG_DOLLAR}{left_reg} absent)\n"
       "assert_scope_lifetime(right ${RXAS_DIAG_DOLLAR}{right_reg} absent)\n"
       "assert_scope_lifetime(object_left ${RXAS_DIAG_DOLLAR}{object_left_reg} present)\n"
       "assert_scope_lifetime(object_right ${RXAS_DIAG_DOLLAR}{object_right_reg} present)\n"
       "assert_scope_lifetime(reference_left ${RXAS_DIAG_DOLLAR}{reference_left_reg} present)\n"
       "assert_scope_lifetime(reference_right ${RXAS_DIAG_DOLLAR}{reference_right_reg} present)\n"
       "assert_scope_lifetime(binary_left ${RXAS_DIAG_DOLLAR}{binary_left_reg} present)\n"
       "assert_scope_lifetime(binary_right ${RXAS_DIAG_DOLLAR}{binary_right_reg} present)\n"
       "assert_scope_lifetime(array_left ${RXAS_DIAG_DOLLAR}{array_left_reg} present)\n"
       "assert_scope_lifetime(array_right ${RXAS_DIAG_DOLLAR}{array_right_reg} present)\n")
file(GENERATE OUTPUT "${RXC_SCOPED_REGISTER_REUSE_SCRIPT}" CONTENT "${RXC_SCOPED_REGISTER_REUSE_CONTENT}")

add_test(NAME rxc_scoped_register_reuse
         COMMAND ${CMAKE_COMMAND} -P "${RXC_SCOPED_REGISTER_REUSE_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(rxc_scoped_register_reuse PROPERTIES LABELS "compiler;registers")

set(RXC_INLINE_BYVALUE_ARG_REUSE_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/rxc_inline_byvalue_arg_reuse.cmake")
set(RXC_INLINE_BYVALUE_ARG_REUSE_CONTENT "")
string(APPEND RXC_INLINE_BYVALUE_ARG_REUSE_CONTENT
       "execute_process(\n"
       "  COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"inline_test_byvalue_arg_reuse_diag\" \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/inline_test_byvalue_arg_reuse.crexx\"\n"
       "  WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
       "  OUTPUT_VARIABLE rxc_out\n"
       "  ERROR_VARIABLE rxc_err\n"
       "  RESULT_VARIABLE rxc_res\n"
       ")\n"
       "set(full_out \"${RXAS_DIAG_DOLLAR}{rxc_out}${RXAS_DIAG_DOLLAR}{rxc_err}\")\n"
       "string(REPLACE \"\\r\\n\" \"\\n\" full_out \"${RXAS_DIAG_DOLLAR}{full_out}\")\n"
       "if(NOT rxc_res EQUAL 0)\n"
       "  message(FATAL_ERROR \"rxc failed for inline by-value arg reuse.\\nOutput:\\n${RXAS_DIAG_DOLLAR}{full_out}\")\n"
       "endif()\n"
       "file(READ \"${CMAKE_CURRENT_BINARY_DIR}/inline_test_byvalue_arg_reuse_diag.rxas\" rxas_out)\n"
       "string(REPLACE \"\\r\\n\" \"\\n\" rxas_out \"${RXAS_DIAG_DOLLAR}{rxas_out}\")\n"
       "string(REGEX MATCH [=[main\\(\\) \\.locals=([0-9]+)]=] main_locals_match \"${RXAS_DIAG_DOLLAR}{rxas_out}\")\n"
       "if(NOT main_locals_match)\n"
       "  message(FATAL_ERROR \"main locals record missing for inline by-value arg reuse.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_out}\")\n"
       "endif()\n"
       "set(main_locals \"${RXAS_DIAG_DOLLAR}{CMAKE_MATCH_1}\")\n"
       "if(main_locals GREATER 12)\n"
       "  message(FATAL_ERROR \"Inlined by-value formals were not recycled; main locals=${RXAS_DIAG_DOLLAR}{main_locals}.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_out}\")\n"
       "endif()\n")
file(GENERATE OUTPUT "${RXC_INLINE_BYVALUE_ARG_REUSE_SCRIPT}" CONTENT "${RXC_INLINE_BYVALUE_ARG_REUSE_CONTENT}")

add_test(NAME rxc_inline_byvalue_arg_reuse
         COMMAND ${CMAKE_COMMAND} -P "${RXC_INLINE_BYVALUE_ARG_REUSE_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(rxc_inline_byvalue_arg_reuse PROPERTIES LABELS "compiler;registers")

# Robustness tests
macro(add_crexx_robustness_test NAME MAIN_FILE)
    # The list of dependencies are passed as additional arguments
    set(DEPS ${ARGN})
    set(DEP_BINS "")
    
    # Generate a CMake script for this test to replace bash -c
    set(SCRIPT_FILE "${CMAKE_CURRENT_BINARY_DIR}/run_robustness_${NAME}.cmake")
    set(SCRIPT_CONTENT "")
    
    # Build main file
    string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/robustness\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"${NAME}\" \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/robustness/${MAIN_FILE}\" RESULT_VARIABLE res)\n")
    string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed on main\")\nendif()\n")
    string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"${NAME}.rxbin\" \"${NAME}\" RESULT_VARIABLE res)\n")
    string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed on main\")\nendif()\n")

    foreach(DEP ${DEPS})
        # Extract lib name from filename
        get_filename_component(DEP_NAME ${DEP} NAME_WE)
        string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/robustness\" -o \"${DEP_NAME}\" \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/robustness/${DEP}\" RESULT_VARIABLE res)\n")
        string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed on ${DEP}\")\nendif()\n")
        string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"${DEP_NAME}.rxbin\" \"${DEP_NAME}\" RESULT_VARIABLE res)\n")
        string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed on ${DEP}\")\nendif()\n")
        
        # Build list for rxvm
        set(DEP_BINS "${DEP_BINS};${DEP_NAME}.rxbin")
    endforeach()

    # Run rxvm and compare output
    string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxvm>\" \"${NAME}.rxbin\" ${DEP_BINS} \"${CMAKE_BINARY_DIR}/bin/library.rxbin\" OUTPUT_VARIABLE vm_out RESULT_VARIABLE res)\n")
    string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxvm failed\")\nendif()\n")
    
    # Read golden file
    string(APPEND SCRIPT_CONTENT "file(READ \"${CMAKE_CURRENT_SOURCE_DIR}/golden/run/${NAME}.out\" golden_out)\n")
    # Normalize line endings
    string(APPEND SCRIPT_CONTENT "string(REPLACE \"\\r\\n\" \"\\n\" vm_out \"\${vm_out}\")\n")
    string(APPEND SCRIPT_CONTENT "string(REPLACE \"\\r\\n\" \"\\n\" golden_out \"\${golden_out}\")\n")
    
    string(APPEND SCRIPT_CONTENT "if(NOT vm_out STREQUAL golden_out)\n  message(FATAL_ERROR \"Output mismatch: \\nExpected:\\n\${golden_out}\\nGot:\\n\${vm_out}\")\nendif()\n")

    file(GENERATE OUTPUT "${SCRIPT_FILE}" CONTENT "${SCRIPT_CONTENT}")

    add_test(NAME robustness_${NAME}
             COMMAND ${CMAKE_COMMAND} -P "${SCRIPT_FILE}"
             WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endmacro()

add_crexx_robustness_test(set1_main set1_main.crexx set1_lib.crexx)
add_crexx_robustness_test(set2_main set2_main.crexx set2_lib.crexx)
add_crexx_robustness_test(set3_main set3_main.crexx set3_lib.crexx)
add_crexx_robustness_test(set4_main set4_main.crexx set4_mid.crexx set4_base.crexx)
add_crexx_robustness_test(set6_main set6_main.crexx Counter.crexx CounterIterator.crexx)

# Set 5 - Encapsulation (Expected to fail compilation)
set(SCRIPT_FILE_SET5 "${CMAKE_CURRENT_BINARY_DIR}/run_robustness_set5.cmake")
set(SCRIPT_CONTENT_SET5
        "include([=[${CMAKE_SOURCE_DIR}/cmake/CrexxTestAssertions.cmake]=])\n")
string(APPEND SCRIPT_CONTENT_SET5
        "execute_process(\n"
        "  COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/robustness\" -o \"set5_main\" \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/robustness/set5_main.crexx\"\n"
        "  RESULT_VARIABLE res\n"
        "  OUTPUT_VARIABLE out\n"
        "  ERROR_VARIABLE err)\n"
        "set(combined \"\${out}\${err}\")\n"
        "if(NOT \"\${combined}\" STREQUAL \"\")\n  message(\"\${combined}\")\nendif()\n"
        "crexx_assert_expected_failure(\n"
        "  NAME robustness_set5_encapsulation\n"
        "  RESULT \"\${res}\"\n"
        "  EXPECTED_EXIT_CODE 2\n"
        "  OUTPUT \"\${combined}\"\n"
        "  DESCRIPTION \"private function access was rejected\"\n"
        "  REQUIRED_OUTPUT_REGEX [=[#FUNCTION_NOT_FOUND, \"private_func\"]=])\n")
file(GENERATE OUTPUT "${SCRIPT_FILE_SET5}" CONTENT "${SCRIPT_CONTENT_SET5}")

add_test(NAME robustness_set5_encapsulation
         COMMAND ${CMAKE_COMMAND} -P "${SCRIPT_FILE_SET5}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# Example tests
add_crexx_compiler_test(01_assign rexx_src/01_assign.crexx)
add_crexx_compiler_test(02_flow rexx_src/02_flow.crexx)
add_crexx_compiler_test(03_calc rexx_src/03_calc.crexx)
add_crexx_compiler_test(04_literals rexx_src/04_literals.crexx)
add_crexx_compiler_test(05_ops_common rexx_src/05_ops_common.crexx)
add_crexx_compiler_test(05_ops_classic rexx_src/05_ops_classic.crexx)
add_crexx_compiler_fail_test(err_xor_common rexx_src/err_xor_common.crexx err_xor_common.txt)
add_crexx_compiler_fail_test(int64_literal_overflow rexx_src/int64_literal_overflow.crexx int64_literal_overflow.txt)
add_crexx_compiler_fail_test(int64_fold_overflow rexx_src/int64_fold_overflow.crexx int64_fold_overflow.txt)
add_crexx_compiler_fail_test(int64_fold_division_zero rexx_src/int64_fold_division_zero.crexx int64_fold_division_zero.txt)
add_crexx_compiler_test(strict_compare_codegen rexx_src/strict_compare_codegen.crexx)
add_crexx_compiler_test(address_expose rexx_src/address_expose.crexx)
add_crexx_compiler_test(div_ops_common rexx_src/div_ops_common.crexx)
add_crexx_compiler_test(div_ops_classic rexx_src/div_ops_classic.crexx)
add_crexx_compiler_test(06_logic rexx_src/06_logic.crexx)
add_crexx_compiler_test(named_operators rexx_src/named_operators.crexx)
add_crexx_compiler_test(07_loops rexx_src/07_loops.crexx)
add_crexx_compiler_test(08_select rexx_src/08_select.crexx)
add_crexx_compiler_test(test_select_classic rexx_src/test_select_classic.crexx)
add_crexx_compiler_test(test_select_c rexx_src/test_select_c.crexx)
add_crexx_compiler_test(select_dispatch_integer rexx_src/select_dispatch_integer.crexx)
add_crexx_compiler_test(select_dispatch_fallback rexx_src/select_dispatch_fallback.crexx)
add_crexx_compiler_test(select_dispatch_mixed rexx_src/select_dispatch_mixed.crexx)
add_crexx_compiler_test(select_dispatch_general rexx_src/select_dispatch_general.crexx)
add_crexx_compiler_test(select_dispatch_strings rexx_src/select_dispatch_strings.crexx)
add_crexx_compiler_test(repro_duplicate_call_argument rexx_src/repro_duplicate_call_argument.crexx)
add_crexx_compiler_test(09_nested rexx_src/09_nested.crexx)
add_crexx_compiler_test(10_procs rexx_src/10_procs.crexx)
add_crexx_compiler_test(11_scope rexx_src/11_scope.crexx)
add_crexx_compiler_test(12_arrays_lvlb rexx_src/12_arrays_lvlb.crexx)
add_crexx_compiler_test(array_mutation_statements rexx_src/array_mutation_statements.crexx)
add_crexx_compiler_test(13_stems rexx_src/13_stems.crexx)
add_crexx_compiler_test(repro_multi_tail_stems rexx_src/repro_multi_tail_stems.crexx)
add_crexx_compiler_test(14_strings rexx_src/14_strings.crexx)
add_crexx_compiler_test(17_string_arrays rexx_src/17_string_arrays.crexx)
add_crexx_compiler_dep_test(18_array_class rexx_src/18_array_class_main.crexx rexx_src/array_class_lib.crexx)
add_crexx_compiler_test(val_magic_01 rexx_src/val_magic_01.crexx)
add_crexx_compiler_test(val_magic_02 rexx_src/val_magic_02.crexx)
add_crexx_compiler_test(opt_decimal_safety rexx_src/opt_decimal_safety.crexx)
add_crexx_compiler_test(opt_numeric_safety rexx_src/opt_numeric_safety.crexx)
add_crexx_compiler_dep_test(15_imports rexx_src/15_imports.crexx rexx_src/mymath.crexx)
add_crexx_compiler_dep_test(class_import rexx_src/class_import_main.crexx rexx_src/class_import_lib.crexx)
add_crexx_parsing_test(16_classes rexx_src/16_classes.crexx)

# Negative tests
add_crexx_compiler_fail_test(err_01_type rexx_src/err_01_type.crexx err_01_type.txt)
add_crexx_compiler_fail_test(err_02_syntax rexx_src/err_02_syntax.crexx err_02_syntax.txt)
add_crexx_compiler_fail_test(err_04_getter rexx_src/err_04_getter.crexx err_04_getter.txt)
add_crexx_compiler_fail_test(err_05_setter rexx_src/err_05_setter.crexx err_05_setter.txt)
add_crexx_compiler_fail_test(err_06_missing_method rexx_src/err_06_missing_method.crexx err_06_missing_method.txt)
add_crexx_compiler_fail_test(array_mutation_bad_index rexx_src/array_mutation_bad_index.crexx array_mutation_bad_index.txt)
add_crexx_compiler_fail_test(array_mutation_bad_value rexx_src/array_mutation_bad_value.crexx array_mutation_bad_value.txt)
add_crexx_compiler_fail_test(array_mutation_fixed_target rexx_src/array_mutation_fixed_target.crexx array_mutation_fixed_target.txt)
add_crexx_compiler_fail_test(array_mutation_scalar_target rexx_src/array_mutation_scalar_target.crexx array_mutation_scalar_target.txt)
add_crexx_compiler_fail_test(err_symbol_boundary_whitespace rexx_src/err_symbol_boundary_whitespace.crexx err_symbol_boundary_whitespace.txt)
add_crexx_compiler_fail_test(err_loop_class rexx_src/err_loop_class.crexx err_loop_class.txt)
add_crexx_compiler_fail_test(err_select_no_when rexx_src/err_select_no_when.crexx err_select_no_when.txt)
add_crexx_compiler_fail_test(err_select_c_no_when rexx_src/err_select_c_no_when.crexx err_select_c_no_when.txt)
add_crexx_compiler_fail_test(err_class_assign rexx_src/err_class_assign.crexx err_class_assign.txt)
add_crexx_compiler_fail_test(err_register_flag_compiler_write rexx_src/err_register_flag_compiler_write.crexx err_register_flag_compiler_write.txt)
add_crexx_compiler_fail_test(err_register_flag_unknown rexx_src/err_register_flag_unknown.crexx err_register_flag_unknown.txt)
add_crexx_compiler_fail_test(err_register_flag_type rexx_src/err_register_flag_type.crexx err_register_flag_type.txt)
add_crexx_compiler_fail_test(err_address_commandless_sandbox rexx_src/err_address_commandless_sandbox.crexx err_address_commandless_sandbox.txt)
add_crexx_compiler_fail_test(err_block_expr_type_mismatch rexx_src/err_block_expr_type_mismatch.crexx err_block_expr_type_mismatch.txt)
add_crexx_compiler_fail_test(err_block_expr_no_leave rexx_src/err_block_expr_no_leave.crexx err_block_expr_no_leave.txt)
add_crexx_compiler_fail_test(err_block_expr_leave_plain rexx_src/err_block_expr_leave_plain.crexx err_block_expr_leave_plain.txt)
add_crexx_compiler_fail_test(err_block_expr_missing_end rexx_src/err_block_expr_missing_end.crexx err_block_expr_missing_end.txt)
add_crexx_compiler_fail_test(err_block_expr_incomplete rexx_src/err_block_expr_incomplete.crexx err_block_expr_incomplete.txt)
add_crexx_compiler_fail_test(err_leave_with_not_in_block rexx_src/err_leave_with_not_in_block.crexx err_leave_with_not_in_block.txt)
add_crexx_compiler_fail_test(interface_missing_impl rexx_src/interface_missing_impl.crexx interface_missing_impl.txt)
add_crexx_compiler_fail_test(interface_match_bad_args rexx_src/interface_match_bad_args.crexx interface_match_bad_args.txt)
add_crexx_compiler_fail_test(interface_match_without_factory rexx_src/interface_match_without_factory.crexx interface_match_without_factory.txt)
add_crexx_compiler_fail_test(interface_default_redefine rexx_src/interface_default_redefine.crexx interface_default_redefine.txt)
add_crexx_compiler_fail_test(interface_factory_return_type rexx_src/interface_factory_return_type.crexx interface_factory_return_type.txt)
add_crexx_compiler_fail_test(interface_signature_static_bad_method rexx_src/interface_signature_static_bad_method.crexx interface_signature_static_bad_method.txt)
add_crexx_compiler_fail_test(qualified_namespace_missing_import rexx_src/qualified_namespace_missing_import.crexx qualified_namespace_missing_import.txt)
add_crexx_compiler_fail_test(type_ops_reserved_keyword rexx_src/type_ops_reserved_keyword.crexx type_ops_reserved_keyword.txt)
add_crexx_compiler_fail_test(reference_source_value_to_ref rexx_src/reference_source_value_to_ref_fail.crexx reference_source_value_to_ref_fail.txt)
add_crexx_compiler_fail_test(reference_source_ref_to_value rexx_src/reference_source_ref_to_value_fail.crexx reference_source_ref_to_value_fail.txt)
add_crexx_compiler_fail_test(reference_source_non_storage rexx_src/reference_source_non_storage_fail.crexx reference_source_non_storage_fail.txt)
add_crexx_compiler_fail_test(reference_source_ref_target rexx_src/reference_source_ref_target_fail.crexx reference_source_ref_target_fail.txt)
add_crexx_compiler_fail_test(reference_source_nested_type rexx_src/reference_source_nested_type_fail.crexx reference_source_nested_type_fail.txt)
add_crexx_compiler_fail_test(reference_source_deref_nonref rexx_src/reference_source_deref_nonref_fail.crexx reference_source_deref_nonref_fail.txt)
add_crexx_compiler_fail_test(reference_source_deref_expr rexx_src/reference_source_deref_expr_fail.crexx reference_source_deref_expr_fail.txt)
add_crexx_compiler_fail_test(reference_source_deref_attr rexx_src/reference_source_deref_attr_fail.crexx reference_source_deref_attr_fail.txt)
add_crexx_compiler_fail_test(reference_source_deref_index rexx_src/reference_source_deref_index_fail.crexx reference_source_deref_index_fail.txt)
add_crexx_compiler_fail_test(reference_source_deref_global rexx_src/reference_source_deref_global_fail.crexx reference_source_deref_global_fail.txt)
add_crexx_compiler_fail_test(reference_source_refvalid_nonref rexx_src/reference_source_refvalid_nonref_fail.crexx reference_source_refvalid_nonref_fail.txt)
add_crexx_compiler_fail_test(binary_memory_invalid_storage rexx_src/binary_memory_invalid_storage.crexx binary_memory_invalid_storage.txt)
add_crexx_compiler_fail_test(binary_memory_fixed_for rexx_src/binary_memory_fixed_for.crexx binary_memory_fixed_for.txt)
add_crexx_compiler_fail_test(binary_memory_missing_offset rexx_src/binary_memory_missing_offset.crexx binary_memory_missing_offset.txt)
add_crexx_compiler_fail_test(binary_memory_readonly_write rexx_src/binary_memory_readonly_write.crexx binary_memory_readonly_write.txt)
add_crexx_compiler_fail_test(binary_memory_span_write rexx_src/binary_memory_span_write.crexx binary_memory_span_write.txt)
add_crexx_compiler_fail_test(binary_memory_target_not_binary rexx_src/binary_memory_target_not_binary.crexx binary_memory_target_not_binary.txt)
add_crexx_compiler_fail_test(binary_memory_length_required rexx_src/binary_memory_length_required.crexx binary_memory_length_required.txt)
add_crexx_compiler_fail_test(binary_memory_decimal_length rexx_src/binary_memory_decimal_length.crexx binary_memory_decimal_length.txt)
add_crexx_compiler_fail_test(binary_memory_extra_argument rexx_src/binary_memory_extra_argument.crexx binary_memory_extra_argument.txt)
add_crexx_compiler_fail_test(binary_memory_missing_length_argument rexx_src/binary_memory_missing_length_argument.crexx binary_memory_missing_length_argument.txt)
add_crexx_compiler_fail_test(binary_memory_generic_types_unsupported rexx_src/binary_memory_generic_types_unsupported.crexx binary_memory_generic_types_unsupported.txt)
add_crexx_compiler_fail_test(binary_memory_blen_syntax rexx_src/binary_memory_blen_syntax.crexx binary_memory_blen_syntax.txt)
add_crexx_compiler_fail_test(binary_memory_blen_target_not_binary rexx_src/binary_memory_blen_target_not_binary.crexx binary_memory_blen_target_not_binary.txt)
add_crexx_compiler_fail_test(binary_memory_compare_arguments rexx_src/binary_memory_compare_arguments.crexx binary_memory_compare_arguments.txt)
add_crexx_compiler_fail_test(binary_memory_compare_explicit_length rexx_src/binary_memory_compare_explicit_length.crexx binary_memory_compare_explicit_length.txt)
add_crexx_compiler_fail_test(binary_memory_compare_needle_type rexx_src/binary_memory_compare_needle_type.crexx binary_memory_compare_needle_type.txt)
add_crexx_compiler_fail_test(binary_memory_compare_target_not_binary rexx_src/binary_memory_compare_target_not_binary.crexx binary_memory_compare_target_not_binary.txt)
add_crexx_compiler_fail_test(binary_memory_compare_type rexx_src/binary_memory_compare_type.crexx binary_memory_compare_type.txt)
add_crexx_compiler_fail_test(literal_continuation_suffix_error rexx_src/literal_continuation_suffix_error.crexx literal_continuation_suffix_error.txt)
add_crexx_compiler_fail_test(intrinsic_argexists_invalid rexx_src/intrinsic_argexists_invalid.crexx intrinsic_argexists_invalid.txt)
add_crexx_compiler_fail_test(legacy_arg_call_removed rexx_src/legacy_arg_call_removed.crexx legacy_arg_call_removed.txt)
add_crexx_compiler_fail_test(legacy_arg_stem_removed rexx_src/legacy_arg_stem_removed.crexx legacy_arg_stem_removed.txt)
add_crexx_compiler_fail_test(legacy_typeof_removed rexx_src/legacy_typeof_removed.crexx legacy_typeof_removed.txt)
add_crexx_compiler_fail_test(legacy_refvalid_removed rexx_src/legacy_refvalid_removed.crexx legacy_refvalid_removed.txt)
add_test(NAME err_address_exit_disabled_fail
         COMMAND crexx_test_driver --expect-fail
         ${CMAKE_CURRENT_SOURCE_DIR}/golden/errors/err_address_exit_disabled.txt
         ${CMAKE_CURRENT_BINARY_DIR}/err_address_exit_disabled_fail.txt
         $<TARGET_FILE:rxc> -x ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/err_address_exit_disabled.crexx)

add_test(NAME err_parse_exit_disabled_fail
         COMMAND crexx_test_driver --expect-fail
         ${CMAKE_CURRENT_SOURCE_DIR}/golden/errors/err_parse_exit_disabled.txt
         ${CMAKE_CURRENT_BINARY_DIR}/err_parse_exit_disabled_fail.txt
         $<TARGET_FILE:rxc> -x ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/err_parse_exit_disabled.crexx)

add_test(NAME err_parse_inline_then_exit_disabled_fail
         COMMAND crexx_test_driver --expect-fail
         ${CMAKE_CURRENT_SOURCE_DIR}/golden/errors/err_parse_inline_then_exit_disabled.txt
         ${CMAKE_CURRENT_BINARY_DIR}/err_parse_inline_then_exit_disabled_fail.txt
         $<TARGET_FILE:rxc> -x ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/err_parse_inline_then_exit_disabled.crexx)

add_test(NAME err_trace_exit_disabled_fail
         COMMAND crexx_test_driver --expect-fail
         ${CMAKE_CURRENT_SOURCE_DIR}/golden/errors/err_trace_exit_disabled.txt
         ${CMAKE_CURRENT_BINARY_DIR}/err_trace_exit_disabled_fail.txt
         $<TARGET_FILE:rxc> -x ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/err_trace_exit_disabled.crexx)

add_crexx_compiler_fail_test(err_parse_missing_arguments rexx_src/err_parse_missing_arguments.crexx err_parse_missing_arguments.txt)

# Warning tests
add_crexx_compiler_warning_test(loop rexx_src/loop.crexx loop.txt)
add_crexx_compiler_warning_test(implicit_member_call_address rexx_src/implicit_member_call_address_warning.crexx implicit_member_call_address.txt)
add_crexx_compiler_warning_test(implicit_exit_reject_address rexx_src/implicit_exit_reject_address_warning.crexx implicit_exit_reject_address.txt)
add_crexx_compiler_warning_test(procedure_expose_no_duplicate rexx_src/procedure_expose_no_duplicate_warning.crexx procedure_expose_no_duplicate.txt)
add_crexx_compiler_warning_test(unused_import rexx_src/unused_import_warning.crexx unused_import.txt)

add_test(NAME unused_import_used_proc_warn
         COMMAND crexx_test_driver --warnings
         --copy ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/unused_import_used_dep.crexx
         ${CMAKE_CURRENT_SOURCE_DIR}/golden/warnings/unused_import_used_proc.txt
         ${CMAKE_CURRENT_BINARY_DIR}/unused_import_used_proc_warn.txt
         $<TARGET_FILE:rxc> ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/unused_import_used_proc_warning.crexx)

add_test(NAME unused_import_used_type_warn
         COMMAND crexx_test_driver --warnings
         --copy ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/unused_import_used_dep.crexx
         ${CMAKE_CURRENT_SOURCE_DIR}/golden/warnings/unused_import_used_type.txt
         ${CMAKE_CURRENT_BINARY_DIR}/unused_import_used_type_warn.txt
         $<TARGET_FILE:rxc> ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/unused_import_used_type_warning.crexx)

add_test(NAME unused_import_inline_used_warn
         COMMAND crexx_test_driver --warnings
         --copy ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/inline_cross_file_dep.crexx
         ${CMAKE_CURRENT_SOURCE_DIR}/golden/warnings/unused_import_inline_used.txt
         ${CMAKE_CURRENT_BINARY_DIR}/unused_import_inline_used_warn.txt
         $<TARGET_FILE:rxc> ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/unused_import_inline_used_warning.crexx)

# Regression tests for rxpp issue
add_crexx_compiler_test(repro_dotted_index rexx_src/repro_dotted_index.crexx)
add_crexx_compiler_test(repro_expand_until rexx_src/repro_expand_until.crexx)
add_crexx_compiler_test(repro_array_arg rexx_src/repro_array_arg.crexx)
add_crexx_compiler_test(repro_opt_args rexx_src/repro_opt_args.crexx)
add_crexx_compiler_test(arg_semantics_scalar rexx_src/arg_semantics_scalar.crexx)
add_crexx_compiler_test(arg_semantics_array rexx_src/arg_semantics_array.crexx)
add_crexx_compiler_test(arg_semantics_object rexx_src/arg_semantics_object.crexx)
add_crexx_compiler_test(object_reference_regression rexx_src/object_reference_regression.crexx)
add_crexx_compiler_test(basic_loops rexx_src/basic_loops.crexx)
add_crexx_compiler_test(basic_strings rexx_src/basic_strings.crexx)
add_crexx_compiler_test(basic_arrays rexx_src/basic_arrays.crexx)
add_crexx_compiler_test(block_expr_basic rexx_src/block_expr_basic.crexx)
add_crexx_compiler_test(block_expr_short_circuit rexx_src/block_expr_short_circuit.crexx)
add_crexx_compiler_test(inline_test1 rexx_src/inline_test1.crexx)
add_crexx_compiler_test(inline_test2 rexx_src/inline_test2.crexx)
add_crexx_compiler_test(inline_test_call rexx_src/inline_test_call.crexx)
add_crexx_compiler_test(inline_test_expr rexx_src/inline_test_expr.crexx)
add_crexx_compiler_test(inline_test_expr_negative rexx_src/inline_test_expr_negative.crexx)
add_crexx_compiler_test(inline_test_nested_call_expr rexx_src/inline_test_nested_call_expr.crexx)
add_crexx_compiler_test(inline_test_concat_expr rexx_src/inline_test_concat_expr.crexx)
add_crexx_compiler_test(inline_test_say_expr rexx_src/inline_test_say_expr.crexx)
add_crexx_compiler_test(inline_test_say_expr_negative rexx_src/inline_test_say_expr_negative.crexx)
add_crexx_compiler_test(inline_test_return_expr rexx_src/inline_test_return_expr.crexx)
add_crexx_compiler_test(inline_test_unary_expr rexx_src/inline_test_unary_expr.crexx)
add_crexx_compiler_test(inline_test_compare_expr rexx_src/inline_test_compare_expr.crexx)
add_crexx_compiler_test(inline_test_call_arg_expr rexx_src/inline_test_call_arg_expr.crexx)
add_crexx_compiler_test(inline_test_call_like_arg_expr rexx_src/inline_test_call_like_arg_expr.crexx)
add_crexx_compiler_test(inline_test_short_circuit_expr rexx_src/inline_test_short_circuit_expr.crexx)
add_crexx_compiler_test(inline_test_composed_expr_contexts rexx_src/inline_test_composed_expr_contexts.crexx)
add_crexx_compiler_test(inline_test_byvalue_arg_reuse rexx_src/inline_test_byvalue_arg_reuse.crexx)
add_crexx_compiler_test(inline_test_stateful_assembler rexx_src/inline_test_stateful_assembler.crexx)
add_crexx_compiler_test(inline_test_condition_contexts rexx_src/inline_test_condition_contexts.crexx)
add_crexx_compiler_test(inline_test_loop_bound_expr rexx_src/inline_test_loop_bound_expr.crexx)
add_crexx_compiler_test(inline_test_member_receiver_expr rexx_src/inline_test_member_receiver_expr.crexx)
add_crexx_compiler_test(inline_test_named_factory_selector rexx_src/inline_test_named_factory_selector.crexx)
add_crexx_compiler_test(inline_test_ref_opt rexx_src/inline_test_ref_opt.crexx)
add_crexx_compiler_test(inline_test_ref_indexed rexx_src/inline_test_ref_indexed.crexx)
add_crexx_compiler_test(inline_test_ref_stem rexx_src/inline_test_ref_stem.crexx)
add_crexx_compiler_test(inline_test_ref_computed rexx_src/inline_test_ref_computed.crexx)
add_crexx_compiler_test(inline_test_ref_vararg rexx_src/inline_test_ref_vararg.crexx)
add_crexx_compiler_test(inline_test_ref_vararg_runtime rexx_src/inline_test_ref_vararg_runtime.crexx)
add_crexx_compiler_test(inline_test_ref_negative rexx_src/inline_test_ref_negative.crexx)
add_crexx_compiler_test(inline_test_bool_expr_negative rexx_src/inline_test_bool_expr_negative.crexx)
add_crexx_compiler_test(inline_test_vararg rexx_src/inline_test_vararg.crexx)
add_crexx_compiler_test(inline_test_vararg_dynamic rexx_src/inline_test_vararg_dynamic.crexx)
add_crexx_compiler_test(inline_test_vararg_dynamic_loop rexx_src/inline_test_vararg_dynamic_loop.crexx)
add_crexx_compiler_test(inline_test_vararg_negative rexx_src/inline_test_vararg_negative.crexx)
add_crexx_compiler_test(inline_test_exposed_nested_call rexx_src/inline_test_exposed_nested_call.crexx)
add_crexx_compiler_test(inline_test_self_recursive_negative rexx_src/inline_test_self_recursive_negative.crexx)
add_crexx_compiler_test(inline_test_mutual_recursive_negative rexx_src/inline_test_mutual_recursive_negative.crexx)
add_crexx_compiler_test(inline_test_nested_scope_do rexx_src/inline_test_nested_scope_do.crexx)
add_crexx_compiler_test(inline_test_nested_scope_if_do rexx_src/inline_test_nested_scope_if_do.crexx)
add_crexx_compiler_test(inline_test_nested_scope_exposed_call rexx_src/inline_test_nested_scope_exposed_call.crexx)
add_crexx_compiler_test(inline_test_multi_return_assign rexx_src/inline_test_multi_return_assign.crexx)
add_crexx_compiler_test(inline_test_multi_return_expr rexx_src/inline_test_multi_return_expr.crexx)
add_crexx_compiler_test(inline_test_multi_return_call rexx_src/inline_test_multi_return_call.crexx)
add_crexx_compiler_test(inline_test_void_fallthrough_call rexx_src/inline_test_void_fallthrough_call.crexx)
add_crexx_compiler_test(inline_test_array_return_assign rexx_src/inline_test_array_return_assign.crexx)
add_crexx_compiler_test(inline_test_array_return_expr rexx_src/inline_test_array_return_expr.crexx)
add_crexx_compiler_test(inline_test_array_multi_return_expr rexx_src/inline_test_array_multi_return_expr.crexx)
add_crexx_compiler_test(inline_test_array_multi_return_assign rexx_src/inline_test_array_multi_return_assign.crexx)
add_crexx_compiler_test(inline_test_object_return_assign rexx_src/inline_test_object_return_assign.crexx)
add_crexx_compiler_test(inline_test_object_return_expr rexx_src/inline_test_object_return_expr.crexx)
add_crexx_compiler_test(inline_test_object_const_arg_mutation rexx_src/inline_test_object_const_arg_mutation.crexx)
add_crexx_compiler_test(inline_test_object_writable_arg rexx_src/inline_test_object_writable_arg.crexx)
add_crexx_compiler_test(inline_test_array_expr_arg rexx_src/inline_test_array_expr_arg.crexx)
add_crexx_compiler_test(inline_test_object_expr_arg rexx_src/inline_test_object_expr_arg.crexx)
add_crexx_compiler_test(inline_test_array_return_temp rexx_src/inline_test_array_return_temp.crexx)
add_crexx_compiler_test(inline_test_complex_lhs_assign rexx_src/inline_test_complex_lhs_assign.crexx)
add_crexx_compiler_test(inline_test_binary_arg_return rexx_src/inline_test_binary_arg_return.crexx)
add_crexx_compiler_test(inline_test_block_expr_live_sibling rexx_src/inline_test_block_expr_live_sibling.crexx)
add_crexx_compiler_test(inline_test_class_methods rexx_src/inline_test_class_methods.crexx)

add_test(NAME nr06_codegen_contract
         COMMAND ${CMAKE_COMMAND}
                 -DRXC=$<TARGET_FILE:rxc>
                 -DBIN_DIR=${CMAKE_BINARY_DIR}/bin
                 -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src
                 -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/nr06_codegen_contract_ws
                 -P ${CMAKE_CURRENT_SOURCE_DIR}/nr06_codegen_contract.cmake)
set_tests_properties(nr06_codegen_contract PROPERTIES
        LABELS "compiler;performance")

add_test(NAME nr21_fixed_call_contract
         COMMAND ${CMAKE_COMMAND}
                 -DRXC=$<TARGET_FILE:rxc>
                 -DRXAS=$<TARGET_FILE:rxas>
                 -DRXLINK=$<TARGET_FILE:rxlink>
                 -DRXDAS=$<TARGET_FILE:rxdas>
                 -DRXVM=$<TARGET_FILE:rxvm>
                 -DRXBVM=$<TARGET_FILE:rxbvm>
                 -DMUTATE_FEATURE=$<TARGET_FILE:mutate_rxbin_feature>
                 -DBIN_DIR=${CMAKE_BINARY_DIR}/bin
                 -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src
                 -DBASIC_RXAS=${CMAKE_SOURCE_DIR}/interpreter/tests/tests_basic.rxas
                 -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/nr21_fixed_call_contract_ws
                 -P ${CMAKE_CURRENT_SOURCE_DIR}/nr21_fixed_call_contract.cmake)
set_tests_properties(nr21_fixed_call_contract PROPERTIES
        LABELS "compiler;performance;rxbin")

add_test(NAME nr14_frozen_parse_contract
         COMMAND ${CMAKE_COMMAND}
                 -DRXC=$<TARGET_FILE:rxc>
                 -DRXAS=$<TARGET_FILE:rxas>
                 -DRXLINK=$<TARGET_FILE:rxlink>
                 -DRXDAS=$<TARGET_FILE:rxdas>
                 -DRXVM=$<TARGET_FILE:rxvm>
                 -DRXBVM=$<TARGET_FILE:rxbvm>
                 -DMUTATE_FEATURE=$<TARGET_FILE:mutate_rxbin_feature>
                 -DBIN_DIR=${CMAKE_BINARY_DIR}/bin
                 -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src
                 -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/nr14_frozen_parse_contract_ws
                 -P ${CMAKE_CURRENT_SOURCE_DIR}/nr14_frozen_parse_contract.cmake)
set_tests_properties(nr14_frozen_parse_contract PROPERTIES
        LABELS "compiler;performance;rxbin;parse")

add_test(NAME nr15_native_stem_contract
         COMMAND ${CMAKE_COMMAND}
                 -DRXAS=$<TARGET_FILE:rxas>
                 -DRXLINK=$<TARGET_FILE:rxlink>
                 -DRXDAS=$<TARGET_FILE:rxdas>
                 -DRXVM=$<TARGET_FILE:rxvm>
                 -DRXBVM=$<TARGET_FILE:rxbvm>
                 -DMUTATE_FEATURE=$<TARGET_FILE:mutate_rxbin_feature>
                 -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/nr15_native_stem_contract.rxas
                 -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/nr15_native_stem_contract_ws
                 -P ${CMAKE_CURRENT_SOURCE_DIR}/nr15_native_stem_contract.cmake)
set_tests_properties(nr15_native_stem_contract PROPERTIES
        LABELS "compiler;performance;rxbin;stem;nr15")

add_test(NAME nr09_numeric_context_contract
         COMMAND ${CMAKE_COMMAND}
                 -DRXC=$<TARGET_FILE:rxc>
                 -DRXAS=$<TARGET_FILE:rxas>
                 -DRXVM=$<TARGET_FILE:rxvm>
                 -DRXBVM=$<TARGET_FILE:rxbvm>
                 -DBIN_DIR=${CMAKE_BINARY_DIR}/bin
                 -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src
                 -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/nr09_numeric_context_contract_ws
                 -P ${CMAKE_CURRENT_SOURCE_DIR}/nr09_numeric_context_contract.cmake)
set_tests_properties(nr09_numeric_context_contract PROPERTIES
        LABELS "compiler;performance;numeric")

add_test(NAME nr09_codegen_contract
         COMMAND ${CMAKE_COMMAND}
                 -DRXC=$<TARGET_FILE:rxc>
                 -DRXAS=$<TARGET_FILE:rxas>
                 -DRXVM=$<TARGET_FILE:rxvm>
                 -DRXBVM=$<TARGET_FILE:rxbvm>
                 -DBIN_DIR=${CMAKE_BINARY_DIR}/bin
                 -DBENCH_DIR=${CMAKE_SOURCE_DIR}/tests/benchmarks
                 -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src
                 -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/nr09_codegen_contract_ws
                 -P ${CMAKE_CURRENT_SOURCE_DIR}/nr09_codegen_contract.cmake)
set_tests_properties(nr09_codegen_contract PROPERTIES
        LABELS "compiler;performance")

add_test(NAME nr26_flow_contract
         COMMAND ${CMAKE_COMMAND}
                 -DRXC=$<TARGET_FILE:rxc>
                 -DRXAS=$<TARGET_FILE:rxas>
                 -DRXVM=$<TARGET_FILE:rxvm>
                 -DRXBVM=$<TARGET_FILE:rxbvm>
                 -DBIN_DIR=${CMAKE_BINARY_DIR}/bin
                 -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src
                 -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/nr26_flow_contract_ws
                 -P ${CMAKE_CURRENT_SOURCE_DIR}/nr26_flow_contract.cmake)
set_tests_properties(nr26_flow_contract PROPERTIES
        LABELS "compiler;performance;flow")

add_test(NAME perf2_04_inline_assembler_effects
         COMMAND ${CMAKE_COMMAND}
                 -DRXC=$<TARGET_FILE:rxc>
                 -DRXAS=$<TARGET_FILE:rxas>
                 -DRXVM=$<TARGET_FILE:rxvm>
                 -DRXBVM=$<TARGET_FILE:rxbvm>
                 -DBIN_DIR=${CMAKE_BINARY_DIR}/bin
                 -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/perf2_04_inline_assembler_effects.crexx
                 -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/perf2_04_inline_assembler_effects_ws
                 -P ${CMAKE_CURRENT_SOURCE_DIR}/perf2_04_inline_assembler_effects.cmake)
set_tests_properties(perf2_04_inline_assembler_effects PROPERTIES
        LABELS "compiler;performance;inlining;perf2_04")

add_test(NAME perf2_04_inline_assembler_imports
         COMMAND ${CMAKE_COMMAND}
                 -DRXC=$<TARGET_FILE:rxc>
                 -DRXAS=$<TARGET_FILE:rxas>
                 -DRXVM=$<TARGET_FILE:rxvm>
                 -DRXBVM=$<TARGET_FILE:rxbvm>
                 -DDEP_SOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/perf2_04_case_import/dep/case_helpers.crexx
                 -DDEP_DIR=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/perf2_04_case_import/dep
                 -DMAIN_SOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/perf2_04_case_import/main/case_main.crexx
                 -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/perf2_04_inline_assembler_imports_ws
                 -P ${CMAKE_CURRENT_SOURCE_DIR}/perf2_04_inline_assembler_imports.cmake)
set_tests_properties(perf2_04_inline_assembler_imports PROPERTIES
        LABELS "compiler;performance;imports;inlining;perf2_04")

add_test(NAME perf2_04_certified_call
         COMMAND ${CMAKE_COMMAND}
                 -DRXC=$<TARGET_FILE:rxc>
                 -DRXAS=$<TARGET_FILE:rxas>
                 -DRXVM=$<TARGET_FILE:rxvm>
                 -DRXBVM=$<TARGET_FILE:rxbvm>
                 -DBIF_SOURCE_DIR=${CMAKE_SOURCE_DIR}/lib/rxfnsb/rexx
                 -DSELECTED_SOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/perf2_04_certified_call/selected.crexx
                 -DFALLBACK_SOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/perf2_04_certified_call/fallback.crexx
                 -DSPOOF_SOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/perf2_04_certified_call/spoof_upper.crexx
                 -DSPOOF_LOWER_SOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/perf2_04_certified_call/spoof_lower.crexx
                 -DSPOOF_WORD_DIR=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/perf2_04_certified_call/spoof_word
                 -DHUGE_WIDTH_SOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/perf2_04_certified_call/huge_width.crexx
                 -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/perf2_04_certified_call_ws
                 -P ${CMAKE_CURRENT_SOURCE_DIR}/perf2_04_certified_call.cmake)
set_tests_properties(perf2_04_certified_call PROPERTIES
        LABELS "compiler;performance;constants;imports;perf2_04")

add_test(NAME perf2_05_partial_call
         COMMAND ${CMAKE_COMMAND}
                 -DRXC=$<TARGET_FILE:rxc>
                 -DRXAS=$<TARGET_FILE:rxas>
                 -DRXVM=$<TARGET_FILE:rxvm>
                 -DRXBVM=$<TARGET_FILE:rxbvm>
                 -DSELECTED_SOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/perf2_05_partial_call/selected.crexx
                 -DCURSOR_SOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/perf2_05_partial_call/cursor_inline.crexx
                 -DARRAY_SOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/perf2_05_partial_call/array_result.crexx
                 -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/perf2_05_partial_call_ws
                 -P ${CMAKE_CURRENT_SOURCE_DIR}/perf2_05_partial_call.cmake)
set_tests_properties(perf2_05_partial_call PROPERTIES
        LABELS "compiler;performance;constants;inlining;perf2_05")

add_test(NAME perf2_07_v3_representation
         COMMAND ${CMAKE_COMMAND}
                 -DRXC=$<TARGET_FILE:rxc>
                 -DRXAS=$<TARGET_FILE:rxas>
                 -DRXVM=$<TARGET_FILE:rxvm>
                 -DRXBVM=$<TARGET_FILE:rxbvm>
                 -DLIBRARY=${CMAKE_BINARY_DIR}/bin/library.rxbin
                 -DLIBRARY_DIR=${CMAKE_BINARY_DIR}/bin
                 -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/perf2_07_v3_representation.crexx
                 -DWORK=${CMAKE_CURRENT_BINARY_DIR}/perf2_07_v3_representation_ws
                 -P ${CMAKE_CURRENT_SOURCE_DIR}/perf2_07_v3_representation.cmake)
set_tests_properties(perf2_07_v3_representation PROPERTIES
        LABELS "compiler;interpreter;representation;reference;performance;perf2_07")

add_test(NAME perf2_07_binary_byvalue_contract
         COMMAND ${CMAKE_COMMAND}
                 -DRXC=$<TARGET_FILE:rxc>
                 -DRXAS=$<TARGET_FILE:rxas>
                 -DRXDAS=$<TARGET_FILE:rxdas>
                 -DRXVM=$<TARGET_FILE:rxvm>
                 -DRXBVM=$<TARGET_FILE:rxbvm>
                 -DLIBRARY=${CMAKE_BINARY_DIR}/bin/library.rxbin
                 -DLIBRARY_DIR=${CMAKE_BINARY_DIR}/bin
                 -DCODEGEN_SOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/perf2_07_binary_byvalue_codegen.crexx
                 -DSEMANTICS_SOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/perf2_07_binary_byvalue_semantics.crexx
                 -DIMPORT_DEP_SOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/perf2_07_binary_byvalue_import_dep.crexx
                 -DIMPORT_MAIN_SOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/perf2_07_binary_byvalue_import_main.crexx
                 -DWORK=${CMAKE_CURRENT_BINARY_DIR}/perf2_07_binary_byvalue_ws
                 -P ${CMAKE_CURRENT_SOURCE_DIR}/perf2_07_binary_byvalue_contract.cmake)
set_tests_properties(perf2_07_binary_byvalue_contract PROPERTIES
        LABELS "compiler;interpreter;binary;inlining;performance;perf2_07")

# Macro to add a run test
macro(add_crexx_run_test NAME SOURCE_FILE)
    set(RUNTIME_MODULES ${ARGN})

    # Keep no-opt and opt runtime coverage paired so semantic shifts in
    # argument handling or inlining are caught even when compiler-output
    # goldens still match within one mode.
    
    # No-opt script
    set(NOOPT_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_${NAME}_noopt.cmake")
    set(NOOPT_CONTENT "set(full_out \"\")\n")
    string(APPEND NOOPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" ${COMPILER_NOOPT_FLAG} -o \"${NAME}_run_noopt\" \"${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
    string(APPEND NOOPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
    string(APPEND NOOPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed: \${out}\")\nendif()\n")
    
    string(APPEND NOOPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"${NAME}_run_noopt.rxbin\" \"${NAME}_run_noopt\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
    string(APPEND NOOPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
    string(APPEND NOOPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed: \${out}\")\nendif()\n")
    
    string(APPEND NOOPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxvm>\" \"${CMAKE_BINARY_DIR}/bin/library.rxbin\" \"${NAME}_run_noopt.rxbin\" ${RUNTIME_MODULES} OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
    string(APPEND NOOPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
    string(APPEND NOOPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxvm failed: \${out}\")\nendif()\n")
    
    string(APPEND NOOPT_CONTENT "file(READ \"${CMAKE_CURRENT_SOURCE_DIR}/golden/run/${NAME}.out\" golden_out)\n")
    string(APPEND NOOPT_CONTENT "string(REPLACE \"\\r\\n\" \"\\n\" full_out \"\${full_out}\")\n")
    string(APPEND NOOPT_CONTENT "string(REPLACE \"\\r\\n\" \"\\n\" golden_out \"\${golden_out}\")\n")
    string(APPEND NOOPT_CONTENT "if(NOT full_out STREQUAL golden_out)\n  message(FATAL_ERROR \"Output mismatch: \\nExpected:\\n\${golden_out}\\nGot:\\n\${full_out}\")\nendif()\n")
    file(GENERATE OUTPUT "${NOOPT_SCRIPT}" CONTENT "${NOOPT_CONTENT}")

    add_test(NAME ${NAME}_run_noopt
             COMMAND ${CMAKE_COMMAND} -P "${NOOPT_SCRIPT}"
             WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

    # Opt script
    set(OPT_LINK_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_${NAME}_opt_linked.cmake")
    crexx_write_linked_runtime_script(
            OUTPUT "${OPT_LINK_SCRIPT}"
            NAME "${NAME}_run_opt"
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
            RUNNER "$<TARGET_FILE:rxvm>"
            RXLINK "$<TARGET_FILE:rxlink>"
            PROGRAM "${NAME}_run_opt.rxbin"
            EXTRA_ARGS "${CMAKE_BINARY_DIR}/bin/library.rxbin" ${RUNTIME_MODULES}
    )
    set(OPT_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_${NAME}_opt.cmake")
    set(OPT_CONTENT "set(full_out \"\")\n")
    string(APPEND OPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"${NAME}_run_opt\" \"${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
    string(APPEND OPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
    string(APPEND OPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed: \${out}\")\nendif()\n")
    
    string(APPEND OPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"${NAME}_run_opt.rxbin\" \"${NAME}_run_opt\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
    string(APPEND OPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
    string(APPEND OPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed: \${out}\")\nendif()\n")

    string(APPEND OPT_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -P \"${OPT_LINK_SCRIPT}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
    string(APPEND OPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
    string(APPEND OPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"linked rxvm failed: \${out}\")\nendif()\n")
    
    string(APPEND OPT_CONTENT "file(READ \"${CMAKE_CURRENT_SOURCE_DIR}/golden/run/${NAME}.out\" golden_out)\n")
    string(APPEND OPT_CONTENT "string(REPLACE \"\\r\\n\" \"\\n\" full_out \"\${full_out}\")\n")
    string(APPEND OPT_CONTENT "string(REPLACE \"\\r\\n\" \"\\n\" golden_out \"\${golden_out}\")\n")
    string(APPEND OPT_CONTENT "if(NOT full_out STREQUAL golden_out)\n  message(FATAL_ERROR \"Output mismatch: \\nExpected:\\n\${golden_out}\\nGot:\\n\${full_out}\")\nendif()\n")
    file(GENERATE OUTPUT "${OPT_SCRIPT}" CONTENT "${OPT_CONTENT}")

    add_test(NAME ${NAME}_run_opt
             COMMAND ${CMAKE_COMMAND} -P "${OPT_SCRIPT}"
             WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    set_tests_properties(${NAME}_run_opt PROPERTIES LABELS "linked_opt")
endmacro()

function(add_crexx_reference_source_test FIXTURE RUNNER MODE)
    set(NAME "${FIXTURE}_${RUNNER}_${MODE}")
    set(BASE "${NAME}")
    set(SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/${NAME}.cmake")
    set(WORK_DIR "${CMAKE_CURRENT_BINARY_DIR}/${NAME}_ws")
    set(RXC_FLAGS "")
    set(RXAS_FLAGS "")
    set(LABELS "reference;compiler;source")

    if(MODE STREQUAL "noopt")
        set(RXC_FLAGS "-n")
        set(RXAS_FLAGS "-n")
    else()
        set(LABELS "reference;compiler;source;linked_opt")
    endif()

    if(MODE STREQUAL "opt")
        set(LINK_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/${NAME}_linked.cmake")
        crexx_write_linked_runtime_script(
                OUTPUT "${LINK_SCRIPT}"
                NAME "${NAME}"
                WORKING_DIRECTORY "${WORK_DIR}"
                RUNNER "$<TARGET_FILE:${RUNNER}>"
                RXLINK "$<TARGET_FILE:rxlink>"
                PROGRAM "${BASE}.rxbin"
                EXTRA_ARGS "${CMAKE_BINARY_DIR}/bin/library.rxbin"
        )
    endif()

    set(CONTENT "set(full_out \"\")\n")
    string(APPEND CONTENT "file(MAKE_DIRECTORY \"${WORK_DIR}\")\n")
    string(APPEND CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" ${RXC_FLAGS} -o \"${BASE}\" \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/${FIXTURE}.crexx\" WORKING_DIRECTORY \"${WORK_DIR}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
    string(APPEND CONTENT "string(APPEND full_out \"\${out}\")\n")
    string(APPEND CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed: \${out}\")\nendif()\n")
    string(APPEND CONTENT "file(READ \"${WORK_DIR}/${BASE}.rxas\" rxas)\n")
    if(FIXTURE STREQUAL "reference_source_dereference_live" OR
       FIXTURE STREQUAL "reference_source_iterator" OR
       FIXTURE STREQUAL "reference_source_inline_lifetime")
        string(APPEND CONTENT "foreach(op IN ITEMS mkref deref linkref refvalid)\n")
    else()
        string(APPEND CONTENT "foreach(op IN ITEMS mkref deref refvalid)\n")
    endif()
    string(APPEND CONTENT "  string(FIND \"\${rxas}\" \"\${op} \" op_pos)\n")
    string(APPEND CONTENT "  if(op_pos EQUAL -1)\n")
    string(APPEND CONTENT "    message(FATAL_ERROR \"Expected \${op} in generated reference source RXAS\")\n")
    string(APPEND CONTENT "  endif()\n")
    string(APPEND CONTENT "endforeach()\n")
    string(APPEND CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" ${RXAS_FLAGS} -o \"${BASE}.rxbin\" \"${BASE}\" WORKING_DIRECTORY \"${WORK_DIR}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
    string(APPEND CONTENT "string(APPEND full_out \"\${out}\")\n")
    string(APPEND CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed: \${out}\")\nendif()\n")
    if(MODE STREQUAL "opt")
        string(APPEND CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -P \"${LINK_SCRIPT}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
        string(APPEND CONTENT "string(APPEND full_out \"\${out}\")\n")
        string(APPEND CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"linked ${RUNNER} failed: \${out}\")\nendif()\n")
    else()
        string(APPEND CONTENT "execute_process(COMMAND \"$<TARGET_FILE:${RUNNER}>\" \"${BASE}.rxbin\" \"${CMAKE_BINARY_DIR}/bin/library.rxbin\" WORKING_DIRECTORY \"${WORK_DIR}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
        string(APPEND CONTENT "string(APPEND full_out \"\${out}\")\n")
        string(APPEND CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"${RUNNER} failed: \${out}\")\nendif()\n")
    endif()
    string(APPEND CONTENT "file(READ \"${CMAKE_CURRENT_SOURCE_DIR}/golden/run/${FIXTURE}.out\" golden_out)\n")
    string(APPEND CONTENT "string(REPLACE \"\\r\\n\" \"\\n\" full_out \"\${full_out}\")\n")
    string(APPEND CONTENT "string(REPLACE \"\\r\\n\" \"\\n\" golden_out \"\${golden_out}\")\n")
    string(APPEND CONTENT "if(NOT full_out STREQUAL golden_out)\n  message(FATAL_ERROR \"Output mismatch: \\nExpected:\\n\${golden_out}\\nGot:\\n\${full_out}\")\nendif()\n")
    file(GENERATE OUTPUT "${SCRIPT}" CONTENT "${CONTENT}")

    add_test(NAME ${NAME}
             COMMAND ${CMAKE_COMMAND} -P "${SCRIPT}"
             WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    set_tests_properties(${NAME} PROPERTIES
            LABELS "${LABELS}"
            FIXTURES_REQUIRED linked_opt_runtime_artifacts)
endfunction()

foreach(_crexx_reference_fixture IN ITEMS reference_source_explicit reference_source_contract reference_source_iterator reference_source_dereference_live reference_source_inline_lifetime reference_source_block_lifetime)
    foreach(_crexx_runner IN ITEMS rxvm rxbvm)
        foreach(_crexx_mode IN ITEMS noopt opt)
            add_crexx_reference_source_test(${_crexx_reference_fixture} ${_crexx_runner} ${_crexx_mode})
        endforeach()
    endforeach()
endforeach()

function(add_crexx_interface_import_run_test NAME MAIN_FILE DEP_FILE MODE)
    get_filename_component(MAIN_BASENAME "${MAIN_FILE}" NAME)
    get_filename_component(DEP_BASENAME "${DEP_FILE}" NAME)
    get_filename_component(DEP_MODULE "${DEP_FILE}" NAME_WE)

    foreach(TEST_MODE IN ITEMS noopt opt)
        if(TEST_MODE STREQUAL "noopt")
            set(COMPILER_FLAG "${COMPILER_NOOPT_FLAG}")
            set(TEST_SUFFIX "_run_noopt")
        else()
            set(COMPILER_FLAG "${COMPILER_OPT_FLAG}")
            set(TEST_SUFFIX "_run_opt")
        endif()

        set(WORK_DIR "${CMAKE_CURRENT_BINARY_DIR}/${NAME}_${MODE}_${TEST_MODE}_ws")
        if(TEST_MODE STREQUAL "opt")
            set(LINK_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_${NAME}_${MODE}_${TEST_MODE}_linked.cmake")
            if(MODE STREQUAL "source")
                set(LINK_EXTRA_ARGS
                        "${CMAKE_BINARY_DIR}/bin/library.rxbin"
                        "${WORK_DIR}/${DEP_MODULE}${TEST_SUFFIX}.rxbin")
            else()
                set(LINK_EXTRA_ARGS
                        "${CMAKE_BINARY_DIR}/bin/library.rxbin"
                        "${WORK_DIR}/${DEP_MODULE}.rxbin")
            endif()
            crexx_write_linked_runtime_script(
                    OUTPUT "${LINK_SCRIPT}"
                    NAME "${NAME}${TEST_SUFFIX}"
                    WORKING_DIRECTORY "${WORK_DIR}"
                    RUNNER "$<TARGET_FILE:rxvm>"
                    RXLINK "$<TARGET_FILE:rxlink>"
                    PROGRAM "${WORK_DIR}/main${TEST_SUFFIX}.rxbin"
                    EXTRA_ARGS ${LINK_EXTRA_ARGS}
            )
        endif()

        set(SCRIPT_FILE "${CMAKE_CURRENT_BINARY_DIR}/run_${NAME}_${MODE}_${TEST_MODE}.cmake")
        set(SCRIPT_CONTENT "set(full_out \"\")\n")
        string(APPEND SCRIPT_CONTENT "set(work \"${WORK_DIR}\")\n")
        string(APPEND SCRIPT_CONTENT "file(REMOVE_RECURSE \"\${work}\")\n")
        string(APPEND SCRIPT_CONTENT "file(MAKE_DIRECTORY \"\${work}\")\n")
        string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E copy_if_different \"${CMAKE_CURRENT_SOURCE_DIR}/${MAIN_FILE}\" \"\${work}/${MAIN_BASENAME}\" RESULT_VARIABLE res)\n")
        string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"Failed to stage main source\")\nendif()\n")
        string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E copy_if_different \"${CMAKE_CURRENT_SOURCE_DIR}/${DEP_FILE}\" \"\${work}/${DEP_BASENAME}\" RESULT_VARIABLE res)\n")
        string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"Failed to stage dependency source\")\nendif()\n")

        if(MODE STREQUAL "source")
            string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" ${COMPILER_FLAG} -o \"main${TEST_SUFFIX}\" \"\${work}/${MAIN_BASENAME}\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
            string(APPEND SCRIPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
            string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed on source-import main: \${out}\")\nendif()\n")
            string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"main${TEST_SUFFIX}.rxbin\" \"main${TEST_SUFFIX}\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
            string(APPEND SCRIPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
            string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed on source-import main: \${out}\")\nendif()\n")
            string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" ${COMPILER_FLAG} -o \"${DEP_MODULE}${TEST_SUFFIX}\" \"\${work}/${DEP_BASENAME}\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
            string(APPEND SCRIPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
            string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed on source-import dependency: \${out}\")\nendif()\n")
            string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"${DEP_MODULE}${TEST_SUFFIX}.rxbin\" \"${DEP_MODULE}${TEST_SUFFIX}\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
            string(APPEND SCRIPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
            string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed on source-import dependency: \${out}\")\nendif()\n")
            if(TEST_MODE STREQUAL "opt")
                string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -P \"${LINK_SCRIPT}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
            else()
                string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxvm>\" \"${CMAKE_BINARY_DIR}/bin/library.rxbin\" \"\${work}/${DEP_MODULE}${TEST_SUFFIX}.rxbin\" \"\${work}/main${TEST_SUFFIX}.rxbin\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
            endif()
        else()
            string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" ${COMPILER_FLAG} -o \"${DEP_MODULE}${TEST_SUFFIX}\" \"\${work}/${DEP_BASENAME}\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
            string(APPEND SCRIPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
            string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed on binary-import dependency: \${out}\")\nendif()\n")
            string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"${DEP_MODULE}.rxbin\" \"${DEP_MODULE}${TEST_SUFFIX}\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
            string(APPEND SCRIPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
            string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed on binary-import dependency: \${out}\")\nendif()\n")
            string(APPEND SCRIPT_CONTENT "file(REMOVE \"\${work}/${DEP_BASENAME}\")\n")
            string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"\${work}\" -i \"${CMAKE_BINARY_DIR}/bin\" ${COMPILER_FLAG} -o \"main${TEST_SUFFIX}\" \"\${work}/${MAIN_BASENAME}\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
            string(APPEND SCRIPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
            string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed on binary-import main: \${out}\")\nendif()\n")
            string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"main${TEST_SUFFIX}.rxbin\" \"main${TEST_SUFFIX}\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
            string(APPEND SCRIPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
            string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed on binary-import main: \${out}\")\nendif()\n")
            if(TEST_MODE STREQUAL "opt")
                string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -P \"${LINK_SCRIPT}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
            else()
                string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxvm>\" \"${CMAKE_BINARY_DIR}/bin/library.rxbin\" \"\${work}/${DEP_MODULE}.rxbin\" \"\${work}/main${TEST_SUFFIX}.rxbin\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
            endif()
        endif()

        string(APPEND SCRIPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
        string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxvm failed: \${out}\")\nendif()\n")
        string(APPEND SCRIPT_CONTENT "file(READ \"${CMAKE_CURRENT_SOURCE_DIR}/golden/run/${NAME}.out\" golden_out)\n")
        string(APPEND SCRIPT_CONTENT "string(REPLACE \"\\r\\n\" \"\\n\" full_out \"\${full_out}\")\n")
        string(APPEND SCRIPT_CONTENT "string(REPLACE \"\\r\\n\" \"\\n\" golden_out \"\${golden_out}\")\n")
        string(APPEND SCRIPT_CONTENT "if(NOT full_out STREQUAL golden_out)\n  message(FATAL_ERROR \"Output mismatch: \\nExpected:\\n\${golden_out}\\nGot:\\n\${full_out}\")\nendif()\n")

        file(GENERATE OUTPUT "${SCRIPT_FILE}" CONTENT "${SCRIPT_CONTENT}")

        add_test(NAME ${NAME}${TEST_SUFFIX}
                 COMMAND ${CMAKE_COMMAND} -P "${SCRIPT_FILE}"
                 WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
        if(TEST_MODE STREQUAL "opt")
            set_tests_properties(${NAME}${TEST_SUFFIX} PROPERTIES LABELS "linked_opt")
        endif()
    endforeach()
endfunction()

function(add_crexx_dual_import_run_test NAME MAIN_FILE DEP_FILE_A DEP_FILE_B MODE)
    get_filename_component(MAIN_BASENAME "${MAIN_FILE}" NAME)
    get_filename_component(DEP_A_BASENAME "${DEP_FILE_A}" NAME)
    get_filename_component(DEP_A_MODULE "${DEP_FILE_A}" NAME_WE)
    get_filename_component(DEP_B_BASENAME "${DEP_FILE_B}" NAME)
    get_filename_component(DEP_B_MODULE "${DEP_FILE_B}" NAME_WE)

    foreach(TEST_MODE IN ITEMS noopt opt)
        if(TEST_MODE STREQUAL "noopt")
            set(COMPILER_FLAG "${COMPILER_NOOPT_FLAG}")
            set(TEST_SUFFIX "_run_noopt")
        else()
            set(COMPILER_FLAG "${COMPILER_OPT_FLAG}")
            set(TEST_SUFFIX "_run_opt")
        endif()

        set(WORK_DIR "${CMAKE_CURRENT_BINARY_DIR}/${NAME}_${MODE}_${TEST_MODE}_ws")
        if(TEST_MODE STREQUAL "opt")
            set(LINK_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_${NAME}_${MODE}_${TEST_MODE}_linked.cmake")
            if(MODE STREQUAL "source")
                set(LINK_EXTRA_ARGS
                        "${CMAKE_BINARY_DIR}/bin/library.rxbin"
                        "${WORK_DIR}/${DEP_A_MODULE}${TEST_SUFFIX}.rxbin"
                        "${WORK_DIR}/${DEP_B_MODULE}${TEST_SUFFIX}.rxbin")
            else()
                set(LINK_EXTRA_ARGS
                        "${CMAKE_BINARY_DIR}/bin/library.rxbin"
                        "${WORK_DIR}/${DEP_A_MODULE}.rxbin"
                        "${WORK_DIR}/${DEP_B_MODULE}.rxbin")
            endif()
            crexx_write_linked_runtime_script(
                    OUTPUT "${LINK_SCRIPT}"
                    NAME "${NAME}${TEST_SUFFIX}"
                    WORKING_DIRECTORY "${WORK_DIR}"
                    RUNNER "$<TARGET_FILE:rxvm>"
                    RXLINK "$<TARGET_FILE:rxlink>"
                    PROGRAM "${WORK_DIR}/main${TEST_SUFFIX}.rxbin"
                    EXTRA_ARGS ${LINK_EXTRA_ARGS}
            )
        endif()

        set(SCRIPT_FILE "${CMAKE_CURRENT_BINARY_DIR}/run_${NAME}_${MODE}_${TEST_MODE}.cmake")
        set(SCRIPT_CONTENT "set(full_out \"\")\n")
        string(APPEND SCRIPT_CONTENT "set(work \"${WORK_DIR}\")\n")
        string(APPEND SCRIPT_CONTENT "file(REMOVE_RECURSE \"\${work}\")\n")
        string(APPEND SCRIPT_CONTENT "file(MAKE_DIRECTORY \"\${work}\")\n")
        string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E copy_if_different \"${CMAKE_CURRENT_SOURCE_DIR}/${MAIN_FILE}\" \"\${work}/${MAIN_BASENAME}\" RESULT_VARIABLE res)\n")
        string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"Failed to stage main source\")\nendif()\n")
        string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E copy_if_different \"${CMAKE_CURRENT_SOURCE_DIR}/${DEP_FILE_A}\" \"\${work}/${DEP_A_BASENAME}\" RESULT_VARIABLE res)\n")
        string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"Failed to stage dependency A source\")\nendif()\n")
        string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E copy_if_different \"${CMAKE_CURRENT_SOURCE_DIR}/${DEP_FILE_B}\" \"\${work}/${DEP_B_BASENAME}\" RESULT_VARIABLE res)\n")
        string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"Failed to stage dependency B source\")\nendif()\n")

        if(MODE STREQUAL "source")
            string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" ${COMPILER_FLAG} -o \"main${TEST_SUFFIX}\" \"\${work}/${MAIN_BASENAME}\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
            string(APPEND SCRIPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
            string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed on source-import main: \${out}\")\nendif()\n")
            string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"main${TEST_SUFFIX}.rxbin\" \"main${TEST_SUFFIX}\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
            string(APPEND SCRIPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
            string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed on source-import main: \${out}\")\nendif()\n")
            foreach(DEP_SPEC IN ITEMS "${DEP_A_MODULE};${DEP_A_BASENAME}" "${DEP_B_MODULE};${DEP_B_BASENAME}")
                list(GET DEP_SPEC 0 DEP_MODULE)
                list(GET DEP_SPEC 1 DEP_BASENAME)
                string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"\${work}\" -i \"${CMAKE_BINARY_DIR}/bin\" ${COMPILER_FLAG} -o \"${DEP_MODULE}${TEST_SUFFIX}\" \"\${work}/${DEP_BASENAME}\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
                string(APPEND SCRIPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
                string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed on source-import dependency ${DEP_MODULE}: \${out}\")\nendif()\n")
                string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"${DEP_MODULE}${TEST_SUFFIX}.rxbin\" \"${DEP_MODULE}${TEST_SUFFIX}\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
                string(APPEND SCRIPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
                string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed on source-import dependency ${DEP_MODULE}: \${out}\")\nendif()\n")
            endforeach()
            if(TEST_MODE STREQUAL "opt")
                string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -P \"${LINK_SCRIPT}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
            else()
                string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxvm>\" \"${CMAKE_BINARY_DIR}/bin/library.rxbin\" \"\${work}/${DEP_A_MODULE}${TEST_SUFFIX}.rxbin\" \"\${work}/${DEP_B_MODULE}${TEST_SUFFIX}.rxbin\" \"\${work}/main${TEST_SUFFIX}.rxbin\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
            endif()
        else()
            foreach(DEP_SPEC IN ITEMS "${DEP_A_MODULE};${DEP_A_BASENAME}" "${DEP_B_MODULE};${DEP_B_BASENAME}")
                list(GET DEP_SPEC 0 DEP_MODULE)
                list(GET DEP_SPEC 1 DEP_BASENAME)
                string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"\${work}\" -i \"${CMAKE_BINARY_DIR}/bin\" ${COMPILER_FLAG} -o \"${DEP_MODULE}${TEST_SUFFIX}\" \"\${work}/${DEP_BASENAME}\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
                string(APPEND SCRIPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
                string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed on binary-import dependency ${DEP_MODULE}: \${out}\")\nendif()\n")
                string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"${DEP_MODULE}.rxbin\" \"${DEP_MODULE}${TEST_SUFFIX}\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
                string(APPEND SCRIPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
                string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed on binary-import dependency ${DEP_MODULE}: \${out}\")\nendif()\n")
                string(APPEND SCRIPT_CONTENT "file(REMOVE \"\${work}/${DEP_BASENAME}\")\n")
            endforeach()
            string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"\${work}\" -i \"${CMAKE_BINARY_DIR}/bin\" ${COMPILER_FLAG} -o \"main${TEST_SUFFIX}\" \"\${work}/${MAIN_BASENAME}\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
            string(APPEND SCRIPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
            string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed on binary-import main: \${out}\")\nendif()\n")
            string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"main${TEST_SUFFIX}.rxbin\" \"main${TEST_SUFFIX}\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
            string(APPEND SCRIPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
            string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed on binary-import main: \${out}\")\nendif()\n")
            if(TEST_MODE STREQUAL "opt")
                string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -P \"${LINK_SCRIPT}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
            else()
                string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxvm>\" \"${CMAKE_BINARY_DIR}/bin/library.rxbin\" \"\${work}/${DEP_A_MODULE}.rxbin\" \"\${work}/${DEP_B_MODULE}.rxbin\" \"\${work}/main${TEST_SUFFIX}.rxbin\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
            endif()
        endif()

        string(APPEND SCRIPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
        string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxvm failed: \${out}\")\nendif()\n")
        string(APPEND SCRIPT_CONTENT "file(READ \"${CMAKE_CURRENT_SOURCE_DIR}/golden/run/${NAME}.out\" golden_out)\n")
        string(APPEND SCRIPT_CONTENT "string(REPLACE \"\\r\\n\" \"\\n\" full_out \"\${full_out}\")\n")
        string(APPEND SCRIPT_CONTENT "string(REPLACE \"\\r\\n\" \"\\n\" golden_out \"\${golden_out}\")\n")
        string(APPEND SCRIPT_CONTENT "if(NOT full_out STREQUAL golden_out)\n  message(FATAL_ERROR \"Output mismatch: \\nExpected:\\n\${golden_out}\\nGot:\\n\${full_out}\")\nendif()\n")

        file(GENERATE OUTPUT "${SCRIPT_FILE}" CONTENT "${SCRIPT_CONTENT}")

        add_test(NAME ${NAME}${TEST_SUFFIX}
                 COMMAND ${CMAKE_COMMAND} -P "${SCRIPT_FILE}"
                 WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
        if(TEST_MODE STREQUAL "opt")
            set_tests_properties(${NAME}${TEST_SUFFIX} PROPERTIES LABELS "linked_opt")
        endif()
    endforeach()
endfunction()

function(add_crexx_isolated_run_test NAME SOURCE_FILE)
    get_filename_component(SOURCE_BASENAME "${SOURCE_FILE}" NAME)

    foreach(TEST_MODE IN ITEMS noopt opt)
        if(TEST_MODE STREQUAL "noopt")
            set(COMPILER_FLAG "${COMPILER_NOOPT_FLAG}")
            set(TEST_SUFFIX "_run_noopt")
        else()
            set(COMPILER_FLAG "${COMPILER_OPT_FLAG}")
            set(TEST_SUFFIX "_run_opt")
        endif()

        set(WORK_DIR "${CMAKE_CURRENT_BINARY_DIR}/${NAME}_isolated_${TEST_MODE}_ws")
        if(TEST_MODE STREQUAL "opt")
            set(LINK_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_${NAME}_isolated_${TEST_MODE}_linked.cmake")
            crexx_write_linked_runtime_script(
                    OUTPUT "${LINK_SCRIPT}"
                    NAME "${NAME}${TEST_SUFFIX}"
                    WORKING_DIRECTORY "${WORK_DIR}"
                    RUNNER "$<TARGET_FILE:rxvm>"
                    RXLINK "$<TARGET_FILE:rxlink>"
                    PROGRAM "${WORK_DIR}/${NAME}${TEST_SUFFIX}.rxbin"
                    EXTRA_ARGS "${CMAKE_BINARY_DIR}/bin/library.rxbin"
            )
        endif()

        set(SCRIPT_FILE "${CMAKE_CURRENT_BINARY_DIR}/run_${NAME}_isolated_${TEST_MODE}.cmake")
        set(SCRIPT_CONTENT "set(full_out \"\")\n")
        string(APPEND SCRIPT_CONTENT "set(work \"${WORK_DIR}\")\n")
        string(APPEND SCRIPT_CONTENT "file(REMOVE_RECURSE \"\${work}\")\n")
        string(APPEND SCRIPT_CONTENT "file(MAKE_DIRECTORY \"\${work}\")\n")
        string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E copy_if_different \"${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE}\" \"\${work}/${SOURCE_BASENAME}\" RESULT_VARIABLE res)\n")
        string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"Failed to stage source\")\nendif()\n")
        string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" ${COMPILER_FLAG} -o \"${NAME}${TEST_SUFFIX}\" \"\${work}/${SOURCE_BASENAME}\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
        string(APPEND SCRIPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
        string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed: \${out}\")\nendif()\n")
        string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"${NAME}${TEST_SUFFIX}.rxbin\" \"${NAME}${TEST_SUFFIX}\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
        string(APPEND SCRIPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
        string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed: \${out}\")\nendif()\n")
        if(TEST_MODE STREQUAL "opt")
            string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -P \"${LINK_SCRIPT}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
        else()
            string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxvm>\" \"${CMAKE_BINARY_DIR}/bin/library.rxbin\" \"\${work}/${NAME}${TEST_SUFFIX}.rxbin\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
        endif()
        string(APPEND SCRIPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
        string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"linked rxvm failed: \${out}\")\nendif()\n")
        string(APPEND SCRIPT_CONTENT "file(READ \"${CMAKE_CURRENT_SOURCE_DIR}/golden/run/${NAME}.out\" golden_out)\n")
        string(APPEND SCRIPT_CONTENT "string(REPLACE \"\\r\\n\" \"\\n\" full_out \"\${full_out}\")\n")
        string(APPEND SCRIPT_CONTENT "string(REPLACE \"\\r\\n\" \"\\n\" golden_out \"\${golden_out}\")\n")
        string(APPEND SCRIPT_CONTENT "if(NOT full_out STREQUAL golden_out)\n  message(FATAL_ERROR \"Output mismatch: \\nExpected:\\n\${golden_out}\\nGot:\\n\${full_out}\")\nendif()\n")

        file(GENERATE OUTPUT "${SCRIPT_FILE}" CONTENT "${SCRIPT_CONTENT}")

        add_test(NAME ${NAME}${TEST_SUFFIX}
                 COMMAND ${CMAKE_COMMAND} -P "${SCRIPT_FILE}"
                 WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
        if(TEST_MODE STREQUAL "opt")
            set_tests_properties(${NAME}${TEST_SUFFIX} PROPERTIES LABELS "linked_opt")
        endif()
    endforeach()
endfunction()

function(add_crexx_isolated_run_fail_test NAME SOURCE_FILE)
    get_filename_component(SOURCE_BASENAME "${SOURCE_FILE}" NAME)

    foreach(TEST_MODE IN ITEMS noopt opt)
        if(TEST_MODE STREQUAL "noopt")
            set(COMPILER_FLAG "${COMPILER_NOOPT_FLAG}")
            set(TEST_SUFFIX "_run_noopt")
        else()
            set(COMPILER_FLAG "${COMPILER_OPT_FLAG}")
            set(TEST_SUFFIX "_run_opt")
        endif()

        set(WORK_DIR "${CMAKE_CURRENT_BINARY_DIR}/${NAME}_isolated_fail_${TEST_MODE}_ws")
        if(TEST_MODE STREQUAL "opt")
            set(LINK_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_${NAME}_isolated_fail_${TEST_MODE}_linked.cmake")
            crexx_write_linked_runtime_script(
                    OUTPUT "${LINK_SCRIPT}"
                    NAME "${NAME}${TEST_SUFFIX}"
                    WORKING_DIRECTORY "${WORK_DIR}"
                    RUNNER "$<TARGET_FILE:rxvm>"
                    RXLINK "$<TARGET_FILE:rxlink>"
                    PROGRAM "${WORK_DIR}/${NAME}${TEST_SUFFIX}.rxbin"
                    ALLOW_RUN_FAILURE
                    EXTRA_ARGS "${CMAKE_BINARY_DIR}/bin/library.rxbin"
            )
        endif()

        set(SCRIPT_FILE "${CMAKE_CURRENT_BINARY_DIR}/run_${NAME}_isolated_fail_${TEST_MODE}.cmake")
        set(SCRIPT_CONTENT "set(full_out \"\")\n")
        string(APPEND SCRIPT_CONTENT "set(work \"${WORK_DIR}\")\n")
        string(APPEND SCRIPT_CONTENT "file(REMOVE_RECURSE \"\${work}\")\n")
        string(APPEND SCRIPT_CONTENT "file(MAKE_DIRECTORY \"\${work}\")\n")
        string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E copy_if_different \"${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE}\" \"\${work}/${SOURCE_BASENAME}\" RESULT_VARIABLE res)\n")
        string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"Failed to stage source\")\nendif()\n")
        string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" ${COMPILER_FLAG} -o \"${NAME}${TEST_SUFFIX}\" \"\${work}/${SOURCE_BASENAME}\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
        string(APPEND SCRIPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
        string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed: \${out}\")\nendif()\n")
        string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"${NAME}${TEST_SUFFIX}.rxbin\" \"${NAME}${TEST_SUFFIX}\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
        string(APPEND SCRIPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
        string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed: \${out}\")\nendif()\n")
        if(TEST_MODE STREQUAL "opt")
            string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -P \"${LINK_SCRIPT}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
        else()
            string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxvm>\" \"${CMAKE_BINARY_DIR}/bin/library.rxbin\" \"\${work}/${NAME}${TEST_SUFFIX}.rxbin\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
        endif()
        string(APPEND SCRIPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
        string(APPEND SCRIPT_CONTENT "file(READ \"${CMAKE_CURRENT_SOURCE_DIR}/golden/run/${NAME}.out\" golden_out)\n")
        string(APPEND SCRIPT_CONTENT "string(REPLACE \"\\r\\n\" \"\\n\" full_out \"\${full_out}\")\n")
        string(APPEND SCRIPT_CONTENT "string(REPLACE \"\\r\\n\" \"\\n\" golden_out \"\${golden_out}\")\n")
        string(APPEND SCRIPT_CONTENT "if(NOT full_out STREQUAL golden_out)\n  message(FATAL_ERROR \"Output mismatch: \\nExpected:\\n\${golden_out}\\nGot:\\n\${full_out}\")\nendif()\n")

        file(GENERATE OUTPUT "${SCRIPT_FILE}" CONTENT "${SCRIPT_CONTENT}")

        add_test(NAME ${NAME}${TEST_SUFFIX}
                 COMMAND ${CMAKE_COMMAND} -P "${SCRIPT_FILE}"
                 WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
        if(TEST_MODE STREQUAL "opt")
            set_tests_properties(${NAME}${TEST_SUFFIX} PROPERTIES LABELS "linked_opt")
        endif()
    endforeach()
endfunction()

macro(add_crexx_run_args_test NAME SOURCE_FILE)
    set(PROGRAM_ARGS ${ARGN})
    set(PROGRAM_ARGS_TEXT "")
    foreach(PROGRAM_ARG ${PROGRAM_ARGS})
        string(APPEND PROGRAM_ARGS_TEXT " \"${PROGRAM_ARG}\"")
    endforeach()

    set(NOOPT_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_${NAME}_args_noopt.cmake")
    set(NOOPT_CONTENT "set(full_out \"\")\n")
    string(APPEND NOOPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" ${COMPILER_NOOPT_FLAG} -o \"${NAME}_run_args_noopt\" \"${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
    string(APPEND NOOPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
    string(APPEND NOOPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed: \${out}\")\nendif()\n")

    string(APPEND NOOPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"${NAME}_run_args_noopt.rxbin\" \"${NAME}_run_args_noopt\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
    string(APPEND NOOPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
    string(APPEND NOOPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed: \${out}\")\nendif()\n")

    string(APPEND NOOPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxvm>\" \"${CMAKE_BINARY_DIR}/bin/library.rxbin\" \"${NAME}_run_args_noopt.rxbin\" -a${PROGRAM_ARGS_TEXT} OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
    string(APPEND NOOPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
    string(APPEND NOOPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxvm failed: \${out}\")\nendif()\n")

    string(APPEND NOOPT_CONTENT "file(READ \"${CMAKE_CURRENT_SOURCE_DIR}/golden/run/${NAME}.out\" golden_out)\n")
    string(APPEND NOOPT_CONTENT "string(REPLACE \"\\r\\n\" \"\\n\" full_out \"\${full_out}\")\n")
    string(APPEND NOOPT_CONTENT "string(REPLACE \"\\r\\n\" \"\\n\" golden_out \"\${golden_out}\")\n")
    string(APPEND NOOPT_CONTENT "if(NOT full_out STREQUAL golden_out)\n  message(FATAL_ERROR \"Output mismatch: \\nExpected:\\n\${golden_out}\\nGot:\\n\${full_out}\")\nendif()\n")
    file(GENERATE OUTPUT "${NOOPT_SCRIPT}" CONTENT "${NOOPT_CONTENT}")

    add_test(NAME ${NAME}_run_args_noopt
             COMMAND ${CMAKE_COMMAND} -P "${NOOPT_SCRIPT}"
             WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

    set(OPT_LINK_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_${NAME}_args_opt_linked.cmake")
    crexx_write_linked_runtime_script(
            OUTPUT "${OPT_LINK_SCRIPT}"
            NAME "${NAME}_run_args_opt"
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
            RUNNER "$<TARGET_FILE:rxvm>"
            RXLINK "$<TARGET_FILE:rxlink>"
            PROGRAM "${NAME}_run_args_opt.rxbin"
            EXTRA_ARGS "${CMAKE_BINARY_DIR}/bin/library.rxbin" -a ${PROGRAM_ARGS}
    )

    set(OPT_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_${NAME}_args_opt.cmake")
    set(OPT_CONTENT "set(full_out \"\")\n")
    string(APPEND OPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"${NAME}_run_args_opt\" \"${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
    string(APPEND OPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
    string(APPEND OPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed: \${out}\")\nendif()\n")

    string(APPEND OPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"${NAME}_run_args_opt.rxbin\" \"${NAME}_run_args_opt\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
    string(APPEND OPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
    string(APPEND OPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed: \${out}\")\nendif()\n")

    string(APPEND OPT_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -P \"${OPT_LINK_SCRIPT}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
    string(APPEND OPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
    string(APPEND OPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"linked rxvm failed: \${out}\")\nendif()\n")

    string(APPEND OPT_CONTENT "file(READ \"${CMAKE_CURRENT_SOURCE_DIR}/golden/run/${NAME}.out\" golden_out)\n")
    string(APPEND OPT_CONTENT "string(REPLACE \"\\r\\n\" \"\\n\" full_out \"\${full_out}\")\n")
    string(APPEND OPT_CONTENT "string(REPLACE \"\\r\\n\" \"\\n\" golden_out \"\${golden_out}\")\n")
    string(APPEND OPT_CONTENT "if(NOT full_out STREQUAL golden_out)\n  message(FATAL_ERROR \"Output mismatch: \\nExpected:\\n\${golden_out}\\nGot:\\n\${full_out}\")\nendif()\n")
    file(GENERATE OUTPUT "${OPT_SCRIPT}" CONTENT "${OPT_CONTENT}")

    add_test(NAME ${NAME}_run_args_opt
             COMMAND ${CMAKE_COMMAND} -P "${OPT_SCRIPT}"
             WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    set_tests_properties(${NAME}_run_args_opt PROPERTIES LABELS "linked_opt")
endmacro()

add_crexx_run_test(repro_dotted_index rexx_src/repro_dotted_index.crexx)
add_crexx_run_test(int64_contract rexx_src/int64_contract.crexx)
add_crexx_run_test(repro_expand_until rexx_src/repro_expand_until.crexx)
add_crexx_run_test(repro_array_arg rexx_src/repro_array_arg.crexx)
add_crexx_run_test(repro_array_trailing_concat rexx_src/repro_array_trailing_concat.crexx)
add_crexx_run_test(stem_index_concat rexx_src/stem_index_concat.crexx)
add_crexx_run_test(repro_opt_args rexx_src/repro_opt_args.crexx)
add_crexx_run_test(arg_semantics_scalar rexx_src/arg_semantics_scalar.crexx)
add_crexx_run_test(arg_semantics_array rexx_src/arg_semantics_array.crexx)
add_crexx_run_test(multiple_exposed_arrays rexx_src/multiple_exposed_arrays.crexx)
add_crexx_run_test(arg_semantics_object rexx_src/arg_semantics_object.crexx)
add_crexx_run_test(object_reference_regression rexx_src/object_reference_regression.crexx)
add_crexx_run_test(basic_loops rexx_src/basic_loops.crexx)
add_crexx_run_test(basic_strings rexx_src/basic_strings.crexx)
add_crexx_run_test(basic_arrays rexx_src/basic_arrays.crexx)
add_crexx_run_test(array_mutation_statements rexx_src/array_mutation_statements.crexx)
add_crexx_run_test(block_expr_basic rexx_src/block_expr_basic.crexx)
add_crexx_run_test(block_expr_short_circuit rexx_src/block_expr_short_circuit.crexx)
add_crexx_run_test(repro_args_parse rexx_src/repro_args_parse.crexx)
add_crexx_run_test(repro_args_alias rexx_src/repro_args_alias.crexx)
add_crexx_run_test(repro_sconcat rexx_src/repro_sconcat.crexx)
add_crexx_run_test(repro_strict_compare rexx_src/repro_strict_compare.crexx)
add_crexx_run_test(08_select rexx_src/08_select.crexx)
add_crexx_run_test(select_dispatch_integer rexx_src/select_dispatch_integer.crexx)
add_crexx_run_test(select_dispatch_fallback rexx_src/select_dispatch_fallback.crexx)
add_crexx_run_test(select_dispatch_mixed rexx_src/select_dispatch_mixed.crexx)
add_crexx_run_test(select_dispatch_general rexx_src/select_dispatch_general.crexx)
add_crexx_run_test(select_dispatch_strings rexx_src/select_dispatch_strings.crexx)
add_crexx_run_test(repro_duplicate_call_argument rexx_src/repro_duplicate_call_argument.crexx)
add_crexx_run_test(nr06_call_window_scalar rexx_src/nr06_call_window_scalar.crexx)
add_crexx_run_test(13_stems rexx_src/13_stems.crexx)
add_crexx_run_test(repro_multi_tail_stems rexx_src/repro_multi_tail_stems.crexx)
add_crexx_run_test(repro_rexx_compare_method rexx_src/repro_rexx_compare_method.crexx "${CMAKE_BINARY_DIR}/bin/classlib.rxbin")
add_crexx_run_test(repro_rexx_format_sign_gap rexx_src/repro_rexx_format_sign_gap.crexx "${CMAKE_BINARY_DIR}/bin/classlib.rxbin")
add_crexx_run_test(numeric_mixed_compare rexx_src/numeric_mixed_compare.crexx)
add_crexx_run_test(repro_implicit_address_abuttal rexx_src/repro_implicit_address_abuttal.crexx)
add_crexx_run_test(implicit_address_indexed_stem rexx_src/implicit_address_indexed_stem.crexx)
add_crexx_run_test(test_explicit_address rexx_src/test_explicit_address.crexx)
add_crexx_run_test(test_address_redirect rexx_src/test_address_redirect.crexx)
add_crexx_run_test(address_crexx_flush_visible rexx_src/address_crexx_flush_visible.crexx)
add_crexx_run_test(lineout_flush_visible rexx_src/lineout_flush_visible.crexx)
add_crexx_run_test(optional_arg_presence_copy rexx_src/optional_arg_presence_copy.crexx)
add_crexx_run_test(test_implicit_address rexx_src/test_implicit_address.crexx)
add_crexx_run_test(test_parse_statement rexx_src/test_parse_statement.crexx)
add_crexx_isolated_run_test(qualified_namespace_same_module rexx_src/qualified_namespace_same_module.crexx)
add_crexx_isolated_run_test(class_attr_string_promotion rexx_src/class_attr_string_promotion.crexx)
add_crexx_run_test(class_attr_register_views rexx_src/class_attr_register_views.crexx)
add_crexx_run_test(constants_levelb_payloads rexx_src/constants_levelb_payloads.crexx)
add_crexx_run_test(binary_memory_source rexx_src/binary_memory_source.crexx)
add_crexx_run_test(binary_memory_span_oob rexx_src/binary_memory_span_oob.crexx)
add_crexx_run_test(binary_memory_packed_lookup rexx_src/binary_memory_packed_lookup.crexx)
add_crexx_run_test(literal_continuation rexx_src/literal_continuation.crexx)
add_crexx_isolated_run_test(qualified_bif_direct_namespace rexx_src/qualified_bif_direct_namespace.crexx)
add_crexx_dual_import_run_test(qualified_namespace_import_source rexx_src/qualified_namespace_main.crexx rexx_src/qualified_namespace_dep_a.crexx rexx_src/qualified_namespace_dep_b.crexx source)
add_crexx_dual_import_run_test(qualified_namespace_import_binary rexx_src/qualified_namespace_main.crexx rexx_src/qualified_namespace_dep_a.crexx rexx_src/qualified_namespace_dep_b.crexx binary)
add_crexx_isolated_run_test(interface_tracer rexx_src/interface_tracer.crexx)
add_crexx_isolated_run_test(interface_runtime_dispatch_multi rexx_src/interface_runtime_dispatch_multi.crexx)
add_crexx_isolated_run_test(interface_method_arg_same_module rexx_src/interface_method_arg_same_module.crexx)
add_crexx_isolated_run_test(interface_method_object_arg_same_module rexx_src/interface_method_object_arg_same_module.crexx)
add_crexx_isolated_run_test(interface_signature_static_positive rexx_src/interface_signature_static_positive.crexx)
add_crexx_isolated_run_test(interface_factory_tiebreak rexx_src/interface_factory_tiebreak.crexx)
add_crexx_isolated_run_fail_test(interface_no_impl rexx_src/interface_no_impl.crexx)
add_crexx_compiler_test(interface_default_codegen rexx_src/interface_default_codegen.crexx)
add_crexx_isolated_run_test(interface_default_same_module rexx_src/interface_default_same_module.crexx)
add_crexx_isolated_run_test(interface_multi_interface_same_module rexx_src/interface_multi_interface_same_module.crexx)
add_crexx_isolated_run_test(interface_showcase_same_module rexx_src/interface_showcase_same_module.crexx)
add_crexx_isolated_run_test(type_ops_showcase rexx_src/type_ops_showcase.crexx)
add_crexx_isolated_run_test(interface_match_same_module rexx_src/interface_match_same_module.crexx)
add_crexx_isolated_run_fail_test(interface_match_reject_single rexx_src/interface_match_reject_single.crexx)
add_crexx_isolated_run_fail_test(type_ops_fail rexx_src/type_ops_fail.crexx)

add_crexx_isolated_run_test(interface_named_factory_same_module rexx_src/interface_named_factory_same_module.crexx)
add_crexx_isolated_run_test(interface_named_factory_tiebreak rexx_src/interface_named_factory_tiebreak.crexx)
add_crexx_isolated_run_fail_test(interface_named_factory_no_impl rexx_src/interface_named_factory_no_impl.crexx)
add_crexx_interface_import_run_test(interface_default_source_import rexx_src/interface_default_main.crexx rexx_src/interface_default_dep_contract.crexx source)
add_crexx_interface_import_run_test(interface_default_binary_import rexx_src/interface_default_binary_main.crexx rexx_src/interface_default_dep_contract.crexx binary)
add_crexx_interface_import_run_test(interface_source_import rexx_src/interface_source_import_main.crexx rexx_src/interface_dep_contract.crexx source)
add_crexx_interface_import_run_test(interface_binary_import rexx_src/interface_binary_import_main.crexx rexx_src/interface_dep_contract.crexx binary)
add_crexx_interface_import_run_test(interface_match_source_import rexx_src/interface_match_main.crexx rexx_src/interface_match_dep_contract.crexx source)
add_crexx_interface_import_run_test(interface_match_binary_import rexx_src/interface_match_main.crexx rexx_src/interface_match_dep_contract.crexx binary)
add_crexx_interface_import_run_test(interface_named_factory_source_import rexx_src/interface_named_factory_main.crexx rexx_src/interface_named_factory_dep_contract.crexx source)
add_crexx_interface_import_run_test(interface_named_factory_binary_import rexx_src/interface_named_factory_main.crexx rexx_src/interface_named_factory_dep_contract.crexx binary)
add_crexx_dual_import_run_test(interface_import_provider_source rexx_src/interface_import_provider_main.crexx rexx_src/interface_import_provider_contract.crexx rexx_src/interface_import_provider_impl.crexx source)
add_crexx_dual_import_run_test(interface_import_provider_binary rexx_src/interface_import_provider_main.crexx rexx_src/interface_import_provider_contract.crexx rexx_src/interface_import_provider_impl.crexx binary)
add_crexx_dual_import_run_test(interface_split_same_namespace_source rexx_src/interface_split_same_namespace_main.crexx rexx_src/interface_split_same_namespace_contract.crexx rexx_src/interface_split_same_namespace_provider.crexx source)
add_crexx_dual_import_run_test(interface_split_same_namespace_binary rexx_src/interface_split_same_namespace_main.crexx rexx_src/interface_split_same_namespace_contract.crexx rexx_src/interface_split_same_namespace_provider.crexx binary)
add_crexx_dual_import_run_test(interface_binary_covariant rexx_src/interface_binary_covariant_main.crexx rexx_src/interface_binary_covariant_contract.crexx rexx_src/interface_binary_covariant_provider.crexx binary)
add_crexx_dual_import_run_test(qualified_interface_import rexx_src/qualified_interface_main.crexx rexx_src/qualified_interface_dep_a.crexx rexx_src/qualified_interface_dep_b.crexx source)
add_crexx_dual_import_run_test(qualified_interface_binary_import rexx_src/qualified_interface_main.crexx rexx_src/qualified_interface_dep_a.crexx rexx_src/qualified_interface_dep_b.crexx binary)

add_test(
        NAME imported_record_return_contract
        COMMAND ${CMAKE_COMMAND}
                -DRXC=$<TARGET_FILE:rxc>
                -DRXAS=$<TARGET_FILE:rxas>
                -DRXVM=$<TARGET_FILE:rxvm>
                -DRXBVM=$<TARGET_FILE:rxbvm>
                -DLIBRARY=${CMAKE_BINARY_DIR}/bin
                -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src
                -DWORK_ROOT=${CMAKE_CURRENT_BINARY_DIR}/imported_record_return_contract_ws
                -P ${CMAKE_CURRENT_SOURCE_DIR}/imported_record_return_contract.cmake
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(imported_record_return_contract PROPERTIES
        LABELS "compiler;imports;runtime;optimizer")

add_test(
        NAME do_forever_return_contract
        COMMAND ${CMAKE_COMMAND}
                -DRXC=$<TARGET_FILE:rxc>
                -DRXAS=$<TARGET_FILE:rxas>
                -DRXVM=$<TARGET_FILE:rxvm>
                -DRXBVM=$<TARGET_FILE:rxbvm>
                -DLIBRARY=${CMAKE_BINARY_DIR}/bin
                -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src
                -DWORK_ROOT=${CMAKE_CURRENT_BINARY_DIR}/do_forever_return_contract_ws
                -P ${CMAKE_CURRENT_SOURCE_DIR}/do_forever_return_contract.cmake
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(do_forever_return_contract PROPERTIES
        LABELS "compiler;control-flow;runtime;optimizer"
        TIMEOUT 120)

add_test(
        NAME parse_levelg_contract
        COMMAND ${CMAKE_COMMAND}
                -DRXC=$<TARGET_FILE:rxc>
                -DRXAS=$<TARGET_FILE:rxas>
                -DRXVM=$<TARGET_FILE:rxvm>
                -DRXBVM=$<TARGET_FILE:rxbvm>
                -DLIBRARY=${CMAKE_BINARY_DIR}/bin
                -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src
                -DWORK_ROOT=${CMAKE_CURRENT_BINARY_DIR}/parse_levelg_contract_ws
                -P ${CMAKE_CURRENT_SOURCE_DIR}/parse_levelg_contract.cmake
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(parse_levelg_contract PROPERTIES
        LABELS "compiler;exits;parse;runtime;optimizer"
        TIMEOUT 120)

set(INLINE_CROSS_FILE_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_inline_cross_file_binary_opt.cmake")
set(INLINE_CROSS_FILE_CONTENT "set(full_out \"\")\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "set(work \"${CMAKE_CURRENT_BINARY_DIR}/inline_cross_file_binary_opt_ws\")\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "file(REMOVE_RECURSE \"\${work}\")\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "file(MAKE_DIRECTORY \"\${work}\")\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E copy_if_different \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/inline_cross_file_dep.crexx\" \"\${work}/inline_cross_file_dep.crexx\" RESULT_VARIABLE res)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"Failed to stage inline dependency\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E copy_if_different \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/inline_cross_file_main.crexx\" \"\${work}/inline_cross_file_main.crexx\" RESULT_VARIABLE res)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"Failed to stage inline main\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"inline_cross_file_dep\" \"\${work}/inline_cross_file_dep.crexx\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(APPEND full_out \"\${out}\")\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed on inline dependency: \${out}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "file(READ \"\${work}/inline_cross_file_dep.rxas\" dep_rxas)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(FIND \"\${dep_rxas}\" \".meta \\\"inline_cross_file_dep.inc\\\"\" dep_meta_pos)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(FIND \"\${dep_rxas}\" \"=\\\".inline\\\" \\\"I6;c,1,\" dep_inline_pos)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(FIND \"\${dep_rxas}\" \".meta \\\"inline_cross_file_dep.classify\\\"\" dep_if_meta_pos)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(FIND \"\${dep_rxas}\" \".meta \\\"inline_cross_file_dep.scoped\\\"\" dep_scoped_meta_pos)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(FIND \"\${dep_rxas}\" \".meta \\\"inline_cross_file_dep.nested\\\"\" dep_nested_meta_pos)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(FIND \"\${dep_rxas}\" \".meta \\\"inline_cross_file_dep.sumto\\\"\" dep_sum_meta_pos)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(FIND \"\${dep_rxas}\" \".meta \\\"inline_cross_file_dep.countuntil\\\"\" dep_count_meta_pos)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(FIND \"\${dep_rxas}\" \".meta \\\"inline_cross_file_dep.buildarray\\\"\" dep_array_meta_pos)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(FIND \"\${dep_rxas}\" \".meta \\\"inline_cross_file_dep.renderarray\\\"\" dep_render_meta_pos)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(FIND \"\${dep_rxas}\" \".meta \\\"inline_cross_file_dep.identitybox\\\"\" dep_object_meta_pos)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(FIND \"\${dep_rxas}\" \".meta \\\"inline_cross_file_dep.refbump\\\"\" dep_ref_meta_pos)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(FIND \"\${dep_rxas}\" \".meta \\\"inline_cross_file_dep.optadd\\\"\" dep_opt_meta_pos)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(FIND \"\${dep_rxas}\" \".meta \\\"inline_cross_file_dep.safelength\\\"\" dep_safe_asm_meta_pos)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(FIND \"\${dep_rxas}\" \".meta \\\"inline_cross_file_dep.typebump\\\"\" dep_type_bump_meta_pos)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(FIND \"\${dep_rxas}\" \".meta \\\"inline_cross_file_dep.vargtotal\\\"\" dep_varg_total_meta_pos)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(FIND \"\${dep_rxas}\" \".meta \\\"inline_cross_file_dep.vargpick\\\"\" dep_varg_pick_meta_pos)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(FIND \"\${dep_rxas}\" \".meta \\\"inline_cross_file_dep.residualfunctiondependency\\\"\" dep_residual_func_meta_pos)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(FIND \"\${dep_rxas}\" \".meta \\\"inline_cross_file_dep.residualcalldependency\\\"\" dep_residual_call_meta_pos)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(FIND \"\${dep_rxas}\" \".meta \\\"inline_cross_file_dep.residualcalltarget\\\"\" dep_residual_target_meta_pos)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(FIND \"\${dep_rxas}\" \".meta \\\"inline_cross_file_dep.box.getname\\\"\" dep_box_getname_meta_pos)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(FIND \"\${dep_rxas}\" \".meta \\\"inline_cross_file_dep.box.setname\\\"\" dep_box_setname_meta_pos)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(FIND \"\${dep_rxas}\" \".meta \\\"inline_cross_file_dep.aliasblocked\\\"\" dep_alias_meta_pos)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(FIND \"\${dep_rxas}\" \".meta \\\"inline_cross_file_dep.refvargdynamicblocked\\\"\" dep_ref_varg_meta_pos)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(FIND \"\${dep_rxas}\" \".meta \\\"inline_cross_file_dep.residualcallblocked\\\"\" dep_residual_meta_pos)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(FIND \"\${dep_rxas}\" \".meta \\\"inline_cross_file_dep.membercallblocked\\\"\" dep_member_meta_pos)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(dep_meta_pos EQUAL -1 OR dep_if_meta_pos EQUAL -1 OR dep_scoped_meta_pos EQUAL -1 OR dep_nested_meta_pos EQUAL -1 OR dep_sum_meta_pos EQUAL -1 OR dep_count_meta_pos EQUAL -1 OR dep_array_meta_pos EQUAL -1 OR dep_render_meta_pos EQUAL -1 OR dep_object_meta_pos EQUAL -1 OR dep_ref_meta_pos EQUAL -1 OR dep_opt_meta_pos EQUAL -1 OR dep_safe_asm_meta_pos EQUAL -1 OR dep_type_bump_meta_pos EQUAL -1 OR dep_varg_total_meta_pos EQUAL -1 OR dep_varg_pick_meta_pos EQUAL -1 OR dep_residual_func_meta_pos EQUAL -1 OR dep_residual_call_meta_pos EQUAL -1 OR dep_residual_target_meta_pos EQUAL -1 OR dep_box_getname_meta_pos EQUAL -1 OR dep_box_setname_meta_pos EQUAL -1 OR dep_inline_pos EQUAL -1)\n  message(FATAL_ERROR \"Dependency did not export expected inline metadata records\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(dep_alias_meta_pos EQUAL -1 OR dep_ref_varg_meta_pos EQUAL -1 OR dep_residual_meta_pos EQUAL -1 OR dep_member_meta_pos EQUAL -1)\n  message(FATAL_ERROR \"Dependency did not emit expected fail-closed callable signatures\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(FIND \"\${dep_rxas}\" \".meta \\\"inline_cross_file_dep.safelength\\\"=\\\".inline\\\" \\\"I6;c,1,\" dep_safe_asm_inline_pos)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(dep_safe_asm_inline_pos EQUAL -1)\n  message(FATAL_ERROR \"Safe assembler helper did not export inline metadata\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(FIND \"\${dep_rxas}\" \";d,\" dep_dependency_record_pos)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(dep_dependency_record_pos EQUAL -1)\n  message(FATAL_ERROR \"Dependency did not export residual-call dependency records\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "foreach(expected_inline_name typebump vargtotal residualfunctiondependency residualcalldependency box.getname box.setname)\n  string(FIND \"\${dep_rxas}\" \".meta \\\"inline_cross_file_dep.\${expected_inline_name}\\\"=\\\".inline\\\" \\\"I6;c,1,\" dep_expected_inline_pos)\n  if(dep_expected_inline_pos EQUAL -1)\n    message(FATAL_ERROR \"Dependency did not export I6 callable summary metadata for \${expected_inline_name}\")\n  endif()\nendforeach()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "foreach(blocked_name aliasblocked refvargdynamicblocked vargpick residualcalltarget residualcallblocked membercallblocked)\n  string(FIND \"\${dep_rxas}\" \".meta \\\"inline_cross_file_dep.\${blocked_name}\\\"=\\\".inline\\\" \\\"I6\" dep_blocked_inline_pos)\n  if(NOT dep_blocked_inline_pos EQUAL -1)\n    message(FATAL_ERROR \"Fail-closed callable \${blocked_name} unexpectedly exported inline metadata\")\n  endif()\nendforeach()\n")
# Evidence-opening regression: remove the legacy ARG is_const_arg hint from the
# serialized body.  The independently reconstructed I6 read-only summary must
# still permit direct scalar binding; the I5/mismatch controls below must not.
string(APPEND INLINE_CROSS_FILE_CONTENT "string(REPLACE \";>,0,0,3,3,3,0,0,-,-,-,-,-,-,4,-1\" \";>,0,0,3,3,3,0,0,-,-,-,-,-,-,0,-1\" dep_without_legacy_hint \"\${dep_rxas}\")\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(dep_without_legacy_hint STREQUAL dep_rxas)\n  message(FATAL_ERROR \"Failed to remove legacy scalar-formal hint for I6 evidence-opening regression\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "file(WRITE \"\${work}/inline_cross_file_dep.rxas\" \"\${dep_without_legacy_hint}\")\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"inline_cross_file_dep.rxbin\" \"inline_cross_file_dep\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(APPEND full_out \"\${out}\")\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed on inline dependency: \${out}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxdas>\" -o \"inline_cross_file_dep.roundtrip.rxas\" \"inline_cross_file_dep.rxbin\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(APPEND full_out \"\${out}\")\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxdas failed on inline dependency: \${out}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "file(READ \"\${work}/inline_cross_file_dep.roundtrip.rxas\" dep_roundtrip_rxas)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(FIND \"\${dep_roundtrip_rxas}\" \";d,\" dep_roundtrip_dependency_record_pos)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(dep_roundtrip_dependency_record_pos EQUAL -1)\n  message(FATAL_ERROR \"RXDAS roundtrip lost residual-call dependency records\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "foreach(roundtrip_inline_name inc typebump vargtotal residualfunctiondependency residualcalldependency box.getname box.setname)\n  string(FIND \"\${dep_roundtrip_rxas}\" \".meta \\\"inline_cross_file_dep.\${roundtrip_inline_name}\\\"=\\\".inline\\\" \\\"I6;c,1,\" dep_roundtrip_inline_pos)\n  if(dep_roundtrip_inline_pos EQUAL -1)\n    message(FATAL_ERROR \"RXDAS roundtrip lost I6 callable summary metadata for \${roundtrip_inline_name}\")\n  endif()\nendforeach()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "foreach(blocked_name aliasblocked refvargdynamicblocked vargpick residualcalltarget residualcallblocked membercallblocked)\n  string(FIND \"\${dep_roundtrip_rxas}\" \".meta \\\"inline_cross_file_dep.\${blocked_name}\\\"=\\\".inline\\\" \\\"I6\" dep_roundtrip_blocked_inline_pos)\n  if(NOT dep_roundtrip_blocked_inline_pos EQUAL -1)\n    message(FATAL_ERROR \"RXDAS roundtrip unexpectedly gained inline metadata for \${blocked_name}\")\n  endif()\nendforeach()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"inline_cross_file_dep.roundtrip\" \"inline_cross_file_dep.roundtrip.rxas\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(APPEND full_out \"\${out}\")\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed on rxdas inline dependency: \${out}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "file(REMOVE \"\${work}/inline_cross_file_dep.rxbin\")\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "file(RENAME \"\${work}/inline_cross_file_dep.roundtrip.rxbin\" \"\${work}/inline_cross_file_dep.rxbin\")\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "file(REMOVE \"\${work}/inline_cross_file_dep.crexx\" \"\${work}/inline_cross_file_dep.rxas\" \"\${work}/inline_cross_file_dep.roundtrip.rxas\")\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"\${work}\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"inline_cross_file_main\" \"\${work}/inline_cross_file_main.crexx\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(APPEND full_out \"\${out}\")\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed on inline main: \${out}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "file(READ \"\${work}/inline_cross_file_main.rxas\" main_rxas)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(FIND \"\${main_rxas}\" \"\\\"inline_cross_file_dep.crexx\\\" 70 3 16 \\\"  result = .int\\\"\" main_dep_source_pos)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(FIND \"\${main_rxas}\" \"\\\"inline_cross_file_dep.crexx\\\" 71 13 32 \\\"  assembler strlen result,value\\\"\" main_dep_asm_source_pos)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(main_dep_source_pos EQUAL -1 OR main_dep_asm_source_pos EQUAL -1)\n  message(FATAL_ERROR \"Optimized binary import did not preserve inlined dependency source metadata:\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(main_rxas MATCHES [=[call [^\\n]*inc\\(\\)]=])\n  message(FATAL_ERROR \"Optimized binary import still calls inc():\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(main_rxas MATCHES [=[call [^\\n]*classify\\(\\)]=])\n  message(FATAL_ERROR \"Optimized binary import still calls classify():\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(main_rxas MATCHES [=[call [^\\n]*scoped\\(\\)]=])\n  message(FATAL_ERROR \"Optimized binary import still calls scoped():\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(main_rxas MATCHES [=[call [^\\n]*nested\\(\\)]=])\n  message(FATAL_ERROR \"Optimized binary import still calls nested():\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(main_rxas MATCHES [=[call [^\\n]*sumto\\(\\)]=])\n  message(FATAL_ERROR \"Optimized binary import still calls sumTo():\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(main_rxas MATCHES [=[call [^\\n]*countuntil\\(\\)]=])\n  message(FATAL_ERROR \"Optimized binary import still calls countUntil():\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(main_rxas MATCHES [=[call [^\\n]*buildarray\\(\\)]=])\n  message(FATAL_ERROR \"Optimized binary import still calls buildArray():\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(main_rxas MATCHES [=[call [^\\n]*renderarray\\(\\)]=])\n  message(FATAL_ERROR \"Optimized binary import still calls renderArray():\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(main_rxas MATCHES [=[call [^\\n]*identitybox\\(\\)]=])\n  message(FATAL_ERROR \"Optimized binary import still calls identityBox():\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(main_rxas MATCHES [=[call [^\\n]*refbump\\(\\)]=])\n  message(FATAL_ERROR \"Optimized binary import still calls refBump():\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(main_rxas MATCHES [=[call [^\\n]*optadd\\(\\)]=])\n  message(FATAL_ERROR \"Optimized binary import still calls optAdd():\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(main_rxas MATCHES [=[call [^\\n]*residualfunctiondependency\\(\\)]=])\n  message(FATAL_ERROR \"Optimized binary import still calls residualFunctionDependency():\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(main_rxas MATCHES [=[call [^\\n]*residualcalldependency\\(\\)]=])\n  message(FATAL_ERROR \"Optimized binary import still calls residualCallDependency():\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(main_rxas MATCHES [=[call [^\\n]*inline_cross_file_dep\\.box\\.getname\\(\\)]=])\n  message(FATAL_ERROR \"Optimized binary import still calls box.getName():\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(main_rxas MATCHES [=[call [^\\n]*inline_cross_file_dep\\.box\\.setname\\(\\)]=])\n  message(FATAL_ERROR \"Optimized binary import still calls box.setName():\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "foreach(inlined_name inc classify scoped nested sumto countuntil buildarray renderarray identitybox refbump optadd safelength typebump vargtotal residualfunctiondependency residualcalldependency)\n  if(main_rxas MATCHES \"call [^\\n]*inline_cross_file_dep\\\\.\${inlined_name}\\\\(\\\\)\")\n    message(FATAL_ERROR \"Optimized binary import still calls \${inlined_name}():\\n\${main_rxas}\")\n  endif()\nendforeach()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(REPLACE \"\\n\" \"|\" flat_main_rxas \"\${main_rxas}\")\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(REGEX MATCH [=[\\.traceevent \"V\"[^|]*\"r\" ([0-9]+)[^|]*\"shared\"[^|]*\\|[ \\t]*\\.traceevent \"A\"[^|]*\"r\" ([0-9]+)[^|]*\"value\"]=] shared_binding \"\${flat_main_rxas}\")\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "set(shared_source_reg \"\${CMAKE_MATCH_1}\")\nset(shared_formal_reg \"\${CMAKE_MATCH_2}\")\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(shared_binding STREQUAL \"\" OR NOT shared_source_reg STREQUAL shared_formal_reg)\n  message(FATAL_ERROR \"Binary-import callable summary did not preserve direct scalar binding equivalence:\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(REGEX MATCH [=[\\.traceevent \"V\"[^|]*\"r\" ([0-9]+)[^|]*\"same\"[^|]*\\|[ \\t]*\\.traceevent \"A\"[^|]*\"r\" ([0-9]+)[^|]*\"§this\"]=] receiver_binding \"\${flat_main_rxas}\")\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "set(receiver_source_reg \"\${CMAKE_MATCH_1}\")\nset(receiver_this_reg \"\${CMAKE_MATCH_2}\")\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(receiver_binding STREQUAL \"\" OR NOT receiver_source_reg STREQUAL receiver_this_reg)\n  message(FATAL_ERROR \"Binary-import method summary did not preserve direct receiver equivalence:\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(NOT main_rxas MATCHES [=[call [^\\n]*inline_cross_file_dep\\.vargpick\\(\\)]=])\n  message(FATAL_ERROR \"Optimized binary import unexpectedly inlined dynamic vargPick():\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(NOT main_rxas MATCHES [=[call [^\\n]*inline_cross_file_dep\\.residualcalltarget\\(\\)]=])\n  message(FATAL_ERROR \"Optimized binary import did not preserve residual call dependency:\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"inline_cross_file_main.rxbin\" \"inline_cross_file_main\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(APPEND full_out \"\${out}\")\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed on inline main: \${out}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxvm>\" \"${CMAKE_BINARY_DIR}/bin/library.rxbin\" \"\${work}/inline_cross_file_dep.rxbin\" \"\${work}/inline_cross_file_main.rxbin\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(APPEND full_out \"\${out}\")\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxvm failed: \${out}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(REPLACE \"\\r\\n\" \"\\n\" full_out \"\${full_out}\")\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(NOT full_out STREQUAL \"42\\n7\\n-1\\n0\\n1\\n20\\n-3\\n42\\n10\\n3\\nred:blue\\nseed\\nafter\\n11\\n11\\n15\\n8\\n21\\n21\\n1\\n22\\n.int\\n23\\n6\\n12\\n9\\n10\\ncall:12\\n\")\n  message(FATAL_ERROR \"Output mismatch: expected inline output, got: \${full_out}\")\nendif()\n")
file(GENERATE OUTPUT "${INLINE_CROSS_FILE_SCRIPT}" CONTENT "${INLINE_CROSS_FILE_CONTENT}")

add_test(NAME inline_cross_file_binary_opt
         COMMAND ${CMAKE_COMMAND} -P "${INLINE_CROSS_FILE_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(inline_cross_file_binary_opt PROPERTIES LABELS "linked_opt")

# Review regression: a body payload without the exact callable-summary schema
# must remain a normal call.  Downgrade only the I6 body header while retaining
# the c-record so the importer exercises its fail-closed version/shape path.
set(INLINE_SUMMARY_VERSION_FALLBACK_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_inline_summary_version_fallback_binary_opt.cmake")
set(INLINE_SUMMARY_VERSION_FALLBACK_CONTENT "set(work \"${CMAKE_CURRENT_BINARY_DIR}/inline_summary_version_fallback_binary_opt_ws\")\n")
string(APPEND INLINE_SUMMARY_VERSION_FALLBACK_CONTENT "file(REMOVE_RECURSE \"\${work}\")\n")
string(APPEND INLINE_SUMMARY_VERSION_FALLBACK_CONTENT "file(MAKE_DIRECTORY \"\${work}\")\n")
string(APPEND INLINE_SUMMARY_VERSION_FALLBACK_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E copy_if_different \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/inline_cross_file_dep.crexx\" \"\${work}/inline_cross_file_dep.crexx\" RESULT_VARIABLE res)\n")
string(APPEND INLINE_SUMMARY_VERSION_FALLBACK_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E copy_if_different \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/inline_cross_file_main.crexx\" \"\${work}/inline_cross_file_main.crexx\" RESULT_VARIABLE res)\n")
string(APPEND INLINE_SUMMARY_VERSION_FALLBACK_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"inline_cross_file_dep\" \"\${work}/inline_cross_file_dep.crexx\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)\n")
string(APPEND INLINE_SUMMARY_VERSION_FALLBACK_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed on summary dependency: \${out}\${err}\")\nendif()\n")
string(APPEND INLINE_SUMMARY_VERSION_FALLBACK_CONTENT "file(READ \"\${work}/inline_cross_file_dep.rxas\" dep_rxas)\n")
string(APPEND INLINE_SUMMARY_VERSION_FALLBACK_CONTENT "string(REPLACE \"\\\"I6;c,1,\" \"\\\"I5;c,1,\" malformed_rxas \"\${dep_rxas}\")\n")
string(APPEND INLINE_SUMMARY_VERSION_FALLBACK_CONTENT "string(FIND \"\${malformed_rxas}\" \"\\\"I5;c,1,\" malformed_pos)\n")
string(APPEND INLINE_SUMMARY_VERSION_FALLBACK_CONTENT "if(malformed_pos EQUAL -1)\n  message(FATAL_ERROR \"Failed to construct old-version callable-summary payload\")\nendif()\n")
string(APPEND INLINE_SUMMARY_VERSION_FALLBACK_CONTENT "file(WRITE \"\${work}/inline_cross_file_dep.rxas\" \"\${malformed_rxas}\")\n")
string(APPEND INLINE_SUMMARY_VERSION_FALLBACK_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"inline_cross_file_dep.rxbin\" \"inline_cross_file_dep\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)\n")
string(APPEND INLINE_SUMMARY_VERSION_FALLBACK_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed on old-version summary dependency: \${out}\${err}\")\nendif()\n")
string(APPEND INLINE_SUMMARY_VERSION_FALLBACK_CONTENT "file(REMOVE \"\${work}/inline_cross_file_dep.crexx\" \"\${work}/inline_cross_file_dep.rxas\")\n")
string(APPEND INLINE_SUMMARY_VERSION_FALLBACK_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"\${work}\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"inline_cross_file_main\" \"\${work}/inline_cross_file_main.crexx\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)\n")
string(APPEND INLINE_SUMMARY_VERSION_FALLBACK_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed on old-version summary consumer: \${out}\${err}\")\nendif()\n")
string(APPEND INLINE_SUMMARY_VERSION_FALLBACK_CONTENT "file(READ \"\${work}/inline_cross_file_main.rxas\" main_rxas)\n")
string(APPEND INLINE_SUMMARY_VERSION_FALLBACK_CONTENT "if(NOT main_rxas MATCHES [=[call [^\\n]*inline_cross_file_dep\\.inc\\(\\)]=])\n  message(FATAL_ERROR \"Old-version summary payload did not retain inc() call:\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_SUMMARY_VERSION_FALLBACK_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"inline_cross_file_main.rxbin\" \"inline_cross_file_main\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)\n")
string(APPEND INLINE_SUMMARY_VERSION_FALLBACK_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed on old-version summary consumer: \${out}\${err}\")\nendif()\n")
string(APPEND INLINE_SUMMARY_VERSION_FALLBACK_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxvm>\" \"${CMAKE_BINARY_DIR}/bin/library.rxbin\" \"\${work}/inline_cross_file_dep.rxbin\" \"\${work}/inline_cross_file_main.rxbin\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE run_out ERROR_VARIABLE err RESULT_VARIABLE res)\n")
string(APPEND INLINE_SUMMARY_VERSION_FALLBACK_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxvm failed on old-version summary fallback: \${run_out}\${err}\")\nendif()\n")
string(APPEND INLINE_SUMMARY_VERSION_FALLBACK_CONTENT "string(REPLACE \"\\r\\n\" \"\\n\" run_out \"\${run_out}\")\n")
string(APPEND INLINE_SUMMARY_VERSION_FALLBACK_CONTENT "if(NOT run_out STREQUAL \"42\\n7\\n-1\\n0\\n1\\n20\\n-3\\n42\\n10\\n3\\nred:blue\\nseed\\nafter\\n11\\n11\\n15\\n8\\n21\\n21\\n1\\n22\\n.int\\n23\\n6\\n12\\n9\\n10\\ncall:12\\n\")\n  message(FATAL_ERROR \"Old-version summary fallback output mismatch: \${run_out}\")\nendif()\n")
file(GENERATE OUTPUT "${INLINE_SUMMARY_VERSION_FALLBACK_SCRIPT}" CONTENT "${INLINE_SUMMARY_VERSION_FALLBACK_CONTENT}")

add_test(NAME inline_summary_version_fallback_binary_opt
         COMMAND ${CMAKE_COMMAND} -P "${INLINE_SUMMARY_VERSION_FALLBACK_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(inline_summary_version_fallback_binary_opt PROPERTIES LABELS "linked_opt")

# Review regression: a same-version summary whose result shape disagrees with
# the independently parsed callable declaration must remain a normal call.
set(INLINE_SUMMARY_SHAPE_FALLBACK_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_inline_summary_shape_fallback_binary_opt.cmake")
set(INLINE_SUMMARY_SHAPE_FALLBACK_CONTENT "set(work \"${CMAKE_CURRENT_BINARY_DIR}/inline_summary_shape_fallback_binary_opt_ws\")\n")
string(APPEND INLINE_SUMMARY_SHAPE_FALLBACK_CONTENT "file(REMOVE_RECURSE \"\${work}\")\n")
string(APPEND INLINE_SUMMARY_SHAPE_FALLBACK_CONTENT "file(MAKE_DIRECTORY \"\${work}\")\n")
string(APPEND INLINE_SUMMARY_SHAPE_FALLBACK_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E copy_if_different \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/inline_cross_file_dep.crexx\" \"\${work}/inline_cross_file_dep.crexx\" RESULT_VARIABLE res)\n")
string(APPEND INLINE_SUMMARY_SHAPE_FALLBACK_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E copy_if_different \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/inline_cross_file_main.crexx\" \"\${work}/inline_cross_file_main.crexx\" RESULT_VARIABLE res)\n")
string(APPEND INLINE_SUMMARY_SHAPE_FALLBACK_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"inline_cross_file_dep\" \"\${work}/inline_cross_file_dep.crexx\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)\n")
string(APPEND INLINE_SUMMARY_SHAPE_FALLBACK_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed on summary dependency: \${out}\${err}\")\nendif()\n")
string(APPEND INLINE_SUMMARY_SHAPE_FALLBACK_CONTENT "file(READ \"\${work}/inline_cross_file_dep.rxas\" dep_rxas)\n")
string(APPEND INLINE_SUMMARY_SHAPE_FALLBACK_CONTENT "string(REPLACE \"I6;c,1,1,3,0,4,0,7,5,0,0,0,0,3,0,400;\" \"I6;c,1,1,6,0,4,0,7,5,0,0,0,0,3,0,400;\" malformed_rxas \"\${dep_rxas}\")\n")
string(APPEND INLINE_SUMMARY_SHAPE_FALLBACK_CONTENT "if(malformed_rxas STREQUAL dep_rxas)\n  message(FATAL_ERROR \"Failed to construct mismatched callable-summary payload\")\nendif()\n")
string(APPEND INLINE_SUMMARY_SHAPE_FALLBACK_CONTENT "file(WRITE \"\${work}/inline_cross_file_dep.rxas\" \"\${malformed_rxas}\")\n")
string(APPEND INLINE_SUMMARY_SHAPE_FALLBACK_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"inline_cross_file_dep.rxbin\" \"inline_cross_file_dep\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)\n")
string(APPEND INLINE_SUMMARY_SHAPE_FALLBACK_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed on mismatched summary dependency: \${out}\${err}\")\nendif()\n")
string(APPEND INLINE_SUMMARY_SHAPE_FALLBACK_CONTENT "file(REMOVE \"\${work}/inline_cross_file_dep.crexx\" \"\${work}/inline_cross_file_dep.rxas\")\n")
string(APPEND INLINE_SUMMARY_SHAPE_FALLBACK_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"\${work}\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"inline_cross_file_main\" \"\${work}/inline_cross_file_main.crexx\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)\n")
string(APPEND INLINE_SUMMARY_SHAPE_FALLBACK_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed on mismatched summary consumer: \${out}\${err}\")\nendif()\n")
string(APPEND INLINE_SUMMARY_SHAPE_FALLBACK_CONTENT "file(READ \"\${work}/inline_cross_file_main.rxas\" main_rxas)\n")
string(APPEND INLINE_SUMMARY_SHAPE_FALLBACK_CONTENT "if(NOT main_rxas MATCHES [=[call [^\\n]*inline_cross_file_dep\\.inc\\(\\)]=])\n  message(FATAL_ERROR \"Mismatched summary payload did not retain inc() call:\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_SUMMARY_SHAPE_FALLBACK_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"inline_cross_file_main.rxbin\" \"inline_cross_file_main\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)\n")
string(APPEND INLINE_SUMMARY_SHAPE_FALLBACK_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed on mismatched summary consumer: \${out}\${err}\")\nendif()\n")
string(APPEND INLINE_SUMMARY_SHAPE_FALLBACK_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxvm>\" \"${CMAKE_BINARY_DIR}/bin/library.rxbin\" \"\${work}/inline_cross_file_dep.rxbin\" \"\${work}/inline_cross_file_main.rxbin\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE run_out ERROR_VARIABLE err RESULT_VARIABLE res)\n")
string(APPEND INLINE_SUMMARY_SHAPE_FALLBACK_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxvm failed on mismatched summary fallback: \${run_out}\${err}\")\nendif()\n")
string(APPEND INLINE_SUMMARY_SHAPE_FALLBACK_CONTENT "string(REPLACE \"\\r\\n\" \"\\n\" run_out \"\${run_out}\")\n")
string(APPEND INLINE_SUMMARY_SHAPE_FALLBACK_CONTENT "if(NOT run_out STREQUAL \"42\\n7\\n-1\\n0\\n1\\n20\\n-3\\n42\\n10\\n3\\nred:blue\\nseed\\nafter\\n11\\n11\\n15\\n8\\n21\\n21\\n1\\n22\\n.int\\n23\\n6\\n12\\n9\\n10\\ncall:12\\n\")\n  message(FATAL_ERROR \"Mismatched summary fallback output mismatch: \${run_out}\")\nendif()\n")
file(GENERATE OUTPUT "${INLINE_SUMMARY_SHAPE_FALLBACK_SCRIPT}" CONTENT "${INLINE_SUMMARY_SHAPE_FALLBACK_CONTENT}")

add_test(NAME inline_summary_shape_fallback_binary_opt
         COMMAND ${CMAKE_COMMAND} -P "${INLINE_SUMMARY_SHAPE_FALLBACK_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(inline_summary_shape_fallback_binary_opt PROPERTIES LABELS "linked_opt")

# Review regression: summary formal effects cannot promote a writable
# reference formal into the exact read-only scalar case. The imported body and
# declaration independently reconstruct the real by-ref/write/escape facts,
# so contradictory payload evidence must retain the ordinary call.
set(INLINE_SUMMARY_FORMAL_FALLBACK_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_inline_summary_formal_fallback_binary_opt.cmake")
set(INLINE_SUMMARY_FORMAL_FALLBACK_CONTENT "set(work \"${CMAKE_CURRENT_BINARY_DIR}/inline_summary_formal_fallback_binary_opt_ws\")\n")
string(APPEND INLINE_SUMMARY_FORMAL_FALLBACK_CONTENT "file(REMOVE_RECURSE \"\${work}\")\n")
string(APPEND INLINE_SUMMARY_FORMAL_FALLBACK_CONTENT "file(MAKE_DIRECTORY \"\${work}\")\n")
string(APPEND INLINE_SUMMARY_FORMAL_FALLBACK_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E copy_if_different \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/inline_cross_file_dep.crexx\" \"\${work}/inline_cross_file_dep.crexx\" RESULT_VARIABLE res)\n")
string(APPEND INLINE_SUMMARY_FORMAL_FALLBACK_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E copy_if_different \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/inline_cross_file_main.crexx\" \"\${work}/inline_cross_file_main.crexx\" RESULT_VARIABLE res)\n")
string(APPEND INLINE_SUMMARY_FORMAL_FALLBACK_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"inline_cross_file_dep\" \"\${work}/inline_cross_file_dep.crexx\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)\n")
string(APPEND INLINE_SUMMARY_FORMAL_FALLBACK_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed on formal-effect dependency: \${out}\${err}\")\nendif()\n")
string(APPEND INLINE_SUMMARY_FORMAL_FALLBACK_CONTENT "file(READ \"\${work}/inline_cross_file_dep.rxas\" dep_rxas)\n")
string(APPEND INLINE_SUMMARY_FORMAL_FALLBACK_CONTENT "string(REPLACE \".meta \\\"inline_cross_file_dep.refbump\\\"=\\\".inline\\\" \\\"I6;c,1,1,3,0,4,0,7,8,1,0,0,0,3,0,481;\" \".meta \\\"inline_cross_file_dep.refbump\\\"=\\\".inline\\\" \\\"I6;c,1,1,3,0,4,0,7,8,1,0,0,0,3,0,400;\" malformed_rxas \"\${dep_rxas}\")\n")
string(APPEND INLINE_SUMMARY_FORMAL_FALLBACK_CONTENT "if(malformed_rxas STREQUAL dep_rxas)\n  message(FATAL_ERROR \"Failed to construct contradictory formal-effect summary\")\nendif()\n")
string(APPEND INLINE_SUMMARY_FORMAL_FALLBACK_CONTENT "file(WRITE \"\${work}/inline_cross_file_dep.rxas\" \"\${malformed_rxas}\")\n")
string(APPEND INLINE_SUMMARY_FORMAL_FALLBACK_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"inline_cross_file_dep.rxbin\" \"inline_cross_file_dep\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)\n")
string(APPEND INLINE_SUMMARY_FORMAL_FALLBACK_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed on contradictory formal-effect dependency: \${out}\${err}\")\nendif()\n")
string(APPEND INLINE_SUMMARY_FORMAL_FALLBACK_CONTENT "file(REMOVE \"\${work}/inline_cross_file_dep.crexx\" \"\${work}/inline_cross_file_dep.rxas\")\n")
string(APPEND INLINE_SUMMARY_FORMAL_FALLBACK_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"\${work}\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"inline_cross_file_main\" \"\${work}/inline_cross_file_main.crexx\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)\n")
string(APPEND INLINE_SUMMARY_FORMAL_FALLBACK_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed on contradictory formal-effect consumer: \${out}\${err}\")\nendif()\n")
string(APPEND INLINE_SUMMARY_FORMAL_FALLBACK_CONTENT "file(READ \"\${work}/inline_cross_file_main.rxas\" main_rxas)\n")
string(APPEND INLINE_SUMMARY_FORMAL_FALLBACK_CONTENT "if(NOT main_rxas MATCHES [=[call [^\\n]*inline_cross_file_dep\\.refbump\\(\\)]=])\n  message(FATAL_ERROR \"Contradictory formal-effect summary did not retain refbump() call:\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_SUMMARY_FORMAL_FALLBACK_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"inline_cross_file_main.rxbin\" \"inline_cross_file_main\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)\n")
string(APPEND INLINE_SUMMARY_FORMAL_FALLBACK_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed on contradictory formal-effect consumer: \${out}\${err}\")\nendif()\n")
string(APPEND INLINE_SUMMARY_FORMAL_FALLBACK_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxvm>\" \"${CMAKE_BINARY_DIR}/bin/library.rxbin\" \"\${work}/inline_cross_file_dep.rxbin\" \"\${work}/inline_cross_file_main.rxbin\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE run_out ERROR_VARIABLE err RESULT_VARIABLE res)\n")
string(APPEND INLINE_SUMMARY_FORMAL_FALLBACK_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxvm failed on contradictory formal-effect fallback: \${run_out}\${err}\")\nendif()\n")
string(APPEND INLINE_SUMMARY_FORMAL_FALLBACK_CONTENT "string(REPLACE \"\\r\\n\" \"\\n\" run_out \"\${run_out}\")\n")
string(APPEND INLINE_SUMMARY_FORMAL_FALLBACK_CONTENT "if(NOT run_out STREQUAL \"42\\n7\\n-1\\n0\\n1\\n20\\n-3\\n42\\n10\\n3\\nred:blue\\nseed\\nafter\\n11\\n11\\n15\\n8\\n21\\n21\\n1\\n22\\n.int\\n23\\n6\\n12\\n9\\n10\\ncall:12\\n\")\n  message(FATAL_ERROR \"Contradictory formal-effect fallback output mismatch: \${run_out}\")\nendif()\n")
file(GENERATE OUTPUT "${INLINE_SUMMARY_FORMAL_FALLBACK_SCRIPT}" CONTENT "${INLINE_SUMMARY_FORMAL_FALLBACK_CONTENT}")

add_test(NAME inline_summary_formal_fallback_binary_opt
         COMMAND ${CMAKE_COMMAND} -P "${INLINE_SUMMARY_FORMAL_FALLBACK_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(inline_summary_formal_fallback_binary_opt PROPERTIES LABELS "linked_opt")

# Review regression: receiver effects are trusted only when the imported I6
# control facts exactly match an independent reconstruction of the method body.
# Forging setName as a non-mutating receiver must retain the ordinary call.
set(INLINE_RECEIVER_SUMMARY_FALLBACK_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_inline_receiver_summary_fallback_binary_opt.cmake")
set(INLINE_RECEIVER_SUMMARY_FALLBACK_CONTENT "set(work \"${CMAKE_CURRENT_BINARY_DIR}/inline_receiver_summary_fallback_binary_opt_ws\")\n")
string(APPEND INLINE_RECEIVER_SUMMARY_FALLBACK_CONTENT "file(REMOVE_RECURSE \"\${work}\")\n")
string(APPEND INLINE_RECEIVER_SUMMARY_FALLBACK_CONTENT "file(MAKE_DIRECTORY \"\${work}\")\n")
string(APPEND INLINE_RECEIVER_SUMMARY_FALLBACK_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E copy_if_different \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/inline_cross_file_dep.crexx\" \"\${work}/inline_cross_file_dep.crexx\" RESULT_VARIABLE res)\n")
string(APPEND INLINE_RECEIVER_SUMMARY_FALLBACK_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E copy_if_different \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/inline_receiver_summary_main.crexx\" \"\${work}/inline_receiver_summary_main.crexx\" RESULT_VARIABLE res)\n")
string(APPEND INLINE_RECEIVER_SUMMARY_FALLBACK_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -o inline_cross_file_dep \"\${work}/inline_cross_file_dep.crexx\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)\n")
string(APPEND INLINE_RECEIVER_SUMMARY_FALLBACK_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed on receiver-summary dependency: \${out}\${err}\")\nendif()\n")
string(APPEND INLINE_RECEIVER_SUMMARY_FALLBACK_CONTENT "file(READ \"\${work}/inline_cross_file_dep.rxas\" dep_rxas)\n")
string(APPEND INLINE_RECEIVER_SUMMARY_FALLBACK_CONTENT "string(REPLACE \".meta \\\"inline_cross_file_dep.box.setname\\\"=\\\".inline\\\" \\\"I6;c,1,1,1,0,4,9,7,5,1,0,0,0,6,0,400;\" \".meta \\\"inline_cross_file_dep.box.setname\\\"=\\\".inline\\\" \\\"I6;c,1,1,1,0,4,1,7,5,1,0,0,0,6,0,400;\" malformed_rxas \"\${dep_rxas}\")\n")
string(APPEND INLINE_RECEIVER_SUMMARY_FALLBACK_CONTENT "if(malformed_rxas STREQUAL dep_rxas)\n  message(FATAL_ERROR \"Failed to construct contradictory receiver-effect summary\")\nendif()\n")
string(APPEND INLINE_RECEIVER_SUMMARY_FALLBACK_CONTENT "file(WRITE \"\${work}/inline_cross_file_dep.rxas\" \"\${malformed_rxas}\")\n")
string(APPEND INLINE_RECEIVER_SUMMARY_FALLBACK_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o inline_cross_file_dep.rxbin inline_cross_file_dep WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)\n")
string(APPEND INLINE_RECEIVER_SUMMARY_FALLBACK_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed on contradictory receiver summary: \${out}\${err}\")\nendif()\n")
string(APPEND INLINE_RECEIVER_SUMMARY_FALLBACK_CONTENT "file(REMOVE \"\${work}/inline_cross_file_dep.crexx\" \"\${work}/inline_cross_file_dep.rxas\")\n")
string(APPEND INLINE_RECEIVER_SUMMARY_FALLBACK_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"\${work}\" -i \"${CMAKE_BINARY_DIR}/bin\" -o inline_receiver_summary_main \"\${work}/inline_receiver_summary_main.crexx\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)\n")
string(APPEND INLINE_RECEIVER_SUMMARY_FALLBACK_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed on contradictory receiver-summary consumer: \${out}\${err}\")\nendif()\n")
string(APPEND INLINE_RECEIVER_SUMMARY_FALLBACK_CONTENT "file(READ \"\${work}/inline_receiver_summary_main.rxas\" main_rxas)\n")
string(APPEND INLINE_RECEIVER_SUMMARY_FALLBACK_CONTENT "if(NOT main_rxas MATCHES [=[call [^\\n]*inline_cross_file_dep\\.box\\.setname\\(\\)]=])\n  message(FATAL_ERROR \"Contradictory receiver summary did not retain setName() call:\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_RECEIVER_SUMMARY_FALLBACK_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o inline_receiver_summary_main.rxbin inline_receiver_summary_main WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)\n")
string(APPEND INLINE_RECEIVER_SUMMARY_FALLBACK_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed on receiver-summary consumer: \${out}\${err}\")\nendif()\n")
string(APPEND INLINE_RECEIVER_SUMMARY_FALLBACK_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxvm>\" \"${CMAKE_BINARY_DIR}/bin/library.rxbin\" \"\${work}/inline_cross_file_dep.rxbin\" \"\${work}/inline_receiver_summary_main.rxbin\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE run_out ERROR_VARIABLE err RESULT_VARIABLE res)\n")
string(APPEND INLINE_RECEIVER_SUMMARY_FALLBACK_CONTENT "string(REPLACE \"\\r\\n\" \"\\n\" run_out \"\${run_out}\")\n")
string(APPEND INLINE_RECEIVER_SUMMARY_FALLBACK_CONTENT "if(NOT res EQUAL 0 OR NOT run_out STREQUAL \"after\\n\")\n  message(FATAL_ERROR \"Receiver-summary fallback runtime mismatch: \${run_out}\${err}\")\nendif()\n")
file(GENERATE OUTPUT "${INLINE_RECEIVER_SUMMARY_FALLBACK_SCRIPT}" CONTENT "${INLINE_RECEIVER_SUMMARY_FALLBACK_CONTENT}")

add_test(NAME inline_receiver_summary_fallback_binary_opt
         COMMAND ${CMAKE_COMMAND} -P "${INLINE_RECEIVER_SUMMARY_FALLBACK_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(inline_receiver_summary_fallback_binary_opt PROPERTIES LABELS "linked_opt")

set(INLINE_REF_ARRAY_COUNT_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_inline_ref_array_count_binary_opt.cmake")
set(INLINE_REF_ARRAY_COUNT_CONTENT "set(work \"${CMAKE_CURRENT_BINARY_DIR}/inline_ref_array_count_binary_opt_ws\")\n")
string(APPEND INLINE_REF_ARRAY_COUNT_CONTENT "file(REMOVE_RECURSE \"\${work}\")\n")
string(APPEND INLINE_REF_ARRAY_COUNT_CONTENT "file(MAKE_DIRECTORY \"\${work}\")\n")
string(APPEND INLINE_REF_ARRAY_COUNT_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E copy_if_different \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/inline_ref_array_count_dep.crexx\" \"\${work}/inline_ref_array_count_dep.crexx\" RESULT_VARIABLE res)\n")
string(APPEND INLINE_REF_ARRAY_COUNT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"Failed to stage inline ref-array-count dependency\")\nendif()\n")
string(APPEND INLINE_REF_ARRAY_COUNT_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E copy_if_different \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/inline_ref_array_count_main.crexx\" \"\${work}/inline_ref_array_count_main.crexx\" RESULT_VARIABLE res)\n")
string(APPEND INLINE_REF_ARRAY_COUNT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"Failed to stage inline ref-array-count main\")\nendif()\n")
string(APPEND INLINE_REF_ARRAY_COUNT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"inline_ref_array_count_dep\" \"\${work}/inline_ref_array_count_dep.crexx\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)\n")
string(APPEND INLINE_REF_ARRAY_COUNT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed on inline ref-array-count dependency: \${out}\${err}\")\nendif()\n")
string(APPEND INLINE_REF_ARRAY_COUNT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"inline_ref_array_count_dep.rxbin\" \"inline_ref_array_count_dep\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)\n")
string(APPEND INLINE_REF_ARRAY_COUNT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed on inline ref-array-count dependency: \${out}\${err}\")\nendif()\n")
string(APPEND INLINE_REF_ARRAY_COUNT_CONTENT "file(REMOVE \"\${work}/inline_ref_array_count_dep.crexx\" \"\${work}/inline_ref_array_count_dep.rxas\")\n")
string(APPEND INLINE_REF_ARRAY_COUNT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"\${work}\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"inline_ref_array_count_main\" \"\${work}/inline_ref_array_count_main.crexx\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)\n")
string(APPEND INLINE_REF_ARRAY_COUNT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed on inline ref-array-count main: \${out}\${err}\")\nendif()\n")
string(APPEND INLINE_REF_ARRAY_COUNT_CONTENT "file(READ \"\${work}/inline_ref_array_count_main.rxas\" main_rxas)\n")
string(APPEND INLINE_REF_ARRAY_COUNT_CONTENT "if(main_rxas MATCHES [=[call [^\\n]*firstor\\(\\)]=])\n  message(FATAL_ERROR \"Optimized binary import still calls firstOr():\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_REF_ARRAY_COUNT_CONTENT "if(main_rxas MATCHES [=[slt [^\\n]*\\\"1\\\"]=])\n  message(FATAL_ERROR \"Optimized binary import lowered array[0] count compare as string compare:\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_REF_ARRAY_COUNT_CONTENT "if(NOT main_rxas MATCHES [=[ilt [^\\n]*,1]=])\n  message(FATAL_ERROR \"Optimized binary import did not lower array[0] count compare as integer compare:\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_REF_ARRAY_COUNT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"inline_ref_array_count_main.rxbin\" \"inline_ref_array_count_main\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)\n")
string(APPEND INLINE_REF_ARRAY_COUNT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed on inline ref-array-count main: \${out}\${err}\")\nendif()\n")
string(APPEND INLINE_REF_ARRAY_COUNT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxvm>\" \"${CMAKE_BINARY_DIR}/bin/library.rxbin\" \"\${work}/inline_ref_array_count_dep.rxbin\" \"\${work}/inline_ref_array_count_main.rxbin\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE run_out ERROR_VARIABLE err RESULT_VARIABLE res)\n")
string(APPEND INLINE_REF_ARRAY_COUNT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxvm failed on inline ref-array-count binary import: \${run_out}\${err}\")\nendif()\n")
string(APPEND INLINE_REF_ARRAY_COUNT_CONTENT "string(REPLACE \"\\r\\n\" \"\\n\" run_out \"\${run_out}\")\n")
string(APPEND INLINE_REF_ARRAY_COUNT_CONTENT "if(NOT run_out STREQUAL \"?\\nA\\n\")\n  message(FATAL_ERROR \"Output mismatch: expected ? and A, got: \${run_out}\")\nendif()\n")
file(GENERATE OUTPUT "${INLINE_REF_ARRAY_COUNT_SCRIPT}" CONTENT "${INLINE_REF_ARRAY_COUNT_CONTENT}")

add_test(NAME inline_ref_array_count_binary_opt
         COMMAND ${CMAKE_COMMAND} -P "${INLINE_REF_ARRAY_COUNT_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(inline_ref_array_count_binary_opt PROPERTIES LABELS "linked_opt")

set(INLINE_RHS_BLOCK_EXPR_CONCAT_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_inline_rhs_block_expr_concat_binary_opt.cmake")
set(INLINE_RHS_BLOCK_EXPR_CONCAT_CONTENT "set(work \"${CMAKE_CURRENT_BINARY_DIR}/inline_rhs_block_expr_concat_binary_opt_ws\")\n")
string(APPEND INLINE_RHS_BLOCK_EXPR_CONCAT_CONTENT "file(REMOVE_RECURSE \"\${work}\")\n")
string(APPEND INLINE_RHS_BLOCK_EXPR_CONCAT_CONTENT "file(MAKE_DIRECTORY \"\${work}\")\n")
string(APPEND INLINE_RHS_BLOCK_EXPR_CONCAT_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E copy_if_different \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/inline_rhs_block_expr_concat.crexx\" \"\${work}/inline_rhs_block_expr_concat.crexx\" RESULT_VARIABLE res)\n")
string(APPEND INLINE_RHS_BLOCK_EXPR_CONCAT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"Failed to stage inline RHS block-expression concat source\")\nendif()\n")
string(APPEND INLINE_RHS_BLOCK_EXPR_CONCAT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"inline_rhs_block_expr_concat\" \"\${work}/inline_rhs_block_expr_concat.crexx\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)\n")
string(APPEND INLINE_RHS_BLOCK_EXPR_CONCAT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed on inline RHS block-expression concat: \${out}\${err}\")\nendif()\n")
string(APPEND INLINE_RHS_BLOCK_EXPR_CONCAT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"inline_rhs_block_expr_concat.rxbin\" \"inline_rhs_block_expr_concat\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)\n")
string(APPEND INLINE_RHS_BLOCK_EXPR_CONCAT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed on inline RHS block-expression concat: \${out}\${err}\")\nendif()\n")
string(APPEND INLINE_RHS_BLOCK_EXPR_CONCAT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxvm>\" \"${CMAKE_BINARY_DIR}/bin/library.rxbin\" \"\${work}/inline_rhs_block_expr_concat.rxbin\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE run_out ERROR_VARIABLE err RESULT_VARIABLE res)\n")
string(APPEND INLINE_RHS_BLOCK_EXPR_CONCAT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxvm failed on inline RHS block-expression concat: \${run_out}\${err}\")\nendif()\n")
string(APPEND INLINE_RHS_BLOCK_EXPR_CONCAT_CONTENT "string(REPLACE \"\\r\\n\" \"\\n\" run_out \"\${run_out}\")\n")
string(APPEND INLINE_RHS_BLOCK_EXPR_CONCAT_CONTENT "if(NOT run_out STREQUAL \"prefix:abc:def\\n\")\n  message(FATAL_ERROR \"Output mismatch: expected prefix:abc:def, got: \${run_out}\")\nendif()\n")
file(GENERATE OUTPUT "${INLINE_RHS_BLOCK_EXPR_CONCAT_SCRIPT}" CONTENT "${INLINE_RHS_BLOCK_EXPR_CONCAT_CONTENT}")

add_test(NAME inline_rhs_block_expr_concat_binary_opt
         COMMAND ${CMAKE_COMMAND} -P "${INLINE_RHS_BLOCK_EXPR_CONCAT_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(inline_rhs_block_expr_concat_binary_opt PROPERTIES
        LABELS "linked_opt"
        FIXTURES_REQUIRED linked_opt_runtime_artifacts)

set(INLINE_CROSS_FILE_SOURCE_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_inline_cross_file_source_opt.cmake")
set(INLINE_CROSS_FILE_SOURCE_CONTENT "set(full_out \"\")\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "set(work \"${CMAKE_CURRENT_BINARY_DIR}/inline_cross_file_source_opt_ws\")\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "file(REMOVE_RECURSE \"\${work}\")\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "file(MAKE_DIRECTORY \"\${work}\")\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E copy_if_different \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/inline_cross_file_dep.crexx\" \"\${work}/inline_cross_file_dep.crexx\" RESULT_VARIABLE res)\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"Failed to stage inline dependency\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E copy_if_different \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/inline_cross_file_main.crexx\" \"\${work}/inline_cross_file_main.crexx\" RESULT_VARIABLE res)\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"Failed to stage inline main\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"\${work}\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"inline_cross_file_main\" \"\${work}/inline_cross_file_main.crexx\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "string(APPEND full_out \"\${out}\")\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed on source-import main: \${out}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "file(READ \"\${work}/inline_cross_file_main.rxas\" main_rxas)\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "string(FIND \"\${main_rxas}\" \"\\\"inline_cross_file_dep.crexx\\\" 70 3 16 \\\"  result = .int\\\"\" main_dep_source_pos)\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "string(FIND \"\${main_rxas}\" \"\\\"inline_cross_file_dep.crexx\\\" 71 13 32 \\\"  assembler strlen result,value\\\"\" main_dep_asm_source_pos)\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "if(main_dep_source_pos EQUAL -1 OR main_dep_asm_source_pos EQUAL -1)\n  message(FATAL_ERROR \"Optimized source import did not preserve inlined dependency source metadata:\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "if(main_rxas MATCHES [=[call [^\\n]*inc\\(\\)]=])\n  message(FATAL_ERROR \"Optimized source import still calls inc():\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "if(main_rxas MATCHES [=[call [^\\n]*classify\\(\\)]=])\n  message(FATAL_ERROR \"Optimized source import still calls classify():\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "if(main_rxas MATCHES [=[call [^\\n]*scoped\\(\\)]=])\n  message(FATAL_ERROR \"Optimized source import still calls scoped():\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "if(main_rxas MATCHES [=[call [^\\n]*nested\\(\\)]=])\n  message(FATAL_ERROR \"Optimized source import still calls nested():\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "if(main_rxas MATCHES [=[call [^\\n]*sumto\\(\\)]=])\n  message(FATAL_ERROR \"Optimized source import still calls sumTo():\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "if(main_rxas MATCHES [=[call [^\\n]*countuntil\\(\\)]=])\n  message(FATAL_ERROR \"Optimized source import still calls countUntil():\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "if(main_rxas MATCHES [=[call [^\\n]*buildarray\\(\\)]=])\n  message(FATAL_ERROR \"Optimized source import still calls buildArray():\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "if(main_rxas MATCHES [=[call [^\\n]*renderarray\\(\\)]=])\n  message(FATAL_ERROR \"Optimized source import still calls renderArray():\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "if(main_rxas MATCHES [=[call [^\\n]*identitybox\\(\\)]=])\n  message(FATAL_ERROR \"Optimized source import still calls identityBox():\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "if(main_rxas MATCHES [=[call [^\\n]*refbump\\(\\)]=])\n  message(FATAL_ERROR \"Optimized source import still calls refBump():\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "if(main_rxas MATCHES [=[call [^\\n]*optadd\\(\\)]=])\n  message(FATAL_ERROR \"Optimized source import still calls optAdd():\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "if(main_rxas MATCHES [=[call [^\\n]*residualfunctiondependency\\(\\)]=])\n  message(FATAL_ERROR \"Optimized source import still calls residualFunctionDependency():\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "if(main_rxas MATCHES [=[call [^\\n]*residualcalldependency\\(\\)]=])\n  message(FATAL_ERROR \"Optimized source import still calls residualCallDependency():\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "if(main_rxas MATCHES [=[call [^\\n]*inline_cross_file_dep\\.box\\.getname\\(\\)]=])\n  message(FATAL_ERROR \"Optimized source import still calls box.getName():\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "if(main_rxas MATCHES [=[call [^\\n]*inline_cross_file_dep\\.box\\.setname\\(\\)]=])\n  message(FATAL_ERROR \"Optimized source import still calls box.setName():\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "foreach(inlined_name inc classify scoped nested sumto countuntil buildarray renderarray identitybox refbump optadd safelength typebump vargtotal residualfunctiondependency residualcalldependency)\n  if(main_rxas MATCHES \"call [^\\n]*inline_cross_file_dep\\\\.\${inlined_name}\\\\(\\\\)\")\n    message(FATAL_ERROR \"Optimized source import still calls \${inlined_name}():\\n\${main_rxas}\")\n  endif()\nendforeach()\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "string(REPLACE \"\\n\" \"|\" flat_main_rxas \"\${main_rxas}\")\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "string(REGEX MATCH [=[\\.traceevent \"V\"[^|]*\"r\" ([0-9]+)[^|]*\"shared\"[^|]*\\|[ \\t]*\\.traceevent \"A\"[^|]*\"r\" ([0-9]+)[^|]*\"value\"]=] shared_binding \"\${flat_main_rxas}\")\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "set(shared_source_reg \"\${CMAKE_MATCH_1}\")\nset(shared_formal_reg \"\${CMAKE_MATCH_2}\")\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "if(shared_binding STREQUAL \"\" OR NOT shared_source_reg STREQUAL shared_formal_reg)\n  message(FATAL_ERROR \"Source-import callable summary did not preserve direct scalar binding equivalence:\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "string(REGEX MATCH [=[\\.traceevent \"V\"[^|]*\"r\" ([0-9]+)[^|]*\"same\"[^|]*\\|[ \\t]*\\.traceevent \"A\"[^|]*\"r\" ([0-9]+)[^|]*\"§this\"]=] receiver_binding \"\${flat_main_rxas}\")\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "set(receiver_source_reg \"\${CMAKE_MATCH_1}\")\nset(receiver_this_reg \"\${CMAKE_MATCH_2}\")\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "if(receiver_binding STREQUAL \"\" OR NOT receiver_source_reg STREQUAL receiver_this_reg)\n  message(FATAL_ERROR \"Source-import method summary did not preserve direct receiver equivalence:\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "if(NOT main_rxas MATCHES [=[call [^\\n]*inline_cross_file_dep\\.vargpick\\(\\)]=])\n  message(FATAL_ERROR \"Optimized source import unexpectedly inlined dynamic vargPick():\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "if(NOT main_rxas MATCHES [=[call [^\\n]*inline_cross_file_dep\\.residualcalltarget\\(\\)]=])\n  message(FATAL_ERROR \"Optimized source import did not preserve residual call dependency:\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"inline_cross_file_main.rxbin\" \"inline_cross_file_main\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "string(APPEND full_out \"\${out}\")\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed on source-import main: \${out}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"inline_cross_file_dep\" \"\${work}/inline_cross_file_dep.crexx\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "string(APPEND full_out \"\${out}\")\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed on source-import dependency: \${out}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"inline_cross_file_dep.rxbin\" \"inline_cross_file_dep\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "string(APPEND full_out \"\${out}\")\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed on source-import dependency: \${out}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxvm>\" \"${CMAKE_BINARY_DIR}/bin/library.rxbin\" \"\${work}/inline_cross_file_dep.rxbin\" \"\${work}/inline_cross_file_main.rxbin\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "string(APPEND full_out \"\${out}\")\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxvm failed: \${out}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "string(REPLACE \"\\r\\n\" \"\\n\" full_out \"\${full_out}\")\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "if(NOT full_out STREQUAL \"42\\n7\\n-1\\n0\\n1\\n20\\n-3\\n42\\n10\\n3\\nred:blue\\nseed\\nafter\\n11\\n11\\n15\\n8\\n21\\n21\\n1\\n22\\n.int\\n23\\n6\\n12\\n9\\n10\\ncall:12\\n\")\n  message(FATAL_ERROR \"Output mismatch: expected inline output, got: \${full_out}\")\nendif()\n")
file(GENERATE OUTPUT "${INLINE_CROSS_FILE_SOURCE_SCRIPT}" CONTENT "${INLINE_CROSS_FILE_SOURCE_CONTENT}")

add_test(NAME inline_cross_file_source_opt
         COMMAND ${CMAKE_COMMAND} -P "${INLINE_CROSS_FILE_SOURCE_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(inline_cross_file_source_opt PROPERTIES LABELS "linked_opt")

set(SOURCE_IMPORT_DOTTED_DECIMAL_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_source_import_dotted_floats_decimal_opt.cmake")
set(SOURCE_IMPORT_DOTTED_DECIMAL_CONTENT "set(work \"${CMAKE_CURRENT_BINARY_DIR}/source_import_dotted_floats_decimal_opt_ws\")\n")
string(APPEND SOURCE_IMPORT_DOTTED_DECIMAL_CONTENT "file(REMOVE_RECURSE \"\${work}\")\n")
string(APPEND SOURCE_IMPORT_DOTTED_DECIMAL_CONTENT "file(MAKE_DIRECTORY \"\${work}\")\n")
string(APPEND SOURCE_IMPORT_DOTTED_DECIMAL_CONTENT "file(WRITE \"\${work}/hello.aap.noot.mies\" \"options levelb\\nnamespace main\\nimport dep_ns\\nsay dep()\\n\")\n")
string(APPEND SOURCE_IMPORT_DOTTED_DECIMAL_CONTENT "file(WRITE \"\${work}/dep.aap.noot.mies.rexx\" \"options levelb floats_decimal\\nnamespace dep_ns expose dep\\ndep: procedure = .decimal\\n  return 1.1 + 2.2\\n\")\n")
string(APPEND SOURCE_IMPORT_DOTTED_DECIMAL_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"main\" \"\${work}/hello.aap.noot.mies\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND SOURCE_IMPORT_DOTTED_DECIMAL_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed on dotted floats_decimal source import: \${out}\")\nendif()\n")
string(APPEND SOURCE_IMPORT_DOTTED_DECIMAL_CONTENT "if(NOT EXISTS \"\${work}/main.rxas\")\n  message(FATAL_ERROR \"Expected main.rxas was not produced\")\nendif()\n")
file(GENERATE OUTPUT "${SOURCE_IMPORT_DOTTED_DECIMAL_SCRIPT}" CONTENT "${SOURCE_IMPORT_DOTTED_DECIMAL_CONTENT}")

add_test(NAME source_import_dotted_floats_decimal_opt
         COMMAND ${CMAKE_COMMAND} -P "${SOURCE_IMPORT_DOTTED_DECIMAL_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(source_import_dotted_floats_decimal_opt PROPERTIES LABELS "compiler;imports;linked_opt")

set(SOURCE_IMPORT_CONTRACT_STUB_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_source_import_contract_stub_implements.cmake")
set(SOURCE_IMPORT_CONTRACT_STUB_CONTENT "set(work \"${CMAKE_CURRENT_BINARY_DIR}/source_import_contract_stub_implements_ws\")\n")
string(APPEND SOURCE_IMPORT_CONTRACT_STUB_CONTENT "file(REMOVE_RECURSE \"\${work}\")\n")
string(APPEND SOURCE_IMPORT_CONTRACT_STUB_CONTENT "file(MAKE_DIRECTORY \"\${work}\")\n")
string(APPEND SOURCE_IMPORT_CONTRACT_STUB_CONTENT "file(WRITE \"\${work}/iface.crexx\" \"options levelb\\nnamespace contract_stub_iface expose contractiface\\ncontractiface: interface\\n  value: method = .int\\n\")\n")
string(APPEND SOURCE_IMPORT_CONTRACT_STUB_CONTENT "file(WRITE \"\${work}/provider.crexx\" \"options levelb\\nnamespace contract_stub_provider expose contractbox\\nimport contract_stub_iface\\ncontractbox: class implements .contractiface\\n  *: factory\\n    return\\n  value: method = .int\\n    return 7\\n\")\n")
string(APPEND SOURCE_IMPORT_CONTRACT_STUB_CONTENT "file(WRITE \"\${work}/main.crexx\" \"options levelb\\nnamespace contract_stub_consumer\\nimport contract_stub_provider\\nmain: procedure = .int\\n  box = .contractbox()\\n  if box.value() <> 7 then return 1\\n  return 0\\n\")\n")
string(APPEND SOURCE_IMPORT_CONTRACT_STUB_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"main\" \"\${work}/main.crexx\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND SOURCE_IMPORT_CONTRACT_STUB_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed on source-import contract stub implements test: \${out}\")\nendif()\n")
string(APPEND SOURCE_IMPORT_CONTRACT_STUB_CONTENT "if(NOT EXISTS \"\${work}/main.rxas\")\n  message(FATAL_ERROR \"Expected main.rxas was not produced\")\nendif()\n")
file(GENERATE OUTPUT "${SOURCE_IMPORT_CONTRACT_STUB_SCRIPT}" CONTENT "${SOURCE_IMPORT_CONTRACT_STUB_CONTENT}")

add_test(NAME source_import_contract_stub_implements
         COMMAND ${CMAKE_COMMAND} -P "${SOURCE_IMPORT_CONTRACT_STUB_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(source_import_contract_stub_implements PROPERTIES LABELS "compiler;imports")

set(SOURCE_IMPORT_OPTIONAL_FACTORY_ARG_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_source_import_optional_factory_arg.cmake")
set(SOURCE_IMPORT_OPTIONAL_FACTORY_ARG_CONTENT "set(work \"${CMAKE_CURRENT_BINARY_DIR}/source_import_optional_factory_arg_ws\")\n")
string(APPEND SOURCE_IMPORT_OPTIONAL_FACTORY_ARG_CONTENT "file(REMOVE_RECURSE \"\${work}\")\n")
string(APPEND SOURCE_IMPORT_OPTIONAL_FACTORY_ARG_CONTENT "file(MAKE_DIRECTORY \"\${work}\")\n")
string(APPEND SOURCE_IMPORT_OPTIONAL_FACTORY_ARG_CONTENT "file(WRITE \"\${work}/provider.crexx\" \"options levelb\\nnamespace optional_factory_provider expose optionalbox\\noptionalbox: class\\n  *: factory\\n    arg expected = 1024\\n    return\\n  value: method = .int\\n    return 7\\n\")\n")
string(APPEND SOURCE_IMPORT_OPTIONAL_FACTORY_ARG_CONTENT "file(WRITE \"\${work}/main.crexx\" \"options levelb\\nnamespace optional_factory_consumer\\nimport optional_factory_provider\\nmain: procedure = .int\\n  box = .optionalbox()\\n  if box.value() <> 7 then return 1\\n  return 0\\n\")\n")
string(APPEND SOURCE_IMPORT_OPTIONAL_FACTORY_ARG_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"main\" \"\${work}/main.crexx\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND SOURCE_IMPORT_OPTIONAL_FACTORY_ARG_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed on source-import optional factory arg test: \${out}\")\nendif()\n")
string(APPEND SOURCE_IMPORT_OPTIONAL_FACTORY_ARG_CONTENT "if(NOT EXISTS \"\${work}/main.rxas\")\n  message(FATAL_ERROR \"Expected main.rxas was not produced\")\nendif()\n")
file(GENERATE OUTPUT "${SOURCE_IMPORT_OPTIONAL_FACTORY_ARG_SCRIPT}" CONTENT "${SOURCE_IMPORT_OPTIONAL_FACTORY_ARG_CONTENT}")

add_test(NAME source_import_optional_factory_arg
         COMMAND ${CMAKE_COMMAND} -P "${SOURCE_IMPORT_OPTIONAL_FACTORY_ARG_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(source_import_optional_factory_arg PROPERTIES LABELS "compiler;imports")

set(SOURCE_IMPORT_CONTRACT_STUB_CYCLE_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_source_import_contract_stub_covariant_cycle.cmake")
set(SOURCE_IMPORT_CONTRACT_STUB_CYCLE_CONTENT "set(work \"${CMAKE_CURRENT_BINARY_DIR}/source_import_contract_stub_covariant_cycle_ws\")\n")
string(APPEND SOURCE_IMPORT_CONTRACT_STUB_CYCLE_CONTENT "file(REMOVE_RECURSE \"\${work}\")\n")
string(APPEND SOURCE_IMPORT_CONTRACT_STUB_CYCLE_CONTENT "file(MAKE_DIRECTORY \"\${work}\")\n")
string(APPEND SOURCE_IMPORT_CONTRACT_STUB_CYCLE_CONTENT "file(WRITE \"\${work}/iface.crexx\" \"options levelb\\nnamespace contract_stub_cycle_iface expose cycleiterator cycleiterable\\ncycleiterator: interface\\n  next: method = .int\\ncycleiterable: interface\\n  iterator: method = .cycleiterator\\n\")\n")
string(APPEND SOURCE_IMPORT_CONTRACT_STUB_CYCLE_CONTENT "file(WRITE \"\${work}/list.crexx\" \"options levelb\\nnamespace contract_stub_cycle_provider expose cyclelist\\nimport contract_stub_cycle_iface\\ncyclelist: class implements .cycleiterable\\n  *: factory\\n    return\\n  iterator: method = .cyclelistiterator\\n    return .cyclelistiterator(reference self)\\n\")\n")
string(APPEND SOURCE_IMPORT_CONTRACT_STUB_CYCLE_CONTENT "file(WRITE \"\${work}/iterator.crexx\" \"options levelb\\nnamespace contract_stub_cycle_provider expose cyclelistiterator\\nimport contract_stub_cycle_iface\\ncyclelistiterator: class implements .cycleiterator\\n  owner = reference .cyclelist\\n  *: factory\\n    arg source = reference .cyclelist\\n    owner = source\\n    return\\n  next: method = .int\\n    return 7\\n\")\n")
string(APPEND SOURCE_IMPORT_CONTRACT_STUB_CYCLE_CONTENT "file(WRITE \"\${work}/main.crexx\" \"options levelb\\nnamespace contract_stub_cycle_main\\nimport contract_stub_cycle_provider\\nmain: procedure = .int\\n  list = .cyclelist()\\n  iterator = list.iterator()\\n  if iterator.next() <> 7 then return 1\\n  return 0\\n\")\n")
string(APPEND SOURCE_IMPORT_CONTRACT_STUB_CYCLE_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"main\" \"\${work}/main.crexx\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND SOURCE_IMPORT_CONTRACT_STUB_CYCLE_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed on recursive source-import covariant contract stub test: \${out}\")\nendif()\n")
string(APPEND SOURCE_IMPORT_CONTRACT_STUB_CYCLE_CONTENT "if(NOT EXISTS \"\${work}/main.rxas\")\n  message(FATAL_ERROR \"Expected main.rxas was not produced\")\nendif()\n")
file(GENERATE OUTPUT "${SOURCE_IMPORT_CONTRACT_STUB_CYCLE_SCRIPT}" CONTENT "${SOURCE_IMPORT_CONTRACT_STUB_CYCLE_CONTENT}")

add_test(NAME source_import_contract_stub_covariant_cycle
         COMMAND ${CMAKE_COMMAND} -P "${SOURCE_IMPORT_CONTRACT_STUB_CYCLE_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(source_import_contract_stub_covariant_cycle PROPERTIES LABELS "compiler;imports")

set(MIXED_AGGREGATE_SOURCE_CONTRACT_STUB_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_mixed_aggregate_source_contract_stub_covariant.cmake")
set(MIXED_AGGREGATE_SOURCE_CONTRACT_STUB_CONTENT "set(work \"${CMAKE_CURRENT_BINARY_DIR}/mixed_aggregate_source_contract_stub_covariant_ws\")\n")
string(APPEND MIXED_AGGREGATE_SOURCE_CONTRACT_STUB_CONTENT "file(REMOVE_RECURSE \"\${work}\")\n")
string(APPEND MIXED_AGGREGATE_SOURCE_CONTRACT_STUB_CONTENT "file(MAKE_DIRECTORY \"\${work}/src\" \"\${work}/binroot\" \"\${work}/build\")\n")
string(APPEND MIXED_AGGREGATE_SOURCE_CONTRACT_STUB_CONTENT "file(WRITE \"\${work}/src/Iterator.crexx\" \"options levelb\\nnamespace aggregate_iterator expose Iterator\\n\\nIterator: interface\\n  next: method = .string\\n\")\n")
string(APPEND MIXED_AGGREGATE_SOURCE_CONTRACT_STUB_CONTENT "file(WRITE \"\${work}/src/Iterable.crexx\" \"options levelb\\nnamespace aggregate_iterable expose Iterable\\nimport aggregate_iterator\\n\\nIterable: interface\\n  iterator: method = .Iterator\\n\")\n")
string(APPEND MIXED_AGGREGATE_SOURCE_CONTRACT_STUB_CONTENT "file(WRITE \"\${work}/src/BinaryIterator.crexx\" \"options levelb\\nnamespace aggregate_provider expose BinaryIterator\\nimport aggregate_iterator\\n\\nBinaryIterator: class implements .Iterator\\n  *: factory\\n    return\\n  next: method = .string\\n    return \\\"ok\\\"\\n\")\n")
string(APPEND MIXED_AGGREGATE_SOURCE_CONTRACT_STUB_CONTENT "file(WRITE \"\${work}/src/Provider.crexx\" \"options levelb\\nnamespace aggregate_provider expose Provider\\nimport aggregate_iterable\\n\\nProvider: class implements .Iterable\\n  *: factory\\n    return\\n  iterator: method = .BinaryIterator\\n    return .BinaryIterator()\\n\")\n")
foreach(_mixed_source IN ITEMS Iterator Iterable BinaryIterator)
    string(APPEND MIXED_AGGREGATE_SOURCE_CONTRACT_STUB_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"\${work}/build/${_mixed_source}\" \"\${work}/src/${_mixed_source}.crexx\" WORKING_DIRECTORY \"\${work}/build\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
    string(APPEND MIXED_AGGREGATE_SOURCE_CONTRACT_STUB_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed on aggregate dependency ${_mixed_source}: \${out}\")\nendif()\n")
    string(APPEND MIXED_AGGREGATE_SOURCE_CONTRACT_STUB_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"\${work}/build/${_mixed_source}.rxbin\" \"\${work}/build/${_mixed_source}\" WORKING_DIRECTORY \"\${work}/build\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
    string(APPEND MIXED_AGGREGATE_SOURCE_CONTRACT_STUB_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed on aggregate dependency ${_mixed_source}: \${out}\")\nendif()\n")
endforeach()
string(APPEND MIXED_AGGREGATE_SOURCE_CONTRACT_STUB_CONTENT "file(WRITE \"\${work}/build/aggregate.ctl\" \"INPUT \${work}/build/Iterator.rxbin\\nINPUT \${work}/build/Iterable.rxbin\\nINPUT \${work}/build/BinaryIterator.rxbin\\nINCLUDE Iterator\\nINCLUDE Iterable\\nINCLUDE BinaryIterator\\nOUTPUT \${work}/binroot/aggregate\\n\")\n")
string(APPEND MIXED_AGGREGATE_SOURCE_CONTRACT_STUB_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxlink>\" -c \"\${work}/build/aggregate.ctl\" WORKING_DIRECTORY \"\${work}/build\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND MIXED_AGGREGATE_SOURCE_CONTRACT_STUB_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxlink failed on aggregate contract fixture: \${out}\")\nendif()\n")
string(APPEND MIXED_AGGREGATE_SOURCE_CONTRACT_STUB_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"\${work}/binroot\" -o \"\${work}/build/Provider\" \"\${work}/src/Provider.crexx\" WORKING_DIRECTORY \"\${work}/build\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND MIXED_AGGREGATE_SOURCE_CONTRACT_STUB_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed on mixed aggregate/source covariant contract test: \${out}\")\nendif()\n")
string(APPEND MIXED_AGGREGATE_SOURCE_CONTRACT_STUB_CONTENT "if(NOT EXISTS \"\${work}/build/Provider.rxas\")\n  message(FATAL_ERROR \"Expected Provider.rxas was not produced\")\nendif()\n")
file(GENERATE OUTPUT "${MIXED_AGGREGATE_SOURCE_CONTRACT_STUB_SCRIPT}" CONTENT "${MIXED_AGGREGATE_SOURCE_CONTRACT_STUB_CONTENT}")

add_test(NAME mixed_aggregate_source_contract_stub_covariant
         COMMAND ${CMAKE_COMMAND} -P "${MIXED_AGGREGATE_SOURCE_CONTRACT_STUB_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(mixed_aggregate_source_contract_stub_covariant PROPERTIES LABELS "compiler;imports")

set(SOURCE_EXTENSION_DIRECT_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_source_extension_direct_defaults.cmake")
set(SOURCE_EXTENSION_DIRECT_CONTENT "set(work \"${CMAKE_CURRENT_BINARY_DIR}/source_extension_direct_defaults_ws\")\n")
string(APPEND SOURCE_EXTENSION_DIRECT_CONTENT "file(REMOVE_RECURSE \"\${work}\")\n")
string(APPEND SOURCE_EXTENSION_DIRECT_CONTENT "file(MAKE_DIRECTORY \"\${work}\")\n")
string(APPEND SOURCE_EXTENSION_DIRECT_CONTENT "file(WRITE \"\${work}/direct.crexx\" \"namespace direct_crexx\\nmain: procedure\\n  say 'direct-crexx'\\n  return\\n\")\n")
string(APPEND SOURCE_EXTENSION_DIRECT_CONTENT "file(WRITE \"\${work}/direct.crx\" \"namespace direct_crx\\nmain: procedure\\n  say 'direct-crx'\\n  return\\n\")\n")
string(APPEND SOURCE_EXTENSION_DIRECT_CONTENT "file(WRITE \"\${work}/direct.the\" \"namespace direct_the\\nmain: procedure\\n  say 'direct-the'\\n  return\\n\")\n")
string(APPEND SOURCE_EXTENSION_DIRECT_CONTENT "file(WRITE \"\${work}/direct.rexx\" \"namespace direct_rexx\\nmain: procedure\\n  say 'direct-rexx'\\n  return\\n\")\n")
foreach(_source_ext IN ITEMS crexx crx the)
    string(APPEND SOURCE_EXTENSION_DIRECT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"direct_${_source_ext}\" \"\${work}/direct.${_source_ext}\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
    string(APPEND SOURCE_EXTENSION_DIRECT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed for direct .${_source_ext} default source: \${out}\")\nendif()\n")
    string(APPEND SOURCE_EXTENSION_DIRECT_CONTENT "if(NOT EXISTS \"\${work}/direct_${_source_ext}.rxas\")\n  message(FATAL_ERROR \"Expected direct_${_source_ext}.rxas was not produced\")\nendif()\n")
endforeach()
string(APPEND SOURCE_EXTENSION_DIRECT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"direct_extensionless\" \"\${work}/direct\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND SOURCE_EXTENSION_DIRECT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed to fall back to .crexx for extensionless source: \${out}\")\nendif()\n")
string(APPEND SOURCE_EXTENSION_DIRECT_CONTENT "if(NOT EXISTS \"\${work}/direct_extensionless.rxas\")\n  message(FATAL_ERROR \"Expected direct_extensionless.rxas was not produced\")\nendif()\n")
string(APPEND SOURCE_EXTENSION_DIRECT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"direct_rexx\" \"\${work}/direct.rexx\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND SOURCE_EXTENSION_DIRECT_CONTENT "if(res EQUAL 0)\n  message(FATAL_ERROR \"Headerless .rexx source unexpectedly compiled as CREXX source\")\nendif()\n")
string(APPEND SOURCE_EXTENSION_DIRECT_CONTENT "if(NOT out MATCHES \"Level C\")\n  message(FATAL_ERROR \"Headerless .rexx should default to Level C; got: \${out}\")\nendif()\n")
file(GENERATE OUTPUT "${SOURCE_EXTENSION_DIRECT_SCRIPT}" CONTENT "${SOURCE_EXTENSION_DIRECT_CONTENT}")

add_test(NAME source_extension_direct_defaults
         COMMAND ${CMAKE_COMMAND} -P "${SOURCE_EXTENSION_DIRECT_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(source_extension_direct_defaults PROPERTIES LABELS "compiler;imports;extensions")

set(SOURCE_EXTENSION_STANDARD_IMPORT_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_source_extension_standard_imports.cmake")
set(SOURCE_EXTENSION_STANDARD_IMPORT_CONTENT "set(work \"${CMAKE_CURRENT_BINARY_DIR}/source_extension_standard_imports_ws\")\n")
string(APPEND SOURCE_EXTENSION_STANDARD_IMPORT_CONTENT "file(REMOVE_RECURSE \"\${work}\")\n")
string(APPEND SOURCE_EXTENSION_STANDARD_IMPORT_CONTENT "file(MAKE_DIRECTORY \"\${work}\")\n")
string(APPEND SOURCE_EXTENSION_STANDARD_IMPORT_CONTENT "file(WRITE \"\${work}/main.crexx\" \"namespace source_extension_standard\\nimport extdepcrexx\\nimport extdepcrx\\nimport extdeprexx\\nsay dep_crexx() || ':' || dep_crx() || ':' || dep_rexx()\\n\")\n")
string(APPEND SOURCE_EXTENSION_STANDARD_IMPORT_CONTENT "file(WRITE \"\${work}/dep_crexx.crexx\" \"namespace extdepcrexx expose dep_crexx\\ndep_crexx: procedure = .string\\n  return 'crexx'\\n\")\n")
string(APPEND SOURCE_EXTENSION_STANDARD_IMPORT_CONTENT "file(WRITE \"\${work}/dep_crx.crx\" \"namespace extdepcrx expose dep_crx\\ndep_crx: procedure = .string\\n  return 'crx'\\n\")\n")
string(APPEND SOURCE_EXTENSION_STANDARD_IMPORT_CONTENT "file(WRITE \"\${work}/dep_rexx.rexx\" \"options levelg\\nnamespace extdeprexx expose dep_rexx\\ndep_rexx: procedure = .string\\n  return 'rexx'\\n\")\n")
string(APPEND SOURCE_EXTENSION_STANDARD_IMPORT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"main\" \"\${work}/main.crexx\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND SOURCE_EXTENSION_STANDARD_IMPORT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed to discover standard source extensions: \${out}\")\nendif()\n")
file(GENERATE OUTPUT "${SOURCE_EXTENSION_STANDARD_IMPORT_SCRIPT}" CONTENT "${SOURCE_EXTENSION_STANDARD_IMPORT_CONTENT}")

add_test(NAME source_extension_standard_imports
         COMMAND ${CMAKE_COMMAND} -P "${SOURCE_EXTENSION_STANDARD_IMPORT_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(source_extension_standard_imports PROPERTIES LABELS "compiler;imports;extensions")

set(SOURCE_EXTENSION_REXX_IMPORT_DEFAULT_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_source_extension_rexx_import_default.cmake")
set(SOURCE_EXTENSION_REXX_IMPORT_DEFAULT_CONTENT "set(work \"${CMAKE_CURRENT_BINARY_DIR}/source_extension_rexx_import_default_ws\")\n")
string(APPEND SOURCE_EXTENSION_REXX_IMPORT_DEFAULT_CONTENT "file(REMOVE_RECURSE \"\${work}\")\n")
string(APPEND SOURCE_EXTENSION_REXX_IMPORT_DEFAULT_CONTENT "file(MAKE_DIRECTORY \"\${work}\")\n")
string(APPEND SOURCE_EXTENSION_REXX_IMPORT_DEFAULT_CONTENT "file(WRITE \"\${work}/main.crexx\" \"namespace source_extension_rexx_default\\nimport extdeprexx\\nsay dep_rexx()\\n\")\n")
string(APPEND SOURCE_EXTENSION_REXX_IMPORT_DEFAULT_CONTENT "file(WRITE \"\${work}/dep_rexx.rexx\" \"namespace extdeprexx expose dep_rexx\\ndep_rexx: procedure = .string\\n  return 'rexx'\\n\")\n")
string(APPEND SOURCE_EXTENSION_REXX_IMPORT_DEFAULT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"main\" \"\${work}/main.crexx\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND SOURCE_EXTENSION_REXX_IMPORT_DEFAULT_CONTENT "if(res EQUAL 0)\n  message(FATAL_ERROR \"Headerless .rexx import unexpectedly inherited the .crexx Level G default\")\nendif()\n")
string(APPEND SOURCE_EXTENSION_REXX_IMPORT_DEFAULT_CONTENT "if(NOT out MATCHES \"FUNCTION_NOT_FOUND\")\n  message(FATAL_ERROR \"Expected headerless .rexx import to remain unavailable as Level C source; got: \${out}\")\nendif()\n")
file(GENERATE OUTPUT "${SOURCE_EXTENSION_REXX_IMPORT_DEFAULT_SCRIPT}" CONTENT "${SOURCE_EXTENSION_REXX_IMPORT_DEFAULT_CONTENT}")

add_test(NAME source_extension_rexx_import_default
         COMMAND ${CMAKE_COMMAND} -P "${SOURCE_EXTENSION_REXX_IMPORT_DEFAULT_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(source_extension_rexx_import_default PROPERTIES LABELS "compiler;imports;extensions")

set(SOURCE_EXTENSION_ARBITRARY_IMPORT_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_source_extension_arbitrary_imports.cmake")
set(SOURCE_EXTENSION_ARBITRARY_IMPORT_CONTENT "set(work \"${CMAKE_CURRENT_BINARY_DIR}/source_extension_arbitrary_imports_ws\")\n")
string(APPEND SOURCE_EXTENSION_ARBITRARY_IMPORT_CONTENT "file(REMOVE_RECURSE \"\${work}\")\n")
string(APPEND SOURCE_EXTENSION_ARBITRARY_IMPORT_CONTENT "file(MAKE_DIRECTORY \"\${work}\")\n")
string(APPEND SOURCE_EXTENSION_ARBITRARY_IMPORT_CONTENT "file(WRITE \"\${work}/main.the\" \"namespace source_extension_the\\nimport thedep\\nsay the_value()\\n\")\n")
string(APPEND SOURCE_EXTENSION_ARBITRARY_IMPORT_CONTENT "file(WRITE \"\${work}/dep.the\" \"namespace thedep expose the_value\\nthe_value: procedure = .string\\n  return 'from-the'\\n\")\n")
string(APPEND SOURCE_EXTENSION_ARBITRARY_IMPORT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"main\" \"\${work}/main.the\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND SOURCE_EXTENSION_ARBITRARY_IMPORT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed to discover arbitrary initial source extension: \${out}\")\nendif()\n")
file(GENERATE OUTPUT "${SOURCE_EXTENSION_ARBITRARY_IMPORT_SCRIPT}" CONTENT "${SOURCE_EXTENSION_ARBITRARY_IMPORT_CONTENT}")

add_test(NAME source_extension_arbitrary_imports
         COMMAND ${CMAKE_COMMAND} -P "${SOURCE_EXTENSION_ARBITRARY_IMPORT_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(source_extension_arbitrary_imports PROPERTIES LABELS "compiler;imports;extensions")

set(INLINE_LOCAL_MEMBER_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_inline_local_member_scalar_opt.cmake")
set(INLINE_LOCAL_MEMBER_CONTENT "set(full_out \"\")\n")
string(APPEND INLINE_LOCAL_MEMBER_CONTENT "set(work \"${CMAKE_CURRENT_BINARY_DIR}/inline_local_member_scalar_opt_ws\")\n")
string(APPEND INLINE_LOCAL_MEMBER_CONTENT "file(REMOVE_RECURSE \"\${work}\")\n")
string(APPEND INLINE_LOCAL_MEMBER_CONTENT "file(MAKE_DIRECTORY \"\${work}\")\n")
string(APPEND INLINE_LOCAL_MEMBER_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E copy_if_different \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/inline_local_member_scalar.crexx\" \"\${work}/inline_local_member_scalar.crexx\" RESULT_VARIABLE res)\n")
string(APPEND INLINE_LOCAL_MEMBER_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"Failed to stage local member inline test\")\nendif()\n")
string(APPEND INLINE_LOCAL_MEMBER_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"inline_local_member_scalar\" \"\${work}/inline_local_member_scalar.crexx\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND INLINE_LOCAL_MEMBER_CONTENT "string(APPEND full_out \"\${out}\")\n")
string(APPEND INLINE_LOCAL_MEMBER_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed on local member inline test: \${out}\")\nendif()\n")
string(APPEND INLINE_LOCAL_MEMBER_CONTENT "file(READ \"\${work}/inline_local_member_scalar.rxas\" main_rxas)\n")
string(APPEND INLINE_LOCAL_MEMBER_CONTENT "string(REGEX MATCH [=[main\\(\\) \\.locals=([0-9]+)]=] main_header \"\${main_rxas}\")\n")
string(APPEND INLINE_LOCAL_MEMBER_CONTENT "if(main_header STREQUAL \"\" OR CMAKE_MATCH_1 GREATER 9)\n  message(FATAL_ERROR \"Direct getter/setter receiver placement used more than 9 main locals:\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_LOCAL_MEMBER_CONTENT "string(REGEX MATCHALL [=[\\n[ \\t]*copy [^\\n]+]=] main_copies \"\${main_rxas}\")\n")
string(APPEND INLINE_LOCAL_MEMBER_CONTENT "list(LENGTH main_copies main_copy_count)\n")
string(APPEND INLINE_LOCAL_MEMBER_CONTENT "if(main_copy_count GREATER 2)\n  message(FATAL_ERROR \"Direct getter/setter receiver placement retained object copies (count=\${main_copy_count}):\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_LOCAL_MEMBER_CONTENT "if(main_rxas MATCHES [=[call [^\\n]*inline_local_member_scalar\\.box\\.§factory\\(\\)]=])\n  message(FATAL_ERROR \"Optimized local member test still calls Box factory:\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_LOCAL_MEMBER_CONTENT "if(main_rxas MATCHES [=[call [^\\n]*inline_local_member_scalar\\.box\\.get\\(\\)]=])\n  message(FATAL_ERROR \"Optimized local member test still calls get():\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_LOCAL_MEMBER_CONTENT "if(main_rxas MATCHES [=[call [^\\n]*inline_local_member_scalar\\.box\\.set\\(\\)]=])\n  message(FATAL_ERROR \"Optimized local member test still calls set():\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_LOCAL_MEMBER_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"inline_local_member_scalar.rxbin\" \"inline_local_member_scalar\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND INLINE_LOCAL_MEMBER_CONTENT "string(APPEND full_out \"\${out}\")\n")
string(APPEND INLINE_LOCAL_MEMBER_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed on local member inline test: \${out}\")\nendif()\n")
string(APPEND INLINE_LOCAL_MEMBER_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxvm>\" \"${CMAKE_BINARY_DIR}/bin/library.rxbin\" \"\${work}/inline_local_member_scalar.rxbin\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND INLINE_LOCAL_MEMBER_CONTENT "string(APPEND full_out \"\${out}\")\n")
string(APPEND INLINE_LOCAL_MEMBER_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxvm failed on local member inline test: \${out}\")\nendif()\n")
string(APPEND INLINE_LOCAL_MEMBER_CONTENT "string(REPLACE \"\\r\\n\" \"\\n\" full_out \"\${full_out}\")\n")
string(APPEND INLINE_LOCAL_MEMBER_CONTENT "if(NOT full_out STREQUAL \"alpha\\nbeta\\ngamma\\n\")\n  message(FATAL_ERROR \"Output mismatch: expected local member inline output, got: \${full_out}\")\nendif()\n")
file(GENERATE OUTPUT "${INLINE_LOCAL_MEMBER_SCRIPT}" CONTENT "${INLINE_LOCAL_MEMBER_CONTENT}")

add_test(NAME inline_local_member_scalar_opt
         COMMAND ${CMAKE_COMMAND} -P "${INLINE_LOCAL_MEMBER_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(inline_local_member_scalar_opt PROPERTIES LABELS "linked_opt")

add_test(NAME inline_receiver_nested_this_direct_placement_opt
         COMMAND ${CMAKE_COMMAND}
                 -DRXC=$<TARGET_FILE:rxc>
                 -DRXAS=$<TARGET_FILE:rxas>
                 -DRXVM=$<TARGET_FILE:rxvm>
                 -DRXBVM=$<TARGET_FILE:rxbvm>
                 -DIMPORT_DIR=${CMAKE_BINARY_DIR}/bin
                 -DLIBRARY=${CMAKE_BINARY_DIR}/bin/library.rxbin
                 -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/inline_test_mutating_method_scalar_attr_return.crexx
                 -DWORK=${CMAKE_CURRENT_BINARY_DIR}/inline_receiver_nested_this_opt_ws
                 -P ${CMAKE_CURRENT_SOURCE_DIR}/run_inline_receiver_nested_this_opt.cmake)
set_tests_properties(inline_receiver_nested_this_direct_placement_opt PROPERTIES
        LABELS "linked_opt;receiver"
        FIXTURES_REQUIRED linked_opt_runtime_artifacts)

add_test(NAME inline_receiver_reference_alias_opt
         COMMAND ${CMAKE_COMMAND}
                 -DRXC=$<TARGET_FILE:rxc>
                 -DRXAS=$<TARGET_FILE:rxas>
                 -DRXVM=$<TARGET_FILE:rxvm>
                 -DRXBVM=$<TARGET_FILE:rxbvm>
                 -DIMPORT_DIR=${CMAKE_BINARY_DIR}/bin
                 -DLIBRARY=${CMAKE_BINARY_DIR}/bin/library.rxbin
                 -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/inline_receiver_reference_alias.crexx
                 -DWORK=${CMAKE_CURRENT_BINARY_DIR}/inline_receiver_reference_alias_opt_ws
                 -P ${CMAKE_CURRENT_SOURCE_DIR}/run_inline_receiver_reference_alias_opt.cmake)
set_tests_properties(inline_receiver_reference_alias_opt PROPERTIES
        LABELS "linked_opt;reference"
        FIXTURES_REQUIRED linked_opt_runtime_artifacts)

add_test(NAME inline_reference_member_accessors_opt
         COMMAND ${CMAKE_COMMAND}
                 -DRXC=$<TARGET_FILE:rxc>
                 -DRXAS=$<TARGET_FILE:rxas>
                 -DRXVM=$<TARGET_FILE:rxvm>
                 -DRXBVM=$<TARGET_FILE:rxbvm>
                 -DIMPORT_DIR=${CMAKE_BINARY_DIR}/bin
                 -DLIBRARY=${CMAKE_BINARY_DIR}/bin/library.rxbin
                 -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/inline_reference_member_accessors.crexx
                 -DWORK=${CMAKE_CURRENT_BINARY_DIR}/inline_reference_member_accessors_opt_ws
                 -P ${CMAKE_CURRENT_SOURCE_DIR}/run_inline_reference_member_accessors_opt.cmake)
set_tests_properties(inline_reference_member_accessors_opt PROPERTIES
        LABELS "linked_opt;reference;receiver"
        FIXTURES_REQUIRED linked_opt_runtime_artifacts)

add_test(NAME inline_reference_accessors_import_opt
         COMMAND ${CMAKE_COMMAND}
                 -DRXC=$<TARGET_FILE:rxc>
                 -DRXAS=$<TARGET_FILE:rxas>
                 -DRXVM=$<TARGET_FILE:rxvm>
                 -DRXBVM=$<TARGET_FILE:rxbvm>
                 -DIMPORT_DIR=${CMAKE_BINARY_DIR}/bin
                 -DLIBRARY=${CMAKE_BINARY_DIR}/bin/library.rxbin
                 -DDEP_SOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/inline_reference_accessors_dep.crexx
                 -DMAIN_SOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/inline_reference_accessors_main.crexx
                 -DWORK=${CMAKE_CURRENT_BINARY_DIR}/inline_reference_accessors_import_opt_ws
                 -P ${CMAKE_CURRENT_SOURCE_DIR}/run_inline_reference_accessors_import_opt.cmake)
set_tests_properties(inline_reference_accessors_import_opt PROPERTIES
        LABELS "linked_opt;reference;receiver;imports"
        FIXTURES_REQUIRED linked_opt_runtime_artifacts)

set(INLINE_COMPUTED_RECEIVER_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_inline_test_computed_receiver_copyback_opt_rewrites.cmake")
set(INLINE_COMPUTED_RECEIVER_CONTENT "set(work \"${CMAKE_CURRENT_BINARY_DIR}/inline_test_computed_receiver_copyback_opt_ws\")\n")
string(APPEND INLINE_COMPUTED_RECEIVER_CONTENT "file(REMOVE_RECURSE \"\${work}\")\n")
string(APPEND INLINE_COMPUTED_RECEIVER_CONTENT "file(MAKE_DIRECTORY \"\${work}\")\n")
string(APPEND INLINE_COMPUTED_RECEIVER_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E copy_if_different \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/inline_test_computed_receiver_copyback.crexx\" \"\${work}/inline_test_computed_receiver_copyback.crexx\" RESULT_VARIABLE res)\n")
string(APPEND INLINE_COMPUTED_RECEIVER_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"Failed to stage computed receiver inline test\")\nendif()\n")
string(APPEND INLINE_COMPUTED_RECEIVER_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"inline_test_computed_receiver_copyback\" \"\${work}/inline_test_computed_receiver_copyback.crexx\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND INLINE_COMPUTED_RECEIVER_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed on computed receiver inline test: \${out}\")\nendif()\n")
string(APPEND INLINE_COMPUTED_RECEIVER_CONTENT "file(READ \"\${work}/inline_test_computed_receiver_copyback.rxas\" main_rxas)\n")
string(APPEND INLINE_COMPUTED_RECEIVER_CONTENT "if(main_rxas MATCHES [=[call [^\\n]*inline_test_computed_receiver_copyback\\.box\\.setandreport\\(\\)]=])\n  message(FATAL_ERROR \"Optimized computed receiver test still calls setAndReport():\\n\${main_rxas}\")\nendif()\n")
file(GENERATE OUTPUT "${INLINE_COMPUTED_RECEIVER_SCRIPT}" CONTENT "${INLINE_COMPUTED_RECEIVER_CONTENT}")

add_test(NAME inline_test_computed_receiver_copyback_opt_rewrites
         COMMAND ${CMAKE_COMMAND} -P "${INLINE_COMPUTED_RECEIVER_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(inline_test_computed_receiver_copyback_opt_rewrites PROPERTIES LABELS "linked_opt")

add_crexx_run_args_test(test_parse_arg_filename rexx_src/test_parse_arg_filename.crexx demo.rexx)
add_crexx_run_args_test(implicit_main_arg_compat rexx_src/implicit_main_arg_compat.crexx alpha beta)
add_crexx_run_args_test(arg_compat_no_vargs rexx_src/arg_compat_no_vargs.crexx alpha beta)

set(CRXC_NOARGS_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_crxc_noargs.cmake")
set(CRXC_NOARGS_CONTENT "set(full_out \"\")\n")
string(APPEND CRXC_NOARGS_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" --level levelb --import rxfnsb -i \"${CMAKE_BINARY_DIR}/bin\" ${COMPILER_NOOPT_FLAG} -o \"crxc_noargs_run\" \"${CMAKE_SOURCE_DIR}/bin/crxc.rexx\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND CRXC_NOARGS_CONTENT "string(APPEND full_out \"\${out}\")\n")
string(APPEND CRXC_NOARGS_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed: \${out}\")\nendif()\n")
string(APPEND CRXC_NOARGS_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"crxc_noargs_run.rxbin\" \"crxc_noargs_run\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND CRXC_NOARGS_CONTENT "string(APPEND full_out \"\${out}\")\n")
string(APPEND CRXC_NOARGS_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed: \${out}\")\nendif()\n")
string(APPEND CRXC_NOARGS_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxvm>\" \"${CMAKE_BINARY_DIR}/bin/library.rxbin\" \"crxc_noargs_run.rxbin\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND CRXC_NOARGS_CONTENT "string(APPEND full_out \"\${out}\")\n")
string(APPEND CRXC_NOARGS_CONTENT "if(NOT res EQUAL 99)\n  message(FATAL_ERROR \"rxvm noargs expected rc 99, got \${res}: \${out}\")\nendif()\n")
string(APPEND CRXC_NOARGS_CONTENT "file(READ \"${CMAKE_CURRENT_SOURCE_DIR}/golden/run/crxc_noargs.out\" golden_out)\n")
string(APPEND CRXC_NOARGS_CONTENT "string(REPLACE \"\\r\\n\" \"\\n\" full_out \"\${full_out}\")\n")
string(APPEND CRXC_NOARGS_CONTENT "string(REPLACE \"\\r\\n\" \"\\n\" golden_out \"\${golden_out}\")\n")
string(APPEND CRXC_NOARGS_CONTENT "if(NOT full_out STREQUAL golden_out)\n  message(FATAL_ERROR \"Output mismatch: \\nExpected:\\n\${golden_out}\\nGot:\\n\${full_out}\")\nendif()\n")
file(GENERATE OUTPUT "${CRXC_NOARGS_SCRIPT}" CONTENT "${CRXC_NOARGS_CONTENT}")

add_test(NAME crxc_noargs_run
         COMMAND ${CMAKE_COMMAND} -P "${CRXC_NOARGS_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

set(STRUCTURED_EXIT_DEBUG_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_structured_exit_debug.cmake")
set(STRUCTURED_EXIT_DEBUG_CONTENT "set(run_out \"\")\n")
string(APPEND STRUCTURED_EXIT_DEBUG_CONTENT "set(debug_log \"${CMAKE_CURRENT_BINARY_DIR}/structured_exit_debug_d2.log\")\n")
string(APPEND STRUCTURED_EXIT_DEBUG_CONTENT "file(REMOVE \"\${debug_log}\")\n")
string(APPEND STRUCTURED_EXIT_DEBUG_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -n -o \"structured_exit_debug_bundle\" \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/structured_exit_debug_bundle.crexx\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND STRUCTURED_EXIT_DEBUG_CONTENT "string(APPEND run_out \"\${out}\")\n")
string(APPEND STRUCTURED_EXIT_DEBUG_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed building structured exit bundle: \${out}\")\nendif()\n")
string(APPEND STRUCTURED_EXIT_DEBUG_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"structured_exit_debug_bundle\" \"structured_exit_debug_bundle\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND STRUCTURED_EXIT_DEBUG_CONTENT "string(APPEND run_out \"\${out}\")\n")
string(APPEND STRUCTURED_EXIT_DEBUG_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed building structured exit bundle: \${out}\")\nendif()\n")
string(APPEND STRUCTURED_EXIT_DEBUG_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E env \"RXCP_EXIT_MODULE=structured_exit_debug_bundle\" \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_CURRENT_BINARY_DIR}\" -i \"${CMAKE_BINARY_DIR}/bin\" -d2 -n -o \"structured_exit_debug_main\" \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/structured_exit_debug_main.crexx\" OUTPUT_FILE \"\${debug_log}\" ERROR_FILE \"\${debug_log}\" RESULT_VARIABLE res)\n")
string(APPEND STRUCTURED_EXIT_DEBUG_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc -d2 failed for structured exit debug case. See \${debug_log}\")\nendif()\n")
string(APPEND STRUCTURED_EXIT_DEBUG_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"structured_exit_debug_main.rxbin\" \"structured_exit_debug_main\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND STRUCTURED_EXIT_DEBUG_CONTENT "string(APPEND run_out \"\${out}\")\n")
string(APPEND STRUCTURED_EXIT_DEBUG_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed for structured exit main: \${out}\")\nendif()\n")
string(APPEND STRUCTURED_EXIT_DEBUG_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxvm>\" \"${CMAKE_BINARY_DIR}/bin/library.rxbin\" \"structured_exit_debug_main.rxbin\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND STRUCTURED_EXIT_DEBUG_CONTENT "string(APPEND run_out \"\${out}\")\n")
string(APPEND STRUCTURED_EXIT_DEBUG_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxvm failed for structured exit main: \${out}\")\nendif()\n")
string(APPEND STRUCTURED_EXIT_DEBUG_CONTENT "file(READ \"${CMAKE_CURRENT_SOURCE_DIR}/golden/run/structured_exit_debug.out\" golden_out)\n")
string(APPEND STRUCTURED_EXIT_DEBUG_CONTENT "string(REPLACE \"\\r\\n\" \"\\n\" run_out \"\${run_out}\")\n")
string(APPEND STRUCTURED_EXIT_DEBUG_CONTENT "string(REPLACE \"\\r\\n\" \"\\n\" golden_out \"\${golden_out}\")\n")
string(APPEND STRUCTURED_EXIT_DEBUG_CONTENT "if(NOT run_out STREQUAL golden_out)\n  message(FATAL_ERROR \"Output mismatch: \\nExpected:\\n\${golden_out}\\nGot:\\n\${run_out}\")\nendif()\n")
file(GENERATE OUTPUT "${STRUCTURED_EXIT_DEBUG_SCRIPT}" CONTENT "${STRUCTURED_EXIT_DEBUG_CONTENT}")

add_test(NAME structured_exit_debug_run
         COMMAND ${CMAKE_COMMAND} -P "${STRUCTURED_EXIT_DEBUG_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
add_crexx_run_test(test_obj_prop rexx_src/test_obj_prop.crexx)
add_crexx_run_test(test_meth_vs_proc rexx_src/test_meth_vs_proc.crexx)
add_crexx_run_test(function_arg_method_call rexx_src/function_arg_method_call.crexx)
add_crexx_run_test(test_class_adv rexx_src/test_class_adv.crexx)
add_crexx_run_test(test_array_edge rexx_src/test_array_edge.crexx)
add_crexx_run_test(test_utf_math rexx_src/test_utf_math.crexx)
add_crexx_run_test(binary_literal_load rexx_src/binary_literal_load.crexx)
add_crexx_run_test(scalar_type_casts rexx_src/scalar_type_casts.crexx)
add_crexx_run_test(test_lib_boundary rexx_src/test_lib_boundary.crexx)
add_crexx_run_test(div_ops_common rexx_src/div_ops_common.crexx)
add_crexx_run_test(div_ops_classic rexx_src/div_ops_classic.crexx)
add_crexx_run_test(classic_xor rexx_src/classic_xor.crexx)
add_crexx_run_test(named_operators rexx_src/named_operators.crexx)

# Shadowing behavior in simple DO blocks
add_crexx_run_test(do_scope_typed_shadows rexx_src/do_scope_typed_shadows.crexx)

# Hoisting and effective scopes tests
add_crexx_run_test(test_global_subscopes rexx_src/test_global_subscopes.crexx)

# Inlining tests
add_crexx_run_test(inline_test1 rexx_src/inline_test1.crexx)
add_crexx_run_test(inline_test2 rexx_src/inline_test2.crexx)
add_crexx_run_test(inline_test_call rexx_src/inline_test_call.crexx)
add_crexx_run_test(inline_test_expr rexx_src/inline_test_expr.crexx)
add_crexx_run_test(inline_test_expr_negative rexx_src/inline_test_expr_negative.crexx)
add_crexx_run_test(inline_test_nested_call_expr rexx_src/inline_test_nested_call_expr.crexx)
add_crexx_run_test(inline_test_concat_expr rexx_src/inline_test_concat_expr.crexx)
add_crexx_run_test(inline_test_say_expr rexx_src/inline_test_say_expr.crexx)
add_crexx_run_test(inline_test_say_expr_negative rexx_src/inline_test_say_expr_negative.crexx)
add_crexx_run_test(inline_test_return_expr rexx_src/inline_test_return_expr.crexx)
add_crexx_run_test(inline_test_unary_expr rexx_src/inline_test_unary_expr.crexx)
add_crexx_run_test(inline_test_compare_expr rexx_src/inline_test_compare_expr.crexx)
add_crexx_run_test(inline_test_call_arg_expr rexx_src/inline_test_call_arg_expr.crexx)
add_crexx_run_test(inline_test_call_like_arg_expr rexx_src/inline_test_call_like_arg_expr.crexx)
add_crexx_run_test(inline_test_short_circuit_expr rexx_src/inline_test_short_circuit_expr.crexx)
add_crexx_run_test(inline_test_composed_expr_contexts rexx_src/inline_test_composed_expr_contexts.crexx)
add_crexx_run_test(inline_test_byvalue_arg_reuse rexx_src/inline_test_byvalue_arg_reuse.crexx)
add_crexx_run_test(inline_test_stateful_assembler rexx_src/inline_test_stateful_assembler.crexx)
add_crexx_run_test(assembler_wide_operands rexx_src/assembler_wide_operands.crexx)
add_crexx_run_test(inline_test_condition_contexts rexx_src/inline_test_condition_contexts.crexx)
add_crexx_run_test(inline_test_loop_bound_expr rexx_src/inline_test_loop_bound_expr.crexx)
add_crexx_run_test(inline_test_member_receiver_expr rexx_src/inline_test_member_receiver_expr.crexx)
add_crexx_run_test(inline_test_named_factory_selector rexx_src/inline_test_named_factory_selector.crexx)
add_crexx_run_test(inline_test_factory_arg_scope rexx_src/inline_test_factory_arg_scope.crexx)
add_crexx_run_test(inline_test_computed_receiver_copyback rexx_src/inline_test_computed_receiver_copyback.crexx)
add_crexx_run_test(inline_test_mutating_method_scalar_attr_return rexx_src/inline_test_mutating_method_scalar_attr_return.crexx)
add_crexx_run_test(inline_test_ref_opt rexx_src/inline_test_ref_opt.crexx)
add_crexx_run_test(inline_test_ref_indexed rexx_src/inline_test_ref_indexed.crexx)
add_crexx_run_test(inline_test_ref_stem rexx_src/inline_test_ref_stem.crexx)
add_crexx_run_test(inline_test_ref_computed rexx_src/inline_test_ref_computed.crexx)
add_crexx_run_test(inline_test_ref_vararg rexx_src/inline_test_ref_vararg.crexx)
add_crexx_run_test(inline_test_ref_vararg_runtime rexx_src/inline_test_ref_vararg_runtime.crexx)
add_crexx_run_test(inline_test_ref_negative rexx_src/inline_test_ref_negative.crexx)
add_crexx_run_test(inline_test_bool_expr_negative rexx_src/inline_test_bool_expr_negative.crexx)
add_crexx_run_test(inline_test_vararg rexx_src/inline_test_vararg.crexx)
add_crexx_run_test(inline_test_vararg_dynamic rexx_src/inline_test_vararg_dynamic.crexx)
add_crexx_run_test(inline_test_vararg_dynamic_loop rexx_src/inline_test_vararg_dynamic_loop.crexx)
add_crexx_run_test(inline_test_vararg_negative rexx_src/inline_test_vararg_negative.crexx)
add_crexx_run_test(inline_test_exposed_nested_call rexx_src/inline_test_exposed_nested_call.crexx)
add_crexx_run_test(inline_test_nested_scope_do rexx_src/inline_test_nested_scope_do.crexx)
add_crexx_run_test(inline_test_nested_scope_if_do rexx_src/inline_test_nested_scope_if_do.crexx)
add_crexx_run_test(inline_test_nested_scope_exposed_call rexx_src/inline_test_nested_scope_exposed_call.crexx)
add_crexx_run_test(inline_test_multi_return_assign rexx_src/inline_test_multi_return_assign.crexx)
add_crexx_run_test(inline_test_multi_return_expr rexx_src/inline_test_multi_return_expr.crexx)
add_crexx_run_test(inline_test_multi_return_call rexx_src/inline_test_multi_return_call.crexx)
add_crexx_run_test(inline_test_void_fallthrough_call rexx_src/inline_test_void_fallthrough_call.crexx)
add_crexx_run_test(inline_test_array_return_assign rexx_src/inline_test_array_return_assign.crexx)
add_crexx_run_test(inline_test_array_return_expr rexx_src/inline_test_array_return_expr.crexx)
add_crexx_run_test(inline_test_array_multi_return_expr rexx_src/inline_test_array_multi_return_expr.crexx)
add_crexx_run_test(inline_test_array_multi_return_assign rexx_src/inline_test_array_multi_return_assign.crexx)
add_crexx_run_test(inline_test_object_return_assign rexx_src/inline_test_object_return_assign.crexx)
add_crexx_run_test(inline_test_object_return_expr rexx_src/inline_test_object_return_expr.crexx)
add_crexx_run_test(inline_test_object_const_arg_mutation rexx_src/inline_test_object_const_arg_mutation.crexx)
add_crexx_run_test(inline_test_object_writable_arg rexx_src/inline_test_object_writable_arg.crexx)
add_crexx_run_test(inline_test_array_expr_arg rexx_src/inline_test_array_expr_arg.crexx)
add_crexx_run_test(inline_test_object_expr_arg rexx_src/inline_test_object_expr_arg.crexx)
add_crexx_run_test(inline_test_array_return_temp rexx_src/inline_test_array_return_temp.crexx)
add_crexx_run_test(inline_test_block_expr_live_sibling rexx_src/inline_test_block_expr_live_sibling.crexx)
add_crexx_run_test(inline_test_imported_bif_block_expr_lifetime rexx_src/inline_test_imported_bif_block_expr_lifetime.crexx)
add_crexx_run_test(attr_array_call_result_lifetime rexx_src/attr_array_call_result_lifetime.crexx)
add_crexx_run_test(inline_test_complex_lhs_assign rexx_src/inline_test_complex_lhs_assign.crexx)
add_crexx_run_test(inline_test_binary_arg_return rexx_src/inline_test_binary_arg_return.crexx)
add_crexx_run_test(inline_test_class_methods rexx_src/inline_test_class_methods.crexx)

# New Phase 2 Scoping Tests
add_crexx_run_test(test_simple_do_subscope rexx_src/test_simple_do_subscope.crexx)
add_crexx_run_test(test_if_do_subscope rexx_src/test_if_do_subscope.crexx)
add_crexx_run_test(test_iterative_do_subscope rexx_src/test_iterative_do_subscope.crexx)
add_crexx_run_test(test_if_no_subscope rexx_src/test_if_no_subscope.crexx)
add_crexx_run_test(test_untyped_subscopes rexx_src/test_untyped_subscopes.crexx)
add_crexx_run_test(test_untyped_subscope_2 rexx_src/test_untyped_subscope_2.crexx)
add_crexx_run_test(test_if_else_nested_subscopes rexx_src/test_if_else_nested_subscopes.crexx)
add_crexx_run_test(test_sub_sub_scopes rexx_src/test_sub_sub_scopes.crexx)
add_crexx_run_test(test_sub_sub_scopes_untyped rexx_src/test_sub_sub_scopes_untyped.crexx)
add_crexx_run_test(test_global_nested_complex rexx_src/test_global_nested_complex.crexx)
add_crexx_run_test(test_function_shadowing rexx_src/test_function_shadowing.crexx)
add_crexx_run_test(test_disjoint_detailed rexx_src/test_disjoint_detailed.crexx)

# New test for detailed warning
set(WARN_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_warn_test.cmake")
set(WARN_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -o test_disjoint_detailed_warn \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/test_disjoint_detailed.crexx\" ERROR_VARIABLE err_out OUTPUT_VARIABLE std_out RESULT_VARIABLE res)\n")
string(APPEND WARN_CONTENT "if(NOT err_out MATCHES \"NOT_IN_SAME_SCOPE line=\\\"4\\\" column=\\\"5\\\"\")\n  message(FATAL_ERROR \"Expected warning not found in output\")\nendif()\n")
file(GENERATE OUTPUT "${WARN_SCRIPT}" CONTENT "${WARN_CONTENT}")

add_test(NAME test_disjoint_detailed_warn
         COMMAND ${CMAKE_COMMAND} -P "${WARN_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})


# Parsing Tests
add_crexx_parsing_test(expr_abut_01 parsing_src/expr_abut_01.crexx)
add_crexx_parsing_test(expr_abut_02 parsing_src/expr_abut_02.crexx)
add_crexx_parsing_test(expr_abut_03 parsing_src/expr_abut_03.crexx)
add_crexx_parsing_test(expr_abut_04 parsing_src/expr_abut_04.crexx)
add_crexx_parsing_test(expr_ambig_01 parsing_src/expr_ambig_01.crexx)
add_crexx_parsing_test(expr_ambig_02 parsing_src/expr_ambig_02.crexx)
add_crexx_parsing_test(expr_ambig_03 parsing_src/expr_ambig_03.crexx)
add_crexx_parsing_test(binary_intrinsics_parse parsing_src/binary_intrinsics_parse.crexx)
add_crexx_parsing_test(err_struct_01 parsing_src/err_struct_01.crexx)
add_crexx_parsing_test(err_struct_02 parsing_src/err_struct_02.crexx)
add_crexx_parsing_test(err_struct_03 parsing_src/err_struct_03.crexx)
add_crexx_parsing_test(err_namespace_class_anchor parsing_src/err_namespace_class_anchor.crexx)
add_crexx_parsing_test(address_exit_extended parsing_src/address_exit_extended.crexx)
add_crexx_parsing_test(address_inline_then parsing_src/address_inline_then.crexx)
# DO constructs parsing coverage
add_crexx_parsing_test(do_simple_parse parsing_src/do_simple_parse.crexx)
add_crexx_parsing_test(do_counted_parse parsing_src/do_counted_parse.crexx)
add_crexx_parsing_test(do_while_parse parsing_src/do_while_parse.crexx)
add_crexx_parsing_test(do_until_parse parsing_src/do_until_parse.crexx)
add_crexx_parsing_test(block_expr_parse parsing_src/block_expr_parse.crexx)

# SELECT constructs parsing coverage
add_crexx_parsing_test(select_classic_parse parsing_src/select_classic_parse.crexx)
add_crexx_parsing_test(select_c_parse parsing_src/select_c_parse.crexx)

if (ENABLE_PARSER_MODE)
    add_crexx_syntax_highlight_test(
            syntaxhighlight_root_file
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/16_classes.crexx
            MUST_MATCH
            "Node: type=71 \\(PARSE_TREE_FILE\\)"
    )

    add_crexx_syntax_highlight_test(
            syntaxhighlight_syntax_error
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/err_02_syntax.crexx
            MUST_MATCH
            "SYNTAX_ERROR"
            "Message:"
    )

    add_crexx_syntax_highlight_test(
            syntaxhighlight_srcmap_generated
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/syntaxhighlight_srcmap_generated.crexx
            MUST_MATCH
            "Node: type=37 \\(LEXER_PREPROCESSOR\\), pos=[0-9]+, len=3, sev=48, text=\"@1c\""
            "Node: type=37 \\(LEXER_PREPROCESSOR\\), pos=[0-9]+, len=7, sev=48, text=\"@\\+4\\+16\\{\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=3, sev=48, text=\"say\""
            "Node: type=57 \\(LEXER_CONSTANT_IDENTIFIER\\), pos=[0-9]+, len=4, sev=48, text=\"totl\""
            MUST_NOT_MATCH
            "text=\"emo\""
            "text=\"xpp"
    )

    add_crexx_syntax_highlight_test(
            syntaxhighlight_compound_names
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/repro_dotted_index.crexx
            MUST_MATCH
            "Node: type=46 \\(LEXER_SEPARATOR\\), pos=[0-9]+, len=1, sev=48, text=\"\\.\""
            "Node: type=54 \\(LEXER_IDENTIFIER\\), pos=[0-9]+, len=4, sev=48, text=\"lino\""
            "Node: type=40 \\(LEXER_NUMBER_LITERAL\\), pos=[0-9]+, len=1, sev=48, text=\"1\""
            MUST_NOT_MATCH
            "Node: type=38 \\(LEXER_COMMENT\\), pos=[0-9]+, len=1, sev=48, text=\"\\.\""
            "Node: type=46 \\(LEXER_SEPARATOR\\), pos=[0-9]+, len=1, sev=48, text=\"[A-Za-z0-9]\""
    )

    add_crexx_syntax_highlight_test(
            syntaxhighlight_structure_nodes
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/16_classes.crexx
            MUST_MATCH
            "PARSE_TREE_STRUCTURE"
            "PARSE_TREE_FUNCTION"
            "PARSE_TREE_CODEBLOCK"
    )

    add_crexx_syntax_highlight_test(
            syntaxhighlight_exit_keyword
            ${CMAKE_SOURCE_DIR}/compiler/exits/fsay/test_fsay.crexx
            MUST_MATCH
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=4, sev=48, text=\"fsay\""
    )

    add_crexx_syntax_highlight_test(
            syntaxhighlight_address_keywords
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/syntaxhighlight_address_keywords.crexx
            MUST_MATCH
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=7, sev=48, text=\"address\""
            "Node: type=54 \\(LEXER_IDENTIFIER\\), pos=[0-9]+, len=5, sev=48, text=\"input\""
            "Node: type=54 \\(LEXER_IDENTIFIER\\), pos=[0-9]+, len=6, sev=48, text=\"output\""
            "Node: type=54 \\(LEXER_IDENTIFIER\\), pos=[0-9]+, len=5, sev=48, text=\"error\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=6, sev=48, text=\"expose\""
    )

    add_crexx_syntax_highlight_test(
            syntaxhighlight_parse_keywords
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/syntaxhighlight_parse_keywords.crexx
            MUST_MATCH
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=5, sev=48, text=\"parse\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=5, sev=48, text=\"upper\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=5, sev=48, text=\"value\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=4, sev=48, text=\"trim\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=4, sev=48, text=\"into\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=3, sev=48, text=\"var\""
    )

    add_crexx_syntax_highlight_test(
            syntaxhighlight_trace_keywords
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/syntaxhighlight_trace_keywords.crexx
            MUST_MATCH
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=5, sev=48, text=\"trace\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=3, sev=48, text=\"off\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=6, sev=48, text=\"normal\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=4, sev=48, text=\"rexx\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=3, sev=48, text=\"asm\""
    )

    add_crexx_syntax_highlight_test(
            syntaxhighlight_top_level_statements
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/01_assign.crexx
            MUST_MATCH
            "PARSE_TREE_STATEMENT"
            MUST_NOT_MATCH
            "PARSE_TREE_FUNCTION"
    )
    set_tests_properties(syntaxhighlight_top_level_statements PROPERTIES LABELS "compiler;syntax_highlighting;stage1")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_tracer
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_tracer.rexx
            MUST_MATCH
            "Node: type=54 \\(LEXER_IDENTIFIER\\), pos=0, len=2, sev=48, text=\"if\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=3, sev=48, text=\"say\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=4, sev=48, text=\"then\""
            "Node: type=56 \\(LEXER_FUNCTION_IDENTIFIER\\), pos=[0-9]+, len=6, sev=48, text=\"label:\""
            "PARSE_TREE_STATEMENT"
            MUST_NOT_MATCH
            "SYNTAX_ERROR"
    )
    set_tests_properties(syntaxhighlight_levelc_tracer PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_keyword_variables
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_keyword_variables.rexx
            MUST_MATCH
            "Node: type=54 \\(LEXER_IDENTIFIER\\), pos=0, len=2, sev=48, text=\"if\""
            "Node: type=54 \\(LEXER_IDENTIFIER\\), pos=[0-9]+, len=3, sev=48, text=\"say\""
            "Node: type=54 \\(LEXER_IDENTIFIER\\), pos=[0-9]+, len=4, sev=48, text=\"then\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=3, sev=48, text=\"say\""
            "PARSE_TREE_EXPR"
            MUST_NOT_MATCH
            "SYNTAX_ERROR"
    )
    set_tests_properties(syntaxhighlight_levelc_keyword_variables PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_simple_instructions
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_simple_instructions.rexx
            MUST_MATCH
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=7, sev=48, text=\"address\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=3, sev=48, text=\"arg\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=4, sev=48, text=\"call\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=4, sev=48, text=\"drop\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=4, sev=48, text=\"exit\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=9, sev=48, text=\"interpret\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=3, sev=48, text=\"nop\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=7, sev=48, text=\"numeric\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=9, sev=48, text=\"procedure\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=4, sev=48, text=\"pull\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=4, sev=48, text=\"push\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=5, sev=48, text=\"queue\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=6, sev=48, text=\"return\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=6, sev=48, text=\"signal\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=5, sev=48, text=\"trace\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=5, sev=48, text=\"value\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=4, sev=48, text=\"with\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=6, sev=48, text=\"output\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=6, sev=48, text=\"stream\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=7, sev=48, text=\"replace\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=2, sev=48, text=\"on\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=3, sev=48, text=\"off\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=5, sev=48, text=\"error\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=4, sev=48, text=\"halt\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=4, sev=48, text=\"name\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=6, sev=48, text=\"digits\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=4, sev=48, text=\"form\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=10, sev=48, text=\"scientific\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=4, sev=48, text=\"fuzz\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=6, sev=48, text=\"expose\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=7, sev=48, text=\"novalue\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=6, sev=48, text=\"syntax\""
            "PARSE_TREE_STATEMENT"
            MUST_NOT_MATCH
            "RXC-LC-"
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_simple_instructions PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_command_instructions
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_command_instructions.rexx
            MUST_MATCH
            "Node: type=39 \\(LEXER_STRING_LITERAL\\), pos=[0-9]+, len=14, sev=48, text=\"'host command'\""
            "Node: type=54 \\(LEXER_IDENTIFIER\\), pos=[0-9]+, len=4, sev=50, text=\"host\""
            "PARSE_TREE_EXPR"
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=2, sev=48, text=\"if\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=4, sev=48, text=\"then\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=6, sev=48, text=\"select\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=9, sev=48, text=\"otherwise\""
            "RXC-LC-IMPLICIT_ADDRESS"
            MUST_NOT_MATCH
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_command_instructions PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_implicit_command_warning
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_implicit_command_warning.rexx
            MUST_MATCH
            "Node: type=39 \\(LEXER_STRING_LITERAL\\), pos=[0-9]+, len=11, sev=48, text=\"'listfiles'\""
            "Node: type=54 \\(LEXER_IDENTIFIER\\), pos=[0-9]+, len=3, sev=50, text=\"SEY\""
            "RXC-LC-IMPLICIT_ADDRESS"
            "Node: type=54 \\(LEXER_IDENTIFIER\\), pos=[0-9]+, len=5, sev=48, text=\"after\""
            MUST_NOT_MATCH
            "Node: type=39 \\(LEXER_STRING_LITERAL\\), pos=[0-9]+, len=11, sev=50, text=\"'listfiles'\""
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_implicit_command_warning PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_instruction_bad_forms
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_instruction_bad_forms.rexx
            MUST_MATCH
            "RXC-LC-20\\.1"
            "RXC-LC-19\\.2"
            "RXC-LC-19\\.4"
            "RXC-LC-25\\.15"
            "RXC-LC-25\\.11"
            "RXC-LC-25\\.5"
            "RXC-LC-35\\.1"
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=3, sev=48, text=\"say\""
            MUST_NOT_MATCH
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_instruction_bad_forms PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_instruction_recovery_boundaries
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_instruction_recovery_boundaries.rexx
            MUST_MATCH
            "RXC-LC-25\\.15 keywords=\"DIGITS FORM FUZZ\" token=\"10\""
            "RXC-LC-25\\.17 keywords=\"EXPOSE\" token=\"10\""
            "RXC-LC-20\\.1 token=\"10\""
            "RXC-LC-25\\.12 keywords=\"ARG PULL SOURCE LINEIN VERSION VALUE VAR\" token=\"10\""
            "RXC-LC-25\\.13 keywords=\"ARG PULL SOURCE LINEIN VERSION VALUE VAR\" token=\"10\""
            "RXC-LC-19\\.2 token=\"10\""
            "RXC-LC-19\\.4 token=\"10\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=3, sev=48, text=\"arg\""
            "Node: type=54 \\(LEXER_IDENTIFIER\\), pos=[0-9]+, len=1, sev=48, text=\"a\""
            "Node: type=54 \\(LEXER_IDENTIFIER\\), pos=[0-9]+, len=11, sev=48, text=\"afterSignal\""
            MUST_NOT_MATCH
            "token=\"a\""
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_instruction_recovery_boundaries PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_instruction_tail_bad_forms
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_instruction_tail_bad_forms.rexx
            MUST_MATCH
            "RXC-LC-25\\.1 keywords=\"ERROR FAILURE HALT NOTREADY\" token=\"end-of-clause\""
            "RXC-LC-25\\.1 keywords=\"ERROR FAILURE HALT NOTREADY\" token=\"syntax\""
            "RXC-LC-25\\.2 keywords=\"ERROR FAILURE HALT NOTREADY\" token=\"10\""
            "RXC-LC-19\\.3 token=\"end-of-clause\""
            "RXC-LC-19\\.3 token=\"10\""
            "RXC-LC-25\\.3 keywords=\"ERROR FAILURE HALT NOTREADY NOVALUE SYNTAX LOSTDIGITS\" token=\"end-of-clause\""
            "RXC-LC-25\\.3 keywords=\"ERROR FAILURE HALT NOTREADY NOVALUE SYNTAX LOSTDIGITS\" token=\"banana\""
            "RXC-LC-25\\.4 keywords=\"ERROR FAILURE HALT NOTREADY NOVALUE SYNTAX LOSTDIGITS\" token=\"10\""
            "RXC-LC-19\\.1 token=\"end-of-clause\""
            "RXC-LC-25\\.5 keywords=\"INPUT OUTPUT ERROR\" token=\"banana\""
            "RXC-LC-25\\.5 keywords=\"INPUT OUTPUT ERROR\" token=\"10\""
            "RXC-LC-25\\.11 keywords=\"ENGINEERING SCIENTIFIC VALUE\" token=\"10\""
            "RXC-LC-35\\.1 token=\"end-of-clause\""
            "RXC-LC-20\\.1 token=\"end-of-clause\""
            "RXC-LC-20\\.1 token=\"\\)\""
            "RXC-LC-20\\.1 token=\"10\""
            "RXC-LC-19\\.7 token=\"\\)\""
            "RXC-LC-46\\.1"
            "RXC-LC-19\\.4 token=\"end-of-clause\""
            "Node: type=54 \\(LEXER_IDENTIFIER\\), pos=[0-9]+, len=24, sev=48, text=\"afterAddressValueBadExpr\""
            MUST_NOT_MATCH
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_instruction_tail_bad_forms PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_labels_validation
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_labels_validation.rexx
            MUST_MATCH
            "RXC-LC-16\\.1 name=\"MISSING\""
            "RXC-LC-16\\.2 name=\"INNER\""
            "RXC-LC-16\\.3 name=\"INNER\""
            "RXC-LC-17\\.1"
            "Node: type=56 \\(LEXER_FUNCTION_IDENTIFIER\\), pos=[0-9]+, len=5, sev=48, text=\"good:\""
            "Node: type=56 \\(LEXER_FUNCTION_IDENTIFIER\\), pos=[0-9]+, len=6, sev=48, text=\"inner:\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=9, sev=48, text=\"procedure\""
            MUST_NOT_MATCH
            "RXC-LC-16\\.1 name=\"GOOD\""
            "RXC-LC-16\\.3 name=\"DATE\""
            "RXC-LC-16\\.3 name=\"EXTERNAL_PROC\""
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_labels_validation PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_numeric_validation
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_numeric_validation.rexx
            MUST_MATCH
            "RXC-LC-26\\.5 value=\"0\""
            "RXC-LC-26\\.5 value=\"buzz\""
            "RXC-LC-6\\.3"
            "RXC-LC-26\\.6 value=\"-1\""
            "RXC-LC-26\\.6 value=\"buzz\""
            "RXC-LC-6\\.2"
            "RXC-LC-33\\.6 value=\"bad\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=7, sev=48, text=\"numeric\""
            "Node: type=39 \\(LEXER_STRING_LITERAL\\), pos=[0-9]+, len=13, sev=48, text=\"'engineering'\""
            "Node: type=39 \\(LEXER_STRING_LITERAL\\), pos=[0-9]+, len=12, sev=48, text=\"'scientific'\""
            MUST_NOT_MATCH
            "RXC-LC-26\\.5 value=\"9\""
            "RXC-LC-26\\.6 value=\"0\""
            "RXC-LC-33\\.6 value=\"engineering\""
            "RXC-LC-33\\.6 value=\"scientific\""
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_numeric_validation PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_deep_validation
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_deep_validation.rexx
            MUST_MATCH
            "RXC-LC-24\\.1 value=\"B\""
            "RXC-LC-19\\.6 token=\"\\+\""
            "RXC-LC-20\\.2 token=\"10\""
            "RXC-LC-31\\.1 token=\"10\""
            "RXC-LC-25\\.6 keywords=\"STREAM STEM NORMAL\" token=\"end-of-clause\""
            "RXC-LC-53\\.1 token=\"end-of-clause\""
            "RXC-LC-53\\.1 token=\"10\""
            "RXC-LC-25\\.8 keywords=\"STREAM STEM\" token=\"normal\""
            "RXC-LC-53\\.3 name=\"out\""
            "RXC-LC-15\\.3 char=\"G\""
            "RXC-LC-15\\.4 char=\"2\""
            "RXC-LC-15\\.1"
            "RXC-LC-15\\.2"
            MUST_NOT_MATCH
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_deep_validation PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_deep_validation_ok
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_deep_validation_ok.rexx
            MUST_MATCH
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=5, sev=48, text=\"trace\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=7, sev=48, text=\"address\""
            "Node: type=39 \\(LEXER_STRING_LITERAL\\), pos=[0-9]+, len=4, sev=48, text=\"'F'X\""
            "Node: type=39 \\(LEXER_STRING_LITERAL\\), pos=[0-9]+, len=12, sev=48, text=\"'0000 1111'B\""
            MUST_NOT_MATCH
            "RXC-LC-"
            "INVALID_HEX"
            "INVALID_BIN"
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_deep_validation_ok PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_simple_instruction_keyword_variables
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_simple_instruction_keyword_variables.rexx
            MUST_MATCH
            "Node: type=54 \\(LEXER_IDENTIFIER\\), pos=0, len=7, sev=48, text=\"address\""
            "Node: type=54 \\(LEXER_IDENTIFIER\\), pos=[0-9]+, len=4, sev=48, text=\"call\""
            "Node: type=54 \\(LEXER_IDENTIFIER\\), pos=[0-9]+, len=6, sev=48, text=\"return\""
            "Node: type=54 \\(LEXER_IDENTIFIER\\), pos=[0-9]+, len=5, sev=48, text=\"trace\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=3, sev=48, text=\"say\""
            MUST_NOT_MATCH
            "RXC-LC-"
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_simple_instruction_keyword_variables PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_parse_templates
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_parse_templates.rexx
            MUST_MATCH
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=5, sev=48, text=\"parse\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=5, sev=48, text=\"upper\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=4, sev=48, text=\"pull\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=6, sev=48, text=\"source\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=6, sev=48, text=\"linein\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=7, sev=48, text=\"version\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=5, sev=48, text=\"value\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=4, sev=48, text=\"with\""
            "Node: type=46 \\(LEXER_SEPARATOR\\), pos=[0-9]+, len=1, sev=48, text=\"\\.\""
            "PARSE_TREE_STATEMENT"
            MUST_NOT_MATCH
            "RXC-LC-"
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_parse_templates PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_parse_bad_forms
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_parse_bad_forms.rexx
            MUST_MATCH
            "RXC-LC-25\\.12"
            "RXC-LC-25\\.13"
            "RXC-LC-38\\.3"
            "RXC-LC-38\\.2"
            "RXC-LC-19\\.7"
            MUST_NOT_MATCH
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_parse_bad_forms PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_if_else
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_if_else.rexx
            MUST_MATCH
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=2, sev=48, text=\"if\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=4, sev=48, text=\"then\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=4, sev=48, text=\"else\""
            "PARSE_TREE_STATEMENT"
            MUST_NOT_MATCH
            "SYNTAX_ERROR"
    )
    set_tests_properties(syntaxhighlight_levelc_if_else PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_labels_literals
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_labels_literals.rexx
            MUST_MATCH
            "Node: type=56 \\(LEXER_FUNCTION_IDENTIFIER\\), pos=0, len=6, sev=48, text=\"start:\""
            "Node: type=39 \\(LEXER_STRING_LITERAL\\), pos=[0-9]+, len=8, sev=48, text=\"\"double\"\""
            "Node: type=40 \\(LEXER_NUMBER_LITERAL\\), pos=[0-9]+, len=3, sev=48, text=\"123\""
            MUST_NOT_MATCH
            "SYNTAX_ERROR"
    )
    set_tests_properties(syntaxhighlight_levelc_labels_literals PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_then_boundary
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_then_boundary.rexx
            MUST_MATCH
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=4, sev=51, text=\"then\""
            "RXC-LC-35\\.1 token=\"then\""
            "RXC-LC-8\\.1"
            MUST_NOT_MATCH
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_then_boundary PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_if_missing_then
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_if_missing_then.rexx
            MUST_MATCH
            "RXC-LC-18\\.1 linenumber=\"1\" token=\"end-of-clause\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=3, sev=48, text=\"say\""
            MUST_NOT_MATCH
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_if_missing_then PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_loose_then
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_loose_then.rexx
            MUST_MATCH
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=0, len=4, sev=51, text=\"then\""
            "RXC-LC-8\\.1"
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=3, sev=48, text=\"say\""
            MUST_NOT_MATCH
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_loose_then PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_loose_else
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_loose_else.rexx
            MUST_MATCH
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=0, len=4, sev=51, text=\"else\""
            "RXC-LC-8\\.2"
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=3, sev=48, text=\"say\""
            MUST_NOT_MATCH
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_loose_else PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_multiline_if_else
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_multiline_if_else.rexx
            MUST_MATCH
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=2, sev=48, text=\"if\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=4, sev=48, text=\"then\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=4, sev=48, text=\"else\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=3, sev=48, text=\"say\""
            MUST_NOT_MATCH
            "RXC-LC-"
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_multiline_if_else PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_select_when_otherwise
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_select_when_otherwise.rexx
            MUST_MATCH
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=6, sev=48, text=\"select\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=4, sev=48, text=\"when\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=9, sev=48, text=\"otherwise\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=3, sev=48, text=\"end\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=3, sev=48, text=\"say\""
            MUST_NOT_MATCH
            "RXC-LC-"
            "PARSE_FAILURE"
            "Node: type=72 \\(PARSE_TREE_CODEBLOCK\\), pos=[0-9]+, len=[0-9]+, sev=48\n[ ]+Node: type=73 \\(PARSE_TREE_STATEMENT\\), pos=[0-9]+, len=[0-9]+, sev=48\n[ ]+Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=3, sev=48, text=\"say\""
    )
    set_tests_properties(syntaxhighlight_levelc_select_when_otherwise PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_select_do_otherwise
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_select_do_otherwise.rexx
            MUST_MATCH
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=6, sev=48, text=\"select\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=4, sev=48, text=\"when\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=2, sev=48, text=\"do\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=9, sev=48, text=\"otherwise\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=3, sev=48, text=\"end\""
            MUST_NOT_MATCH
            "RXC-LC-"
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_select_do_otherwise PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_select_do_otherwise_upper
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_select_do_otherwise_upper.rexx
            MUST_MATCH
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=6, sev=48, text=\"SELECT\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=4, sev=48, text=\"WHEN\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=2, sev=48, text=\"DO\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=9, sev=48, text=\"OTHERWISE\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=3, sev=48, text=\"END\""
            MUST_NOT_MATCH
            "RXC-LC-"
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_select_do_otherwise_upper PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_do_keyword_variables
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_do_keyword_variables.rexx
            MUST_MATCH
            "Node: type=54 \\(LEXER_IDENTIFIER\\), pos=0, len=2, sev=48, text=\"do\""
            "Node: type=54 \\(LEXER_IDENTIFIER\\), pos=[0-9]+, len=2, sev=48, text=\"to\""
            "Node: type=54 \\(LEXER_IDENTIFIER\\), pos=[0-9]+, len=2, sev=48, text=\"by\""
            "Node: type=54 \\(LEXER_IDENTIFIER\\), pos=[0-9]+, len=3, sev=48, text=\"for\""
            "Node: type=54 \\(LEXER_IDENTIFIER\\), pos=[0-9]+, len=5, sev=48, text=\"while\""
            "Node: type=54 \\(LEXER_IDENTIFIER\\), pos=[0-9]+, len=5, sev=48, text=\"until\""
            "Node: type=54 \\(LEXER_IDENTIFIER\\), pos=[0-9]+, len=7, sev=48, text=\"forever\""
            "Node: type=54 \\(LEXER_IDENTIFIER\\), pos=[0-9]+, len=5, sev=48, text=\"leave\""
            "Node: type=54 \\(LEXER_IDENTIFIER\\), pos=[0-9]+, len=7, sev=48, text=\"iterate\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=3, sev=48, text=\"say\""
            MUST_NOT_MATCH
            "RXC-LC-"
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_do_keyword_variables PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_do_control
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_do_control.rexx
            MUST_MATCH
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=2, sev=48, text=\"do\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=2, sev=48, text=\"to\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=2, sev=48, text=\"by\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=3, sev=48, text=\"for\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=3, sev=48, text=\"end\""
            MUST_NOT_MATCH
            "RXC-LC-"
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_do_control PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_do_conditions_leave
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_do_conditions_leave.rexx
            MUST_MATCH
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=5, sev=48, text=\"while\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=5, sev=48, text=\"until\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=7, sev=48, text=\"forever\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=5, sev=48, text=\"leave\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=7, sev=48, text=\"iterate\""
            MUST_NOT_MATCH
            "RXC-LC-"
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_do_conditions_leave PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_do_end_bad_symbol
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_do_end_bad_symbol.rexx
            MUST_MATCH
            "RXC-LC-10\\.2 linenumber=\"1\" token=\"j\""
            "Node: type=54 \\(LEXER_IDENTIFIER\\), pos=[0-9]+, len=1, sev=51, text=\"j\""
            MUST_NOT_MATCH
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_do_end_bad_symbol PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_do_end_uncontrolled_symbol
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_do_end_uncontrolled_symbol.rexx
            MUST_MATCH
            "RXC-LC-10\\.3 linenumber=\"1\" token=\"i\""
            "Node: type=54 \\(LEXER_IDENTIFIER\\), pos=[0-9]+, len=1, sev=51, text=\"i\""
            MUST_NOT_MATCH
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_do_end_uncontrolled_symbol PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_select_end_symbol
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_select_end_symbol.rexx
            MUST_MATCH
            "RXC-LC-10\\.4 linenumber=\"1\" token=\"i\""
            "Node: type=54 \\(LEXER_IDENTIFIER\\), pos=[0-9]+, len=1, sev=51, text=\"i\""
            MUST_NOT_MATCH
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_select_end_symbol PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_leave_outside_loop
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_leave_outside_loop.rexx
            MUST_MATCH
            "RXC-LC-28\\.1"
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=0, len=5, sev=51, text=\"leave\""
            MUST_NOT_MATCH
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_leave_outside_loop PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_iterate_bad_target
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_iterate_bad_target.rexx
            MUST_MATCH
            "RXC-LC-28\\.4 token=\"k\""
            "Node: type=54 \\(LEXER_IDENTIFIER\\), pos=[0-9]+, len=1, sev=51, text=\"k\""
            MUST_NOT_MATCH
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_iterate_bad_target PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_do_forever_bad_tail
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_do_forever_bad_tail.rexx
            MUST_MATCH
            "RXC-LC-25\\.16 keywords=\"WHILE UNTIL\" token=\"to\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=2, sev=51, text=\"to\""
            MUST_NOT_MATCH
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_do_forever_bad_tail PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_do_invalid_initial_to
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_do_invalid_initial_to.rexx
            MUST_MATCH
            "RXC-LC-27\\.1 token=\"to\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=2, sev=51, text=\"to\""
            MUST_NOT_MATCH
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_do_invalid_initial_to PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_select_keyword_variables
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_select_keyword_variables.rexx
            MUST_MATCH
            "Node: type=54 \\(LEXER_IDENTIFIER\\), pos=0, len=6, sev=48, text=\"select\""
            "Node: type=54 \\(LEXER_IDENTIFIER\\), pos=[0-9]+, len=4, sev=48, text=\"when\""
            "Node: type=54 \\(LEXER_IDENTIFIER\\), pos=[0-9]+, len=9, sev=48, text=\"otherwise\""
            "Node: type=54 \\(LEXER_IDENTIFIER\\), pos=[0-9]+, len=3, sev=48, text=\"end\""
            MUST_NOT_MATCH
            "RXC-LC-"
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_select_keyword_variables PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_select_missing_when
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_select_missing_when.rexx
            MUST_MATCH
            "RXC-LC-7\\.1 linenumber=\"1\" token=\"end\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=3, sev=51, text=\"end\""
            MUST_NOT_MATCH
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_select_missing_when PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_select_missing_end
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_select_missing_end.rexx
            MUST_MATCH
            "RXC-LC-14\\.2"
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=0, len=6, sev=51, text=\"select\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=4, sev=48, text=\"when\""
            MUST_NOT_MATCH
            "MISSING_END"
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_select_missing_end PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_loose_when
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_loose_when.rexx
            MUST_MATCH
            "RXC-LC-9\\.1"
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=0, len=4, sev=51, text=\"when\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=3, sev=48, text=\"say\""
            MUST_NOT_MATCH
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_loose_when PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_loose_otherwise
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_loose_otherwise.rexx
            MUST_MATCH
            "RXC-LC-9\\.2"
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=0, len=9, sev=51, text=\"otherwise\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=3, sev=48, text=\"say\""
            MUST_NOT_MATCH
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_loose_otherwise PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_loose_end
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_loose_end.rexx
            MUST_MATCH
            "RXC-LC-10\\.1"
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=0, len=3, sev=51, text=\"end\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=3, sev=48, text=\"say\""
            MUST_NOT_MATCH
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_loose_end PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_when_missing_then
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_when_missing_then.rexx
            MUST_MATCH
            "RXC-LC-18\\.2 linenumber=\"2\" token=\"end\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=4, sev=51, text=\"when\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=3, sev=48, text=\"end\""
            MUST_NOT_MATCH
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_when_missing_then PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_expression_precedence
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_expression_precedence.rexx
            MUST_MATCH
            "Node: type=44 \\(LEXER_OPERATOR_ARITHMETIC\\), pos=[0-9]+, len=2, sev=48, text=\"\\*\\*\""
            "Node: type=44 \\(LEXER_OPERATOR_ARITHMETIC\\), pos=[0-9]+, len=1, sev=48, text=\"%\""
            "Node: type=44 \\(LEXER_OPERATOR_ARITHMETIC\\), pos=[0-9]+, len=2, sev=48, text=\"//\""
            "Node: type=42 \\(LEXER_OPERATOR\\), pos=[0-9]+, len=2, sev=48, text=\"\\|\\|\""
            "Node: type=45 \\(LEXER_OPERATOR_LOGICAL\\), pos=[0-9]+, len=1, sev=48, text=\"&\""
            "Node: type=45 \\(LEXER_OPERATOR_LOGICAL\\), pos=[0-9]+, len=1, sev=48, text=\"\\|\""
            "Node: type=45 \\(LEXER_OPERATOR_LOGICAL\\), pos=[0-9]+, len=2, sev=48, text=\"&&\""
            MUST_NOT_MATCH
            "RXC-LC-"
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_expression_precedence PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_expression_comparisons
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_expression_comparisons.rexx
            MUST_MATCH
            "Node: type=43 \\(LEXER_OPERATOR_ASSIGN\\), pos=[0-9]+, len=1, sev=48, text=\"=\""
            "Node: type=45 \\(LEXER_OPERATOR_LOGICAL\\), pos=[0-9]+, len=2, sev=48, text=\"<>\""
            "Node: type=45 \\(LEXER_OPERATOR_LOGICAL\\), pos=[0-9]+, len=2, sev=48, text=\"><\""
            "Node: type=45 \\(LEXER_OPERATOR_LOGICAL\\), pos=[0-9]+, len=2, sev=48, text=\"==\""
            "Node: type=45 \\(LEXER_OPERATOR_LOGICAL\\), pos=[0-9]+, len=2, sev=48, text=\">>\""
            "Node: type=45 \\(LEXER_OPERATOR_LOGICAL\\), pos=[0-9]+, len=2, sev=48, text=\"<<\""
            "Node: type=45 \\(LEXER_OPERATOR_LOGICAL\\), pos=[0-9]+, len=3, sev=48, text=\">>=\""
            "Node: type=45 \\(LEXER_OPERATOR_LOGICAL\\), pos=[0-9]+, len=3, sev=48, text=\"<<=\""
            "Node: type=45 \\(LEXER_OPERATOR_LOGICAL\\), pos=[0-9]+, len=3, sev=48, text=\"\\\\==\""
            "Node: type=45 \\(LEXER_OPERATOR_LOGICAL\\), pos=[0-9]+, len=3, sev=48, text=\"\\\\>>\""
            "Node: type=45 \\(LEXER_OPERATOR_LOGICAL\\), pos=[0-9]+, len=3, sev=48, text=\"\\\\<<\""
            MUST_NOT_MATCH
            "RXC-LC-"
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_expression_comparisons PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_expression_bad_comma
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_expression_bad_comma.rexx
            MUST_MATCH
            "RXC-LC-37\\.1"
            "Node: type=46 \\(LEXER_SEPARATOR\\), pos=[0-9]+, len=1, sev=51, text=\",\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=3, sev=48, text=\"say\""
            "Node: type=39 \\(LEXER_STRING_LITERAL\\), pos=[0-9]+, len=7, sev=48, text=\"'after'\""
            MUST_NOT_MATCH
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_expression_bad_comma PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_expression_bad_rparen
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_expression_bad_rparen.rexx
            MUST_MATCH
            "RXC-LC-37\\.2"
            "Node: type=53 \\(LEXER_RH_EXPR\\), pos=[0-9]+, len=1, sev=51, text=\"\\)\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=3, sev=48, text=\"say\""
            "Node: type=39 \\(LEXER_STRING_LITERAL\\), pos=[0-9]+, len=7, sev=48, text=\"'after'\""
            MUST_NOT_MATCH
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_expression_bad_rparen PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_expression_bad_trailing_compare
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_expression_bad_trailing_compare.rexx
            MUST_MATCH
            "RXC-LC-35\\.1 token=\"end-of-clause\""
            "Node: type=43 \\(LEXER_OPERATOR_ASSIGN\\), pos=[0-9]+, len=1, sev=51, text=\"=\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=3, sev=48, text=\"say\""
            "Node: type=39 \\(LEXER_STRING_LITERAL\\), pos=[0-9]+, len=7, sev=48, text=\"'after'\""
            MUST_NOT_MATCH
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_expression_bad_trailing_compare PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_expression_bad_trailing_operator
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_expression_bad_trailing_operator.rexx
            MUST_MATCH
            "RXC-LC-35\\.1 token=\"end-of-clause\""
            "Node: type=44 \\(LEXER_OPERATOR_ARITHMETIC\\), pos=[0-9]+, len=1, sev=51, text=\"\\+\""
            "Node: type=44 \\(LEXER_OPERATOR_ARITHMETIC\\), pos=[0-9]+, len=1, sev=51, text=\"\\*\""
            "Node: type=44 \\(LEXER_OPERATOR_ARITHMETIC\\), pos=[0-9]+, len=2, sev=51, text=\"\\*\\*\""
            "Node: type=42 \\(LEXER_OPERATOR\\), pos=[0-9]+, len=2, sev=51, text=\"\\|\\|\""
            "Node: type=45 \\(LEXER_OPERATOR_LOGICAL\\), pos=[0-9]+, len=1, sev=51, text=\"&\""
            "Node: type=45 \\(LEXER_OPERATOR_LOGICAL\\), pos=[0-9]+, len=1, sev=51, text=\"\\|\""
            "Node: type=45 \\(LEXER_OPERATOR_LOGICAL\\), pos=[0-9]+, len=2, sev=51, text=\"&&\""
            "Node: type=39 \\(LEXER_STRING_LITERAL\\), pos=[0-9]+, len=11, sev=48, text=\"'after-xor'\""
            MUST_NOT_MATCH
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_expression_bad_trailing_operator PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_expression_bad_prefix_operator
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_expression_bad_prefix_operator.rexx
            MUST_MATCH
            "RXC-LC-35\\.1 token=\"end-of-clause\""
            "Node: type=44 \\(LEXER_OPERATOR_ARITHMETIC\\), pos=[0-9]+, len=1, sev=51, text=\"\\+\""
            "Node: type=44 \\(LEXER_OPERATOR_ARITHMETIC\\), pos=[0-9]+, len=1, sev=51, text=\"-\""
            "Node: type=45 \\(LEXER_OPERATOR_LOGICAL\\), pos=[0-9]+, len=1, sev=51, text=\"\\\\\""
            "Node: type=39 \\(LEXER_STRING_LITERAL\\), pos=[0-9]+, len=11, sev=48, text=\"'after-not'\""
            MUST_NOT_MATCH
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_expression_bad_prefix_operator PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_expression_bad_leading_operator
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_expression_bad_leading_operator.rexx
            MUST_MATCH
            "RXC-LC-35\\.1 token=\"\\*\""
            "RXC-LC-35\\.1 token=\"/\""
            "RXC-LC-35\\.1 token=\"&\""
            "RXC-LC-35\\.1 token=\"<>\""
            "Node: type=44 \\(LEXER_OPERATOR_ARITHMETIC\\), pos=[0-9]+, len=1, sev=51, text=\"\\*\""
            "Node: type=44 \\(LEXER_OPERATOR_ARITHMETIC\\), pos=[0-9]+, len=1, sev=51, text=\"/\""
            "Node: type=45 \\(LEXER_OPERATOR_LOGICAL\\), pos=[0-9]+, len=1, sev=51, text=\"&\""
            "Node: type=45 \\(LEXER_OPERATOR_LOGICAL\\), pos=[0-9]+, len=2, sev=51, text=\"<>\""
            "Node: type=39 \\(LEXER_STRING_LITERAL\\), pos=[0-9]+, len=11, sev=48, text=\"'after-neq'\""
            MUST_NOT_MATCH
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_expression_bad_leading_operator PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_expression_bad_paren
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_expression_bad_paren.rexx
            MUST_MATCH
            "RXC-LC-36"
            "RXC-LC-35\\.1 token=\"\\)\""
            "Node: type=52 \\(LEXER_LH_EXPR\\), pos=[0-9]+, len=1, sev=51, text=\"\\(\""
            "Node: type=44 \\(LEXER_OPERATOR_ARITHMETIC\\), pos=[0-9]+, len=1, sev=51, text=\"\\+\""
            "Node: type=39 \\(LEXER_STRING_LITERAL\\), pos=[0-9]+, len=11, sev=48, text=\"'after-rhs'\""
            MUST_NOT_MATCH
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_expression_bad_paren PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_block_comment
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/syntaxhighlight_block_comment.crexx
            MUST_MATCH
            "Node: type=38 \\(LEXER_COMMENT\\), pos=[0-9]+, len=[0-9]+, sev=48"
            MUST_NOT_MATCH
            "LEXER_OPERATOR_ARITHMETIC"
    )

    add_crexx_syntax_highlight_test(
            syntaxhighlight_comments_dash
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/syntaxhighlight_comments_dash.crexx
            MUST_MATCH
            "Node: type=38 \\(LEXER_COMMENT\\), pos=[0-9]+, len=[0-9]+, sev=48, text=\"-- dash line comment\""
            MUST_NOT_MATCH
            "Node: type=44 \\(LEXER_OPERATOR_ARITHMETIC\\), pos=[0-9]+, len=1, sev=48, text=\"-\""
    )
endif()

function(add_levelc_unsupported_compile_test NAME SOURCE_FILE)
    set(SCRIPT_FILE "${CMAKE_CURRENT_BINARY_DIR}/${NAME}.cmake")
    set(SCRIPT_CONTENT "")

    string(APPEND SCRIPT_CONTENT
           "execute_process(\n"
           "    COMMAND \"$<TARGET_FILE:rxc>\" \"${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE}\"\n"
           "    WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
           "    OUTPUT_VARIABLE out\n"
           "    ERROR_VARIABLE err\n"
           "    RESULT_VARIABLE res\n"
           ")\n"
           "set(all \"${RXAS_DIAG_DOLLAR}{out}${RXAS_DIAG_DOLLAR}{err}\")\n"
           "if(res EQUAL 0)\n"
           "  message(FATAL_ERROR \"Level C compile unexpectedly succeeded for ${NAME}.\\nSTDOUT:\\n${RXAS_DIAG_DOLLAR}{out}\\nSTDERR:\\n${RXAS_DIAG_DOLLAR}{err}\")\n"
           "endif()\n"
           "if(NOT all MATCHES \"REXX Level C \\\\(Classic REXX\\\\) compilation does not yet support this program shape:\")\n"
           "  message(FATAL_ERROR \"Missing Level C unsupported diagnostic for ${NAME}.\\nSTDOUT:\\n${RXAS_DIAG_DOLLAR}{out}\\nSTDERR:\\n${RXAS_DIAG_DOLLAR}{err}\")\n"
           "endif()\n")

    file(GENERATE OUTPUT "${SCRIPT_FILE}" CONTENT "${SCRIPT_CONTENT}")
    add_test(NAME ${NAME}
             COMMAND ${CMAKE_COMMAND} -P "${SCRIPT_FILE}"
             WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    set_tests_properties(${NAME} PROPERTIES LABELS "compiler;levelc")
endfunction()

add_levelc_unsupported_compile_test(levelc_compile_unsupported
        rexx_src/levelc_unsupported.rexx)

add_levelc_unsupported_compile_test(levelc_slice2_procedure_expose_unsupported
        rexx_src/levelc_slice2_procedure_expose_unsupported.rexx)

add_levelc_unsupported_compile_test(levelc_slice3_unsupported_bif
        rexx_src/levelc_slice3_unsupported_bif.rexx)

add_levelc_unsupported_compile_test(levelc_slice4_procedure_arg_count_unsupported
        rexx_src/levelc_slice4_procedure_arg_count_unsupported.rexx)

add_levelc_unsupported_compile_test(levelc_slice4_omitted_call_unsupported
        rexx_src/levelc_slice4_omitted_call_unsupported.rexx)

add_levelc_unsupported_compile_test(levelc_slice5_compound_expose_unsupported
        rexx_src/levelc_slice5_compound_expose_unsupported.rexx)

add_levelc_unsupported_compile_test(levelc_slice6_bare_stem_value_unsupported
        rexx_src/levelc_slice6_bare_stem_value_unsupported.rexx)

add_levelc_unsupported_compile_test(levelc_slice6_local_function_omitted_arg_unsupported
        rexx_src/levelc_slice6_local_function_omitted_arg_unsupported.rexx)

add_levelc_unsupported_compile_test(levelc_slice6_procedure_parse_unsupported
        rexx_src/levelc_slice6_procedure_parse_unsupported.rexx)

function(add_levelc_invalid_compile_test NAME SOURCE_FILE)
    set(SCRIPT_FILE "${CMAKE_CURRENT_BINARY_DIR}/${NAME}.cmake")
    set(SCRIPT_CONTENT "")

    string(APPEND SCRIPT_CONTENT
           "execute_process(\n"
           "    COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"${NAME}\" \"${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE}\"\n"
           "    WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
           "    OUTPUT_VARIABLE out\n"
           "    ERROR_VARIABLE err\n"
           "    RESULT_VARIABLE res\n"
           ")\n"
           "set(all \"${RXAS_DIAG_DOLLAR}{out}${RXAS_DIAG_DOLLAR}{err}\")\n"
           "if(res EQUAL 0)\n"
           "  message(FATAL_ERROR \"Invalid Level C compile unexpectedly succeeded for ${NAME}.\\nSTDOUT:\\n${RXAS_DIAG_DOLLAR}{out}\\nSTDERR:\\n${RXAS_DIAG_DOLLAR}{err}\")\n"
           "endif()\n"
           "if(all MATCHES \"REXX Level C \\\\(Classic REXX\\\\) compilation does not yet support this program shape\")\n"
           "  message(FATAL_ERROR \"Invalid Level C compile fell through to the unsupported lowering diagnostic for ${NAME}.\\nSTDOUT:\\n${RXAS_DIAG_DOLLAR}{out}\\nSTDERR:\\n${RXAS_DIAG_DOLLAR}{err}\")\n"
           "endif()\n")

    foreach(REQUIRED_PATTERN IN LISTS ARGN)
        string(APPEND SCRIPT_CONTENT
               "if(NOT all MATCHES \"${REQUIRED_PATTERN}\")\n"
               "  message(FATAL_ERROR \"Invalid Level C compile for ${NAME} is missing required pattern ${REQUIRED_PATTERN}.\\nSTDOUT:\\n${RXAS_DIAG_DOLLAR}{out}\\nSTDERR:\\n${RXAS_DIAG_DOLLAR}{err}\")\n"
               "endif()\n")
    endforeach()

    file(GENERATE OUTPUT "${SCRIPT_FILE}" CONTENT "${SCRIPT_CONTENT}")
    add_test(NAME ${NAME}
             COMMAND ${CMAKE_COMMAND} -P "${SCRIPT_FILE}"
             WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    set_tests_properties(${NAME} PROPERTIES LABELS "compiler;levelc")
endfunction()

add_levelc_invalid_compile_test(levelc_compile_invalid_if_missing_then
        rexx_src/levelc_if_missing_then.rexx
        "RXC-LC-18\\.1")

add_levelc_invalid_compile_test(levelc_compile_invalid_expression_trailing_operator
        rexx_src/levelc_expression_bad_trailing_operator.rexx
        "RXC-LC-35\\.1")

add_levelc_invalid_compile_test(levelc_compile_invalid_do_end_symbol
        rexx_src/levelc_do_end_bad_symbol.rexx
        "RXC-LC-10\\.2")

add_levelc_invalid_compile_test(levelc_compile_invalid_select_missing_when
        rexx_src/levelc_select_missing_when.rexx
        "RXC-LC-7\\.1")

function(add_levelc_runtime_slice_test NAME SOURCE_FILE EXPECTED_OUTPUT)
    set(SCRIPT_FILE "${CMAKE_CURRENT_BINARY_DIR}/${NAME}.cmake")
    set(SCRIPT_CONTENT "")

    string(APPEND SCRIPT_CONTENT
           "execute_process(\n"
           "    COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"${NAME}\" \"${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE}\"\n"
           "    WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
           "    OUTPUT_VARIABLE rxc_out\n"
           "    ERROR_VARIABLE rxc_err\n"
           "    RESULT_VARIABLE rxc_res\n"
           ")\n"
           "set(full_out \"${RXAS_DIAG_DOLLAR}{rxc_out}${RXAS_DIAG_DOLLAR}{rxc_err}\")\n"
           "if(NOT rxc_res EQUAL 0)\n"
           "  message(FATAL_ERROR \"rxc failed for ${NAME}.\\nOutput:\\n${RXAS_DIAG_DOLLAR}{full_out}\")\n"
           "endif()\n")
    string(APPEND SCRIPT_CONTENT
           "execute_process(\n"
           "    COMMAND \"$<TARGET_FILE:rxas>\" -o \"${NAME}.rxbin\" \"${NAME}\"\n"
           "    WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
           "    OUTPUT_VARIABLE rxas_out\n"
           "    ERROR_VARIABLE rxas_err\n"
           "    RESULT_VARIABLE rxas_res\n"
           ")\n"
           "set(full_out \"${RXAS_DIAG_DOLLAR}{full_out}${RXAS_DIAG_DOLLAR}{rxas_out}${RXAS_DIAG_DOLLAR}{rxas_err}\")\n"
           "if(NOT rxas_res EQUAL 0)\n"
           "  message(FATAL_ERROR \"rxas failed for ${NAME}.\\nOutput:\\n${RXAS_DIAG_DOLLAR}{full_out}\")\n"
           "endif()\n")
    string(APPEND SCRIPT_CONTENT
           "execute_process(\n"
           "    COMMAND \"$<TARGET_FILE:rxvm>\" \"${NAME}.rxbin\" \"${CMAKE_BINARY_DIR}/bin/library\" \"${CMAKE_BINARY_DIR}/bin/classlib\" \"${CMAKE_BINARY_DIR}/bin/rxfnsc\"\n"
           "    WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
           "    OUTPUT_VARIABLE rxvm_out\n"
           "    ERROR_VARIABLE rxvm_err\n"
           "    RESULT_VARIABLE rxvm_res\n"
           ")\n"
           "set(full_out \"${RXAS_DIAG_DOLLAR}{full_out}${RXAS_DIAG_DOLLAR}{rxvm_out}${RXAS_DIAG_DOLLAR}{rxvm_err}\")\n"
           "string(REPLACE \"\\r\\n\" \"\\n\" rxvm_out \"${RXAS_DIAG_DOLLAR}{rxvm_out}\")\n"
           "if(NOT rxvm_res EQUAL 0)\n"
           "  message(FATAL_ERROR \"rxvm failed for ${NAME}.\\nOutput:\\n${RXAS_DIAG_DOLLAR}{full_out}\")\n"
           "endif()\n"
           "if(NOT rxvm_out STREQUAL \"${EXPECTED_OUTPUT}\")\n"
           "  message(FATAL_ERROR \"Unexpected runtime output for ${NAME}.\\nExpected:\\n${EXPECTED_OUTPUT}\\nActual:\\n${RXAS_DIAG_DOLLAR}{rxvm_out}\\nFull output:\\n${RXAS_DIAG_DOLLAR}{full_out}\")\n"
           "endif()\n")

    file(GENERATE OUTPUT "${SCRIPT_FILE}" CONTENT "${SCRIPT_CONTENT}")
    add_test(NAME ${NAME}
             COMMAND ${CMAKE_COMMAND} -P "${SCRIPT_FILE}"
             WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    set_tests_properties(${NAME} PROPERTIES LABELS "compiler;levelc")
endfunction()

function(add_levelc_lowered_tree_shape_test NAME SOURCE_FILE)
    set(SCRIPT_FILE "${CMAKE_CURRENT_BINARY_DIR}/${NAME}.cmake")
    set(SCRIPT_CONTENT "")

    string(APPEND SCRIPT_CONTENT
           "execute_process(\n"
           "    COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -d1 -o \"${NAME}\" \"${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE}\"\n"
           "    WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
           "    OUTPUT_VARIABLE rxc_out\n"
           "    ERROR_VARIABLE rxc_err\n"
           "    RESULT_VARIABLE rxc_res\n"
           ")\n"
           "set(full_out \"${RXAS_DIAG_DOLLAR}{rxc_out}${RXAS_DIAG_DOLLAR}{rxc_err}\")\n"
           "if(NOT rxc_res EQUAL 0)\n"
           "  message(FATAL_ERROR \"rxc debug probe failed for ${NAME}.\\nOutput:\\n${RXAS_DIAG_DOLLAR}{full_out}\")\n"
           "endif()\n"
           "string(FIND \"${RXAS_DIAG_DOLLAR}{full_out}\" \"STAGE_LEVELC_LOWERED\" lowered_pos)\n"
           "if(lowered_pos EQUAL -1)\n"
           "  message(FATAL_ERROR \"Missing STAGE_LEVELC_LOWERED debug stage for ${NAME}.\\nOutput:\\n${RXAS_DIAG_DOLLAR}{full_out}\")\n"
           "endif()\n"
           "string(LENGTH \"STAGE_LEVELC_LOWERED\" marker_len)\n"
           "math(EXPR lowered_start \"${RXAS_DIAG_DOLLAR}{lowered_pos} + ${RXAS_DIAG_DOLLAR}{marker_len}\")\n"
           "string(SUBSTRING \"${RXAS_DIAG_DOLLAR}{full_out}\" ${RXAS_DIAG_DOLLAR}{lowered_start} -1 lowered)\n"
           "if(lowered MATCHES \"LEVELC_\")\n"
           "  message(FATAL_ERROR \"Lowered tree still contains Level C-only nodes for ${NAME}.\\nLowered debug output:\\n${RXAS_DIAG_DOLLAR}{lowered}\")\n"
           "endif()\n"
           "if(lowered MATCHES \"\\\\(INSTRUCTIONS\\\\)\")\n"
           "  message(FATAL_ERROR \"Lowered tree kept the builder-only INSTRUCTIONS wrapper for ${NAME}.\\nLowered debug output:\\n${RXAS_DIAG_DOLLAR}{lowered}\")\n"
           "endif()\n")

    foreach(REQUIRED_PATTERN IN LISTS ARGN)
        string(APPEND SCRIPT_CONTENT
               "if(NOT lowered MATCHES \"${REQUIRED_PATTERN}\")\n"
               "  message(FATAL_ERROR \"Lowered tree for ${NAME} is missing required pattern ${REQUIRED_PATTERN}.\\nLowered debug output:\\n${RXAS_DIAG_DOLLAR}{lowered}\")\n"
               "endif()\n")
    endforeach()

    file(GENERATE OUTPUT "${SCRIPT_FILE}" CONTENT "${SCRIPT_CONTENT}")
    add_test(NAME ${NAME}
             COMMAND ${CMAKE_COMMAND} -P "${SCRIPT_FILE}"
             WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    set_tests_properties(${NAME} PROPERTIES LABELS "compiler;levelc")
endfunction()

function(add_levelc_no_unused_generated_import_warning_test NAME SOURCE_FILE)
    set(SCRIPT_FILE "${CMAKE_CURRENT_BINARY_DIR}/${NAME}.cmake")
    set(SCRIPT_CONTENT "")

    string(APPEND SCRIPT_CONTENT
           "execute_process(\n"
           "    COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"${NAME}\" \"${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE}\"\n"
           "    WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
           "    OUTPUT_VARIABLE rxc_out\n"
           "    ERROR_VARIABLE rxc_err\n"
           "    RESULT_VARIABLE rxc_res\n"
           ")\n"
           "set(full_out \"${RXAS_DIAG_DOLLAR}{rxc_out}${RXAS_DIAG_DOLLAR}{rxc_err}\")\n"
           "if(NOT rxc_res EQUAL 0)\n"
           "  message(FATAL_ERROR \"rxc failed for ${NAME}.\\nOutput:\\n${RXAS_DIAG_DOLLAR}{full_out}\")\n"
           "endif()\n"
           "if(full_out MATCHES \"UNUSED_IMPORT\")\n"
           "  message(FATAL_ERROR \"Generated Level C imports produced an unused-import warning for ${NAME}.\\nOutput:\\n${RXAS_DIAG_DOLLAR}{full_out}\")\n"
           "endif()\n")

    file(GENERATE OUTPUT "${SCRIPT_FILE}" CONTENT "${SCRIPT_CONTENT}")
    add_test(NAME ${NAME}
             COMMAND ${CMAKE_COMMAND} -P "${SCRIPT_FILE}"
             WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    set_tests_properties(${NAME} PROPERTIES LABELS "compiler;levelc")
endfunction()

add_levelc_runtime_slice_test(levelc_slice1_target_shape
        rexx_src/levelc_slice1_target_shape.crexx
        "3\n")

add_levelc_runtime_slice_test(levelc_slice1_pool_say
        rexx_src/levelc_slice1_pool_say.rexx
        "3\n")

add_levelc_no_unused_generated_import_warning_test(levelc_generated_imports_no_unused_warning
        rexx_src/levelc_slice1_pool_say.rexx)

add_levelc_runtime_slice_test(levelc_slice2_procedure_expose_target_shape
        rexx_src/levelc_slice2_procedure_expose_target_shape.crexx
        "3\n")

add_levelc_runtime_slice_test(levelc_slice2_procedure_expose
        rexx_src/levelc_slice2_procedure_expose.rexx
        "3\n")

add_levelc_runtime_slice_test(levelc_slice3_bif_length_target_shape
        rexx_src/levelc_slice3_bif_length_target_shape.crexx
        "6\n")

add_levelc_runtime_slice_test(levelc_slice3_bif_length
        rexx_src/levelc_slice3_bif_length.rexx
        "6\n")

add_levelc_runtime_slice_test(levelc_slice4_bif_substr_target_shape
        rexx_src/levelc_slice4_bif_substr_target_shape.crexx
        "bcd\nef.\nbcdef\n")

add_levelc_runtime_slice_test(levelc_slice4_bif_substr
        rexx_src/levelc_slice4_bif_substr.rexx
        "bcd\nef.\nbcdef\n")

add_levelc_runtime_slice_test(levelc_slice4_procedure_args_return_target_shape
        rexx_src/levelc_slice4_procedure_args_return_target_shape.crexx
        "5\n")

add_levelc_runtime_slice_test(levelc_slice4_procedure_args_return
        rexx_src/levelc_slice4_procedure_args_return.rexx
        "5\n")

add_levelc_runtime_slice_test(levelc_slice4_call_args_expose
        rexx_src/levelc_slice4_call_args_expose.rexx
        "3\n")

add_levelc_runtime_slice_test(levelc_slice5_stem_expose_target_shape
        rexx_src/levelc_slice5_stem_expose_target_shape.crexx
        "after\n")

add_levelc_runtime_slice_test(levelc_slice5_stem_expose
        rexx_src/levelc_slice5_stem_expose.rexx
        "after\n")

add_levelc_runtime_slice_test(levelc_slice6_expressions_target_shape
        rexx_src/levelc_slice6_expressions_target_shape.crexx
        "11\n5\n24\n4\n2\n2\n8\n-5\n6\nab c\n1\n1\n0\n1\n0\n1\n1\n0\n0\n1\n0\n1\n1\n1\n1\n3\n")

add_levelc_runtime_slice_test(levelc_slice6_expressions
        rexx_src/levelc_slice6_expressions.rexx
        "11\n5\n24\n4\n2\n2\n8\n-5\n6\nab c\n1\n1\n0\n1\n0\n1\n1\n0\n0\n1\n0\n1\n1\n1\n1\n3\n")

add_levelc_lowered_tree_shape_test(levelc_slice2_procedure_expose_tree_shape
        rexx_src/levelc_slice2_procedure_expose.rexx
        "PROCEDURE"
        "ARGS"
        "OP_REFERENCE"
        "OP_DEREFERENCE"
        "RexxVariablePool"
        "exposeValue"
        "setValue"
        "value"
        "add"
        "asString")

add_levelc_lowered_tree_shape_test(levelc_slice3_bif_length_tree_shape
        rexx_src/levelc_slice3_bif_length.rexx
        "FUNCTION"
        "rexxclassicbif_length"
        "rexxclassicbifs"
        "RexxVariablePool"
        "RexxValue"
        "setValue"
        "value"
        "asString")

add_levelc_lowered_tree_shape_test(levelc_slice4_bif_substr_tree_shape
        rexx_src/levelc_slice4_bif_substr.rexx
        "RexxBifCallContext"
        "setArguments"
        "setCallerPool"
        "rexxclassicbif_call"
        "__rxcp_levelc_bif_args"
        "__rxcp_levelc_bif_exists"
        ".RexxValue"
        ".int")

add_levelc_lowered_tree_shape_test(levelc_slice4_procedure_args_return_tree_shape
        rexx_src/levelc_slice4_procedure_args_return.rexx
        "__rxcp_levelc_proc_DOUBLE"
        "__rxcp_levelc_arg_1"
        "__rxcp_levelc_arg_2"
        ".RexxValue"
        "setValue"
        "RETURN"
        "add")

add_levelc_lowered_tree_shape_test(levelc_slice5_stem_expose_tree_shape
        rexx_src/levelc_slice5_stem_expose.rexx
        "RexxVariablePool"
        "setStemValue"
        "stemValue"
        "exposeStem"
        "__rxcp_levelc_tail"
        "asString")

add_levelc_lowered_tree_shape_test(levelc_slice6_expressions_tree_shape
        rexx_src/levelc_slice6_expressions.rexx
        "subtract"
        "multiply"
        "divide"
        "integerDivide"
        "remainder"
        "power"
        "spaceConcat"
        "compareEqual"
        "strictCompareEqual"
        "logicalNot"
        "logicalXor"
        "logicalValue"
        "logicalResult"
        "IF"
        "RexxBifCallContext")

# Bridge Connectivity Test
add_custom_command(
        OUTPUT stub_plugin.rxbin
        COMMAND $<TARGET_FILE:rxc> -n -o stub_plugin ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/stub_plugin.crexx
        COMMAND $<TARGET_FILE:rxas> -o stub_plugin stub_plugin
        DEPENDS rxc rxas compiler_exit_bin ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/stub_plugin.crexx
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Compiling stub_plugin.crexx ..."
)

add_custom_target(stub_plugin_bin DEPENDS stub_plugin.rxbin)

add_executable(test_bridge src/test_bridge.c)
target_link_libraries(test_bridge PRIVATE rxvml rxpashim)
target_include_directories(test_bridge PRIVATE ${CMAKE_SOURCE_DIR}/interpreter ${CMAKE_SOURCE_DIR}/avl_tree ${CMAKE_SOURCE_DIR}/platform ${CMAKE_SOURCE_DIR}/utf8 ${CMAKE_SOURCE_DIR}/rxpa ${CMAKE_SOURCE_DIR}/binutils/include)
target_compile_definitions(test_bridge PRIVATE
        CREXX_TEST_RXCEXITS_MODULE="${CMAKE_BINARY_DIR}/bin/rxcexits.rxbin")
add_dependencies(test_bridge stub_plugin_bin)

add_executable(test_address_bridge src/test_address_bridge.c)
target_link_libraries(test_address_bridge PRIVATE rxvml rxpashim)
target_include_directories(test_address_bridge PRIVATE ${CMAKE_SOURCE_DIR}/interpreter ${CMAKE_SOURCE_DIR}/avl_tree ${CMAKE_SOURCE_DIR}/platform ${CMAKE_SOURCE_DIR}/utf8 ${CMAKE_SOURCE_DIR}/rxpa ${CMAKE_SOURCE_DIR}/binutils/include)
target_compile_definitions(test_address_bridge PRIVATE
        CREXX_TEST_LIBRARY_PATH="${CMAKE_BINARY_DIR}/bin/library"
        CREXX_TEST_CMS_ADDRESS_MODULE="${CMAKE_CURRENT_BINARY_DIR}/address_cms_provider.rxbin"
        CREXX_TEST_CMS_ADDRESS_HOST_MODULE="${CMAKE_CURRENT_BINARY_DIR}/address_cms_host.rxbin")

set(CREXX_CMS_DEMO_DIR "${CMAKE_SOURCE_DIR}/demos/cms")
set(CREXX_LLM_DEMO_DIR "${CMAKE_SOURCE_DIR}/demos/llm")
set(CREXX_NATIVE_KV_DEMO_DIR "${CMAKE_SOURCE_DIR}/demos/native/kv")
set(CREXX_NATIVE_SQLITE_DEMO_DIR "${CMAKE_SOURCE_DIR}/demos/native/sqlite")

add_custom_command(
        OUTPUT address_cms_provider.rxbin
        COMMAND $<TARGET_FILE:rxc> -i ${CMAKE_BINARY_DIR}/bin -o address_cms_provider ${CREXX_CMS_DEMO_DIR}/cms_address_environment.crexx
        COMMAND $<TARGET_FILE:rxas> address_cms_provider
        DEPENDS rxc rxas library compiler_exit_bin ${CREXX_CMS_DEMO_DIR}/cms_address_environment.crexx
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Compiling cms_address_environment.crexx ..."
)

add_custom_command(
        OUTPUT address_cms_host.rxbin
        COMMAND $<TARGET_FILE:rxc> -i ${CMAKE_BINARY_DIR}/bin -o address_cms_host ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/address_cms_host.crexx
        COMMAND $<TARGET_FILE:rxas> address_cms_host
        DEPENDS rxc rxas library compiler_exit_bin ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/address_cms_host.crexx
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Compiling address_cms_host.crexx ..."
)

add_custom_command(
        OUTPUT address_cms_demo.rxbin
        COMMAND $<TARGET_FILE:rxc> -i ${CMAKE_BINARY_DIR}/bin -o address_cms_demo ${CREXX_CMS_DEMO_DIR}/cms_address_demo.crexx
        COMMAND $<TARGET_FILE:rxas> address_cms_demo
        DEPENDS rxc rxas library compiler_exit_bin ${CREXX_CMS_DEMO_DIR}/cms_address_demo.crexx
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Compiling cms_address_demo.crexx ..."
)

add_custom_target(address_cms_fixture_bin DEPENDS address_cms_provider.rxbin address_cms_host.rxbin address_cms_demo.rxbin)
add_custom_target(address_cms_demo_bin DEPENDS address_cms_provider.rxbin address_cms_demo.rxbin)
add_dependencies(test_address_bridge address_cms_fixture_bin library)

add_custom_command(
        OUTPUT address_llm_provider.rxbin
        COMMAND $<TARGET_FILE:rxc> -i ${CMAKE_BINARY_DIR}/bin -o address_llm_provider ${CREXX_LLM_DEMO_DIR}/llm_address_environment.crexx
        COMMAND $<TARGET_FILE:rxas> address_llm_provider
        DEPENDS rxc rxas library rxfnsg compiler_exit_bin ${CREXX_LLM_DEMO_DIR}/llm_address_environment.crexx
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Compiling llm_address_environment.crexx ..."
)

add_custom_command(
        OUTPUT address_llm_host.rxbin
        COMMAND $<TARGET_FILE:rxc> -i ${CMAKE_BINARY_DIR}/bin -o address_llm_host ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/address_llm_host.crexx
        COMMAND $<TARGET_FILE:rxas> address_llm_host
        DEPENDS rxc rxas library rxfnsg compiler_exit_bin ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/address_llm_host.crexx
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Compiling address_llm_host.crexx ..."
)

add_custom_target(address_llm_fixture_bin ALL DEPENDS address_llm_provider.rxbin address_llm_host.rxbin)

add_custom_command(
        OUTPUT native_kv_address_demo.rxbin
        COMMAND $<TARGET_FILE:rxc> -i ${CMAKE_BINARY_DIR}/bin -o native_kv_address_demo ${CREXX_NATIVE_KV_DEMO_DIR}/kv_address_demo.crexx
        COMMAND $<TARGET_FILE:rxas> native_kv_address_demo
        DEPENDS rxc rxas library compiler_exit_bin ${CREXX_NATIVE_KV_DEMO_DIR}/kv_address_demo.crexx
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Compiling kv_address_demo.crexx ..."
)

add_custom_target(native_kv_address_demo_bin DEPENDS native_kv_address_demo.rxbin)

add_executable(native_kv_address_demo ${CREXX_NATIVE_KV_DEMO_DIR}/kv_address_demo.c)
target_link_libraries(native_kv_address_demo PRIVATE rxvml rxpashim)
target_include_directories(native_kv_address_demo PRIVATE ${CMAKE_SOURCE_DIR}/interpreter ${CMAKE_SOURCE_DIR}/avl_tree ${CMAKE_SOURCE_DIR}/platform ${CMAKE_SOURCE_DIR}/utf8 ${CMAKE_SOURCE_DIR}/rxpa ${CMAKE_SOURCE_DIR}/binutils/include)
target_compile_definitions(native_kv_address_demo PRIVATE
        CREXX_NATIVE_KV_LIBRARY_PATH="${CMAKE_BINARY_DIR}/bin/library"
        CREXX_NATIVE_KV_DEMO_MODULE="${CMAKE_CURRENT_BINARY_DIR}/native_kv_address_demo.rxbin")
add_dependencies(native_kv_address_demo native_kv_address_demo_bin library)

if(CREXX_BUILD_SQLITE_ADDRESS_DEMO)
    include(FetchContent)
    message(STATUS "Fetching SQLite amalgamation ${CREXX_SQLITE_AMALGAMATION_VERSION} for native ADDRESS demo")
    FetchContent_Declare(
            crexx_sqlite_amalgamation
            URL "https://www.sqlite.org/${CREXX_SQLITE_AMALGAMATION_YEAR}/sqlite-amalgamation-${CREXX_SQLITE_AMALGAMATION_VERSION}.zip"
            URL_HASH "SHA3_256=${CREXX_SQLITE_AMALGAMATION_SHA3_256}"
    )
    FetchContent_MakeAvailable(crexx_sqlite_amalgamation)
    FetchContent_GetProperties(crexx_sqlite_amalgamation)

    add_library(crexx_demo_sqlite3 STATIC
            "${crexx_sqlite_amalgamation_SOURCE_DIR}/sqlite3.c")
    target_include_directories(crexx_demo_sqlite3 PUBLIC
            "${crexx_sqlite_amalgamation_SOURCE_DIR}")
    target_compile_definitions(crexx_demo_sqlite3 PRIVATE
            SQLITE_THREADSAFE=0
            SQLITE_OMIT_LOAD_EXTENSION
            SQLITE_DQS=0)
    if (CMAKE_C_COMPILER_ID MATCHES "Clang|GNU")
        target_compile_options(crexx_demo_sqlite3 PRIVATE -w)
    elseif (CMAKE_C_COMPILER_ID STREQUAL "MSVC")
        target_compile_options(crexx_demo_sqlite3 PRIVATE /w)
    endif()

    add_custom_command(
            OUTPUT native_sqlite_address_demo.rxbin
            COMMAND $<TARGET_FILE:rxc> -i ${CMAKE_BINARY_DIR}/bin -o native_sqlite_address_demo ${CREXX_NATIVE_SQLITE_DEMO_DIR}/sqlite_address_demo.crexx
            COMMAND $<TARGET_FILE:rxas> native_sqlite_address_demo
            DEPENDS rxc rxas library compiler_exit_bin ${CREXX_NATIVE_SQLITE_DEMO_DIR}/sqlite_address_demo.crexx
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Compiling sqlite_address_demo.crexx ..."
    )

    add_custom_target(native_sqlite_address_demo_bin DEPENDS native_sqlite_address_demo.rxbin)

    add_executable(native_sqlite_address_demo ${CREXX_NATIVE_SQLITE_DEMO_DIR}/sqlite_address_demo.c)
    target_link_libraries(native_sqlite_address_demo PRIVATE rxvml rxpashim crexx_demo_sqlite3)
    target_include_directories(native_sqlite_address_demo PRIVATE ${CMAKE_SOURCE_DIR}/interpreter ${CMAKE_SOURCE_DIR}/avl_tree ${CMAKE_SOURCE_DIR}/platform ${CMAKE_SOURCE_DIR}/utf8 ${CMAKE_SOURCE_DIR}/rxpa ${CMAKE_SOURCE_DIR}/binutils/include)
    target_compile_definitions(native_sqlite_address_demo PRIVATE
            CREXX_NATIVE_SQLITE_LIBRARY_PATH="${CMAKE_BINARY_DIR}/bin/library"
            CREXX_NATIVE_SQLITE_DEMO_MODULE="${CMAKE_CURRENT_BINARY_DIR}/native_sqlite_address_demo.rxbin")
    add_dependencies(native_sqlite_address_demo native_sqlite_address_demo_bin library)
endif()

add_custom_command(
        OUTPUT address_callback_host.rxbin
        COMMAND $<TARGET_FILE:rxc> -i ${CMAKE_BINARY_DIR}/bin -o address_callback_host ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/address_callback_host.crexx
        COMMAND $<TARGET_FILE:rxas> address_callback_host
        DEPENDS rxc rxas library compiler_exit_bin ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/address_callback_host.crexx
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Compiling address_callback_host.crexx ..."
)

add_custom_target(address_callback_host_bin DEPENDS address_callback_host.rxbin)

add_executable(test_address_callback_host src/test_address_callback_host.c)
target_link_libraries(test_address_callback_host PRIVATE rxvml rxpashim)
target_include_directories(test_address_callback_host PRIVATE ${CMAKE_SOURCE_DIR}/interpreter ${CMAKE_SOURCE_DIR}/avl_tree ${CMAKE_SOURCE_DIR}/platform ${CMAKE_SOURCE_DIR}/utf8 ${CMAKE_SOURCE_DIR}/rxpa ${CMAKE_SOURCE_DIR}/binutils/include)
target_compile_definitions(test_address_callback_host PRIVATE
        CREXX_TEST_LIBRARY_PATH="${CMAKE_BINARY_DIR}/bin/library"
        CREXX_TEST_ADDRESS_CALLBACK_MODULE="${CMAKE_CURRENT_BINARY_DIR}/address_callback_host.rxbin")
add_dependencies(test_address_callback_host address_callback_host_bin library)

add_custom_command(
        OUTPUT dynamic_load_provider.rxbin
        COMMAND $<TARGET_FILE:rxc> -i ${CMAKE_BINARY_DIR}/bin -o dynamic_load_provider ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/dynamic_load_provider.crexx
        COMMAND $<TARGET_FILE:rxas> dynamic_load_provider
        DEPENDS rxc rxas library compiler_exit_bin ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/dynamic_load_provider.crexx
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Compiling dynamic_load_provider.crexx ..."
)

add_custom_command(
        OUTPUT dynamic_load_host.rxbin
        COMMAND $<TARGET_FILE:rxc> -i ${CMAKE_CURRENT_BINARY_DIR} -i ${CMAKE_BINARY_DIR}/bin -o dynamic_load_host ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/dynamic_load_host.crexx
        COMMAND $<TARGET_FILE:rxas> dynamic_load_host
        DEPENDS rxc rxas library compiler_exit_bin dynamic_load_provider.rxbin ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/dynamic_load_host.crexx
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Compiling dynamic_load_host.crexx ..."
)

add_custom_target(dynamic_load_fixture_bin DEPENDS dynamic_load_provider.rxbin dynamic_load_host.rxbin)

add_executable(test_dynamic_load src/test_dynamic_load.c)
target_link_libraries(test_dynamic_load PRIVATE rxvml rxpashim)
target_include_directories(test_dynamic_load PRIVATE ${CMAKE_SOURCE_DIR}/interpreter ${CMAKE_SOURCE_DIR}/avl_tree ${CMAKE_SOURCE_DIR}/platform ${CMAKE_SOURCE_DIR}/utf8 ${CMAKE_SOURCE_DIR}/rxpa ${CMAKE_SOURCE_DIR}/binutils/include)
target_compile_definitions(test_dynamic_load PRIVATE
        CREXX_TEST_LIBRARY_PATH="${CMAKE_BINARY_DIR}/bin/library"
        CREXX_TEST_DYNAMIC_LOAD_HOST_MODULE="${CMAKE_CURRENT_BINARY_DIR}/dynamic_load_host.rxbin"
        CREXX_TEST_DYNAMIC_LOAD_PROVIDER_MODULE="${CMAKE_CURRENT_BINARY_DIR}/dynamic_load_provider.rxbin")
add_dependencies(test_dynamic_load dynamic_load_fixture_bin library)

add_custom_command(
        OUTPUT dynamic_interface_contract.rxbin
        COMMAND $<TARGET_FILE:rxc> -i ${CMAKE_BINARY_DIR}/bin -o dynamic_interface_contract ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/dynamic_interface_contract.crexx
        COMMAND $<TARGET_FILE:rxas> dynamic_interface_contract
        DEPENDS rxc rxas library compiler_exit_bin ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/dynamic_interface_contract.crexx
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Compiling dynamic interface contract ..."
)

add_custom_command(
        OUTPUT dynamic_interface_provider.rxbin
        COMMAND $<TARGET_FILE:rxc> -i ${CMAKE_CURRENT_BINARY_DIR} -i ${CMAKE_BINARY_DIR}/bin -o dynamic_interface_provider ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/dynamic_interface_provider.crexx
        COMMAND $<TARGET_FILE:rxas> dynamic_interface_provider
        DEPENDS rxc rxas library compiler_exit_bin dynamic_interface_contract.rxbin ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/dynamic_interface_provider.crexx
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Compiling dynamic interface provider ..."
)

add_custom_command(
        OUTPUT dynamic_interface_host.rxbin
        COMMAND $<TARGET_FILE:rxc> -i ${CMAKE_CURRENT_BINARY_DIR} -i ${CMAKE_BINARY_DIR}/bin -o dynamic_interface_host ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/dynamic_interface_host.crexx
        COMMAND $<TARGET_FILE:rxas> dynamic_interface_host
        DEPENDS rxc rxas library compiler_exit_bin dynamic_interface_contract.rxbin ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/dynamic_interface_host.crexx
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Compiling dynamic interface host ..."
)

# These files are direct CTest inputs, so the normal build must create them.
add_custom_target(dynamic_interface_fixture_bin ALL DEPENDS
        dynamic_interface_contract.rxbin
        dynamic_interface_provider.rxbin
        dynamic_interface_host.rxbin)
add_dependencies(dynamic_interface_fixture_bin crexx)

set(DYNAMIC_INTERFACE_DRIVER_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/dynamic_interface_driver.cmake")
file(GENERATE OUTPUT "${DYNAMIC_INTERFACE_DRIVER_SCRIPT}" CONTENT
"file(REMOVE_RECURSE \"${CMAKE_CURRENT_BINARY_DIR}/dynamic_interface_driver\")
file(MAKE_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}/dynamic_interface_driver\")
file(COPY \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/dynamic_interface_host.crexx\" DESTINATION \"${CMAKE_CURRENT_BINARY_DIR}/dynamic_interface_driver\")
execute_process(
  COMMAND \"$<TARGET_FILE:crexx>\"
          \"${CMAKE_CURRENT_BINARY_DIR}/dynamic_interface_driver/dynamic_interface_host.crexx\"
          \"-i${CMAKE_CURRENT_BINARY_DIR}\"
          \"-l${CMAKE_CURRENT_BINARY_DIR}/dynamic_interface_contract.rxbin\"
          -args \"${CMAKE_CURRENT_BINARY_DIR}/dynamic_interface_provider.rxbin\"
  WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}/dynamic_interface_driver\"
  OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)
set(all_output \"\${out}\${err}\")
if(NOT res EQUAL 0)
  message(FATAL_ERROR \"Late-loaded factory driver failed (\${res}):\\n\${all_output}\")
endif()
if(NOT all_output MATCHES \"PASS: explicit late-loaded interface factory\")
  message(FATAL_ERROR \"Late-loaded factory driver omitted PASS marker:\\n\${all_output}\")
endif()
")

add_executable(test_rxvml_utf_boundaries src/test_rxvml_utf_boundaries.c)
target_link_libraries(test_rxvml_utf_boundaries PRIVATE rxvml rxpashim)
target_include_directories(test_rxvml_utf_boundaries PRIVATE ${CMAKE_SOURCE_DIR}/interpreter ${CMAKE_SOURCE_DIR}/avl_tree ${CMAKE_SOURCE_DIR}/platform ${CMAKE_SOURCE_DIR}/utf8 ${CMAKE_SOURCE_DIR}/rxpa ${CMAKE_SOURCE_DIR}/binutils/include)

add_custom_command(
        OUTPUT crexxsaa_variables_host.rxbin
        COMMAND $<TARGET_FILE:rxc> -i ${CMAKE_BINARY_DIR}/bin -o crexxsaa_variables_host ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/crexxsaa_variables_host.crexx
        COMMAND $<TARGET_FILE:rxas> crexxsaa_variables_host
        DEPENDS rxc rxas library compiler_exit_bin ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/crexxsaa_variables_host.crexx
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Compiling crexxsaa_variables_host.crexx ..."
)

add_custom_target(crexxsaa_variables_host_bin DEPENDS crexxsaa_variables_host.rxbin)

add_executable(test_crexxsaa_variables src/test_crexxsaa_variables.c)
target_link_libraries(test_crexxsaa_variables PRIVATE crexxsaa)
target_include_directories(test_crexxsaa_variables PRIVATE ${CMAKE_SOURCE_DIR}/interpreter)
target_compile_definitions(test_crexxsaa_variables PRIVATE
        CREXX_TEST_LOCATION="${CMAKE_BINARY_DIR}/bin"
        CREXX_TEST_LIBRARY_RXBIN="${CMAKE_BINARY_DIR}/bin/library.rxbin"
        CREXX_TEST_CREXXSAA_VARIABLE_MODULE="${CMAKE_CURRENT_BINARY_DIR}/crexxsaa_variables_host.rxbin")
add_dependencies(test_crexxsaa_variables crexxsaa_variables_host_bin library crexxsaa)

add_executable(test_crexxsaa_status src/test_crexxsaa_status.c)
target_link_libraries(test_crexxsaa_status PRIVATE crexxsaa)
target_include_directories(test_crexxsaa_status PRIVATE ${CMAKE_SOURCE_DIR}/interpreter)
target_compile_definitions(test_crexxsaa_status PRIVATE
        CREXX_TEST_LOCATION="${CMAKE_BINARY_DIR}/bin"
        CREXX_TEST_LIBRARY_RXBIN="${CMAKE_BINARY_DIR}/bin/library.rxbin"
        CREXX_TEST_RXC="$<TARGET_FILE:rxc>"
        CREXX_TEST_RXAS="$<TARGET_FILE:rxas>"
        CREXX_TEST_IMPORT_DIR="${CMAKE_BINARY_DIR}/bin"
        CREXX_TEST_CREXXSAA_EXIT7_SOURCE="${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/crexxsaa_exit7_hosted.crexx"
        CREXX_TEST_CREXXSAA_RETURN7_SOURCE="${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/crexxsaa_return7_hosted.crexx")
add_dependencies(test_crexxsaa_status library crexxsaa rxc rxas compiler_exit_bin)

add_executable(test_source_provenance src/test_source_provenance.c)
target_link_libraries(test_source_provenance PRIVATE rxclib)
target_include_directories(test_source_provenance PRIVATE
        ${CMAKE_SOURCE_DIR}/compiler
        ${CMAKE_BINARY_DIR}/compiler
        ${CMAKE_SOURCE_DIR}/assembler
        ${CMAKE_SOURCE_DIR}/avl_tree
        ${CMAKE_SOURCE_DIR}/platform
        ${CMAKE_SOURCE_DIR}/utf8
        ${CMAKE_SOURCE_DIR}/rxpa
        ${CMAKE_SOURCE_DIR}/interpreter
        ${CMAKE_SOURCE_DIR}/interpreter/rxvmplugin
        ${CMAKE_SOURCE_DIR}/binutils/include)

if (ENABLE_PARSER_MODE)
    add_executable(test_highlight_cache src/test_highlight_cache.c)
    target_link_libraries(test_highlight_cache PRIVATE rxclib DSLSH::editor)
    target_compile_definitions(test_highlight_cache PRIVATE ENABLE_PARSER_MODE)
    target_include_directories(test_highlight_cache PRIVATE
            ${CMAKE_SOURCE_DIR}/compiler
            ${CMAKE_BINARY_DIR}/compiler
            ${CMAKE_SOURCE_DIR}/assembler
            ${CMAKE_SOURCE_DIR}/avl_tree
            ${CMAKE_SOURCE_DIR}/platform
            ${CMAKE_SOURCE_DIR}/utf8
            ${CMAKE_SOURCE_DIR}/rxpa
            ${CMAKE_SOURCE_DIR}/interpreter
            ${CMAKE_SOURCE_DIR}/interpreter/rxvmplugin
            ${CMAKE_SOURCE_DIR}/binutils/include)

    add_executable(test_source_semantics src/test_source_semantics.c)
    target_link_libraries(test_source_semantics PRIVATE rxclib DSLSH::editor)
    target_compile_definitions(test_source_semantics PRIVATE ENABLE_PARSER_MODE)
    target_include_directories(test_source_semantics PRIVATE
            ${CMAKE_SOURCE_DIR}/compiler
            ${CMAKE_BINARY_DIR}/compiler
            ${CMAKE_SOURCE_DIR}/assembler
            ${CMAKE_SOURCE_DIR}/avl_tree
            ${CMAKE_SOURCE_DIR}/platform
            ${CMAKE_SOURCE_DIR}/utf8
            ${CMAKE_SOURCE_DIR}/rxpa
            ${CMAKE_SOURCE_DIR}/interpreter
            ${CMAKE_SOURCE_DIR}/interpreter/rxvmplugin
            ${CMAKE_SOURCE_DIR}/binutils/include)

    add_executable(test_highlight_editor_diagnostics src/test_highlight_editor_diagnostics.c)
    target_link_libraries(test_highlight_editor_diagnostics PRIVATE rxclib DSLSH::editor)
    target_compile_definitions(test_highlight_editor_diagnostics PRIVATE
            ENABLE_PARSER_MODE
            RXC_SYNTAXHIGHLIGHT_CMD="$<TARGET_FILE:rxc> --syntaxhighlight")
    target_include_directories(test_highlight_editor_diagnostics PRIVATE
            ${CMAKE_SOURCE_DIR}/compiler
            ${CMAKE_BINARY_DIR}/compiler
            ${CMAKE_SOURCE_DIR}/assembler
            ${CMAKE_SOURCE_DIR}/avl_tree
            ${CMAKE_SOURCE_DIR}/platform
            ${CMAKE_SOURCE_DIR}/utf8
            ${CMAKE_SOURCE_DIR}/rxpa
            ${CMAKE_SOURCE_DIR}/interpreter
            ${CMAKE_SOURCE_DIR}/interpreter/rxvmplugin
            ${CMAKE_SOURCE_DIR}/binutils/include)
    add_dependencies(test_highlight_editor_diagnostics rxc)
endif()

# Commented out until fix found for multiple calls to the VM
add_test(NAME BridgeConnectivity COMMAND test_bridge)
add_test(NAME rxvml_utf_boundaries COMMAND test_rxvml_utf_boundaries)
set_tests_properties(rxvml_utf_boundaries PROPERTIES LABELS "compiler;rxvml;utf")
add_test(NAME address_bridge COMMAND test_address_bridge)
set_tests_properties(address_bridge PROPERTIES
        LABELS "compiler;stage3"
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
add_test(NAME address_cms_provider COMMAND
        $<TARGET_FILE:rxvm>
        ${CMAKE_BINARY_DIR}/bin/library.rxbin
        ${CMAKE_CURRENT_BINARY_DIR}/address_cms_provider.rxbin
        ${CMAKE_CURRENT_BINARY_DIR}/address_cms_host.rxbin)
set_tests_properties(address_cms_provider PROPERTIES LABELS "compiler;stage3")
add_test(NAME address_cms_demo COMMAND
        $<TARGET_FILE:rxvm>
        ${CMAKE_BINARY_DIR}/bin/library.rxbin
        ${CMAKE_CURRENT_BINARY_DIR}/address_cms_provider.rxbin
        ${CMAKE_CURRENT_BINARY_DIR}/address_cms_demo.rxbin)
set_tests_properties(address_cms_demo PROPERTIES LABELS "compiler;stage3")
add_test(NAME address_llm_provider COMMAND
        $<TARGET_FILE:rxvm>
        ${CMAKE_BINARY_DIR}/bin/library.rxbin
        ${CMAKE_BINARY_DIR}/bin/rxfnsg.rxbin
        ${CMAKE_CURRENT_BINARY_DIR}/address_llm_provider.rxbin
        ${CMAKE_CURRENT_BINARY_DIR}/address_llm_host.rxbin)
set_tests_properties(address_llm_provider PROPERTIES LABELS "compiler;stage3")
set_tests_properties(address_llm_provider PROPERTIES
        PASS_REGULAR_EXPRESSION "PASS: address llm host"
        FAIL_REGULAR_EXPRESSION "address llm host failures")
add_test(NAME address_native_kv_demo COMMAND native_kv_address_demo)
set_tests_properties(address_native_kv_demo PROPERTIES LABELS "compiler;stage3")
if(CREXX_BUILD_SQLITE_ADDRESS_DEMO)
    add_test(NAME address_native_sqlite_demo COMMAND native_sqlite_address_demo)
    set_tests_properties(address_native_sqlite_demo PROPERTIES LABELS "compiler;stage3")
endif()
add_test(NAME address_callback_host COMMAND test_address_callback_host)
set_tests_properties(address_callback_host PROPERTIES LABELS "compiler;stage3")
add_test(NAME dynamic_load COMMAND test_dynamic_load)
set_tests_properties(dynamic_load PROPERTIES LABELS "compiler;stage3")
foreach(_dynamic_interface_runner IN ITEMS rxvm rxbvm)
    add_test(NAME dynamic_interface_load_${_dynamic_interface_runner}
            COMMAND $<TARGET_FILE:${_dynamic_interface_runner}>
                    ${CMAKE_BINARY_DIR}/bin/library.rxbin
                    ${CMAKE_CURRENT_BINARY_DIR}/dynamic_interface_contract.rxbin
                    ${CMAKE_CURRENT_BINARY_DIR}/dynamic_interface_host.rxbin
                    -a ${CMAKE_CURRENT_BINARY_DIR}/dynamic_interface_provider.rxbin)
    set_tests_properties(dynamic_interface_load_${_dynamic_interface_runner} PROPERTIES
            LABELS "compiler;stage3;vm;interfaces;late-load"
            PASS_REGULAR_EXPRESSION "PASS: explicit late-loaded interface factory"
            FAIL_REGULAR_EXPRESSION "FAIL:|PANIC:")
endforeach()
add_test(NAME dynamic_interface_load_driver
        COMMAND ${CMAKE_COMMAND} -P ${DYNAMIC_INTERFACE_DRIVER_SCRIPT})
set_tests_properties(dynamic_interface_load_driver PROPERTIES
        LABELS "compiler;stage3;driver;interfaces;late-load"
        ENVIRONMENT "CREXX_HOME=${CMAKE_BINARY_DIR}")
add_test(NAME crexxsaa_variables COMMAND test_crexxsaa_variables)
set_tests_properties(crexxsaa_variables PROPERTIES
        LABELS "compiler;stage3"
        ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:${CMAKE_BINARY_DIR}/bin")
add_test(NAME crexxsaa_status COMMAND test_crexxsaa_status)
set_tests_properties(crexxsaa_status PROPERTIES
        LABELS "compiler;stage3"
        ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:${CMAKE_BINARY_DIR}/bin")
add_test(NAME source_provenance COMMAND test_source_provenance)
set_tests_properties(source_provenance PROPERTIES LABELS "compiler;stage2")

add_test(NAME rxc_srcmap
        COMMAND ${CMAKE_COMMAND}
        -DRXC=$<TARGET_FILE:rxc>
        -DWORK=${CMAKE_CURRENT_BINARY_DIR}/rxc_srcmap
        -P ${CMAKE_CURRENT_SOURCE_DIR}/run_rxc_srcmap.cmake)
set_tests_properties(rxc_srcmap PROPERTIES LABELS "compiler;diagnostics;srcmap")

add_test(NAME trace_event_metadata
        COMMAND ${CMAKE_COMMAND}
        -DRXC=$<TARGET_FILE:rxc>
        -DWORK=${CMAKE_CURRENT_BINARY_DIR}/trace_event_metadata
        -P ${CMAKE_CURRENT_SOURCE_DIR}/run_trace_event_metadata.cmake)
set_tests_properties(trace_event_metadata PROPERTIES LABELS "compiler;stage4")

set(TRACE_STEM_SUGAR_RXVM_TIMEOUT 60)
set(_trace_stem_sugar_sanitizer_flags
        "${CMAKE_C_FLAGS} ${CMAKE_CXX_FLAGS} ${CMAKE_EXE_LINKER_FLAGS} ${CMAKE_SHARED_LINKER_FLAGS}")
if(CMAKE_BUILD_TYPE)
    string(TOUPPER "${CMAKE_BUILD_TYPE}" _trace_stem_sugar_build_type)
    string(APPEND _trace_stem_sugar_sanitizer_flags
            " ${CMAKE_C_FLAGS_${_trace_stem_sugar_build_type}}"
            " ${CMAKE_CXX_FLAGS_${_trace_stem_sugar_build_type}}"
            " ${CMAKE_EXE_LINKER_FLAGS_${_trace_stem_sugar_build_type}}"
            " ${CMAKE_SHARED_LINKER_FLAGS_${_trace_stem_sugar_build_type}}")
endif()
string(FIND "${_trace_stem_sugar_sanitizer_flags}" "-fsanitize=address" _trace_stem_sugar_asan_flag)
if(NOT _trace_stem_sugar_asan_flag EQUAL -1)
    set(TRACE_STEM_SUGAR_RXVM_TIMEOUT 60)
endif()

add_test(NAME trace_stem_sugar
        COMMAND ${CMAKE_COMMAND}
        -DRXC=$<TARGET_FILE:rxc>
        -DRXAS=$<TARGET_FILE:rxas>
        -DRXVM=$<TARGET_FILE:rxvm>
        -DBIN_DIR=${CMAKE_BINARY_DIR}/bin
        -DLIBRARY=${CMAKE_BINARY_DIR}/bin/library
        -DWORK=${CMAKE_CURRENT_BINARY_DIR}/trace_stem_sugar
        -DRXVM_TIMEOUT=${TRACE_STEM_SUGAR_RXVM_TIMEOUT}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/run_trace_stem_sugar.cmake)
set_tests_properties(trace_stem_sugar PROPERTIES LABELS "compiler;stage4")

if (ENABLE_PARSER_MODE)
    add_test(NAME highlight_cache COMMAND test_highlight_cache)
    set_tests_properties(highlight_cache PROPERTIES
            LABELS "compiler;syntax_highlighting;stage4"
            RUN_SERIAL TRUE)

    add_test(NAME source_semantics COMMAND test_source_semantics)
    set_tests_properties(source_semantics PROPERTIES LABELS "compiler;syntax_highlighting;stage5")

    add_test(NAME highlight_editor_diagnostics COMMAND test_highlight_editor_diagnostics)
    set_tests_properties(highlight_editor_diagnostics PROPERTIES LABELS "compiler;syntax_highlighting;stage5")
endif()

file(GLOB_RECURSE RXC_DIAGNOSTIC_CATALOG_SCAN_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_SOURCE_DIR}/compiler/*.c"
        "${CMAKE_SOURCE_DIR}/compiler/*.h"
        "${CMAKE_SOURCE_DIR}/compiler/*.l"
        "${CMAKE_SOURCE_DIR}/compiler/*.y"
        "${CMAKE_SOURCE_DIR}/compiler/exits/*.crexx")
list(SORT RXC_DIAGNOSTIC_CATALOG_SCAN_SOURCES)
set(RXC_DIAGNOSTIC_CATALOG_SOURCE_LIST "${CMAKE_CURRENT_BINARY_DIR}/rxc_diagnostic_catalog_sources.txt")
file(WRITE "${RXC_DIAGNOSTIC_CATALOG_SOURCE_LIST}" "")
foreach(_rxc_diagnostic_catalog_source IN LISTS RXC_DIAGNOSTIC_CATALOG_SCAN_SOURCES)
    file(APPEND "${RXC_DIAGNOSTIC_CATALOG_SOURCE_LIST}" "${_rxc_diagnostic_catalog_source}\n")
endforeach()

set(RXC_DIAGNOSTIC_CATALOG_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/rxc_diagnostic_catalogs.cmake")
set(RXC_DIAGNOSTIC_CATALOG_CONTENT "")
string(APPEND RXC_DIAGNOSTIC_CATALOG_CONTENT
       "set(tool_base \"${CMAKE_CURRENT_BINARY_DIR}/check_diagnostic_catalogs\")\n"
       "execute_process(\n"
       "  COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"\${tool_base}\" \"${CMAKE_SOURCE_DIR}/tools/check_diagnostic_catalogs.crexx\"\n"
       "  WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
       "  OUTPUT_VARIABLE out\n"
       "  ERROR_VARIABLE err\n"
       "  RESULT_VARIABLE res\n"
       ")\n"
       "if(NOT res EQUAL 0)\n"
       "  message(FATAL_ERROR \"diagnostic catalogue checker compile failed with \${res}:\\n\${out}\${err}\")\n"
       "endif()\n"
       "execute_process(\n"
       "  COMMAND \"$<TARGET_FILE:rxas>\" -o \"\${tool_base}.rxbin\" \"\${tool_base}\"\n"
       "  WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
       "  OUTPUT_VARIABLE out\n"
       "  ERROR_VARIABLE err\n"
       "  RESULT_VARIABLE res\n"
       ")\n"
       "if(NOT res EQUAL 0)\n"
       "  message(FATAL_ERROR \"diagnostic catalogue checker assembly failed with \${res}:\\n\${out}\${err}\")\n"
       "endif()\n"
       "execute_process(\n"
       "  COMMAND \"$<TARGET_FILE:rxvm>\" \"${CMAKE_BINARY_DIR}/bin/library.rxbin\" \"\${tool_base}.rxbin\" -a \"${CMAKE_SOURCE_DIR}/messages/diagnostics.en_GB.msg\" \"${RXC_DIAGNOSTIC_CATALOG_SOURCE_LIST}\" \"2\" \"${CMAKE_SOURCE_DIR}/messages/diagnostics.de_DE.msg\" \"${CMAKE_SOURCE_DIR}/messages/diagnostics.nl_NL.msg\" \"1\" \"${CMAKE_SOURCE_DIR}/messages/diagnostics.en_US.msg\"\n"
       "  WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
       "  OUTPUT_VARIABLE out\n"
       "  ERROR_VARIABLE err\n"
       "  RESULT_VARIABLE res\n"
       ")\n"
       "if(NOT res EQUAL 0)\n"
       "  message(FATAL_ERROR \"diagnostic catalogue checker failed with \${res}:\\n\${out}\${err}\")\n"
       "endif()\n"
       "message(STATUS \"\${out}\")\n")
file(GENERATE OUTPUT "${RXC_DIAGNOSTIC_CATALOG_SCRIPT}" CONTENT "${RXC_DIAGNOSTIC_CATALOG_CONTENT}")
add_test(NAME rxc_diagnostic_catalogs
         COMMAND ${CMAKE_COMMAND} -P "${RXC_DIAGNOSTIC_CATALOG_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(rxc_diagnostic_catalogs PROPERTIES LABELS "compiler;diagnostics")

set(RXC_DIAGNOSTIC_LOCALIZATION_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/rxc_diagnostic_localization.cmake")
set(RXC_DIAGNOSTIC_LOCALIZATION_CONTENT "")
string(APPEND RXC_DIAGNOSTIC_LOCALIZATION_CONTENT
       "set(unused_src \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/unused_import_warning.crexx\")\n"
       "set(address_src \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/test_implicit_address.crexx\")\n"
       "set(missing_end_src \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/err_02_syntax.crexx\")\n"
       "set(binary_memory_src \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/binary_memory_invalid_storage.crexx\")\n"
       "set(legacy_arg_src \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/legacy_arg_call_removed.crexx\")\n"
       "set(top_level_constant_src \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/top_level_constant_removed.crexx\")\n"
       "set(literal_continuation_src \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/literal_continuation_suffix_error.crexx\")\n"
       "function(run_diag label locale source suffix expected unexpected)\n"
       "  execute_process(\n"
       "    COMMAND \"${CMAKE_COMMAND}\" -E env \"CREXX_DIAGNOSTICS=localized\" \"CREXX_DIAGNOSTIC_LOCALE=\${locale}\" \"$<TARGET_FILE:rxc>\" -n -o \"\${label}_\${suffix}_diag\" \"\${source}\"\n"
       "    WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
       "    OUTPUT_VARIABLE out\n"
       "    ERROR_VARIABLE err\n"
       "    RESULT_VARIABLE res\n"
       "    ENCODING UTF-8\n"
       "  )\n"
       "  set(full_out \"\${out}\${err}\")\n"
       "  if(NOT res EQUAL 0)\n"
       "    message(FATAL_ERROR \"\${label} localized diagnostic run failed with \${res}:\\n\${full_out}\")\n"
       "  endif()\n"
       "  string(FIND \"\${full_out}\" \"\${expected}\" expected_pos)\n"
       "  if(expected_pos EQUAL -1)\n"
       "    message(FATAL_ERROR \"\${label} localized diagnostic missing '\${expected}':\\n\${full_out}\")\n"
       "  endif()\n"
       "  if(NOT \"\${unexpected}\" STREQUAL \"\")\n"
       "    string(FIND \"\${full_out}\" \"\${unexpected}\" unexpected_pos)\n"
       "    if(NOT unexpected_pos EQUAL -1)\n"
       "      message(FATAL_ERROR \"\${label} localized diagnostic contained unexpected '\${unexpected}':\\n\${full_out}\")\n"
       "    endif()\n"
       "  endif()\n"
       "  string(REPLACE \"\\n\" \" | \" sample \"\${full_out}\")\n"
       "  message(STATUS \"\${label}/\${suffix}: \${sample}\")\n"
       "endfunction()\n"
       "function(run_diag_fail label locale source suffix expected unexpected)\n"
       "  execute_process(\n"
       "    COMMAND \"${CMAKE_COMMAND}\" -E env \"CREXX_DIAGNOSTICS=localized\" \"CREXX_DIAGNOSTIC_LOCALE=\${locale}\" \"$<TARGET_FILE:rxc>\" -n -o \"\${label}_\${suffix}_diag\" \"\${source}\"\n"
       "    WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
       "    OUTPUT_VARIABLE out\n"
       "    ERROR_VARIABLE err\n"
       "    RESULT_VARIABLE res\n"
       "    ENCODING UTF-8\n"
       "  )\n"
       "  set(full_out \"\${out}\${err}\")\n"
       "  if(res EQUAL 0)\n"
       "    message(FATAL_ERROR \"\${label} localized diagnostic unexpectedly succeeded:\\n\${full_out}\")\n"
       "  endif()\n"
       "  string(FIND \"\${full_out}\" \"\${expected}\" expected_pos)\n"
       "  if(expected_pos EQUAL -1)\n"
       "    message(FATAL_ERROR \"\${label} localized diagnostic missing '\${expected}':\\n\${full_out}\")\n"
       "  endif()\n"
       "  if(NOT \"\${unexpected}\" STREQUAL \"\")\n"
       "    string(FIND \"\${full_out}\" \"\${unexpected}\" unexpected_pos)\n"
       "    if(NOT unexpected_pos EQUAL -1)\n"
       "      message(FATAL_ERROR \"\${label} localized diagnostic contained unexpected '\${unexpected}':\\n\${full_out}\")\n"
       "    endif()\n"
       "  endif()\n"
       "  string(REPLACE \"\\n\" \" | \" sample \"\${full_out}\")\n"
       "  message(STATUS \"\${label}/\${suffix}: \${sample}\")\n"
       "endfunction()\n"
       "run_diag(\"en_GB\" \"en_GB\" \"\${unused_src}\" \"unused\" \"UNUSED_IMPORT: Import is recognised but not used.\" \"recognized\")\n"
       "run_diag(\"en_US\" \"en_US\" \"\${unused_src}\" \"unused\" \"UNUSED_IMPORT: Import is recognized but not used.\" \"recognised\")\n"
       "run_diag(\"de_DE\" \"de_DE\" \"\${unused_src}\" \"unused\" \"UNUSED_IMPORT: Import wird erkannt, aber nicht verwendet.\" \"recognised\")\n"
       "run_diag(\"nl_NL\" \"nl_NL\" \"\${unused_src}\" \"unused\" \"UNUSED_IMPORT: Import wordt herkend, maar niet gebruikt.\" \"recognised\")\n"
       "run_diag(\"en_GB\" \"en_GB\" \"\${address_src}\" \"address\" \"IMPLICIT_ADDRESS: Implicit ADDRESS command\" \"\")\n"
       "run_diag(\"en_US\" \"en_US\" \"\${address_src}\" \"address\" \"IMPLICIT_ADDRESS: Implicit ADDRESS command\" \"\")\n"
       "run_diag(\"de_DE\" \"de_DE\" \"\${address_src}\" \"address\" \"IMPLICIT_ADDRESS: Impliziter ADDRESS-Befehl\" \"\")\n"
       "run_diag(\"nl_NL\" \"nl_NL\" \"\${address_src}\" \"address\" \"IMPLICIT_ADDRESS: Impliciete ADDRESS-opdracht\" \"\")\n"
       "run_diag_fail(\"en_GB\" \"en_GB\" \"\${missing_end_src}\" \"missing_end\" \"MISSING_END: Missing END\" \"#MISSING_END, \\\"do\\\"\")\n"
       "run_diag_fail(\"en_US\" \"en_US\" \"\${missing_end_src}\" \"missing_end\" \"MISSING_END: Missing END\" \"#MISSING_END, \\\"do\\\"\")\n"
       "run_diag_fail(\"de_DE\" \"de_DE\" \"\${missing_end_src}\" \"missing_end\" \"MISSING_END: Fehlendes END\" \"#MISSING_END, \\\"do\\\"\")\n"
       "run_diag_fail(\"nl_NL\" \"nl_NL\" \"\${missing_end_src}\" \"missing_end\" \"MISSING_END: Ontbrekende END\" \"#MISSING_END, \\\"do\\\"\")\n"
       "run_diag_fail(\"en_GB\" \"en_GB\" \"\${binary_memory_src}\" \"binary_memory\" \"BINARY_MEMORY_INVALID_STORAGE_TYPE: Binary memory access has an unsupported storage type.\" \"#BINARY_MEMORY_INVALID_STORAGE_TYPE, \\\"<at..u64>(0) data\\\"\")\n"
       "run_diag_fail(\"de_DE\" \"de_DE\" \"\${binary_memory_src}\" \"binary_memory\" \"BINARY_MEMORY_INVALID_STORAGE_TYPE: Binärer Speicherzugriff verwendet einen nicht unterstützten Speichertyp.\" \"#BINARY_MEMORY_INVALID_STORAGE_TYPE, \\\"<at..u64>(0) data\\\"\")\n"
       "run_diag_fail(\"nl_NL\" \"nl_NL\" \"\${binary_memory_src}\" \"binary_memory\" \"BINARY_MEMORY_INVALID_STORAGE_TYPE: Binaire-geheugentoegang gebruikt een niet-ondersteund opslagtype.\" \"#BINARY_MEMORY_INVALID_STORAGE_TYPE, \\\"<at..u64>(0) data\\\"\")\n"
       "run_diag_fail(\"en_GB\" \"en_GB\" \"\${legacy_arg_src}\" \"legacy_arg\" \"LEGACY_ARG_CALL_SYNTAX: ARG call syntax has been removed; use arg[], arg[n], or <argexists>(n).\" \"\")\n"
       "run_diag_fail(\"de_DE\" \"de_DE\" \"\${legacy_arg_src}\" \"legacy_arg\" \"LEGACY_ARG_CALL_SYNTAX: ARG-Aufrufsyntax wurde entfernt; verwenden Sie arg[], arg[n] oder <argexists>(n).\" \"\")\n"
       "run_diag_fail(\"nl_NL\" \"nl_NL\" \"\${legacy_arg_src}\" \"legacy_arg\" \"LEGACY_ARG_CALL_SYNTAX: ARG-aanroepsyntaxis is verwijderd; gebruik arg[], arg[n] of <argexists>(n).\" \"\")\n"
       "run_diag_fail(\"en_GB\" \"en_GB\" \"\${top_level_constant_src}\" \"top_level_constant\" \"CONSTANT_OUTSIDE_ROUTINE: Constant declarations must be inside an explicit routine body.\" \"\")\n"
       "run_diag_fail(\"de_DE\" \"de_DE\" \"\${top_level_constant_src}\" \"top_level_constant\" \"CONSTANT_OUTSIDE_ROUTINE: Konstantendeklarationen müssen sich in einem expliziten Routinenrumpf befinden.\" \"\")\n"
       "run_diag_fail(\"nl_NL\" \"nl_NL\" \"\${top_level_constant_src}\" \"top_level_constant\" \"CONSTANT_OUTSIDE_ROUTINE: Constantdeclaraties moeten binnen een expliciete routinebody staan.\" \"\")\n"
       "run_diag_fail(\"en_GB\" \"en_GB\" \"\${literal_continuation_src}\" \"literal_continuation\" \"LITERAL_CONTINUATION_SUFFIX: Only the final chunk of a continued string literal may use an x or b suffix.\" \"\")\n"
       "run_diag_fail(\"de_DE\" \"de_DE\" \"\${literal_continuation_src}\" \"literal_continuation\" \"LITERAL_CONTINUATION_SUFFIX: Nur das letzte Stück eines fortgesetzten Zeichenfolgenliterals darf ein x- oder b-Suffix verwenden.\" \"\")\n"
       "run_diag_fail(\"nl_NL\" \"nl_NL\" \"\${literal_continuation_src}\" \"literal_continuation\" \"LITERAL_CONTINUATION_SUFFIX: Alleen het laatste deel van een voortgezet tekenreeksliteral mag een x- of b-suffix gebruiken.\" \"\")\n"
       "run_diag(\"fallback\" \"fr_FR\" \"\${unused_src}\" \"unused\" \"UNUSED_IMPORT: Import is recognised but not used.\" \"recognized\")\n")
file(GENERATE OUTPUT "${RXC_DIAGNOSTIC_LOCALIZATION_SCRIPT}" CONTENT "${RXC_DIAGNOSTIC_LOCALIZATION_CONTENT}")
add_test(NAME rxc_diagnostic_localization
         COMMAND ${CMAKE_COMMAND} -P "${RXC_DIAGNOSTIC_LOCALIZATION_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(rxc_diagnostic_localization PROPERTIES LABELS "compiler;diagnostics")

set(RXC_DIAGNOSTIC_RAW_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/rxc_diagnostic_raw.cmake")
set(RXC_DIAGNOSTIC_RAW_CONTENT "")
string(APPEND RXC_DIAGNOSTIC_RAW_CONTENT
       "execute_process(\n"
       "  COMMAND \"${CMAKE_COMMAND}\" -E env \"CREXX_DIAGNOSTICS=raw\" \"$<TARGET_FILE:rxc>\" -o raw_diag \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/unused_import_warning.crexx\"\n"
       "  WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
       "  OUTPUT_VARIABLE out\n"
       "  ERROR_VARIABLE err\n"
       "  RESULT_VARIABLE res\n"
       ")\n"
       "set(full_out \"\${out}\${err}\")\n"
       "if(NOT res EQUAL 0)\n"
       "  message(FATAL_ERROR \"raw diagnostic run failed with \${res}:\\n\${full_out}\")\n"
       "endif()\n"
       "string(FIND \"\${full_out}\" \"UNUSED_IMPORT\" code_pos)\n"
       "if(code_pos EQUAL -1)\n"
       "  message(FATAL_ERROR \"raw diagnostic missing code:\\n\${full_out}\")\n"
       "endif()\n"
       "string(FIND \"\${full_out}\" \"recognised\" text_pos)\n"
       "if(NOT text_pos EQUAL -1)\n"
       "  message(FATAL_ERROR \"raw diagnostic unexpectedly used localized text:\\n\${full_out}\")\n"
       "endif()\n")
file(GENERATE OUTPUT "${RXC_DIAGNOSTIC_RAW_SCRIPT}" CONTENT "${RXC_DIAGNOSTIC_RAW_CONTENT}")
add_test(NAME rxc_diagnostic_raw
         COMMAND ${CMAKE_COMMAND} -P "${RXC_DIAGNOSTIC_RAW_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(rxc_diagnostic_raw PROPERTIES LABELS "compiler;diagnostics")

crexx_label_smoke_tests(
        rxc_help_syntaxhighlight
        rxas_invalid_diagnostic
        rxc_function_lookup_diagnostic
        rxc_constants_levelb
        01_assign_noopt
        03_calc_opt
        07_loops_noopt
        08_select_opt
        10_procs_noopt
        15_imports_opt
        err_01_type_fail
        err_02_syntax_fail
        err_06_missing_method_fail
        unused_import_warn
        repro_opt_args_run_noopt
        basic_arrays_run_noopt
        13_stems_run_noopt
        optional_arg_presence_copy_run_noopt
        interface_runtime_dispatch_multi_run_noopt
        interface_source_import_run_noopt
        source_import_optional_factory_arg
        source_import_contract_stub_implements
        syntaxhighlight_trace_keywords
        levelc_slice1_pool_say
        levelc_slice6_expressions
        address_bridge
        address_callback_host
        dynamic_load
        crexxsaa_variables
        crexxsaa_status
        source_provenance
        rxc_srcmap
        trace_stem_sugar
        source_semantics
        rxc_diagnostic_catalogs
)

function(crexx_force_raw_diagnostics_for_directory_tests)
    get_property(_crexx_tests DIRECTORY PROPERTY TESTS)
    foreach(_crexx_test IN LISTS _crexx_tests)
        if(_crexx_test MATCHES "^rxc_diagnostic_(catalogs|localization|raw)$")
            continue()
        endif()
        get_test_property(${_crexx_test} ENVIRONMENT _crexx_env)
        if(NOT _crexx_env OR _crexx_env MATCHES "NOTFOUND$")
            set(_crexx_env)
        endif()
        list(APPEND _crexx_env "CREXX_DIAGNOSTICS=raw")
        set_tests_properties(${_crexx_test} PROPERTIES ENVIRONMENT "${_crexx_env}")
    endforeach()
endfunction()

crexx_force_raw_diagnostics_for_directory_tests()

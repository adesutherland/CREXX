cmake_minimum_required(VERSION 3.24)

project(test_rexxscript_runtime)

enable_testing()

include(${CMAKE_SOURCE_DIR}/cmake/CrexxTestModes.cmake)

set(REXXBINS
  testRexxScriptRuntime
  testRexxScriptCompat
)

set(LIBRPATH ${CMAKE_BINARY_DIR}/bin/library)
set(LIBRPATH_RXBIN ${LIBRPATH}.rxbin)
set(CLASSLIB_RXBIN ${CMAKE_BINARY_DIR}/bin/classlib.rxbin)
set(RXFNSC_RXBIN ${CMAKE_BINARY_DIR}/bin/rxfnsc.rxbin)
set(REXXSCRIPT_RXBIN ${CMAKE_BINARY_DIR}/bin/rexxscript.rxbin)
set(REXXSCRIPT_RUNTIMES
  ${LIBRPATH}
  ${CMAKE_BINARY_DIR}/bin/classlib
  ${CMAKE_BINARY_DIR}/bin/rxfnsc
  ${CMAKE_BINARY_DIR}/bin/rexxscript
)

add_custom_target(testrexxscript ALL)
add_dependencies(testrexxscript rexxscript_cli)

unset(_prev_target)
foreach(_binfile ${REXXBINS})
    get_filename_component(_basename ${_binfile} NAME_WE)

    set(_rexx_file "${CMAKE_CURRENT_SOURCE_DIR}/${_basename}.crexx")
    set(_rexx_dep "${_basename}.crexx")

    add_custom_target(${_basename})
    add_dependencies(testrexxscript ${_basename} rexxscript rxfnsc)
    crexx_add_rexx_opt_matrix(
            NAME ${_basename}
            SOURCE ${_rexx_file}
            TARGET_GROUP ${_basename}
            IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin
            RXC_FLAGS -x
            DEPENDS library classlib rxfnsc rexxscript ${LIBRPATH_RXBIN} ${CLASSLIB_RXBIN} ${RXFNSC_RXBIN} ${REXXSCRIPT_RXBIN} ${_rexx_dep} ${_prev_target}
            RUNTIME_ARGS ${REXXSCRIPT_RUNTIMES}
    )
    set(_prev_target ${_basename}_noopt_artifact ${_basename}_opt_artifact)
endforeach()

set_tests_properties(testRexxScriptRuntime_noopt testRexxScriptRuntime_opt
        PROPERTIES PASS_REGULAR_EXPRESSION "PASS: RexxScript runtime")
set_tests_properties(testRexxScriptCompat_noopt testRexxScriptCompat_opt
        PROPERTIES PASS_REGULAR_EXPRESSION "PASS: evaluate helpers")

set(REXXSCRIPT_CLI_SMOKE_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_rexxscript_cli_smoke.cmake")
set(REXXSCRIPT_CLI_SMOKE_CONTENT "set(work \"${CMAKE_CURRENT_BINARY_DIR}/rexxscript_cli_smoke_ws\")\n")
string(APPEND REXXSCRIPT_CLI_SMOKE_CONTENT "file(REMOVE_RECURSE \"\${work}\")\n")
string(APPEND REXXSCRIPT_CLI_SMOKE_CONTENT "file(MAKE_DIRECTORY \"\${work}\")\n")
string(APPEND REXXSCRIPT_CLI_SMOKE_CONTENT "file(WRITE \"\${work}/rule.rxs\" \"say 'cli'; x = length('abcd'); say x; signal done; say 'bad'; done:; say 'done'\\n\")\n")
string(APPEND REXXSCRIPT_CLI_SMOKE_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rexxscript_cli>\" \"\${work}/rule.rxs\" OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)\n")
string(APPEND REXXSCRIPT_CLI_SMOKE_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"RexxScript CLI smoke failed with rc \${res}.\\nstdout:\\n\${out}\\nstderr:\\n\${err}\")\nendif()\n")
string(APPEND REXXSCRIPT_CLI_SMOKE_CONTENT "if(NOT out STREQUAL \"cli\\n4\\ndone\\n\")\n  message(FATAL_ERROR \"Unexpected RexxScript CLI output. Got:\\n\${out}\")\nendif()\n")
string(APPEND REXXSCRIPT_CLI_SMOKE_CONTENT "file(WRITE \"\${work}/bad.rxs\" \"say 'before'; signal missing\\n\")\n")
string(APPEND REXXSCRIPT_CLI_SMOKE_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rexxscript_cli>\" \"\${work}/bad.rxs\" OUTPUT_VARIABLE bad_out ERROR_VARIABLE bad_err RESULT_VARIABLE bad_res)\n")
string(APPEND REXXSCRIPT_CLI_SMOKE_CONTENT "if(bad_res EQUAL 0)\n  message(FATAL_ERROR \"RexxScript CLI error smoke unexpectedly succeeded.\\nstdout:\\n\${bad_out}\\nstderr:\\n\${bad_err}\")\nendif()\n")
string(APPEND REXXSCRIPT_CLI_SMOKE_CONTENT "if(NOT bad_out MATCHES \"ERROR: undefined label\")\n  message(FATAL_ERROR \"Unexpected RexxScript CLI error output. Got:\\n\${bad_out}\")\nendif()\n")
file(GENERATE OUTPUT "${REXXSCRIPT_CLI_SMOKE_SCRIPT}" CONTENT "${REXXSCRIPT_CLI_SMOKE_CONTENT}")

add_test(NAME rexxscript_cli_smoke
         COMMAND ${CMAKE_COMMAND} -P "${REXXSCRIPT_CLI_SMOKE_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

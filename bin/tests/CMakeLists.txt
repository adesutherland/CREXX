configure_file(${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/headerless_simple.crx
               ${CMAKE_CURRENT_BINARY_DIR}/headerless_simple.crx
               COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/headerless_levelc.rexx
               ${CMAKE_CURRENT_BINARY_DIR}/headerless_levelc.rexx
               COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/explicit_header.crexx
               ${CMAKE_CURRENT_BINARY_DIR}/explicit_header.crexx
               COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/bin/crxc.rexx
               ${CMAKE_CURRENT_BINARY_DIR}/crxc.rexx
               COPYONLY)

set(CREXX_BIN_TEST_RESOURCE_LOCK "crexx_bin_workspace")

set(CREXX_HELP_SMOKE_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_crexx_help_smoke.cmake")
set(CREXX_HELP_SMOKE_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:crexx>\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND CREXX_HELP_SMOKE_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"crexx help invocation failed: \\n\${out}\")\nendif()\n")
string(APPEND CREXX_HELP_SMOKE_CONTENT "string(FIND \"\${out}\" \"Arguments are:\" pos)\n")
string(APPEND CREXX_HELP_SMOKE_CONTENT "if(pos EQUAL -1)\n  message(FATAL_ERROR \"Missing help banner in output: \\n\${out}\")\nendif()\n")
file(GENERATE OUTPUT "${CREXX_HELP_SMOKE_SCRIPT}" CONTENT "${CREXX_HELP_SMOKE_CONTENT}")

add_test(NAME crexx_help_smoke
         COMMAND ${CMAKE_COMMAND} -P "${CREXX_HELP_SMOKE_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

set(CREXX_HEADERLESS_SMOKE_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_crexx_headerless_smoke.cmake")
set(CREXX_HEADERLESS_SMOKE_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:crexx>\" \"${CMAKE_CURRENT_BINARY_DIR}/headerless_simple.crx\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND CREXX_HEADERLESS_SMOKE_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"crexx headerless script failed: \\n\${out}\")\nendif()\n")
string(APPEND CREXX_HEADERLESS_SMOKE_CONTENT "if(NOT out STREQUAL \"HELLO\\n\")\n  message(FATAL_ERROR \"Unexpected headerless output. Got: \\n\${out}\")\nendif()\n")
string(APPEND CREXX_HEADERLESS_SMOKE_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:crexx>\" \"-verbose2\" \"${CMAKE_CURRENT_BINARY_DIR}/headerless_simple.crx\" OUTPUT_VARIABLE verbose_out ERROR_VARIABLE verbose_err RESULT_VARIABLE verbose_res)\n")
string(APPEND CREXX_HEADERLESS_SMOKE_CONTENT "if(NOT verbose_res EQUAL 0)\n  message(FATAL_ERROR \"crexx verbose headerless .crx smoke failed.\\nstdout:\\n\${verbose_out}\\nstderr:\\n\${verbose_err}\")\nendif()\n")
string(APPEND CREXX_HEADERLESS_SMOKE_CONTENT "string(FIND \"\${verbose_out}\" \"--level levelb\" levelb_pos)\n")
string(APPEND CREXX_HEADERLESS_SMOKE_CONTENT "if(NOT levelb_pos EQUAL -1)\n  message(FATAL_ERROR \"crexx forced headerless .crx through Level B. Got: \\n\${verbose_out}\")\nendif()\n")
string(APPEND CREXX_HEADERLESS_SMOKE_CONTENT "string(FIND \"\${verbose_out}\" \"--import rxfnsb\" import_pos)\n")
string(APPEND CREXX_HEADERLESS_SMOKE_CONTENT "if(NOT import_pos EQUAL -1)\n  message(FATAL_ERROR \"crexx injected rxfnsb for headerless .crx. Got: \\n\${verbose_out}\")\nendif()\n")
file(GENERATE OUTPUT "${CREXX_HEADERLESS_SMOKE_SCRIPT}" CONTENT "${CREXX_HEADERLESS_SMOKE_CONTENT}")

add_test(NAME crexx_headerless_simple_smoke
         COMMAND ${CMAKE_COMMAND} -P "${CREXX_HEADERLESS_SMOKE_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

set(CREXX_HEADERLESS_REXX_LEVELC_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_crexx_headerless_rexx_levelc_smoke.cmake")
set(CREXX_HEADERLESS_REXX_LEVELC_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:crexx>\" \"${CMAKE_CURRENT_BINARY_DIR}/headerless_levelc.rexx\" OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)\n")
string(APPEND CREXX_HEADERLESS_REXX_LEVELC_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"crexx headerless .rexx Level C smoke failed.\\nstdout:\\n\${out}\\nstderr:\\n\${err}\")\nendif()\n")
string(APPEND CREXX_HEADERLESS_REXX_LEVELC_CONTENT "if(NOT out STREQUAL \"3\\n\")\n  message(FATAL_ERROR \"Unexpected headerless .rexx Level C output. Got: \\n\${out}\")\nendif()\n")
string(APPEND CREXX_HEADERLESS_REXX_LEVELC_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:crexx>\" \"-verbose2\" \"${CMAKE_CURRENT_BINARY_DIR}/headerless_levelc.rexx\" OUTPUT_VARIABLE verbose_out ERROR_VARIABLE verbose_err RESULT_VARIABLE verbose_res)\n")
string(APPEND CREXX_HEADERLESS_REXX_LEVELC_CONTENT "if(NOT verbose_res EQUAL 0)\n  message(FATAL_ERROR \"crexx verbose headerless .rexx Level C smoke failed.\\nstdout:\\n\${verbose_out}\\nstderr:\\n\${verbose_err}\")\nendif()\n")
string(APPEND CREXX_HEADERLESS_REXX_LEVELC_CONTENT "string(FIND \"\${verbose_out}\" \"--level levelb\" levelb_pos)\n")
string(APPEND CREXX_HEADERLESS_REXX_LEVELC_CONTENT "if(NOT levelb_pos EQUAL -1)\n  message(FATAL_ERROR \"crexx forced headerless .rexx through Level B. Got: \\n\${verbose_out}\")\nendif()\n")
string(APPEND CREXX_HEADERLESS_REXX_LEVELC_CONTENT "string(FIND \"\${verbose_out}\" \"--import rxfnsb\" import_pos)\n")
string(APPEND CREXX_HEADERLESS_REXX_LEVELC_CONTENT "if(NOT import_pos EQUAL -1)\n  message(FATAL_ERROR \"crexx injected rxfnsb for headerless .rexx. Got: \\n\${verbose_out}\")\nendif()\n")
file(GENERATE OUTPUT "${CREXX_HEADERLESS_REXX_LEVELC_SCRIPT}" CONTENT "${CREXX_HEADERLESS_REXX_LEVELC_CONTENT}")

add_test(NAME crexx_headerless_rexx_levelc_smoke
         COMMAND ${CMAKE_COMMAND} -P "${CREXX_HEADERLESS_REXX_LEVELC_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

set(CREXX_EXPLICIT_SMOKE_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_crexx_explicit_smoke.cmake")
set(CREXX_EXPLICIT_SMOKE_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:crexx>\" \"${CMAKE_CURRENT_BINARY_DIR}/explicit_header.crexx\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND CREXX_EXPLICIT_SMOKE_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"crexx explicit-header script failed: \\n\${out}\")\nendif()\n")
string(APPEND CREXX_EXPLICIT_SMOKE_CONTENT "if(NOT out STREQUAL \"EXPLICIT\\n\")\n  message(FATAL_ERROR \"Unexpected explicit-header output. Got: \\n\${out}\")\nendif()\n")
file(GENERATE OUTPUT "${CREXX_EXPLICIT_SMOKE_SCRIPT}" CONTENT "${CREXX_EXPLICIT_SMOKE_CONTENT}")

add_test(NAME crexx_explicit_header_smoke
         COMMAND ${CMAKE_COMMAND} -P "${CREXX_EXPLICIT_SMOKE_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

set(CREXX_REXXSCRIPT_SMOKE_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_crexx_rexxscript_smoke.cmake")
set(CREXX_REXXSCRIPT_SMOKE_CONTENT "set(work \"${CMAKE_CURRENT_BINARY_DIR}/rexxscript_smoke_ws\")\n")
string(APPEND CREXX_REXXSCRIPT_SMOKE_CONTENT "file(REMOVE_RECURSE \"\${work}\")\n")
string(APPEND CREXX_REXXSCRIPT_SMOKE_CONTENT "file(MAKE_DIRECTORY \"\${work}\")\n")
string(APPEND CREXX_REXXSCRIPT_SMOKE_CONTENT "file(WRITE \"\${work}/rexxscript_smoke.crexx\" \"options levelb\\nimport rexxscript\\nrule = \\\"say 'inside'; a = a + 2\\\"\\na = \\\"3\\\"\\nout = .string[]\\nraw = .string[]\\nREXXSCRIPT rule EXPOSE a OUTPUT out RESULT raw\\nsay raw[1] || \\\"-\\\" || a || \\\"-\\\" || out[1]\\n\")\n")
string(APPEND CREXX_REXXSCRIPT_SMOKE_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:crexx>\" \"rexxscript_smoke.crexx\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND CREXX_REXXSCRIPT_SMOKE_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"crexx RexxScript smoke failed: \\n\${out}\")\nendif()\n")
string(APPEND CREXX_REXXSCRIPT_SMOKE_CONTENT "if(NOT out STREQUAL \"OK-5-inside\\n\")\n  message(FATAL_ERROR \"Unexpected RexxScript smoke output. Got: \\n\${out}\")\nendif()\n")
file(GENERATE OUTPUT "${CREXX_REXXSCRIPT_SMOKE_SCRIPT}" CONTENT "${CREXX_REXXSCRIPT_SMOKE_CONTENT}")

add_test(NAME crexx_rexxscript_smoke
         COMMAND ${CMAKE_COMMAND} -P "${CREXX_REXXSCRIPT_SMOKE_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

set(CREXX_LEVELC_CLASSIC_RUNTIME_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_crexx_levelc_classic_runtime_smoke.cmake")
set(CREXX_LEVELC_CLASSIC_RUNTIME_CONTENT "set(work \"${CMAKE_CURRENT_BINARY_DIR}/levelc_classic_runtime_ws\")\n")
string(APPEND CREXX_LEVELC_CLASSIC_RUNTIME_CONTENT "file(REMOVE_RECURSE \"\${work}\")\n")
string(APPEND CREXX_LEVELC_CLASSIC_RUNTIME_CONTENT "file(MAKE_DIRECTORY \"\${work}\")\n")
string(APPEND CREXX_LEVELC_CLASSIC_RUNTIME_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E copy_if_different \"${CMAKE_SOURCE_DIR}/compiler/tests/rexx_src/levelc_slice4_bif_substr.rexx\" \"\${work}/classic_bif.rexx\" RESULT_VARIABLE res)\n")
string(APPEND CREXX_LEVELC_CLASSIC_RUNTIME_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"Failed to stage Level C Classic BIF source\")\nendif()\n")
string(APPEND CREXX_LEVELC_CLASSIC_RUNTIME_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:crexx>\" \"classic_bif.rexx\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND CREXX_LEVELC_CLASSIC_RUNTIME_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"crexx Level C Classic runtime smoke failed: \\n\${out}\")\nendif()\n")
string(APPEND CREXX_LEVELC_CLASSIC_RUNTIME_CONTENT "if(NOT out STREQUAL \"bcd\\nef.\\nbcdef\\n\")\n  message(FATAL_ERROR \"Unexpected Level C Classic runtime output. Got: \\n\${out}\")\nendif()\n")
string(APPEND CREXX_LEVELC_CLASSIC_RUNTIME_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:crexx>\" \"-verbose2\" \"classic_bif.rexx\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE verbose_out ERROR_VARIABLE verbose_out RESULT_VARIABLE verbose_res)\n")
string(APPEND CREXX_LEVELC_CLASSIC_RUNTIME_CONTENT "if(NOT verbose_res EQUAL 0)\n  message(FATAL_ERROR \"crexx verbose Level C Classic runtime smoke failed: \\n\${verbose_out}\")\nendif()\n")
string(APPEND CREXX_LEVELC_CLASSIC_RUNTIME_CONTENT "string(REPLACE [[\\\\]] \"/\" verbose_out_normalized \"\${verbose_out}\")\n")
foreach(_needle IN ITEMS "crexx executes:" "bin/rxfnsc" "bin/classlib")
    string(APPEND CREXX_LEVELC_CLASSIC_RUNTIME_CONTENT "string(FIND \"\${verbose_out_normalized}\" \"${_needle}\" pos)\n")
    string(APPEND CREXX_LEVELC_CLASSIC_RUNTIME_CONTENT "if(pos EQUAL -1)\n  message(FATAL_ERROR \"crexx verbose Level C Classic runtime output missing ${_needle}. Got: \\n\${verbose_out}\")\nendif()\n")
endforeach()
file(GENERATE OUTPUT "${CREXX_LEVELC_CLASSIC_RUNTIME_SCRIPT}" CONTENT "${CREXX_LEVELC_CLASSIC_RUNTIME_CONTENT}")

add_test(NAME crexx_levelc_classic_runtime_smoke
         COMMAND ${CMAKE_COMMAND} -P "${CREXX_LEVELC_CLASSIC_RUNTIME_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

set(CREXX_STATUS_DIAGNOSTICS_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_crexx_status_diagnostics.cmake")
set(CREXX_STATUS_DIAGNOSTICS_CONTENT "set(work \"${CMAKE_CURRENT_BINARY_DIR}/status_diagnostics_ws\")\n")
string(APPEND CREXX_STATUS_DIAGNOSTICS_CONTENT "file(REMOVE_RECURSE \"\${work}\")\n")
string(APPEND CREXX_STATUS_DIAGNOSTICS_CONTENT "file(MAKE_DIRECTORY \"\${work}\")\n")
string(APPEND CREXX_STATUS_DIAGNOSTICS_CONTENT "file(WRITE \"\${work}/exit7.crexx\" \"options levelb\\nexit 7\\n\")\n")
string(APPEND CREXX_STATUS_DIAGNOSTICS_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:crexx>\" \"exit7.crexx\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)\n")
string(APPEND CREXX_STATUS_DIAGNOSTICS_CONTENT "if(NOT res EQUAL 7)\n  message(FATAL_ERROR \"crexx exit 7 returned \${res}.\\nstdout:\\n\${out}\\nstderr:\\n\${err}\")\nendif()\n")
string(APPEND CREXX_STATUS_DIAGNOSTICS_CONTENT "file(WRITE \"\${work}/return9.crexx\" \"options levelb\\nmain: procedure = .int\\n  return 9\\n\")\n")
string(APPEND CREXX_STATUS_DIAGNOSTICS_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:crexx>\" \"return9.crexx\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)\n")
string(APPEND CREXX_STATUS_DIAGNOSTICS_CONTENT "if(NOT res EQUAL 9)\n  message(FATAL_ERROR \"crexx return 9 returned \${res}.\\nstdout:\\n\${out}\\nstderr:\\n\${err}\")\nendif()\n")
string(APPEND CREXX_STATUS_DIAGNOSTICS_CONTENT "file(WRITE \"\${work}/bad.crexx\" \"options levelb\\nthis is invalid\\n\")\n")
string(APPEND CREXX_STATUS_DIAGNOSTICS_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:crexx>\" \"bad.crexx\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)\n")
string(APPEND CREXX_STATUS_DIAGNOSTICS_CONTENT "if(res EQUAL 0)\n  message(FATAL_ERROR \"crexx bad syntax unexpectedly returned 0\")\nendif()\n")
string(APPEND CREXX_STATUS_DIAGNOSTICS_CONTENT "if(NOT out STREQUAL \"\")\n  message(FATAL_ERROR \"crexx bad syntax wrote diagnostics to stdout:\\n\${out}\")\nendif()\n")
string(APPEND CREXX_STATUS_DIAGNOSTICS_CONTENT "string(FIND \"\${err}\" \"rxc error: Error in bad.crexx\" pos)\n")
string(APPEND CREXX_STATUS_DIAGNOSTICS_CONTENT "if(pos EQUAL -1)\n  message(FATAL_ERROR \"crexx bad syntax missing stderr diagnostic. stderr:\\n\${err}\")\nendif()\n")
file(GENERATE OUTPUT "${CREXX_STATUS_DIAGNOSTICS_SCRIPT}" CONTENT "${CREXX_STATUS_DIAGNOSTICS_CONTENT}")

add_test(NAME crexx_status_diagnostics
         COMMAND ${CMAKE_COMMAND} -P "${CREXX_STATUS_DIAGNOSTICS_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

set(CREXX_SPACED_SOURCE_SMOKE_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_crexx_spaced_source_smoke.cmake")
set(CREXX_SPACED_SOURCE_SMOKE_CONTENT "set(work \"${CMAKE_CURRENT_BINARY_DIR}/spaced_source_ws\")\n")
string(APPEND CREXX_SPACED_SOURCE_SMOKE_CONTENT "file(REMOVE_RECURSE \"\${work}\")\n")
string(APPEND CREXX_SPACED_SOURCE_SMOKE_CONTENT "file(MAKE_DIRECTORY \"\${work}\")\n")
string(APPEND CREXX_SPACED_SOURCE_SMOKE_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E copy_if_different \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/headerless_simple.crx\" \"\${work}/hel lo.crx\" RESULT_VARIABLE res)\n")
string(APPEND CREXX_SPACED_SOURCE_SMOKE_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"Failed to stage spaced source\")\nendif()\n")
string(APPEND CREXX_SPACED_SOURCE_SMOKE_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:crexx>\" \"hel lo.crx\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND CREXX_SPACED_SOURCE_SMOKE_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"crexx spaced source failed with rc \${res}:\\n\${out}\")\nendif()\n")
string(APPEND CREXX_SPACED_SOURCE_SMOKE_CONTENT "if(NOT out STREQUAL \"HELLO\\n\")\n  message(FATAL_ERROR \"Unexpected spaced source output. Got: \\n\${out}\")\nendif()\n")
string(APPEND CREXX_SPACED_SOURCE_SMOKE_CONTENT "if(NOT EXISTS \"\${work}/hel lo.rxbin\")\n  message(FATAL_ERROR \"Expected spaced rxbin was not produced\")\nendif()\n")
string(APPEND CREXX_SPACED_SOURCE_SMOKE_CONTENT "set(work \"${CMAKE_CURRENT_BINARY_DIR}/spaced_source_nokeep_ws\")\n")
string(APPEND CREXX_SPACED_SOURCE_SMOKE_CONTENT "file(REMOVE_RECURSE \"\${work}\")\n")
string(APPEND CREXX_SPACED_SOURCE_SMOKE_CONTENT "file(MAKE_DIRECTORY \"\${work}\")\n")
string(APPEND CREXX_SPACED_SOURCE_SMOKE_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E copy_if_different \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/headerless_simple.crx\" \"\${work}/hel lo.crx\" RESULT_VARIABLE res)\n")
string(APPEND CREXX_SPACED_SOURCE_SMOKE_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"Failed to stage spaced source for -nokeep\")\nendif()\n")
string(APPEND CREXX_SPACED_SOURCE_SMOKE_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:crexx>\" \"-nokeep\" \"hel lo.crx\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND CREXX_SPACED_SOURCE_SMOKE_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"crexx spaced source -nokeep failed with rc \${res}:\\n\${out}\")\nendif()\n")
string(APPEND CREXX_SPACED_SOURCE_SMOKE_CONTENT "if(NOT out STREQUAL \"HELLO\\n\")\n  message(FATAL_ERROR \"Unexpected spaced source -nokeep output. Got: \\n\${out}\")\nendif()\n")
string(APPEND CREXX_SPACED_SOURCE_SMOKE_CONTENT "if(EXISTS \"\${work}/hel lo.rxas\" OR EXISTS \"\${work}/hel lo.rxbin\")\n  message(FATAL_ERROR \"Spaced -nokeep left compile intermediates behind\")\nendif()\n")
string(APPEND CREXX_SPACED_SOURCE_SMOKE_CONTENT "set(work \"${CMAKE_CURRENT_BINARY_DIR}/spaced_source_native_ws\")\n")
string(APPEND CREXX_SPACED_SOURCE_SMOKE_CONTENT "file(REMOVE_RECURSE \"\${work}\")\n")
string(APPEND CREXX_SPACED_SOURCE_SMOKE_CONTENT "file(MAKE_DIRECTORY \"\${work}\")\n")
string(APPEND CREXX_SPACED_SOURCE_SMOKE_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E copy_if_different \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/headerless_simple.crx\" \"\${work}/hel lo.crx\" RESULT_VARIABLE res)\n")
string(APPEND CREXX_SPACED_SOURCE_SMOKE_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"Failed to stage spaced source for native\")\nendif()\n")
string(APPEND CREXX_SPACED_SOURCE_SMOKE_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:crexx>\" \"-native\" \"hel lo.crx\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND CREXX_SPACED_SOURCE_SMOKE_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"crexx spaced source native failed with rc \${res}:\\n\${out}\")\nendif()\n")
string(APPEND CREXX_SPACED_SOURCE_SMOKE_CONTENT "if(NOT EXISTS \"\${work}/hel lo${CMAKE_EXECUTABLE_SUFFIX}\")\n  message(FATAL_ERROR \"Expected spaced native executable was not produced.\\ncrexx output:\\n\${out}\")\nendif()\n")
string(APPEND CREXX_SPACED_SOURCE_SMOKE_CONTENT "execute_process(COMMAND \"\${work}/hel lo${CMAKE_EXECUTABLE_SUFFIX}\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE native_out ERROR_VARIABLE native_err RESULT_VARIABLE native_res)\n")
string(APPEND CREXX_SPACED_SOURCE_SMOKE_CONTENT "if(NOT native_res EQUAL 0)\n  message(FATAL_ERROR \"Spaced native executable failed.\\nstdout:\\n\${native_out}\\nstderr:\\n\${native_err}\")\nendif()\n")
string(APPEND CREXX_SPACED_SOURCE_SMOKE_CONTENT "if(NOT native_out STREQUAL \"HELLO\\n\")\n  message(FATAL_ERROR \"Unexpected spaced native output. Got: \\n\${native_out}\")\nendif()\n")
file(GENERATE OUTPUT "${CREXX_SPACED_SOURCE_SMOKE_SCRIPT}" CONTENT "${CREXX_SPACED_SOURCE_SMOKE_CONTENT}")

add_test(NAME crexx_spaced_source_smoke
         COMMAND ${CMAKE_COMMAND} -P "${CREXX_SPACED_SOURCE_SMOKE_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

set(CREXX_CRXC_NOARGS_SMOKE_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_crexx_crxc_noargs_smoke.cmake")
set(CREXX_CRXC_NOARGS_SMOKE_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:crexx>\" \"${CMAKE_CURRENT_BINARY_DIR}/crxc.rexx\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND CREXX_CRXC_NOARGS_SMOKE_CONTENT "if(NOT res EQUAL 99)\n  message(FATAL_ERROR \"crexx crxc noargs expected rc 99, got \${res}. Output: \\n\${out}\")\nendif()\n")
string(APPEND CREXX_CRXC_NOARGS_SMOKE_CONTENT "if(NOT out STREQUAL \"usage: crxc.rexx crexx_home execName[.ext]\\n\")\n  message(FATAL_ERROR \"Unexpected crxc noargs output. Got: \\n\${out}\")\nendif()\n")
file(GENERATE OUTPUT "${CREXX_CRXC_NOARGS_SMOKE_SCRIPT}" CONTENT "${CREXX_CRXC_NOARGS_SMOKE_CONTENT}")

add_test(NAME crexx_crxc_noargs_smoke
         COMMAND ${CMAKE_COMMAND} -P "${CREXX_CRXC_NOARGS_SMOKE_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

set(CREXX_LICENSE_SMOKE_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_crexx_license_summary_smoke.cmake")
set(CREXX_LICENSE_SMOKE_CONTENT "set(expected_license \"cREXX License (MIT)\")\n")
string(APPEND CREXX_LICENSE_SMOKE_CONTENT "set(expected_copyright \"Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, René Jansen\")\n")
string(APPEND CREXX_LICENSE_SMOKE_CONTENT "set(expected_project \"See https://github.com/adesutherland/CREXX for project details\")\n")
string(APPEND CREXX_LICENSE_SMOKE_CONTENT "function(check_license label)\n")
string(APPEND CREXX_LICENSE_SMOKE_CONTENT "  execute_process(COMMAND \${ARGN} OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND CREXX_LICENSE_SMOKE_CONTENT "  if(NOT res EQUAL 0)\n    message(FATAL_ERROR \"\${label} failed with rc \${res}:\\n\${out}\")\n  endif()\n")
string(APPEND CREXX_LICENSE_SMOKE_CONTENT "  foreach(expected IN ITEMS \"\${expected_license}\" \"\${expected_copyright}\" \"\${expected_project}\")\n")
string(APPEND CREXX_LICENSE_SMOKE_CONTENT "    string(FIND \"\${out}\" \"\${expected}\" pos)\n")
string(APPEND CREXX_LICENSE_SMOKE_CONTENT "    if(pos EQUAL -1)\n      message(FATAL_ERROR \"\${label} did not print expected license summary line: \${expected}\\nOutput:\\n\${out}\")\n    endif()\n")
string(APPEND CREXX_LICENSE_SMOKE_CONTENT "  endforeach()\n")
string(APPEND CREXX_LICENSE_SMOKE_CONTENT "endfunction()\n")
string(APPEND CREXX_LICENSE_SMOKE_CONTENT "check_license(\"rxc -c\" \"$<TARGET_FILE:rxc>\" -c)\n")
string(APPEND CREXX_LICENSE_SMOKE_CONTENT "check_license(\"rxas -c\" \"$<TARGET_FILE:rxas>\" -c)\n")
string(APPEND CREXX_LICENSE_SMOKE_CONTENT "check_license(\"rxcpack -c\" \"$<TARGET_FILE:rxcpack>\" -c)\n")
string(APPEND CREXX_LICENSE_SMOKE_CONTENT "check_license(\"rxdas -c\" \"$<TARGET_FILE:rxdas>\" -c)\n")
string(APPEND CREXX_LICENSE_SMOKE_CONTENT "check_license(\"rxvm -c\" \"$<TARGET_FILE:rxvm>\" -c)\n")
string(APPEND CREXX_LICENSE_SMOKE_CONTENT "check_license(\"rxbvm -c\" \"$<TARGET_FILE:rxbvm>\" -c)\n")
string(APPEND CREXX_LICENSE_SMOKE_CONTENT "check_license(\"rxvme -c\" \"$<TARGET_FILE:rxvme>\" -c)\n")
string(APPEND CREXX_LICENSE_SMOKE_CONTENT "check_license(\"rxbvme -c\" \"$<TARGET_FILE:rxbvme>\" -c)\n")
string(APPEND CREXX_LICENSE_SMOKE_CONTENT "check_license(\"crexx -version\" \"$<TARGET_FILE:crexx>\" -version)\n")
file(GENERATE OUTPUT "${CREXX_LICENSE_SMOKE_SCRIPT}" CONTENT "${CREXX_LICENSE_SMOKE_CONTENT}")

add_test(NAME crexx_license_summary_smoke
         COMMAND ${CMAKE_COMMAND} -P "${CREXX_LICENSE_SMOKE_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
add_test(NAME crexx_tool_output_path_policy
         COMMAND ${CMAKE_COMMAND}
                 -DRXC=$<TARGET_FILE:rxc>
                 -DRXAS=$<TARGET_FILE:rxas>
                 -DRXLINK=$<TARGET_FILE:rxlink>
                 -DCREXX=$<TARGET_FILE:crexx>
                 -DBIN_DIR=${CMAKE_BINARY_DIR}/bin
                 -DWORK_ROOT=${CMAKE_CURRENT_BINARY_DIR}
                 -P ${CMAKE_CURRENT_SOURCE_DIR}/tool_output_path_policy.cmake
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(crexx_help_smoke PROPERTIES RESOURCE_LOCK ${CREXX_BIN_TEST_RESOURCE_LOCK})
set_tests_properties(crexx_headerless_simple_smoke PROPERTIES RESOURCE_LOCK ${CREXX_BIN_TEST_RESOURCE_LOCK})
set_tests_properties(crexx_headerless_rexx_levelc_smoke PROPERTIES RESOURCE_LOCK ${CREXX_BIN_TEST_RESOURCE_LOCK})
set_tests_properties(crexx_explicit_header_smoke PROPERTIES RESOURCE_LOCK ${CREXX_BIN_TEST_RESOURCE_LOCK})
set_tests_properties(crexx_rexxscript_smoke PROPERTIES RESOURCE_LOCK ${CREXX_BIN_TEST_RESOURCE_LOCK})
set_tests_properties(crexx_levelc_classic_runtime_smoke PROPERTIES RESOURCE_LOCK ${CREXX_BIN_TEST_RESOURCE_LOCK})
set_tests_properties(crexx_status_diagnostics PROPERTIES RESOURCE_LOCK ${CREXX_BIN_TEST_RESOURCE_LOCK})
set_tests_properties(crexx_spaced_source_smoke PROPERTIES RESOURCE_LOCK ${CREXX_BIN_TEST_RESOURCE_LOCK})
set_tests_properties(crexx_crxc_noargs_smoke PROPERTIES RESOURCE_LOCK ${CREXX_BIN_TEST_RESOURCE_LOCK})
set_tests_properties(crexx_license_summary_smoke PROPERTIES RESOURCE_LOCK ${CREXX_BIN_TEST_RESOURCE_LOCK})
set_tests_properties(crexx_tool_output_path_policy PROPERTIES RESOURCE_LOCK ${CREXX_BIN_TEST_RESOURCE_LOCK})

function(add_crexx_compile_only_import_test NAME MAIN_FILE DEP_FILE MODE)
    get_filename_component(MAIN_BASENAME "${MAIN_FILE}" NAME)
    get_filename_component(MAIN_MODULE "${MAIN_FILE}" NAME_WE)
    get_filename_component(DEP_BASENAME "${DEP_FILE}" NAME)

    set(SCRIPT_FILE "${CMAKE_CURRENT_BINARY_DIR}/run_${NAME}.cmake")
    set(SCRIPT_CONTENT "set(work \"${CMAKE_CURRENT_BINARY_DIR}/${NAME}_ws\")\n")
    string(APPEND SCRIPT_CONTENT "set(dep \"\${work}/dep\")\n")
    string(APPEND SCRIPT_CONTENT "file(REMOVE_RECURSE \"\${work}\")\n")
    string(APPEND SCRIPT_CONTENT "file(MAKE_DIRECTORY \"\${work}\")\n")
    string(APPEND SCRIPT_CONTENT "file(MAKE_DIRECTORY \"\${dep}\")\n")
    string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E copy_if_different \"${CMAKE_CURRENT_SOURCE_DIR}/${MAIN_FILE}\" \"\${work}/${MAIN_BASENAME}\" RESULT_VARIABLE res)\n")
    string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"Failed to stage main source\")\nendif()\n")
    string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E copy_if_different \"${CMAKE_CURRENT_SOURCE_DIR}/${DEP_FILE}\" \"\${dep}/${DEP_BASENAME}\" RESULT_VARIABLE res)\n")
    string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"Failed to stage dependency source\")\nendif()\n")

    if(MODE STREQUAL "source")
        string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:crexx>\" \"--source\" \"\${dep}\" \"-noexec\" \"${MAIN_BASENAME}\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
    elseif(MODE STREQUAL "rxas")
        string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:crexx>\" \"--import-rxas\" \"-i\${dep}\" \"-noexec\" \"${MAIN_BASENAME}\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
    else()
        message(FATAL_ERROR "Unknown mode for add_crexx_compile_only_import_test: ${MODE}")
    endif()

    string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"crexx compile-only import test failed: \\n\${out}\")\nendif()\n")
    string(APPEND SCRIPT_CONTENT "if(NOT EXISTS \"\${work}/${MAIN_MODULE}.rxbin\")\n  message(FATAL_ERROR \"Expected ${MAIN_MODULE}.rxbin was not produced\")\nendif()\n")

    file(GENERATE OUTPUT "${SCRIPT_FILE}" CONTENT "${SCRIPT_CONTENT}")

    add_test(NAME ${NAME}
             COMMAND ${CMAKE_COMMAND} -P "${SCRIPT_FILE}"
             WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    set_tests_properties(${NAME} PROPERTIES RESOURCE_LOCK ${CREXX_BIN_TEST_RESOURCE_LOCK})
endfunction()

function(add_crexx_nocompile_test NAME SOURCE_FILE)
    set(options NATIVE)
    set(oneValueArgs EXPECT_OUTPUT)
    cmake_parse_arguments(CREXXNC "${options}" "${oneValueArgs}" "" ${ARGN})

    get_filename_component(SOURCE_BASENAME "${SOURCE_FILE}" NAME)
    get_filename_component(SOURCE_MODULE "${SOURCE_FILE}" NAME_WE)
    set(SOURCE_EXECUTABLE "${SOURCE_MODULE}${CMAKE_EXECUTABLE_SUFFIX}")

    set(SCRIPT_FILE "${CMAKE_CURRENT_BINARY_DIR}/run_${NAME}.cmake")
    set(SCRIPT_CONTENT "set(work \"${CMAKE_CURRENT_BINARY_DIR}/${NAME}_ws\")\n")
    string(APPEND SCRIPT_CONTENT "file(REMOVE_RECURSE \"\${work}\")\n")
    string(APPEND SCRIPT_CONTENT "file(MAKE_DIRECTORY \"\${work}\")\n")
    string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E copy_if_different \"${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE}\" \"\${work}/${SOURCE_BASENAME}\" RESULT_VARIABLE res)\n")
    string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"Failed to stage source for ${NAME}\")\nendif()\n")
    string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:crexx>\" \"-noexec\" \"${SOURCE_BASENAME}\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
    string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"Failed to prepare rxbin for ${NAME}.\\n\${out}\")\nendif()\n")

    string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:crexx>\"")
    if(CREXXNC_NATIVE)
        string(APPEND SCRIPT_CONTENT " \"--verbose2\"")
    endif()
    string(APPEND SCRIPT_CONTENT " \"-nocompile\"")
    if(CREXXNC_NATIVE)
        string(APPEND SCRIPT_CONTENT " \"-native\"")
    endif()
    string(APPEND SCRIPT_CONTENT " \"${SOURCE_BASENAME}\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
    string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"crexx -nocompile test ${NAME} failed: \\n\${out}\")\nendif()\n")

    if(CREXXNC_NATIVE)
        string(APPEND SCRIPT_CONTENT "if(NOT EXISTS \"\${work}/${SOURCE_EXECUTABLE}\")\n  message(FATAL_ERROR \"Expected native executable ${SOURCE_EXECUTABLE} was not produced.\\ncrexx output:\\n\${out}\")\nendif()\n")
        if(CREXXNC_EXPECT_OUTPUT)
            string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"\${work}/${SOURCE_EXECUTABLE}\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE native_out ERROR_VARIABLE native_err RESULT_VARIABLE native_res)\n")
            string(APPEND SCRIPT_CONTENT "if(NOT native_res EQUAL 0)\n  message(FATAL_ERROR \"Native executable ${SOURCE_EXECUTABLE} failed.\\nstdout:\\n\${native_out}\\nstderr:\\n\${native_err}\")\nendif()\n")
            string(APPEND SCRIPT_CONTENT "if(NOT native_out STREQUAL [=[${CREXXNC_EXPECT_OUTPUT}]=])\n  message(FATAL_ERROR \"Unexpected native output for ${NAME}.\\nGot:\\n\${native_out}\")\nendif()\n")
        endif()
    else()
        if(CREXXNC_EXPECT_OUTPUT)
            string(APPEND SCRIPT_CONTENT "if(NOT out STREQUAL [=[${CREXXNC_EXPECT_OUTPUT}]=])\n  message(FATAL_ERROR \"Unexpected crexx output for ${NAME}.\\nGot:\\n\${out}\")\nendif()\n")
        endif()
    endif()

    file(GENERATE OUTPUT "${SCRIPT_FILE}" CONTENT "${SCRIPT_CONTENT}")

    add_test(NAME ${NAME}
             COMMAND ${CMAKE_COMMAND} -P "${SCRIPT_FILE}"
             WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    set_tests_properties(${NAME} PROPERTIES RESOURCE_LOCK ${CREXX_BIN_TEST_RESOURCE_LOCK})
endfunction()

function(add_crexx_native_test NAME SOURCE_FILE)
    set(options EXPECT_NO_LINKED_INTERMEDIATE)
    set(oneValueArgs EXPECT_OUTPUT MAP_FILE)
    set(multiValueArgs CREXX_ARGS EXPECT_DAS_CONTAINS EXPECT_DAS_ABSENT EXPECT_EXISTS EXPECT_ABSENT)
    cmake_parse_arguments(CREXXN "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    get_filename_component(SOURCE_BASENAME "${SOURCE_FILE}" NAME)
    get_filename_component(SOURCE_MODULE "${SOURCE_FILE}" NAME_WE)
    set(SOURCE_EXECUTABLE "${SOURCE_MODULE}${CMAKE_EXECUTABLE_SUFFIX}")

    set(SCRIPT_FILE "${CMAKE_CURRENT_BINARY_DIR}/run_${NAME}.cmake")
    set(SCRIPT_CONTENT "set(work \"${CMAKE_CURRENT_BINARY_DIR}/${NAME}_ws\")\n")
    string(APPEND SCRIPT_CONTENT "file(REMOVE_RECURSE \"\${work}\")\n")
    string(APPEND SCRIPT_CONTENT "file(MAKE_DIRECTORY \"\${work}\")\n")
    string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E copy_if_different \"${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE}\" \"\${work}/${SOURCE_BASENAME}\" RESULT_VARIABLE res)\n")
    string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"Failed to stage source for ${NAME}\")\nendif()\n")

    string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:crexx>\" \"--verbose2\"")
    foreach(_arg IN LISTS CREXXN_CREXX_ARGS)
        string(APPEND SCRIPT_CONTENT " \"${_arg}\"")
    endforeach()
    string(APPEND SCRIPT_CONTENT " \"-native\" \"${SOURCE_BASENAME}\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
    string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"crexx native test ${NAME} failed: \\n\${out}\")\nendif()\n")
    string(APPEND SCRIPT_CONTENT "if(NOT EXISTS \"\${work}/${SOURCE_EXECUTABLE}\")\n  message(FATAL_ERROR \"Expected native executable ${SOURCE_EXECUTABLE} was not produced.\\ncrexx output:\\n\${out}\")\nendif()\n")
    if(NOT CREXXN_EXPECT_NO_LINKED_INTERMEDIATE)
        string(APPEND SCRIPT_CONTENT "if(NOT EXISTS \"\${work}/${SOURCE_MODULE}_linked.rxbin\")\n  message(FATAL_ERROR \"Expected linked intermediate ${SOURCE_MODULE}_linked.rxbin was not produced\")\nendif()\n")
    endif()

    if(CREXXN_EXPECT_OUTPUT)
        string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"\${work}/${SOURCE_EXECUTABLE}\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE native_out ERROR_VARIABLE native_err RESULT_VARIABLE native_res)\n")
        string(APPEND SCRIPT_CONTENT "if(NOT native_res EQUAL 0)\n  message(FATAL_ERROR \"Native executable ${SOURCE_EXECUTABLE} failed.\\nstdout:\\n\${native_out}\\nstderr:\\n\${native_err}\")\nendif()\n")
        string(APPEND SCRIPT_CONTENT "if(NOT native_out STREQUAL [=[${CREXXN_EXPECT_OUTPUT}]=])\n  message(FATAL_ERROR \"Unexpected native output for ${NAME}.\\nGot:\\n\${native_out}\")\nendif()\n")
    endif()

    if(CREXXN_MAP_FILE)
        string(APPEND SCRIPT_CONTENT "if(NOT EXISTS \"\${work}/${CREXXN_MAP_FILE}\")\n  message(FATAL_ERROR \"Expected link map ${CREXXN_MAP_FILE} was not produced\")\nendif()\n")
    endif()

    foreach(_path IN LISTS CREXXN_EXPECT_EXISTS)
        string(APPEND SCRIPT_CONTENT "if(NOT EXISTS \"\${work}/${_path}\")\n  message(FATAL_ERROR \"Expected file ${_path} was not produced\")\nendif()\n")
    endforeach()

    foreach(_path IN LISTS CREXXN_EXPECT_ABSENT)
        string(APPEND SCRIPT_CONTENT "if(EXISTS \"\${work}/${_path}\")\n  message(FATAL_ERROR \"Unexpected file ${_path} remains after native build\")\nendif()\n")
    endforeach()

    if(CREXXN_EXPECT_DAS_CONTAINS OR CREXXN_EXPECT_DAS_ABSENT)
        string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxdas>\" \"${SOURCE_MODULE}_linked\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE das_out ERROR_VARIABLE das_err RESULT_VARIABLE das_res)\n")
        string(APPEND SCRIPT_CONTENT "if(NOT das_res EQUAL 0)\n  message(FATAL_ERROR \"rxdas failed for ${NAME}.\\nstdout:\\n\${das_out}\\nstderr:\\n\${das_err}\")\nendif()\n")
        foreach(_needle IN LISTS CREXXN_EXPECT_DAS_CONTAINS)
            string(APPEND SCRIPT_CONTENT "string(FIND \"\${das_out}\" [=[${_needle}]=] pos)\n")
            string(APPEND SCRIPT_CONTENT "if(pos EQUAL -1)\n  message(FATAL_ERROR \"Missing expected rxdas text for ${NAME}.\\nOutput:\\n\${das_out}\")\nendif()\n")
        endforeach()
        foreach(_needle IN LISTS CREXXN_EXPECT_DAS_ABSENT)
            string(APPEND SCRIPT_CONTENT "string(FIND \"\${das_out}\" [=[${_needle}]=] pos)\n")
            string(APPEND SCRIPT_CONTENT "if(NOT pos EQUAL -1)\n  message(FATAL_ERROR \"Unexpected rxdas text for ${NAME}.\\nOutput:\\n\${das_out}\")\nendif()\n")
        endforeach()
    endif()

    file(GENERATE OUTPUT "${SCRIPT_FILE}" CONTENT "${SCRIPT_CONTENT}")

    add_test(NAME ${NAME}
             COMMAND ${CMAKE_COMMAND} -P "${SCRIPT_FILE}"
             WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    set_tests_properties(${NAME} PROPERTIES RESOURCE_LOCK ${CREXX_BIN_TEST_RESOURCE_LOCK})
endfunction()

add_crexx_compile_only_import_test(crexx_source_root_compile_only rexx_src/source_import_main.crexx rexx_src/source_import_dep.crexx source)
add_crexx_compile_only_import_test(crexx_rxas_import_compile_only rexx_src/rxas_import_main.crexx rexx_src/wrapperasm.rxas rxas)
add_crexx_nocompile_test(
        crexx_nocompile_exec_smoke
        rexx_src/explicit_header.crexx
        EXPECT_OUTPUT "EXPLICIT\n"
)
add_crexx_nocompile_test(
        crexx_native_nocompile_smoke
        rexx_src/explicit_header.crexx
        NATIVE
        EXPECT_OUTPUT "EXPLICIT\n"
)
add_crexx_native_test(
        crexx_native_explicit_header_smoke
        rexx_src/explicit_header.crexx
        EXPECT_OUTPUT "EXPLICIT\n"
)
add_crexx_native_test(
        crexx_native_default_link_strip
        rexx_src/explicit_header.crexx
        EXPECT_DAS_ABSENT ".srcstep"
)
add_crexx_native_test(
        crexx_native_linkmap_keep_source
        rexx_src/explicit_header.crexx
        CREXX_ARGS "--linkmap" "explicit_header.map" "--link-keep-source"
        MAP_FILE "explicit_header.map"
        EXPECT_DAS_CONTAINS ".srcstep"
)
add_crexx_native_test(
        crexx_native_link_keep_inline
        rexx_src/native_inline_helper.crexx
        CREXX_ARGS "--link-keep-inline"
        EXPECT_OUTPUT "7\n"
        EXPECT_DAS_CONTAINS ".meta \"native_inline_helper.helper\"=\".inline\""
)
add_crexx_native_test(
        crexx_native_nokeep_cleanup
        rexx_src/explicit_header.crexx
        CREXX_ARGS "-nokeep"
        EXPECT_OUTPUT "EXPLICIT\n"
        EXPECT_NO_LINKED_INTERMEDIATE
        EXPECT_ABSENT "explicit_header.rxas" "explicit_header.rxbin" "explicit_header_linked.rxbin" "explicit_header.c"
)

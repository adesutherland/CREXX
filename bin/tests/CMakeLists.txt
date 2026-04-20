configure_file(${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/headerless_simple.rexx
               ${CMAKE_CURRENT_BINARY_DIR}/headerless_simple.rexx
               COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/explicit_header.rexx
               ${CMAKE_CURRENT_BINARY_DIR}/explicit_header.rexx
               COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/bin/crxc.rexx
               ${CMAKE_CURRENT_BINARY_DIR}/crxc.rexx
               COPYONLY)

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
set(CREXX_HEADERLESS_SMOKE_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:crexx>\" \"${CMAKE_CURRENT_BINARY_DIR}/headerless_simple.rexx\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND CREXX_HEADERLESS_SMOKE_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"crexx headerless script failed: \\n\${out}\")\nendif()\n")
string(APPEND CREXX_HEADERLESS_SMOKE_CONTENT "if(NOT out STREQUAL \"HELLO\\n\")\n  message(FATAL_ERROR \"Unexpected headerless output. Got: \\n\${out}\")\nendif()\n")
file(GENERATE OUTPUT "${CREXX_HEADERLESS_SMOKE_SCRIPT}" CONTENT "${CREXX_HEADERLESS_SMOKE_CONTENT}")

add_test(NAME crexx_headerless_simple_smoke
         COMMAND ${CMAKE_COMMAND} -P "${CREXX_HEADERLESS_SMOKE_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

set(CREXX_EXPLICIT_SMOKE_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_crexx_explicit_smoke.cmake")
set(CREXX_EXPLICIT_SMOKE_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:crexx>\" \"${CMAKE_CURRENT_BINARY_DIR}/explicit_header.rexx\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND CREXX_EXPLICIT_SMOKE_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"crexx explicit-header script failed: \\n\${out}\")\nendif()\n")
string(APPEND CREXX_EXPLICIT_SMOKE_CONTENT "if(NOT out STREQUAL \"EXPLICIT\\n\")\n  message(FATAL_ERROR \"Unexpected explicit-header output. Got: \\n\${out}\")\nendif()\n")
file(GENERATE OUTPUT "${CREXX_EXPLICIT_SMOKE_SCRIPT}" CONTENT "${CREXX_EXPLICIT_SMOKE_CONTENT}")

add_test(NAME crexx_explicit_header_smoke
         COMMAND ${CMAKE_COMMAND} -P "${CREXX_EXPLICIT_SMOKE_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

set(CREXX_CRXC_NOARGS_SMOKE_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_crexx_crxc_noargs_smoke.cmake")
set(CREXX_CRXC_NOARGS_SMOKE_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:crexx>\" \"${CMAKE_CURRENT_BINARY_DIR}/crxc.rexx\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND CREXX_CRXC_NOARGS_SMOKE_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"crexx crxc noargs expected rc 0, got \${res}. Output: \\n\${out}\")\nendif()\n")
string(APPEND CREXX_CRXC_NOARGS_SMOKE_CONTENT "if(NOT out STREQUAL \"usage: crxc.rexx crexx_home execName[.ext]\\n\")\n  message(FATAL_ERROR \"Unexpected crxc noargs output. Got: \\n\${out}\")\nendif()\n")
file(GENERATE OUTPUT "${CREXX_CRXC_NOARGS_SMOKE_SCRIPT}" CONTENT "${CREXX_CRXC_NOARGS_SMOKE_CONTENT}")

add_test(NAME crexx_crxc_noargs_smoke
         COMMAND ${CMAKE_COMMAND} -P "${CREXX_CRXC_NOARGS_SMOKE_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

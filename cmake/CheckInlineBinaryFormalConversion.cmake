if(NOT DEFINED RXC OR NOT EXISTS "${RXC}")
  message(FATAL_ERROR "rxc executable was not provided")
endif()
if(NOT DEFINED SOURCE OR NOT EXISTS "${SOURCE}")
  message(FATAL_ERROR "binary-formal conversion source was not provided")
endif()
if(NOT DEFINED INCLUDE_DIR OR NOT IS_DIRECTORY "${INCLUDE_DIR}")
  message(FATAL_ERROR "compiler include directory was not provided")
endif()
if(NOT DEFINED WORK)
  message(FATAL_ERROR "compiler inspection work directory was not provided")
endif()

file(REMOVE_RECURSE "${WORK}")
file(MAKE_DIRECTORY "${WORK}")
set(output_base "${WORK}/inline_binary_formal_string_conversion")

execute_process(
  COMMAND "${RXC}" -i "${INCLUDE_DIR}" -o "${output_base}" "${SOURCE}"
  OUTPUT_VARIABLE compiler_output
  ERROR_VARIABLE compiler_error
  RESULT_VARIABLE compiler_result)
if(NOT compiler_result EQUAL 0)
  message(FATAL_ERROR
    "optimized binary-formal conversion compile failed:\n${compiler_output}${compiler_error}")
endif()

set(rxas_file "${output_base}.rxas")
if(NOT EXISTS "${rxas_file}")
  message(FATAL_ERROR "optimized compiler did not create ${rxas_file}")
endif()
file(READ "${rxas_file}" rxas)

if(rxas MATCHES "call[0-9]*[^\n]*rxfnsb\\.binlength\\(\\)")
  message(FATAL_ERROR
    "optimized string-to-binary formal conversion retained binlength() call")
endif()
if(NOT rxas MATCHES "(^|[\r\n])[ \t]+stobin[ \t]")
  message(FATAL_ERROR
    "optimized string-to-binary formal conversion omitted stobin")
endif()
if(NOT rxas MATCHES "(^|[\r\n])[ \t]+blen[ \t]")
  message(FATAL_ERROR
    "optimized string-to-binary formal conversion omitted inlined blen")
endif()

message(STATUS
  "binary formal remains inlined with the required string-to-binary promotion")

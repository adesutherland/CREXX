if(NOT DEFINED RXC OR NOT EXISTS "${RXC}")
  message(FATAL_ERROR "rxc executable was not provided")
endif()
if(NOT DEFINED SOURCE OR NOT EXISTS "${SOURCE}")
  message(FATAL_ERROR "binary class-attribute inline source was not provided")
endif()
if(NOT DEFINED INCLUDE_DIR OR NOT IS_DIRECTORY "${INCLUDE_DIR}")
  message(FATAL_ERROR "compiler include directory was not provided")
endif()
if(NOT DEFINED WORK)
  message(FATAL_ERROR "compiler inspection work directory was not provided")
endif()

file(REMOVE_RECURSE "${WORK}")
file(MAKE_DIRECTORY "${WORK}")
set(output_base "${WORK}/inline_binary_class_attribute")

execute_process(
  COMMAND "${RXC}" -i "${INCLUDE_DIR}" -o "${output_base}" "${SOURCE}"
  OUTPUT_VARIABLE compiler_output
  ERROR_VARIABLE compiler_error
  RESULT_VARIABLE compiler_result)
if(NOT compiler_result EQUAL 0)
  message(FATAL_ERROR
    "optimized binary class-attribute compile failed:\n${compiler_output}${compiler_error}")
endif()

set(rxas_file "${output_base}.rxas")
if(NOT EXISTS "${rxas_file}")
  message(FATAL_ERROR "optimized compiler did not create ${rxas_file}")
endif()
file(READ "${rxas_file}" rxas)

foreach(callee IN ITEMS inner outer)
  if(rxas MATCHES "call[0-9]*[^\n]*inline_test_binary_class_attr_assign\\.${callee}\\(\\)")
    message(FATAL_ERROR
      "optimized binary class-attribute assignment retained ${callee}() call")
  endif()
endforeach()

if(NOT rxas MATCHES "linkattr1[^\n]*,1[\r\n \t]+copy[ \t]")
  message(FATAL_ERROR
    "optimized binary class-attribute assignment lacks receiver-slot copyback")
endif()

message(STATUS
  "binary class-attribute assignment retains inlining and receiver-slot copyback")

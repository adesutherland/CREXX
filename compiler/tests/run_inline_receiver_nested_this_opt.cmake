foreach(required_var RXC RXAS RXVM RXBVM IMPORT_DIR LIBRARY SOURCE WORK)
  if(NOT DEFINED ${required_var})
    message(FATAL_ERROR "Missing required variable: ${required_var}")
  endif()
endforeach()

file(REMOVE_RECURSE "${WORK}")
file(MAKE_DIRECTORY "${WORK}")
execute_process(
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${SOURCE}" "${WORK}/probe.crexx"
  RESULT_VARIABLE res)
if(NOT res EQUAL 0)
  message(FATAL_ERROR "Failed to stage nested-this receiver test")
endif()

execute_process(
  COMMAND "${RXC}" -i "${IMPORT_DIR}" -o probe probe.crexx
  WORKING_DIRECTORY "${WORK}"
  OUTPUT_VARIABLE out
  ERROR_VARIABLE out
  RESULT_VARIABLE res)
if(NOT res EQUAL 0)
  message(FATAL_ERROR "rxc failed on nested-this receiver test: ${out}")
endif()

file(READ "${WORK}/probe.rxas" rxas)
set(direct_header "§inline_test_mutating_method_scalar_attr_return.probe.directattr()")
set(next_header "§inline_test_mutating_method_scalar_attr_return.probe.localthenattr()")
string(FIND "${rxas}" "${direct_header}" direct_start)
string(FIND "${rxas}" "${next_header}" next_start)
if(direct_start LESS 0 OR next_start LESS 0 OR next_start LESS_EQUAL direct_start)
  message(FATAL_ERROR "Could not isolate optimized directAttr() body:\n${rxas}")
endif()
math(EXPR direct_length "${next_start} - ${direct_start}")
string(SUBSTRING "${rxas}" ${direct_start} ${direct_length} direct_body)

if(direct_body MATCHES [=[\n[ \t]*call[ \t]+[^\n]*(reset|changeandreturndifferent)\(\)]=])
  message(FATAL_ERROR "Nested methods were not inlined:\n${direct_body}")
endif()
if(NOT direct_body MATCHES "copy[ \t]+r[0-9]+,a1")
  message(FATAL_ERROR
    "Nested §this receiver lost its required materialization boundary:\n${direct_body}")
endif()
if(NOT direct_body MATCHES "copy[ \t]+a1,r[0-9]+")
  message(FATAL_ERROR
    "Nested §this receiver lost its required copyback boundary:\n${direct_body}")
endif()

execute_process(
  COMMAND "${RXAS}" -o probe.rxbin probe
  WORKING_DIRECTORY "${WORK}"
  OUTPUT_VARIABLE out
  ERROR_VARIABLE out
  RESULT_VARIABLE res)
if(NOT res EQUAL 0)
  message(FATAL_ERROR "rxas failed on nested-this receiver test: ${out}")
endif()

foreach(vm RXVM RXBVM)
  execute_process(
    COMMAND "${${vm}}" "${LIBRARY}" "${WORK}/probe.rxbin"
    WORKING_DIRECTORY "${WORK}"
    OUTPUT_VARIABLE out
    ERROR_VARIABLE err
    RESULT_VARIABLE res)
  if(NOT res EQUAL 0)
    message(FATAL_ERROR "${vm} failed on nested-this receiver test: ${out}${err}")
  endif()
  string(REPLACE "\r\n" "\n" out "${out}")
  if(NOT out STREQUAL "115\n115\n15\n")
    message(FATAL_ERROR "${vm} output mismatch: ${out}${err}")
  endif()
endforeach()

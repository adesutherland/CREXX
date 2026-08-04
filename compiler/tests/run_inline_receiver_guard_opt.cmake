foreach(required_var RXC RXAS RXVM RXBVM IMPORT_DIR LIBRARY LOCAL_SOURCE DEP_SOURCE MAIN_SOURCE WORK)
  if(NOT DEFINED ${required_var})
    message(FATAL_ERROR "Missing required variable: ${required_var}")
  endif()
endforeach()

function(extract_body image header next_header out_var)
  string(FIND "${image}" "${header}" body_start)
  string(FIND "${image}" "${next_header}" body_end)
  if(body_start LESS 0 OR body_end LESS_EQUAL body_start)
    message(FATAL_ERROR "Could not isolate ${header}")
  endif()
  math(EXPR body_length "${body_end} - ${body_start}")
  string(SUBSTRING "${image}" ${body_start} ${body_length} body)
  set(${out_var} "${body}" PARENT_SCOPE)
endfunction()

function(require_fallback body name call_name)
  if("${body}" MATCHES "__inline_receiver_guard_")
    message(FATAL_ERROR "${name} crossed the detached receiver-guard gate:\n${body}")
  endif()
  if(NOT "${body}" MATCHES "copy[ \t]+r[0-9]+,a1" AND
     NOT "${body}" MATCHES "call[0-9]*[ \t]+[^\n]*${call_name}\\(\\)")
    message(FATAL_ERROR "${name} retained neither the materialized receiver nor the call fallback:\n${body}")
  endif()
endfunction()

file(REMOVE_RECURSE "${WORK}")
file(MAKE_DIRECTORY "${WORK}")
foreach(source LOCAL_SOURCE DEP_SOURCE MAIN_SOURCE)
  get_filename_component(name "${${source}}" NAME)
  execute_process(COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${${source}}" "${WORK}/${name}"
                  RESULT_VARIABLE res)
  if(NOT res EQUAL 0)
    message(FATAL_ERROR "Failed to stage ${name}")
  endif()
endforeach()

execute_process(COMMAND "${RXC}" -i "${IMPORT_DIR}" -o local inline_receiver_guard_local.crexx
                WORKING_DIRECTORY "${WORK}" OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)
if(NOT res EQUAL 0)
  message(FATAL_ERROR "rxc failed on local detached receiver guards: ${out}${err}")
endif()
file(READ "${WORK}/local.rxas" local_rxas)

set(ns "§inline_receiver_guard_local.probe.")
extract_body("${local_rxas}" "${ns}positivecaller()" "${ns}booleanidentity()" positive_body)
extract_body("${local_rxas}" "${ns}callcaller()" "${ns}writeguard()" call_body)
extract_body("${local_rxas}" "${ns}writecaller()" "${ns}nonguardread()" write_body)
extract_body("${local_rxas}" "${ns}nonguardcaller()" "${ns}loopguard()" non_guard_body)
extract_body("${local_rxas}" "${ns}loopcaller()" "${ns}referenceguard()" loop_body)
extract_body("${local_rxas}" "${ns}referencecaller()" "${ns}nestedguard()" reference_body)
extract_body("${local_rxas}" "${ns}nestedcaller()" "${ns}signalcaller()" nested_body)
string(FIND "${local_rxas}" "${ns}signalcaller()" signal_start)
if(signal_start LESS 0)
  message(FATAL_ERROR "Could not isolate signalCaller")
endif()
string(SUBSTRING "${local_rxas}" ${signal_start} -1 signal_body)

if(NOT positive_body MATCHES "__inline_receiver_guard_" OR
   positive_body MATCHES "copy[ \t]+r[0-9]+,a1" OR
   positive_body MATCHES "call[0-9]*[ \t]+[^\n]*positiveguard\\(\\)")
  message(FATAL_ERROR "Exact local receiver guard was not detached and directly bound:\n${positive_body}")
endif()
require_fallback("${call_body}" "call-bearing guard" "callguard")
require_fallback("${write_body}" "receiver-writing guard" "writeguard")
require_fallback("${non_guard_body}" "non-guard receiver read" "nonguardread")
require_fallback("${loop_body}" "loop guard" "loopguard")
require_fallback("${reference_body}" "reference formal guard" "referenceguard")
require_fallback("${nested_body}" "nested return block" "nestedguard")
require_fallback("${signal_body}" "same-frame signal continuation" "positiveguard")

execute_process(COMMAND "${RXAS}" -o local.rxbin local WORKING_DIRECTORY "${WORK}"
                OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)
if(NOT res EQUAL 0)
  message(FATAL_ERROR "rxas failed on local detached receiver guards: ${out}${err}")
endif()
foreach(vm RXVM RXBVM)
  execute_process(COMMAND "${${vm}}" "${LIBRARY}" "${WORK}/local.rxbin"
                  WORKING_DIRECTORY "${WORK}" OUTPUT_VARIABLE run_out ERROR_VARIABLE err RESULT_VARIABLE res)
  string(REPLACE "\r\n" "\n" run_out "${run_out}")
  if(NOT res EQUAL 0 OR NOT run_out STREQUAL "1\n1\n1\n1\n1\n1\n1\n1\n")
    message(FATAL_ERROR "${vm} local receiver-guard mismatch: ${run_out}${err}")
  endif()
endforeach()

execute_process(COMMAND "${RXC}" -i "${IMPORT_DIR}" -o inline_receiver_guard_dep
                        inline_receiver_guard_dep.crexx
                WORKING_DIRECTORY "${WORK}" OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)
if(NOT res EQUAL 0)
  message(FATAL_ERROR "rxc failed on receiver-guard dependency: ${out}${err}")
endif()
execute_process(COMMAND "${RXAS}" -o inline_receiver_guard_dep.rxbin inline_receiver_guard_dep
                WORKING_DIRECTORY "${WORK}" OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)
if(NOT res EQUAL 0)
  message(FATAL_ERROR "rxas failed on receiver-guard dependency: ${out}${err}")
endif()
file(REMOVE "${WORK}/inline_receiver_guard_dep.crexx" "${WORK}/inline_receiver_guard_dep.rxas")

execute_process(COMMAND "${RXC}" -i "${WORK}" -i "${IMPORT_DIR}" -o imported
                        inline_receiver_guard_main.crexx
                WORKING_DIRECTORY "${WORK}" OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)
if(NOT res EQUAL 0)
  message(FATAL_ERROR "rxc failed on imported receiver guard: ${out}${err}")
endif()
file(READ "${WORK}/imported.rxas" imported_rxas)
if(imported_rxas MATCHES "call[0-9]*[ \t]+[^\n]*inline_receiver_guard_dep\\.probe\\.guard\\(\\)" OR
   imported_rxas MATCHES "copy[ \t]+r[0-9]+,r1")
  message(FATAL_ERROR "Imported exact receiver guard was not validated and directly placed:\n${imported_rxas}")
endif()
execute_process(COMMAND "${RXAS}" -o imported.rxbin imported WORKING_DIRECTORY "${WORK}"
                OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)
if(NOT res EQUAL 0)
  message(FATAL_ERROR "rxas failed on imported receiver guard: ${out}${err}")
endif()
foreach(vm RXVM RXBVM)
  execute_process(COMMAND "${${vm}}" "${LIBRARY}" "${WORK}/inline_receiver_guard_dep.rxbin"
                          "${WORK}/imported.rxbin"
                  WORKING_DIRECTORY "${WORK}" OUTPUT_VARIABLE run_out ERROR_VARIABLE err RESULT_VARIABLE res)
  string(REPLACE "\r\n" "\n" run_out "${run_out}")
  if(NOT res EQUAL 0 OR NOT run_out STREQUAL "1\n")
    message(FATAL_ERROR "${vm} imported receiver-guard mismatch: ${run_out}${err}")
  endif()
endforeach()

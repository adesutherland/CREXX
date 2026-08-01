foreach(required_var RXC RXAS RXVM RXBVM IMPORT_DIR LIBRARY SOURCE WORK)
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

file(REMOVE_RECURSE "${WORK}")
file(MAKE_DIRECTORY "${WORK}")
execute_process(
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${SOURCE}"
          "${WORK}/inline_receiver_production_ladder.crexx"
  RESULT_VARIABLE res)
if(NOT res EQUAL 0)
  message(FATAL_ERROR "Failed to stage production receiver-ladder source")
endif()

foreach(mode opt noopt)
  if(mode STREQUAL "noopt")
    set(opt_arg -n)
  else()
    set(opt_arg "")
  endif()
  execute_process(
    COMMAND "${RXC}" ${opt_arg} -i "${IMPORT_DIR}" -o "probe-${mode}"
            inline_receiver_production_ladder.crexx
    WORKING_DIRECTORY "${WORK}"
    OUTPUT_VARIABLE out
    ERROR_VARIABLE err
    RESULT_VARIABLE res)
  if(NOT res EQUAL 0)
    message(FATAL_ERROR "rxc failed on ${mode} production receiver ladder: ${out}${err}")
  endif()
  execute_process(
    COMMAND "${RXAS}" -o "probe-${mode}.rxbin" "probe-${mode}"
    WORKING_DIRECTORY "${WORK}"
    OUTPUT_VARIABLE out
    ERROR_VARIABLE err
    RESULT_VARIABLE res)
  if(NOT res EQUAL 0)
    message(FATAL_ERROR "rxas failed on ${mode} production receiver ladder: ${out}${err}")
  endif()

  foreach(vm RXVM RXBVM)
    execute_process(
      COMMAND "${${vm}}" "${LIBRARY}" "${WORK}/probe-${mode}.rxbin"
      WORKING_DIRECTORY "${WORK}"
      OUTPUT_VARIABLE run_out
      ERROR_VARIABLE err
      RESULT_VARIABLE res)
    string(REPLACE "\r\n" "\n" run_out "${run_out}")
    if(NOT res EQUAL 0 OR NOT run_out STREQUAL "41\n42\n7\n7\n9\n9\n")
      message(FATAL_ERROR
        "${vm} ${mode} production receiver-ladder mismatch: ${run_out}${err}")
    endif()
  endforeach()
endforeach()

file(READ "${WORK}/probe-opt.rxas" image)
set(ns "§inline_receiver_production_ladder.probe.")
extract_body("${image}" "${ns}unusedcaller()" "${ns}formalcaller()" unused_body)
extract_body("${image}" "${ns}formalcaller()" "${ns}referencecaller()" formal_body)
string(FIND "${image}" "${ns}referencecaller()" reference_start)
if(reference_start LESS 0)
  message(FATAL_ERROR "Could not isolate referenceCaller")
endif()
string(SUBSTRING "${image}" ${reference_start} -1 reference_body)

if(unused_body MATCHES [=[call[0-9]* [^\n]*\.unused\(\)]=] OR
   unused_body MATCHES "copy[ \t]+r[0-9]+,a1")
  message(FATAL_ERROR
    "Unused multi-return receiver was not directly placed:\n${unused_body}")
endif()

if(formal_body MATCHES [=[call[0-9]* [^\n]*\.replace\(\)]=] OR
   formal_body MATCHES "copy[ \t]+r[0-9]+,a2" OR
   formal_body MATCHES "copy[ \t]+a2,r[0-9]+")
  message(FATAL_ERROR
    "Isolated by-value object-formal receiver retained materialisation/copyback:\n${formal_body}")
endif()

if(reference_body MATCHES [=[call[0-9]* [^\n]*\.replace\(\)]=])
  message(FATAL_ERROR "Reference-formal method unexpectedly retained a call:\n${reference_body}")
endif()
if(NOT reference_body MATCHES "copy[ \t]+r[0-9]+,a2" OR
   NOT reference_body MATCHES "copy[ \t]+a2,r[0-9]+")
  message(FATAL_ERROR
    "Reference-formal receiver lost its conservative materialisation/copyback:\n${reference_body}")
endif()

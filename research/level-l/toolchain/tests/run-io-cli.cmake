cmake_minimum_required(VERSION 3.24)
foreach(_name VM ASSEMBLER DISASSEMBLER CLI GENERATED FIXTURES OPTIMIZER_FIXTURES WORK)
  if(NOT DEFINED ${_name})
    message(FATAL_ERROR "Missing ${_name}")
  endif()
endforeach()
file(MAKE_DIRECTORY "${WORK}")
function(run_checked _description)
  execute_process(COMMAND ${ARGN} RESULT_VARIABLE _status
    OUTPUT_VARIABLE _stdout ERROR_VARIABLE _stderr TIMEOUT 90)
  if(NOT _status EQUAL 0)
    message(FATAL_ERROR "${_description}: ${_status}\n${_stdout}${_stderr}")
  endif()
  set(_last_stdout "${_stdout}" PARENT_SCOPE)
  set(_last_stderr "${_stderr}" PARENT_SCOPE)
endfunction()

# Inspect only source-visible instructions in the selected caller, excluding
# helper bodies, source records and opaque compiler inline transport.
function(caller_instructions _path _name _output)
  file(STRINGS "${_path}" _lines ENCODING UTF-8)
  set(_inside OFF)
  set(_instructions "")
  foreach(_line IN LISTS _lines)
    if(_line MATCHES "^[^ \t]+\\(\\)[ \t]+\\.locals=")
      if(_line MATCHES "^${_name}\\(\\)[ \t]")
        set(_inside ON)
      else()
        set(_inside OFF)
      endif()
    elseif(_inside AND _line MATCHES "^[ \t]*[A-Za-z][A-Za-z0-9]*([ \t]|$)")
      list(APPEND _instructions "${_line}")
    endif()
  endforeach()
  if(_instructions STREQUAL "")
    message(FATAL_ERROR "No instructions found for ${_name} in ${_path}")
  endif()
  set(${_output} "${_instructions}" PARENT_SCOPE)
endfunction()

# This is the actual ordinary compiler -n output, not a handwritten substitute.
run_checked("read compiler output" "${VM}" "${CLI}" ${RUNTIME} -a
  --module io-generated.rxas --policy never -o "${WORK}/io-generated.rxas" "${GENERATED}")
run_checked("assemble compiler roundtrip without cleanup" "${ASSEMBLER}" -n
  -o "${WORK}/io-generated" "${WORK}/io-generated.rxas")
run_checked("execute compiler roundtrip" "${VM}" "${WORK}/io-generated.rxbin")
string(STRIP "${_last_stdout}" _result)
if(NOT _result STREQUAL "16")
  message(FATAL_ERROR "compiler roundtrip expected 16, got '${_result}'")
endif()

# Expand both original arithmetic calls atomically. Execution is independent
# of cleanup and native assembler optimization in both products.
foreach(_mode conservative cleaned)
  set(_cleanup_option)
  if(_mode STREQUAL "conservative")
    set(_cleanup_option --no-cleanup)
  endif()
  run_checked("inline generated arithmetic (${_mode})" "${VM}" "${CLI}" ${RUNTIME} -a
    --module io-generated.rxas --profile closed-main-default-halt.v1 ${_cleanup_option}
    -o "${WORK}/generated-${_mode}.rxas" "${GENERATED}")
  caller_instructions("${WORK}/generated-${_mode}.rxas" main _caller)
  set(_has_add OFF)
  set(_has_multiply OFF)
  set(_transport_${_mode} 0)
  foreach(_instruction IN LISTS _caller)
    if(_instruction MATCHES "^[ \t]*(call[0-9]*|swapcall|settpswapcall|settpcall)[ \t]")
      message(FATAL_ERROR "generated ${_mode} caller retained a selected arithmetic call")
    endif()
    if(_instruction MATCHES "^[ \t]*iadd[ \t]")
      set(_has_add ON)
    endif()
    if(_instruction MATCHES "^[ \t]*imult[ \t]")
      set(_has_multiply ON)
    endif()
    if(_instruction MATCHES "^[ \t]*(swap|link|unlink|settp)[ \t]")
      math(EXPR _transport_${_mode} "${_transport_${_mode}} + 1")
    endif()
  endforeach()
  if(NOT _has_add OR NOT _has_multiply)
    message(FATAL_ERROR "generated ${_mode} caller lost checked integer arithmetic")
  endif()
  run_checked("assemble generated arithmetic (${_mode})" "${ASSEMBLER}" -n
    -o "${WORK}/generated-${_mode}" "${WORK}/generated-${_mode}.rxas")
  run_checked("execute generated arithmetic (${_mode})" "${VM}" "${WORK}/generated-${_mode}.rxbin")
  string(STRIP "${_last_stdout}" _result)
  if(NOT _result STREQUAL "16")
    message(FATAL_ERROR "generated ${_mode} arithmetic expected 16, got '${_result}'")
  endif()
endforeach()
if(NOT _transport_cleaned LESS _transport_conservative)
  message(FATAL_ERROR "cleaned generated caller did not remove redundant transport")
endif()

# Execute the independent storage/permutation shapes with cleanup both disabled
# and enabled. Native assembly optimization stays disabled for each product.
foreach(_case reordered snapshot)
  foreach(_mode conservative cleaned)
    set(_cleanup_option)
    if(_mode STREQUAL "conservative")
      set(_cleanup_option --no-cleanup)
    endif()
    set(_product "${WORK}/${_case}-${_mode}")
    run_checked("inline ${_case} (${_mode})" "${VM}" "${CLI}" ${RUNTIME} -a
      --module "io-${_case}.rxas" --profile closed-main-default-halt.v1 ${_cleanup_option}
      -o "${_product}.rxas" "${FIXTURES}/io-${_case}.rxas")
    caller_instructions("${_product}.rxas" main _caller)
    set(_snapshots 0)
    set(_moves 0)
    foreach(_instruction IN LISTS _caller)
      if(_instruction MATCHES "^[ \t]*(call[0-9]*|swapcall|settpswapcall|settpcall)[ \t]")
        message(FATAL_ERROR "${_case} ${_mode} caller retained its selected call")
      endif()
      if(_case STREQUAL "reordered" AND _mode STREQUAL "cleaned"
          AND _instruction MATCHES "^[ \t]*(swap|link|unlink|settp)[ \t]")
        message(FATAL_ERROR "cleaned reordered caller retained outer setup/restoration")
      endif()
      if(_instruction MATCHES "^[ \t]*icopy[ \t]")
        math(EXPR _snapshots "${_snapshots} + 1")
      endif()
      if(_instruction MATCHES "^[ \t]*move[ \t]")
        math(EXPR _moves "${_moves} + 1")
      endif()
    endforeach()
    if(_case STREQUAL "snapshot" AND (NOT _snapshots EQUAL 1 OR NOT _moves EQUAL 1))
      message(FATAL_ERROR "${_mode} alias caller lost the required snapshot or owned-return MOVE")
    endif()
    if(_case STREQUAL "reordered")
      file(STRINGS "${_product}.rxas" _lines ENCODING UTF-8)
      set(_frame -1)
      set(_trace OFF)
      foreach(_line IN LISTS _lines)
        if(_line MATCHES "^main\\(\\)[ \t]+\\.locals=([0-9]+)")
          set(_frame "${CMAKE_MATCH_1}")
        endif()
        if(_line MATCHES [=[^[ \t]*\.traceevent[ \t]+"V"[ \t]+4[ \t]+"R"[ \t]+"I"[ \t]+"r"[ \t]+[0-9]+[ \t]+0[ \t]+0[ \t]+0[ \t]+"y"[ \t]+""$]=])
          set(_trace ON)
          if(_mode STREQUAL "cleaned" AND NOT _line MATCHES [=["r"[ \t]+1[ \t]+0[ \t]+0[ \t]+0[ \t]+"y"]=])
            message(FATAL_ERROR "cleaned reordered TRACE no longer names the original actual storage")
          endif()
        endif()
      endforeach()
      if(NOT _trace)
        message(FATAL_ERROR "${_mode} reordered caller lost its TRACE observation")
      endif()
      if(_mode STREQUAL "cleaned" AND (_frame LESS 0 OR _frame GREATER 6))
        message(FATAL_ERROR "cleaned reordered leaf added caller frame cells: ${_frame}")
      endif()
    endif()
    run_checked("assemble ${_case} (${_mode})" "${ASSEMBLER}" -n
      -o "${_product}" "${_product}.rxas")
    run_checked("execute ${_case} (${_mode})" "${VM}" "${_product}.rxbin")
    string(REPLACE "\r\n" "\n" _normalized "${_last_stdout}")
    string(STRIP "${_normalized}" _result)
    set(_expected "23")
    if(_case STREQUAL "snapshot")
      set(_expected "7\n8")
    endif()
    if(NOT _result STREQUAL _expected)
      message(FATAL_ERROR "${_case} ${_mode} expected '${_expected}', got '${_result}'")
    endif()
  endforeach()
endforeach()

# MAX_INT+1 has no available-success write proof. The default root ignores its
# overflow signal; require the precise retained-call decision, not a guessed
# arithmetic result or a newly introduced handler policy.
run_checked("retain unproved overflow write" "${VM}" "${CLI}" ${RUNTIME} -a
  --module io-overflow.rxas --profile closed-main-default-halt.v1
  -o "${WORK}/io-overflow.rxas" "${FIXTURES}/io-overflow.rxas")
string(STRIP "${_last_stderr}" _overflow_reason)
if(NOT _overflow_reason STREQUAL "LL_INLINE_KEEP: arithmetic.write.availability.unproved")
  message(FATAL_ERROR "overflow retention reason differs: '${_overflow_reason}'")
endif()
caller_instructions("${WORK}/io-overflow.rxas" main _overflow_caller)
set(_overflow_retained OFF)
foreach(_instruction IN LISTS _overflow_caller)
  if(_instruction MATCHES "^[ \t]*call1[ \t]")
    set(_overflow_retained ON)
  endif()
endforeach()
if(NOT _overflow_retained)
  message(FATAL_ERROR "unproved overflow call was expanded")
endif()
run_checked("assemble retained overflow control" "${ASSEMBLER}" -n
  -o "${WORK}/io-overflow" "${WORK}/io-overflow.rxas")

# A separate producer exercises real expansion with assembler optimization off.
run_checked("inline handwritten input" "${VM}" "${CLI}" ${RUNTIME} -a
  --module io-handwritten.rxas --profile closed-main-default-halt.v1 --no-cleanup
  -o "${WORK}/io-handwritten.rxas" "${FIXTURES}/io-handwritten.rxas")
file(READ "${WORK}/io-handwritten.rxas" _expanded)
if(_expanded MATCHES "(^|\n)[ \t]*call1[ \t]")
  message(FATAL_ERROR "selected handwritten call was retained")
endif()
run_checked("assemble expansion without cleanup" "${ASSEMBLER}" -n
  -o "${WORK}/io-handwritten" "${WORK}/io-handwritten.rxas")
run_checked("execute expansion" "${VM}" "${WORK}/io-handwritten.rxbin")
string(STRIP "${_last_stdout}" _result)
if(NOT _result STREQUAL "7")
  message(FATAL_ERROR "handwritten expansion expected 7, got '${_result}'")
endif()

# Native preparation is outside the B/L request. The optimized instruction body
# is then admitted by the same RXAS reader, with no request-time subprocess.
run_checked("prepare optimized module" "${ASSEMBLER}"
  -o "${WORK}/prepared-opt" "${FIXTURES}/io-handwritten.rxas")
run_checked("render optimized module externally" "${DISASSEMBLER}"
  -o "${WORK}/prepared-opt.rxas" "${WORK}/prepared-opt.rxbin")
run_checked("read prepared optimized body" "${VM}" "${CLI}" ${RUNTIME} -a
  --module io-handwritten.rxas --policy never
  -o "${WORK}/prepared-roundtrip.rxas" "${WORK}/prepared-opt.rxas")
run_checked("assemble prepared roundtrip" "${ASSEMBLER}" -n
  -o "${WORK}/prepared-roundtrip" "${WORK}/prepared-roundtrip.rxas")
run_checked("execute prepared roundtrip" "${VM}" "${WORK}/prepared-roundtrip.rxbin")
string(STRIP "${_last_stdout}" _result)
if(NOT _result STREQUAL "7")
  message(FATAL_ERROR "prepared optimized roundtrip expected 7, got '${_result}'")
endif()

# Reuse maintained storage/alias/component oracles unchanged. This exercises
# adapter preservation, including their wider handlers/globals, without asking
# the inliner to admit those shapes or using native optimization as the oracle.
set(_preservation_cases storage_identity_runtime entry_alias_runtime
  copied_xtoy_component_placement_runtime)
set(_preservation_markers "PASS: C2-E2 storage identity runtime"
  "PASS: incoming argument alias runtime"
  "PASS: PERF3-12A X1 component placement runtime")
foreach(_index RANGE 0 2)
  list(GET _preservation_cases ${_index} _case)
  list(GET _preservation_markers ${_index} _marker)
  set(_product "${WORK}/preserved-${_case}")
  run_checked("roundtrip maintained ${_case}" "${VM}" "${CLI}" ${RUNTIME} -a
    --module "${_case}.rxas" --policy never
    -o "${_product}.rxas" "${OPTIMIZER_FIXTURES}/${_case}.rxas")
  run_checked("assemble maintained ${_case} without optimization" "${ASSEMBLER}" -n
    -o "${_product}" "${_product}.rxas")
  run_checked("execute maintained ${_case}" "${VM}" "${_product}.rxbin")
  string(FIND "${_last_stdout}" "${_marker}" _pass)
  if(_pass LESS 0 OR _last_stdout MATCHES "FAIL" OR _last_stderr MATCHES "FAIL|PANIC:")
    message(FATAL_ERROR "maintained ${_case} preservation failed:\n${_last_stdout}${_last_stderr}")
  endif()
endforeach()

file(WRITE "${WORK}/last-good.rxas" "LAST_GOOD\n")
execute_process(COMMAND "${VM}" "${CLI}" ${RUNTIME} -a
  --module invalid.rxas --policy never -o "${WORK}/last-good.rxas"
  "${FIXTURES}/io-invalid.rxas" RESULT_VARIABLE _status
  OUTPUT_VARIABLE _stdout ERROR_VARIABLE _stderr TIMEOUT 90)
if(_status EQUAL 0)
  message(FATAL_ERROR "invalid input unexpectedly succeeded")
endif()
file(READ "${WORK}/last-good.rxas" _preserved)
if(NOT _preserved STREQUAL "LAST_GOOD\n")
  message(FATAL_ERROR "invalid request replaced last-good output")
endif()
file(REMOVE_RECURSE "${WORK}")
message(STATUS "RXAS_IO_CLI|failures=0")

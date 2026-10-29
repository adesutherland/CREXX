cmake_minimum_required(VERSION 3.24)
foreach(_name VM ASSEMBLER CLI FIXTURES WORK CONSTRUCTED)
  if(NOT DEFINED ${_name})
    message(FATAL_ERROR "Missing ${_name}")
  endif()
endforeach()
file(MAKE_DIRECTORY "${WORK}")
include("${FIXTURES}/signal-pair-helpers.cmake")
set(_lane scope-reentry)
set(_marker "SCOPE_REENTRY[|]failures=0")
if(RUNTIME_SCOPE)
  set(_lane runtime-scope)
  set(_marker "RUNTIME_SCOPE[|]failures=0")
endif()
run_checked("constructed ${_lane}" "${VM}" "${CONSTRUCTED}" -a "${_lane}")
if(NOT _last_stdout MATCHES "${_marker}")
  message(FATAL_ERROR "Constructed scope controls: ${_last_stdout}${_last_stderr}")
endif()

# Independent native readback after transformation. This observer never enters
# an inliner request. Before return73's address group, left/hidden/right are
# 1/1/0; within its authored ordered group they are 1/1/1. The skipped opening
# and missing left clear have lexical effect in the original too.
function(readback _stem _source _anchors)
  string(REGEX MATCHALL "[.]meta \"scope[.](left|hidden|right)[^\"]*\"" _rows "${_source}")
  set(_names "")
  foreach(_row IN LISTS _rows)
    string(REGEX REPLACE "[.]meta \"([^\"]*)\"" "\\1" _symbol "${_row}")
    list(APPEND _names "${_symbol}")
  endforeach()
  list(REMOVE_DUPLICATES _names)
  set(_dispatch "")
  foreach(_symbol IN LISTS _names)
    string(REGEX REPLACE "^scope[.](left|hidden|right).*$" "\\1" _kind "${_symbol}")
    string(APPEND _dispatch "seq r4,r5,\"${_symbol}\"\nbrt probe_${_kind},r4\n")
  endforeach()
  set(_probe [=[.globals=0
main() .locals=17
load r0,0
load r6,0
load r7,0
load r8,0
load r14,1
probe_next:
igt r4,r0,4096
brt probe_done,r4
icopy r9,r6
icopy r10,r7
icopy r11,r8
metaloaddata r1,r14,r0
load r2,1
probe_record:
igt r4,r2,r1
brt probe_advance,r4
linkattr1 r3,r1,r2
seq r13,r3,".meta_reg"
brt probe_symbol,r13
seq r4,r3,".meta_clear"
brf probe_source,r4
probe_symbol:
linkattr1 r5,r3,1
]=])
  string(APPEND _probe "${_dispatch}br probe_end_record\nprobe_left:\nicopy r6,r13\nbr probe_end_record\nprobe_hidden:\nicopy r7,r13\nbr probe_end_record\nprobe_right:\nicopy r8,r13\nbr probe_end_record\n")
  string(APPEND _probe [=[probe_source:
seq r4,r3,".meta_source_step"
brf probe_trace,r4
linkattr1 r5,r3,4
scopy r15,r5
unlink r5
linkattr1 r5,r3,5
icopy r16,r5
ieq r4,r5,2
brf probe_end_record,r4
say "ANCHOR"
]=])
  foreach(_cell 9 10 11 6 7 8)
    string(APPEND _probe "icopy r12,r${_cell}\nitos r12\nsay r12\n")
  endforeach()
  string(APPEND _probe [=[br probe_end_record
probe_trace:
seq r4,r3,".meta_trace_event"
brf probe_end_record,r4
linkattr1 r5,r3,10
seq r4,r5,"helper"
brf probe_end_record,r4
say "CONTINUATION"
say r15
icopy r12,r16
itos r12
say r12
probe_end_record:
unlink r5
unlink r3
iadd r2,r2,1
br probe_record
probe_advance:
iadd r0,r0,1
br probe_next
probe_done:
ret 0
]=])
  string(REPLACE ".globals=0" "" _sample "${_source}")
  string(REPLACE "main() .locals=" "sample() .locals=" _sample "${_sample}")
  file(WRITE "${WORK}/${_stem}-probe.rxas" "${_probe}\n${_sample}")
  run_checked("assemble ${_stem} metadata readback" "${ASSEMBLER}" -n
    -o "${WORK}/${_stem}-probe" "${WORK}/${_stem}-probe.rxas")
  run_checked("read ${_stem} native metadata" "${VM}" "${WORK}/${_stem}-probe.rxbin")
  string(REPLACE "\r\n" "\n" _observed "${_last_stdout}")
  string(REGEX MATCHALL "ANCHOR\n1\n1\n0\n1\n1\n1\n" _correct_anchors "${_observed}")
  list(LENGTH _correct_anchors _actual_anchors)
  string(REGEX MATCHALL "CONTINUATION\nscoped-caller[.]rxas\n3\n" _continuations "${_observed}")
  list(LENGTH _continuations _actual_continuations)
  string(REPLACE "ANCHOR\n1\n1\n0\n1\n1\n1\n" "" _remaining "${_observed}")
  string(REPLACE "CONTINUATION\nscoped-caller.rxas\n3\n" "" _remaining "${_remaining}")
  string(STRIP "${_remaining}" _remaining)
  if(NOT _actual_anchors EQUAL _anchors OR NOT _actual_continuations EQUAL 2 OR NOT _remaining STREQUAL "")
    message(FATAL_ERROR "${_stem} native scope/source state differs: ${_observed}")
  endif()
endfunction()

if(RUNTIME_SCOPE)
  file(READ "${FIXTURES}/io-runtime-scope.rxas" _original)
  readback(runtime-original "${_original}" 1)
  foreach(_mode conservative cleaned)
    set(_option "")
    if(_mode STREQUAL conservative)
      set(_option --no-cleanup)
    endif()
    set(_product "${WORK}/runtime-scope-${_mode}")
    run_checked("expand runtime suffix scopes ${_mode}" "${VM}" "${CLI}" ${RUNTIME} -a
      --module runtime-scope.rxas --profile closed-main-default-halt.v1 ${_option}
      --work 12000000 -o "${_product}.rxas" "${FIXTURES}/io-runtime-scope.rxas")
    file(READ "${_product}.rxas" _expanded)
    string(FIND "${_expanded}" "helper() .locals=" _callee)
    string(SUBSTRING "${_expanded}" 0 ${_callee} _caller)
    string(REGEX MATCHALL "ichkrng " _guards "${_caller}")
    list(LENGTH _guards _guard_count)
    string(REGEX MATCHALL "[.]meta \"scope[.]hidden[.][$]inline[^\"]*\"[^\n]*\n([ \t]*[.]meta[^\n]*\n)*[ \t]*cnop" _carriers "${_caller}")
    list(LENGTH _carriers _carrier_count)
    if(_caller MATCHES "call4 " OR NOT _guard_count EQUAL 2 OR NOT _carrier_count EQUAL 6)
      message(FATAL_ERROR "${_mode} lost once-only guards or case-local scope carriers: ${_last_stderr}")
    endif()
    bound_pair("runtime-scope-${_mode}" "${_original}" "${_expanded}" "17\n26\nten\n256\n26")
    readback("runtime-scope-${_mode}" "${_expanded}" 7)
  endforeach()
  file(REMOVE_RECURSE "${WORK}")
  message("RXAS_RUNTIME_SCOPE|failures=0")
  return()
endif()

file(READ "${FIXTURES}/io-scope-reentry.rxas" _original)
readback(original "${_original}" 1)
foreach(_mode conservative cleaned)
  set(_option "")
  if(_mode STREQUAL conservative)
    set(_option --no-cleanup)
  endif()
  set(_product "${WORK}/scope-${_mode}")
  run_checked("expand scoped direct DAG ${_mode}" "${VM}" "${CLI}" ${RUNTIME} -a
    --module scoped-dag.rxas --profile closed-main-default-halt.v1 ${_option}
    -o "${_product}.rxas" "${FIXTURES}/io-scope-reentry.rxas")
  file(READ "${_product}.rxas" _expanded)
  string(FIND "${_expanded}" "helper() .locals=" _callee)
  string(SUBSTRING "${_expanded}" 0 ${_callee} _caller)
  if(_caller MATCHES "call1 " OR NOT _caller MATCHES "scope[.]hidden[.][$]inline")
    message(FATAL_ERROR "Scoped caller did not expand with the skipped declaration: ${_last_stderr}")
  endif()
  string(REGEX MATCHALL "[.]meta \"scope[.]hidden[.][$]inline[^\"]*\"[^\n]*\n[ \t]*cnop" _hidden_carriers "${_caller}")
  list(LENGTH _hidden_carriers _hidden_carrier_count)
  if(NOT _hidden_carrier_count EQUAL 2)
    message(FATAL_ERROR "${_mode} moved a hidden scope off its distinct preceding CNOP")
  endif()
  # No new caller source row occurs after either original CALL. Its restored
  # source row must precede each original continuation result TRACE.
  string(REGEX MATCHALL "[.]srcstep 3 3 17 \"scoped-caller.rxas\"" _restored "${_caller}")
  list(LENGTH _restored _restores)
  if(_restores LESS 3)
    message(FATAL_ERROR "Caller continuation TRACE lost its original source state")
  endif()
  bound_pair("scope-${_mode}" "${_original}" "${_expanded}" "73\n41")
  readback("scope-${_mode}" "${_expanded}" 3)
endforeach()

string(REPLACE "  brf later,a1" "  .meta \"scope.numeric\"=\"b\" \".int\" r0\n  numsci 18,1,1\n  brf later,a1" _numeric "${_original}")
string(REPLACE "main() .locals=4" "main() .locals=4\n  numsci 18,1,1" _numeric "${_numeric}")
file(WRITE "${WORK}/numeric.rxas" "${_numeric}")
run_checked("preserve scoped NUMSCI address" "${VM}" "${CLI}" ${RUNTIME} -a
  --module scoped-numeric.rxas --profile closed-main-default-halt.v1 --no-cleanup
  -o "${WORK}/numeric-expanded.rxas" "${WORK}/numeric.rxas")
file(READ "${WORK}/numeric-expanded.rxas" _numeric_expanded)
if(NOT _numeric_expanded MATCHES "[.]meta \"scope[.]numeric[.][$]inline[^\"]*\"[^\n]*\n[ \t]*cnop")
  message(FATAL_ERROR "Scoped NUMSCI metadata drifted away from its own carrier")
endif()
bound_pair(scope-numeric "${_numeric}" "${_numeric_expanded}" "73\n41")

foreach(_negative same-address undefined changing argument)
  set(_source "${_original}")
  if(_negative STREQUAL same-address)
    string(REPLACE "  .meta \"scope.right\"=" "  .meta \"scope.right\"\n  .meta \"scope.right\"=" _source "${_source}")
    set(_reason metadata.scope.address.pending)
  elseif(_negative STREQUAL undefined)
    string(REPLACE "helper() .locals=2" "helper() .locals=3" _source "${_source}")
    string(REPLACE "\"scope.hidden\"=\"b\" \".int\" r0" "\"scope.hidden\"=\"b\" \".int\" r2" _source "${_source}")
    set(_reason metadata.runtime.observer.unproved)
  elseif(_negative STREQUAL changing)
    string(REPLACE "  ret r0\n" "  link r0,a1\n  ret r0\n" _source "${_source}")
    set(_reason metadata.binding.change.pending)
  else()
    string(REPLACE "\"scope.left\"=\"b\" \".int\" r0" "\"scope.left\"=\"b\" \".int\" a1" _source "${_source}")
    set(_reason metadata.scope.cfg.pending)
  endif()
  file(WRITE "${WORK}/${_negative}.rxas" "${_source}")
  run_checked("retain scoped ${_negative}" "${VM}" "${CLI}" ${RUNTIME} -a
    --module scoped-negative.rxas --profile closed-main-default-halt.v1 --no-cleanup
    -o "${WORK}/${_negative}-kept.rxas" "${WORK}/${_negative}.rxas")
  file(READ "${WORK}/${_negative}-kept.rxas" _kept)
  if(NOT _last_stderr MATCHES "${_reason}" OR NOT _kept MATCHES "call1 r1,helper[(][)],r0")
    message(FATAL_ERROR "Scoped ${_negative} lost its precise refusal: ${_last_stderr}")
  endif()
endforeach()
file(WRITE "${WORK}/last-good.rxas" "last-good\n")
execute_process(COMMAND "${VM}" "${CLI}" ${RUNTIME} -a --module scoped-limit.rxas
  --profile closed-main-default-halt.v1 --work 1 -o "${WORK}/last-good.rxas"
  "${FIXTURES}/io-scope-reentry.rxas"
  RESULT_VARIABLE _status OUTPUT_VARIABLE _stdout ERROR_VARIABLE _stderr TIMEOUT 60)
file(READ "${WORK}/last-good.rxas" _lastgood)
if(_status EQUAL 0 OR NOT _stderr MATCHES "LL_INLINE_LIMIT:" OR NOT _lastgood STREQUAL "last-good\n")
  message(FATAL_ERROR "Scoped resource refusal changed last-good output: ${_stdout}${_stderr}")
endif()
file(REMOVE_RECURSE "${WORK}")
message("RXAS_SCOPE_REENTRY|failures=0")

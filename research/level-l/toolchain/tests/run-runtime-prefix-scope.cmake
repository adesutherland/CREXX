cmake_minimum_required(VERSION 3.24)
foreach(_name VM ASSEMBLER CLI FIXTURES WORK CONSTRUCTED)
  if(NOT DEFINED ${_name})
    message(FATAL_ERROR "Missing ${_name}")
  endif()
endforeach()
file(MAKE_DIRECTORY "${WORK}")
include("${FIXTURES}/signal-pair-helpers.cmake")
run_checked("constructed runtime prefix scopes" "${VM}" "${CONSTRUCTED}" -a runtime-prefix-scope)
if(NOT _last_stdout MATCHES "RUNTIME_PREFIX_SCOPE[|]failures=0")
  message(FATAL_ERROR "Constructed prefix controls: ${_last_stdout}${_last_stderr}")
endif()

# The observer is appended only after transformation. Expected physical slots
# come from independently mapped executable operands: ICHKRNG a1, ICOPY a0
# and LOAD r0,91, never from the META whose payload is under test.
function(prefix_readback _stem _source _clones)
  string(FIND "${_source}" "helper() .locals=" _callee)
  string(SUBSTRING "${_source}" 0 ${_callee} _caller)
  if(_clones GREATER 0)
    string(REGEX MATCHALL "ichkrng r[0-9]+" _guards "${_caller}")
    string(REGEX MATCHALL "[.]srcstep [^\n]*\"prefix-body[.]rxas\" 20 [^\n]*\n[ \t]*icopy r[0-9]+,r[0-9]+" _counts "${_caller}")
    string(REGEX MATCHALL "load r[0-9]+,91" _locals "${_caller}")
    string(REGEX MATCHALL "[.]meta \"prefix[.]a1[.][$]inline[^\"]*\"" _names "${_caller}")
    list(REMOVE_DUPLICATES _names)
    foreach(_rows _guards _counts _locals _names)
      list(LENGTH ${_rows} _count)
      if(NOT _count EQUAL _clones)
        message(FATAL_ERROR "${_stem}: cannot identify independent prefix operands (${_rows})")
      endif()
    endforeach()
  endif()
  set(_dispatch "")
  set(_checks "")
  set(_marked "${_source}")
  set(_bit 1)
  set(_id 0)
  set(_expected_returns 2)
  foreach(_site RANGE 0 ${_clones})
    if(_site LESS _clones)
      list(GET _guards ${_site} _guard)
      string(REGEX REPLACE "ichkrng r([0-9]+)" "\\1" _a1 "${_guard}")
      list(GET _counts ${_site} _count)
      string(REGEX REPLACE "^.*icopy r[0-9]+,r([0-9]+)$" "\\1" _a0 "${_count}")
      string(REGEX REPLACE "^.*icopy r([0-9]+),r[0-9]+$" "\\1" _selected "${_count}")
      list(GET _locals ${_site} _local)
      string(REGEX REPLACE "load r([0-9]+),91" "\\1" _r "${_local}")
      list(GET _names ${_site} _name)
      string(REGEX REPLACE "[.]meta \"prefix[.]a1([^\"]*)\"" "\\1" _suffix "${_name}")
    else()
      # Native helper has five locals, zero globals, then a0 and a1.
      set(_a0 5)
      set(_a1 6)
      set(_r 0)
      set(_suffix "")
    endif()
    # Observer-only CONST markers are added after the inliner request. They
    # preserve source/TRACE and identify the binding address before execution.
    math(EXPR _site_mask "${_bit} * 15")
    if(_site LESS _clones)
      foreach(_choice RANGE 1 3)
        math(EXPR _actual "${_a1} + ${_choice}")
        set(_binding "link r${_selected},r${_actual}\n")
        string(REGEX MATCHALL "${_binding}" _bindings "${_marked}")
        list(LENGTH _bindings _binding_count)
        if(NOT _binding_count EQUAL 1)
          message(FATAL_ERROR "${_stem}: expected one case binding ${_binding}")
        endif()
        string(REPLACE "${_binding}" ".meta \"probe.binding\"=\"b\" \".int\" \"${_site_mask}\"\n${_binding}" _marked "${_marked}")
      endforeach()
      # The root's two independently identified captured result pointers
      # distinguish feasible RET commits from retained dead source rows.
      math(EXPR _root_result "4 + ${_site}")
      string(REGEX MATCHALL "link r[0-9]+,r${_root_result}\n" _result_links "${_caller}")
      list(LENGTH _result_links _result_count)
      if(NOT _result_count EQUAL 1)
        message(FATAL_ERROR "${_stem}: cannot identify captured result ${_site}")
      endif()
      list(GET _result_links 0 _result_link)
      string(REGEX REPLACE "link r([0-9]+),r${_root_result}\n" "\\1" _result "${_result_link}")
      math(EXPR _owned "${_selected} + 1")
      # A global count cannot let an optional conservative owned RET in case2
      # hide a missing mandatory RET in case1. Check the source-derived minimum
      # separately in each independently identified case binding region.
      foreach(_choice RANGE 1 3)
        math(EXPR _actual "${_a1} + ${_choice}")
        string(FIND "${_caller}" "link r${_selected},r${_actual}\n" _case_start)
        if(_choice LESS 3)
          math(EXPR _next_actual "${_actual} + 1")
          string(FIND "${_caller}" "link r${_selected},r${_next_actual}\n" _case_end)
        else()
          string(LENGTH "${_caller}" _case_end)
        endif()
        math(EXPR _case_length "${_case_end} - ${_case_start}")
        string(SUBSTRING "${_caller}" ${_case_start} ${_case_length} _case_source)
        string(REGEX MATCHALL "copy r${_result},r${_selected}\n" _borrowed "${_case_source}")
        string(REGEX MATCHALL "move r${_result},r${_owned}\n" _owned_returns "${_case_source}")
        list(LENGTH _borrowed _borrowed_count)
        list(LENGTH _owned_returns _owned_count)
        if(NOT _borrowed_count EQUAL 1 OR _owned_count GREATER 1 OR
            (_choice EQUAL 1 AND NOT _owned_count EQUAL 1))
          message(FATAL_ERROR "${_stem}: missing or duplicate feasible RET at site ${_site}, case ${_choice}")
        endif()
      endforeach()
      string(REGEX MATCHALL "(copy|move) r${_result},r(${_selected}|${_owned})\n" _returns "${_marked}")
      list(LENGTH _returns _return_count)
      if(_return_count LESS 4 OR _return_count GREATER 6 OR (_site EQUAL 0 AND NOT _return_count EQUAL 4))
        message(FATAL_ERROR "${_stem}: unexpected feasible return membership at site ${_site}")
      endif()
      math(EXPR _expected_returns "${_expected_returns} + ${_return_count}")
      list(REMOVE_DUPLICATES _returns)
      foreach(_return IN LISTS _returns)
        set(_line 4)
        if(_return MATCHES ",r${_selected}\n$")
          set(_line 2)
        endif()
        string(REPLACE "${_return}" ".meta \"probe.return${_line}\"=\"b\" \".int\" \"${_site_mask}\"\n${_return}" _marked "${_marked}")
      endforeach()
    else()
      string(REPLACE "linkarg r3,a1,1" ".meta \"probe.binding\"=\"b\" \".int\" \"${_site_mask}\"\nlinkarg r3,a1,1" _marked "${_marked}")
      string(REPLACE "ret r3\n" ".meta \"probe.return2\"=\"b\" \".int\" \"${_site_mask}\"\nret r3\n" _marked "${_marked}")
      string(REPLACE "ret r4\n" ".meta \"probe.return4\"=\"b\" \".int\" \"${_site_mask}\"\nret r4\n" _marked "${_marked}")
    endif()
    foreach(_kind r a0 a1 const closed)
      set(_symbol "prefix.${_kind}${_suffix}")
      set(_type ".int")
      set(_constant 0)
      set(_slot "${_r}")
      if(_kind STREQUAL a0)
        set(_slot "${_a0}")
      elseif(_kind STREQUAL a1)
        set(_slot "${_a1}")
      elseif(_kind STREQUAL const)
        set(_slot -1)
        set(_type ".string")
        set(_constant 1)
      endif()
      math(EXPR _inverse "~${_bit}")
      string(APPEND _dispatch "seq r4,r5,\"${_symbol}\"\nbrt prefix_${_id},r4\n")
      string(APPEND _checks "prefix_${_id}:\nload r18,${_bit}\nload r20,${_slot}\nload r22,\"${_type}\"\nload r24,${_constant}\nload r25,${_inverse}\nbr prefix_check\n")
      math(EXPR _bit "${_bit} * 2")
      math(EXPR _id "${_id} + 1")
    endforeach()
  endforeach()
  set(_probe [=[.globals=0
main() .locals=30
load r0,0
load r6,0
load r14,1
probe_next:
igt r4,r0,4096
brt probe_done,r4
icopy r9,r6
load r17,0
load r11,0
metaloaddata r1,r14,r0
load r2,1
probe_record:
igt r4,r2,r1
brt probe_advance,r4
linkattr1 r3,r1,r2
seq r13,r3,".meta_reg"
seq r23,r3,".meta_const"
brt probe_symbol,r13
brt probe_symbol,r23
seq r4,r3,".meta_clear"
brf probe_source,r4
probe_symbol:
linkattr1 r5,r3,1
seq r4,r5,"probe.binding"
brt probe_binding,r4
load r28,2
seq r4,r5,"probe.return2"
brt probe_commit,r4
load r28,4
seq r4,r5,"probe.return4"
brt probe_commit,r4
]=])
  string(APPEND _probe "${_dispatch}br probe_end_record\n${_checks}")
  string(APPEND _probe [=[prefix_check:
iand r4,r17,r18
brt probe_bad,r4
ior r17,r17,r18
ior r4,r13,r23
brt prefix_open,r4
iand r6,r6,r25
br probe_end_record
prefix_open:
ieq r4,r23,r24
brf probe_bad,r4
linkattr1 r5,r3,2
seq r4,r5,"b"
brf probe_bad,r4
linkattr1 r5,r3,3
seq r4,r5,r22
brf probe_bad,r4
linkattr1 r5,r3,4
brt prefix_constant,r23
ieq r4,r5,r20
br prefix_payload
prefix_constant:
seq r4,r5,"prefix-literal"
prefix_payload:
brf probe_bad,r4
ior r6,r6,r18
br probe_end_record
probe_binding:
linkattr1 r5,r3,4
scopy r10,r5
stoi r10
load r11,1
say "BINDING"
br probe_end_record
probe_commit:
linkattr1 r5,r3,4
scopy r29,r5
stoi r29
ieq r4,r29,r10
brf probe_bad,r4
ieq r4,r9,r29
brf probe_bad,r4
ieq r4,r6,r29
brf probe_bad,r4
seq r4,r15,"prefix-body.rxas"
brf probe_bad,r4
ieq r4,r16,r28
brf probe_bad,r4
say "RETURN"
br probe_end_record
probe_source:
seq r4,r3,".meta_source_step"
brf probe_trace,r4
linkattr1 r5,r3,4
scopy r15,r5
linkattr1 r5,r3,5
icopy r16,r5
seq r4,r15,"prefix-body.rxas"
brf probe_end_record,r4
ieq r4,r16,21
brf probe_return,r4
ieq r4,r9,r10
brf probe_bad,r4
ieq r4,r6,r10
brf probe_bad,r4
say "ENTRY"
br probe_end_record
probe_return:
ieq r4,r16,2
ieq r12,r16,4
ior r4,r4,r12
brf probe_end_record,r4
say "SOURCEROW"
ieq r4,r6,r10
brt probe_end_record,r4
ieq r4,r16,4
brf probe_bad,r4
ieq r4,r6,0
brf probe_bad,r4
ieq r4,r9,0
brf probe_bad,r4
say "DEADROW"
br probe_end_record
probe_trace:
seq r4,r3,".meta_trace_event"
brf probe_end_record,r4
linkattr1 r5,r3,10
seq r4,r5,"helper"
brf probe_end_record,r4
seq r4,r15,"scoped-caller.rxas"
brf probe_bad,r4
ieq r4,r16,3
brf probe_bad,r4
ieq r4,r6,0
brf probe_bad,r4
say "CONTINUATION"
probe_end_record:
unlink r5
unlink r3
iadd r2,r2,1
br probe_record
probe_advance:
brf probe_no_binding,r11
ieq r4,r17,0
brf probe_bad,r4
ieq r4,r9,r10
brf probe_bad,r4
ieq r4,r6,r10
brf probe_bad,r4
probe_no_binding:
iadd r0,r0,1
br probe_next
probe_bad:
signal "FAILURE","prefix metadata payload, transition order or source state differs"
ret 1
probe_done:
ret 0
]=])
  string(REPLACE ".globals=0" "" _sample "${_marked}")
  string(REPLACE "main() .locals=" "sample() .locals=" _sample "${_sample}")
  file(WRITE "${WORK}/${_stem}-probe.rxas" "${_probe}\n${_sample}")
  run_checked("assemble ${_stem} full prefix metadata" "${ASSEMBLER}" -n
    -o "${WORK}/${_stem}-probe" "${WORK}/${_stem}-probe.rxas")
  run_checked("read ${_stem} full prefix metadata" "${VM}" "${WORK}/${_stem}-probe.rxbin")
  string(REPLACE "\r\n" "\n" _observed "${_last_stdout}")
  string(REGEX MATCHALL "ENTRY\n" _entries "${_observed}")
  string(REGEX MATCHALL "RETURN\n" _returns "${_observed}")
  string(REGEX MATCHALL "CONTINUATION\n" _continuations "${_observed}")
  string(REGEX MATCHALL "BINDING\n" _bindings "${_observed}")
  string(REGEX MATCHALL "SOURCEROW\n" _source_returns "${_observed}")
  string(REGEX MATCHALL "DEADROW\n" _dead_returns "${_observed}")
  list(LENGTH _entries _entry_count)
  list(LENGTH _returns _return_count)
  list(LENGTH _continuations _continuation_count)
  list(LENGTH _bindings _binding_count)
  list(LENGTH _source_returns _source_count)
  list(LENGTH _dead_returns _dead_count)
  math(EXPR _expected_entries "1 + ${_clones} * 3")
  # Lexical source rows survive even where a case proves that RET infeasible.
  # Count actual independently identified commits separately from these rows.
  math(EXPR _expected_sources "2 + ${_clones} * 6")
  math(EXPR _expected_dead "${_expected_sources} - ${_expected_returns}")
  string(REGEX REPLACE "ENTRY\n|RETURN\n|CONTINUATION\n|BINDING\n|SOURCEROW\n|DEADROW\n" "" _remaining "${_observed}")
  string(STRIP "${_remaining}" _remaining)
  if(NOT _entry_count EQUAL _expected_entries OR NOT _return_count EQUAL _expected_returns OR
      NOT _binding_count EQUAL _expected_entries OR NOT _source_count EQUAL _expected_sources OR
      NOT _dead_count EQUAL _expected_dead OR NOT _continuation_count EQUAL 2 OR NOT _remaining STREQUAL "")
    message(FATAL_ERROR "${_stem}: native prefix address coverage differs: ${_observed}")
  endif()
endfunction()

file(READ "${FIXTURES}/io-runtime-prefix-scope.rxas" _original)
prefix_readback(prefix-original "${_original}" 0)
foreach(_mode conservative cleaned)
  set(_option "")
  if(_mode STREQUAL conservative)
    set(_option --no-cleanup)
  endif()
  set(_product "${WORK}/prefix-${_mode}")
  run_checked("expand runtime prefix scopes ${_mode}" "${VM}" "${CLI}" ${RUNTIME} -a
    --module runtime-prefix-scope.rxas --profile closed-main-default-halt.v1 ${_option}
    --work 12000000 -o "${_product}.rxas" "${FIXTURES}/io-runtime-prefix-scope.rxas")
  file(READ "${_product}.rxas" _expanded)
  string(FIND "${_expanded}" "helper() .locals=" _callee)
  string(SUBSTRING "${_expanded}" 0 ${_callee} _caller)
  if(_caller MATCHES "call4 " OR NOT _caller MATCHES "prefix[.]a1[.][$]inline")
    message(FATAL_ERROR "${_mode}: stable nonempty prefix did not expand: ${_last_stderr}")
  endif()
  bound_pair("prefix-${_mode}" "${_original}" "${_expanded}" "17\n26\nten\n256\n26")
  prefix_readback("prefix-${_mode}" "${_expanded}" 2)
endforeach()

foreach(_negative group changing)
  set(_source "${_original}")
  if(_negative STREQUAL group)
    string(REPLACE "  linkarg r3,a1,1" "  .srcstep 0 0 6 \"\" 0 0 0 \"\"\ngroup_alias:\n  linkarg r3,a1,1" _source "${_source}")
    set(_reason metadata.scope.linkarg.group.pending)
  else()
    string(REPLACE "  icopy r3,a0" "  link a1,a2\n  unlink a1\n  icopy r3,a0" _source "${_source}")
    set(_reason metadata.binding.change.pending)
  endif()
  file(WRITE "${WORK}/${_negative}.rxas" "${_source}")
  run_checked("retain runtime prefix ${_negative}" "${VM}" "${CLI}" ${RUNTIME} -a
    --module prefix-negative.rxas --profile closed-main-default-halt.v1 --no-cleanup
    --work 12000000 -o "${WORK}/${_negative}-kept.rxas" "${WORK}/${_negative}.rxas")
  file(READ "${WORK}/${_negative}-kept.rxas" _kept)
  if(NOT _last_stderr MATCHES "${_reason}" OR NOT _kept MATCHES "call4 r4,helper[(][)],r0,r1,r2,r3")
    message(FATAL_ERROR "Prefix ${_negative} lost its precise refusal: ${_last_stderr}")
  endif()
endforeach()
file(WRITE "${WORK}/last-good.rxas" "last-good\n")
execute_process(COMMAND "${VM}" "${CLI}" ${RUNTIME} -a --module prefix-limit.rxas
  --profile closed-main-default-halt.v1 --work 1 -o "${WORK}/last-good.rxas"
  "${FIXTURES}/io-runtime-prefix-scope.rxas"
  RESULT_VARIABLE _status OUTPUT_VARIABLE _stdout ERROR_VARIABLE _stderr TIMEOUT 60)
file(READ "${WORK}/last-good.rxas" _lastgood)
if(_status EQUAL 0 OR NOT _stderr MATCHES "LL_INLINE_LIMIT:" OR NOT _lastgood STREQUAL "last-good\n")
  message(FATAL_ERROR "Prefix resource refusal changed last-good output: ${_stdout}${_stderr}")
endif()
file(REMOVE_RECURSE "${WORK}")
message("RXAS_RUNTIME_PREFIX_SCOPE|failures=0")

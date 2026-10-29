cmake_minimum_required(VERSION 3.24)
foreach(_name VM ASSEMBLER CLI FIXTURES WORK CONSTRUCTED)
  if(NOT DEFINED ${_name})
    message(FATAL_ERROR "Missing ${_name}")
  endif()
endforeach()
file(MAKE_DIRECTORY "${WORK}")
include("${FIXTURES}/signal-pair-helpers.cmake")
set(_lane runtime-prefix-scope)
set(_fixture io-runtime-prefix-scope.rxas)
set(_marker RUNTIME_PREFIX_SCOPE)
if(GROUP_SCOPE)
  set(_lane runtime-group-scope)
  set(_fixture io-runtime-group-scope.rxas)
  set(_marker RUNTIME_GROUP_SCOPE)
endif()
run_checked("constructed ${_lane}" "${VM}" "${CONSTRUCTED}" -a "${_lane}")
if(NOT _last_stdout MATCHES "${_marker}[|]failures=0")
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
    string(REGEX MATCHALL "[.]srcstep [^\n]*\"prefix-body[.]rxas\" 20 [^\n]*\n([ \t]*null r[0-9]+\n)?[ \t]*icopy r[0-9]+,r[0-9]+" _counts "${_caller}")
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
  set(_group_setup "")
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
    set(_outgoing_mask ${_site_mask})
    set(_transitions 0)
    set(_kinds r a0 a1 const closed)
    if(GROUP_SCOPE)
      math(EXPR _outgoing_mask "${_bit} * 110")
      math(EXPR _transitions "${_bit} * 97")
      list(APPEND _kinds group_r group_const)
      set(_destination ${_selected})
      set(_selector ${_a1})
      set(_selector_bank 114)
      if(_site EQUAL _clones)
        set(_destination 3)
        set(_selector 1)
        set(_selector_bank 97)
      endif()
      string(APPEND _group_setup "probe_setup_${_site}:\nload r27,${_site_mask}\nload r10,${_outgoing_mask}\nload r32,${_destination}\nload r33,${_selector}\nload r34,${_selector_bank}\nload r36,${_transitions}\nload r35,0\nbr probe_end_record\n")
      string(APPEND _dispatch "seq r4,r5,\"probe.group.${_site}\"\nbrt probe_setup_${_site},r4\n")
      # A marker before the authored clear names the complete original group.
      string(REGEX MATCHALL "[.]meta \"prefix[.]r[^\"]*\"\n[ \t]*[.]meta \"group[.]r[^\"]*\"[^\n]*" _all_groups "${_marked}")
      set(_groups "")
      foreach(_group IN LISTS _all_groups)
        string(FIND "${_group}" ".meta \"prefix.r${_suffix}\"\n" _owner)
        if(_owner EQUAL 0)
          list(APPEND _groups "${_group}")
        endif()
      endforeach()
      list(REMOVE_DUPLICATES _groups)
      list(LENGTH _groups _group_count)
      if(NOT _group_count EQUAL 1)
        message(FATAL_ERROR "${_stem}: cannot identify the ordered original group ${_site}")
      endif()
      list(GET _groups 0 _group)
      string(REPLACE "${_group}" ".meta \"probe.group.${_site}\"=\"b\" \".int\" \"group\"\n${_group}" _marked "${_marked}")
    endif()
    if(_site LESS _clones)
      foreach(_choice RANGE 1 3)
        math(EXPR _actual "${_a1} + ${_choice}")
        set(_binding "link r${_selected},r${_actual}\n")
        string(REGEX MATCHALL "${_binding}" _bindings "${_marked}")
        list(LENGTH _bindings _binding_count)
        if(NOT _binding_count EQUAL 1)
          message(FATAL_ERROR "${_stem}: expected one case binding ${_binding}")
        endif()
        string(REPLACE "${_binding}" ".meta \"probe.binding\"=\"b\" \".int\" \"${_outgoing_mask}\"\n${_binding}" _marked "${_marked}")
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
        string(REPLACE "${_return}" ".meta \"probe.return${_line}\"=\"b\" \".int\" \"${_outgoing_mask}\"\n${_return}" _marked "${_marked}")
      endforeach()
    else()
      string(REPLACE "linkarg r3,a1,1" ".meta \"probe.binding\"=\"b\" \".int\" \"${_outgoing_mask}\"\nlinkarg r3,a1,1" _marked "${_marked}")
      string(REPLACE "ret r3\n" ".meta \"probe.return2\"=\"b\" \".int\" \"${_outgoing_mask}\"\nret r3\n" _marked "${_marked}")
      string(REPLACE "ret r4\n" ".meta \"probe.return4\"=\"b\" \".int\" \"${_outgoing_mask}\"\nret r4\n" _marked "${_marked}")
    endif()
    foreach(_kind IN LISTS _kinds)
      set(_symbol "prefix.${_kind}${_suffix}")
      set(_type ".int")
      set(_constant 0)
      set(_repeat 0)
      set(_role 0)
      set(_literal prefix-literal)
      if(_kind STREQUAL r)
        set(_role 1)
      endif()
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
      if(_kind STREQUAL group_r)
        set(_symbol "group.r${_suffix}")
        set(_repeat 1)
        set(_role 2)
      elseif(_kind STREQUAL group_const)
        set(_symbol "group.const${_suffix}")
        set(_slot -1)
        set(_type ".string")
        set(_constant 1)
        set(_literal binding-literal)
        set(_role 6)
      endif()
      math(EXPR _inverse "~${_bit}")
      string(APPEND _dispatch "seq r4,r5,\"${_symbol}\"\nbrt prefix_${_id},r4\n")
      string(APPEND _checks "prefix_${_id}:\nload r18,${_bit}\nload r20,${_slot}\nload r22,\"${_type}\"\nload r24,${_constant}\nload r25,${_inverse}\nload r26,${_repeat}\nload r37,${_role}\nload r21,\"${_literal}\"\nbr prefix_check\n")
      math(EXPR _bit "${_bit} * 2")
      math(EXPR _id "${_id} + 1")
    endforeach()
  endforeach()
  if(GROUP_SCOPE)
    foreach(_alias_pattern
        "(^|\n)[_A-Za-z0-9.]+:\n[ \t]*[.]meta \"probe[.]group"
        "[.]traceevent[^\n]*\"pre-destination\"[^\n]*\n[_A-Za-z0-9.]+:\n[ \t]*[.]meta \"group[.]const")
      string(REGEX MATCHALL "${_alias_pattern}" _alias_rows "${_marked}")
      list(LENGTH _alias_rows _alias_count)
      math(EXPR _expected_aliases "1 + ${_clones} * 3")
      if(NOT _alias_count EQUAL _expected_aliases)
        message(FATAL_ERROR "${_stem}: missing original binding alias labels")
      endif()
      foreach(_alias IN LISTS _alias_rows)
        string(REPLACE ":\n" ":\n.meta \"probe.alias\"=\"b\" \".string\" \"alias\"\n" _alias_marked "${_alias}")
        string(REPLACE "${_alias}" "${_alias_marked}" _marked "${_marked}")
      endforeach()
    endforeach()
  endif()
  set(_probe [=[.globals=0
main() .locals=45
load r0,0
load r6,0
load r14,1
probe_next:
igt r4,r0,4096
brt probe_done,r4
icopy r9,r6
load r17,0
load r11,0
load r35,-1
load r38,0
load r42,0
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
seq r4,r5,"probe.alias"
brt probe_alias,r4
seq r4,r5,"probe.binding"
brt probe_binding,r4
load r28,2
seq r4,r5,"probe.return2"
brt probe_commit,r4
load r28,4
seq r4,r5,"probe.return4"
brt probe_commit,r4
]=])
  string(APPEND _probe "${_dispatch}br probe_end_record\n${_checks}${_group_setup}")
  string(APPEND _probe [=[prefix_check:
iand r4,r17,r18
brf prefix_unique,r4
brf probe_bad,r26
prefix_unique:
ior r17,r17,r18
ilt r4,r35,0
brt prefix_ordered,r4
ieq r4,r37,1
brt group_clear,r4
ieq r4,r37,2
brt group_reg,r4
ieq r4,r37,6
brt group_const,r4
br probe_bad
group_clear:
ieq r4,r35,0
brf probe_bad,r4
ior r4,r13,r23
brt probe_bad,r4
load r35,1
br prefix_ordered
group_reg:
ieq r4,r35,1
ieq r12,r35,2
ior r4,r4,r12
brf probe_bad,r4
iadd r35,r35,1
br prefix_ordered
group_const:
ieq r4,r35,5
brf probe_bad,r4
load r35,6
prefix_ordered:
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
seq r4,r5,r21
prefix_payload:
brf probe_bad,r4
ior r6,r6,r18
br probe_end_record
probe_binding:
linkattr1 r5,r3,4
scopy r10,r5
stoi r10
load r11,1
ilt r4,r35,0
brf binding_staged,r4
icopy r27,r10
load r36,0
br binding_ready
binding_staged:
ieq r4,r35,7
brf probe_bad,r4
load r42,2
binding_ready:
say "BINDING"
br probe_end_record
probe_alias:
iadd r38,r38,1
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
seq r4,r15,"group-body.rxas"
brt probe_group_source,r4
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
probe_group_source:
ieq r4,r35,3
brf probe_bad,r4
ieq r4,r16,40
brf probe_bad,r4
linkattr1 r5,r3,1
icopy r30,r5
linkattr1 r5,r3,2
icopy r31,r5
ieq r4,r30,r31
brt probe_bad,r4
linkattr1 r5,r3,3
ieq r4,r5,17
brf probe_bad,r4
linkattr1 r5,r3,6
ieq r4,r5,2
brf probe_bad,r4
linkattr1 r5,r3,7
ieq r4,r5,19
brf probe_bad,r4
linkattr1 r5,r3,8
seq r4,r5,"bind selected item"
brf probe_bad,r4
load r35,4
say "GROUPSOURCE"
br probe_end_record
probe_trace:
seq r4,r3,".meta_trace_event"
brf probe_end_record,r4
linkattr1 r5,r3,10
seq r4,r5,"pre-destination"
brt probe_group_destination,r4
seq r4,r5,"pre-selector"
brt probe_group_selector,r4
seq r4,r5,"helper"
brf probe_end_record,r4
seq r4,r15,"scoped-caller.rxas"
brf probe_bad,r4
ieq r4,r16,3
brf probe_bad,r4
ieq r4,r6,0
brf probe_bad,r4
say "CONTINUATION"
br probe_end_record
probe_group_destination:
ieq r4,r35,4
brf probe_bad,r4
load r35,5
load r39,114
icopy r40,r32
load r41,"resolved-pre-destination"
br probe_group_trace
probe_group_selector:
ieq r4,r35,6
brf probe_bad,r4
load r35,7
icopy r39,r34
icopy r40,r33
load r41,"resolved-pre-selector"
probe_group_trace:
linkattr1 r5,r3,1
ieq r4,r5,86
brf probe_bad,r4
linkattr1 r5,r3,2
ieq r4,r5,6
brf probe_bad,r4
linkattr1 r5,r3,3
ieq r4,r5,82
brf probe_bad,r4
linkattr1 r5,r3,4
ieq r4,r5,73
brf probe_bad,r4
linkattr1 r5,r3,5
ieq r4,r5,r39
brf probe_bad,r4
linkattr1 r5,r3,6
ieq r4,r5,r40
brf probe_bad,r4
linkattr1 r5,r3,7
ieq r4,r5,r30
brf probe_bad,r4
linkattr1 r5,r3,8
ieq r4,r5,r31
brf probe_bad,r4
linkattr1 r5,r3,9
ieq r4,r5,0
brf probe_bad,r4
linkattr1 r5,r3,11
seq r4,r5,r41
brf probe_bad,r4
say "GROUPTRACE"
probe_end_record:
unlink r5
unlink r3
iadd r2,r2,1
br probe_record
probe_advance:
brf probe_no_binding,r11
ieq r4,r17,r36
brf probe_bad,r4
ieq r4,r9,r27
brf probe_bad,r4
ieq r4,r6,r10
brf probe_bad,r4
ieq r4,r38,r42
brf probe_bad,r4
load r38,0
probe_no_binding:
brt probe_bad,r38
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
  if(GROUP_SCOPE)
    string(REGEX MATCHALL "GROUPSOURCE\n" _group_sources "${_observed}")
    string(REGEX MATCHALL "GROUPTRACE\n" _group_traces "${_observed}")
    list(LENGTH _group_sources _group_source_count)
    list(LENGTH _group_traces _group_trace_count)
    math(EXPR _expected_traces "${_expected_entries} * 2")
    if(NOT _group_source_count EQUAL _expected_entries OR NOT _group_trace_count EQUAL _expected_traces)
      message(FATAL_ERROR "${_stem}: authored group duplicated or moved: ${_observed}")
    endif()
  endif()
  string(REGEX REPLACE "ENTRY\n|RETURN\n|CONTINUATION\n|BINDING\n|SOURCEROW\n|DEADROW\n|GROUPSOURCE\n|GROUPTRACE\n" "" _remaining "${_observed}")
  string(STRIP "${_remaining}" _remaining)
  if(NOT _entry_count EQUAL _expected_entries OR NOT _return_count EQUAL _expected_returns OR
      NOT _binding_count EQUAL _expected_entries OR NOT _source_count EQUAL _expected_sources OR
      NOT _dead_count EQUAL _expected_dead OR NOT _continuation_count EQUAL 2 OR NOT _remaining STREQUAL "")
    message(FATAL_ERROR "${_stem}: native prefix address coverage differs: ${_observed}")
  endif()
endfunction()

# Read the register named by each authored event at its actual boundary only
# after the inliner request. The test instrumentation never supplies a premise
# to facts or expansion, and does not replace native address/payload readback.
function(group_values _stem _source)
  set(_alias "${ARGV2}")
  string(REPLACE "\n" ";" _lines "${_source}")
  set(_observed "")
  set(_scratch "")
  foreach(_line IN LISTS _lines)
    if(_line MATCHES "^(main|helper)[(][)] [.]locals=([0-9]+)$")
      set(_scratch "r${CMAKE_MATCH_2}")
      math(EXPR _locals "${CMAKE_MATCH_2} + 1")
      set(_line "${CMAKE_MATCH_1}() .locals=${_locals}")
    endif()
    string(APPEND _observed "${_line}\n")
    if(_line MATCHES "^[ \t]*[.]traceevent \"V\" 6 \"R\" \"I\" \"([ra])\" ([0-9]+) [0-9]+ [0-9]+ 0 \"(pre-destination|pre-selector)\" \"resolved-[^\"]+\"$")
      set(_value "${CMAKE_MATCH_1}${CMAKE_MATCH_2}")
      set(_event "${CMAKE_MATCH_3}")
      string(APPEND _observed "icopy ${_scratch},${_value}\nitos ${_scratch}\nsay \"${_event}\"\nsay ${_scratch}\n")
    endif()
  endforeach()
  foreach(_choice first last failing)
    set(_sample "${_observed}")
    set(_expected "pre-destination\n3\npre-selector\n1\npre-destination\n3\npre-selector\n1\n17\n26\nten\n256\n26")
    if(_alias)
      set(_expected "pre-destination\n1\npre-selector\n1\npre-destination\n1\npre-selector\n1\n17\n24\nten\n256\n24")
    endif()
    if(_choice STREQUAL last)
      string(REPLACE "load r7,0x00" "load r7,0x01" _sample "${_sample}")
      string(REPLACE "load r8,0x00" "load r8,0x01" _sample "${_sample}")
      set(_expected "pre-destination\n3\npre-selector\n3\npre-destination\n3\npre-selector\n3\n39\n48\nthirty\n768\n11")
      if(_alias)
        set(_expected "pre-destination\n3\npre-selector\n3\npre-destination\n3\npre-selector\n3\n37\n44\nthirty\n768\n11")
      endif()
    elseif(_choice STREQUAL failing)
      string(REPLACE "iadd r0,r0,1" "iadd r0,r0,4" _sample "${_sample}")
    endif()
    set(_product "${WORK}/${_stem}-events-${_choice}")
    file(WRITE "${_product}.rxas" "${_sample}")
    run_checked("assemble ${_stem} pre-binding events ${_choice}" "${ASSEMBLER}" -n
      -o "${_product}" "${_product}.rxas")
    if(_choice STREQUAL failing)
      execute_process(COMMAND "${VM}" "${_product}.rxbin" RESULT_VARIABLE _status
        OUTPUT_VARIABLE _stdout ERROR_VARIABLE _stderr TIMEOUT 60)
      if(NOT _status MATCHES "^[1-9][0-9]*$" OR NOT _stderr MATCHES "SIGNAL OUT_OF_RANGE[)]" OR
          NOT _stdout STREQUAL "" OR _stderr MATCHES "bind selected item")
        message(FATAL_ERROR "Failing range check reached staged group: ${_status}\n${_stdout}${_stderr}")
      endif()
    else()
      run_checked("execute ${_stem} pre-binding events ${_choice}" "${VM}" "${_product}.rxbin")
      expect_output("${_expected}")
    endif()
  endforeach()
endfunction()

# Destination/selector alias with no authored source rows. Its native probe is
# deliberately independent of the nonempty source/prefix-mask observer above.
function(empty_alias_readback _stem _source _clones)
  set(_marked "${_source}")
  if(_clones GREATER 0)
    string(FIND "${_source}" "helper() .locals=" _end)
    string(SUBSTRING "${_source}" 0 ${_end} _caller)
    string(REGEX MATCHALL "ichkrng r[0-9]+" _guards "${_caller}")
    list(LENGTH _guards _guard_count)
    if(NOT _guard_count EQUAL _clones OR _caller MATCHES "call4 ")
      message(FATAL_ERROR "Alias/empty-source consumer did not expand both root sites")
    endif()
    foreach(_guard IN LISTS _guards)
      string(REGEX REPLACE "ichkrng r([0-9]+)" "\\1" _selector "${_guard}")
      foreach(_choice RANGE 1 3)
        math(EXPR _actual "${_selector} + ${_choice}")
        set(_binding "link r${_selector},r${_actual}\n")
        string(REGEX MATCHALL "${_binding}" _rows "${_marked}")
        list(LENGTH _rows _count)
        if(NOT _count EQUAL 1)
          message(FATAL_ERROR "Missing exact destination/selector alias binding")
        endif()
        string(REPLACE "${_binding}" ".meta \"probe.empty-binding\"=\"b\" \"114\" \"${_selector}\"\n${_binding}" _marked "${_marked}")
      endforeach()
    endforeach()
  endif()
  string(REPLACE "linkarg a1,a1,1" ".meta \"probe.empty-binding\"=\"b\" \"97\" \"1\"\nlinkarg a1,a1,1" _marked "${_marked}")
  set(_empty_rows 0)
  if(_clones GREATER 0)
    set(_empty_rows 1)
  endif()
  set(_source_checks "")
  foreach(_attribute 1 2 3 4 5 6 7 8)
    set(_value 0)
    if(_attribute EQUAL 3)
      set(_value 6)
    endif()
    string(APPEND _source_checks "linkattr1 r5,r3,${_attribute}\n")
    if(_attribute EQUAL 4 OR _attribute EQUAL 8)
      string(APPEND _source_checks "seq r4,r5,\"\"\n")
    else()
      string(APPEND _source_checks "ieq r4,r5,${_value}\n")
    endif()
    string(APPEND _source_checks "brf empty_bad,r4\n")
  endforeach()
  set(_trace_checks "")
  foreach(_attribute 1 2 3 4 7 8 9)
    set(_value 0)
    if(_attribute EQUAL 1)
      set(_value 86)
    elseif(_attribute EQUAL 2)
      set(_value 6)
    elseif(_attribute EQUAL 3)
      set(_value 82)
    elseif(_attribute EQUAL 4)
      set(_value 73)
    endif()
    string(APPEND _trace_checks "linkattr1 r5,r3,${_attribute}\nieq r4,r5,${_value}\nbrf empty_bad,r4\n")
  endforeach()
  # Continuations belong to the artifact's caller. Each binding marker then
  # chooses its own row count: r-bank clones have one synthetic empty row;
  # the retained a-bank helper has none. r18 snapshots the pre-event row count
  # for validation after the exact binding marker has identified that owner.
  set(_probe ".globals=0\nmain() .locals=25\nload r0,0\nload r14,1\nload r17,${_empty_rows}\n")
  string(APPEND _probe [=[empty_next:
igt r4,r0,4096
brt empty_done,r4
load r6,0
load r7,0
load r8,0
metaloaddata r1,r14,r0
load r2,1
empty_record:
igt r4,r2,r1
brt empty_advance,r4
linkattr1 r3,r1,r2
seq r4,r3,".meta_source_step"
brt empty_source,r4
seq r4,r3,".meta_trace_event"
brt empty_trace,r4
seq r4,r3,".meta_const"
brf empty_end_record,r4
linkattr1 r5,r3,1
seq r4,r5,"probe.empty-binding"
brf empty_end_record,r4
linkattr1 r5,r3,3
seq r4,r5,"97"
brt empty_original_marker,r4
seq r4,r5,"114"
brf empty_bad,r4
load r10,114
load r17,1
linkattr1 r5,r3,4
scopy r11,r5
stoi r11
ilt r4,r11,0
brt empty_bad,r4
icopy r12,r11
itos r12
seq r4,r5,r12
brf empty_bad,r4
br empty_marker_ready
empty_original_marker:
load r10,97
load r17,0
linkattr1 r5,r3,4
seq r4,r5,"1"
brf empty_bad,r4
load r11,1
empty_marker_ready:
load r7,1
br empty_end_record
empty_source:
]=])
  string(APPEND _probe "${_source_checks}iadd r8,r8,1\nbr empty_end_record\n")
  string(APPEND _probe [=[empty_trace:
linkattr1 r5,r3,10
seq r4,r5,"helper"
brt empty_continuation,r4
seq r4,r5,"pre-destination"
brt empty_destination,r4
seq r4,r5,"pre-selector"
brf empty_end_record,r4
ieq r4,r6,1
brf empty_bad,r4
load r6,2
linkattr1 r5,r3,5
icopy r22,r5
linkattr1 r5,r3,6
icopy r23,r5
load r24,"resolved-pre-selector"
br empty_payload
empty_destination:
ieq r4,r6,0
brf empty_bad,r4
icopy r18,r8
load r6,1
linkattr1 r5,r3,5
icopy r20,r5
linkattr1 r5,r3,6
icopy r21,r5
load r24,"resolved-pre-destination"
empty_payload:
]=])
  string(APPEND _probe "${_trace_checks}linkattr1 r5,r3,11\nseq r4,r5,r24\nbrf empty_bad,r4\nbr empty_end_record\n")
  string(APPEND _probe [=[empty_continuation:
ieq r4,r8,r17
brf empty_bad,r4
linkattr1 r5,r3,7
ieq r4,r5,0
brf empty_bad,r4
linkattr1 r5,r3,8
ieq r4,r5,0
brf empty_bad,r4
say "EMPTY_CONTINUATION"
empty_end_record:
unlink r5
unlink r3
iadd r2,r2,1
br empty_record
empty_advance:
brf empty_unbound,r7
ieq r4,r6,2
brf empty_bad,r4
ieq r4,r8,r17
brf empty_bad,r4
ieq r4,r18,r17
brf empty_bad,r4
ieq r4,r20,r10
brf empty_bad,r4
ieq r4,r22,r10
brf empty_bad,r4
ieq r4,r21,r11
brf empty_bad,r4
ieq r4,r23,r11
brf empty_bad,r4
say "EMPTY_BINDING"
load r6,0
empty_unbound:
brt empty_bad,r6
iadd r0,r0,1
br empty_next
empty_bad:
signal "FAILURE","alias TRACE or synthetic empty source differs"
ret 1
empty_done:
ret 0
]=])
  string(REPLACE ".globals=0" "" _sample "${_marked}")
  string(REPLACE "main() .locals=" "sample() .locals=" _sample "${_sample}")
  set(_product "${WORK}/${_stem}-empty-readback")
  file(WRITE "${_product}.rxas" "${_probe}\n${_sample}")
  run_checked("assemble alias empty-source readback" "${ASSEMBLER}" -n -o "${_product}" "${_product}.rxas")
  run_checked("read alias empty-source metadata" "${VM}" "${_product}.rxbin")
  string(REPLACE "\r\n" "\n" _observed "${_last_stdout}")
  string(REGEX MATCHALL "EMPTY_BINDING\n" _bindings "${_observed}")
  string(REGEX MATCHALL "EMPTY_CONTINUATION\n" _continuations "${_observed}")
  list(LENGTH _bindings _binding_count)
  list(LENGTH _continuations _continuation_count)
  math(EXPR _expected "1 + ${_clones} * 3")
  string(REGEX REPLACE "EMPTY_BINDING\n|EMPTY_CONTINUATION\n" "" _remaining "${_observed}")
  string(STRIP "${_remaining}" _remaining)
  if(NOT _binding_count EQUAL _expected OR NOT _continuation_count EQUAL 2 OR NOT _remaining STREQUAL "")
    message(FATAL_ERROR "Alias/empty-source address coverage differs: ${_observed}")
  endif()
endfunction()

file(READ "${FIXTURES}/${_fixture}" _original)
prefix_readback(prefix-original "${_original}" 0)
if(GROUP_SCOPE)
  group_values(group-original "${_original}")
endif()
foreach(_mode conservative cleaned)
  set(_option "")
  if(_mode STREQUAL conservative)
    set(_option --no-cleanup)
  endif()
  set(_product "${WORK}/prefix-${_mode}")
  run_checked("expand runtime prefix scopes ${_mode}" "${VM}" "${CLI}" ${RUNTIME} -a
    --module runtime-prefix-scope.rxas --profile closed-main-default-halt.v1 ${_option}
    --work 12000000 -o "${_product}.rxas" "${FIXTURES}/${_fixture}")
  file(READ "${_product}.rxas" _expanded)
  string(FIND "${_expanded}" "helper() .locals=" _callee)
  string(SUBSTRING "${_expanded}" 0 ${_callee} _caller)
  if(_caller MATCHES "call4 " OR NOT _caller MATCHES "prefix[.]a1[.][$]inline")
    message(FATAL_ERROR "${_mode}: stable nonempty prefix did not expand: ${_last_stderr}")
  endif()
  bound_pair("prefix-${_mode}" "${_original}" "${_expanded}" "17\n26\nten\n256\n26")
  prefix_readback("prefix-${_mode}" "${_expanded}" 2)
  if(GROUP_SCOPE)
    string(REGEX MATCHALL "[.]meta \"group[.](callable|inline)\"" _definition_rows "${_expanded}")
    list(LENGTH _definition_rows _definitions)
    if(NOT _definitions EQUAL 2 OR _caller MATCHES "group[.](callable|inline)")
      message(FATAL_ERROR "Callable or opaque inline META was cloned into cases")
    endif()
    group_values("group-${_mode}" "${_expanded}")
  endif()
endforeach()

if(GROUP_SCOPE)
  # The same handwritten consumer now binds into its selector slot. Removing
  # all authored source anchors makes both incoming and caller source empty;
  # native readback must see the explicit synthetic checkpoint on each clone.
  string(FIND "${_original}" "  .srcstep 21 21 17" _tail)
  string(SUBSTRING "${_original}" 0 ${_tail} _alias_original)
  string(APPEND _alias_original [=[  iadd a1,a1,7
  copy r4,a1
  unlink a1
  ret r4
  .meta "prefix.r"
  .meta "prefix.a0"
  .meta "prefix.a1"
  .meta "prefix.const"
  .meta "group.r"
  .meta "group.const"
]=])
  string(REPLACE "  .meta \"prefix.r\"\n  .meta \"group.r\"" "  .meta \"prefix.r\"\n  .meta \"prefix.a1\"\n  .meta \"group.r\"" _alias_original "${_alias_original}")
  string(REPLACE "linkarg r3,a1,1" "linkarg a1,a1,1" _alias_original "${_alias_original}")
  string(REPLACE "\"r\" 3 40 7 0 \"pre-destination\"" "\"a\" 1 40 7 0 \"pre-destination\"" _alias_original "${_alias_original}")
  string(REGEX REPLACE "(^|\n)[ \t]*[.]srcstep[^\n]*\n" "\\1" _alias_original "${_alias_original}")
  string(REPLACE " 40 7 0 \"pre-" " 0 0 0 \"pre-" _alias_original "${_alias_original}")
  foreach(_result 4 5)
    string(REPLACE "\"r\" ${_result} 3 3 0 \"helper\"" "\"r\" ${_result} 0 0 0 \"helper\"" _alias_original "${_alias_original}")
  endforeach()
  file(WRITE "${WORK}/alias-empty-original.rxas" "${_alias_original}")
  empty_alias_readback(alias-original "${_alias_original}" 0)
  group_values(alias-original "${_alias_original}" TRUE)
  foreach(_mode conservative cleaned)
    set(_option "")
    if(_mode STREQUAL conservative)
      set(_option --no-cleanup)
    endif()
    set(_output "${WORK}/alias-empty-${_mode}.rxas")
    run_checked("expand destination/selector alias with empty source ${_mode}" "${VM}" "${CLI}" ${RUNTIME} -a
      --module alias-empty.rxas --profile closed-main-default-halt.v1 ${_option}
      --work 12000000 -o "${_output}" "${WORK}/alias-empty-original.rxas")
    file(READ "${_output}" _alias_expanded)
    bound_pair("alias-empty-${_mode}" "${_alias_original}" "${_alias_expanded}" "17\n24\nten\n256\n24")
    empty_alias_readback("alias-${_mode}" "${_alias_expanded}" 2)
    group_values("alias-${_mode}" "${_alias_expanded}" TRUE)
  endforeach()
endif()

foreach(_negative changing)
  string(REPLACE "  icopy r3,a0" "  link a1,a2\n  unlink a1\n  icopy r3,a0" _source "${_original}")
  set(_reason metadata.binding.change.pending)
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
  "${FIXTURES}/${_fixture}"
  RESULT_VARIABLE _status OUTPUT_VARIABLE _stdout ERROR_VARIABLE _stderr TIMEOUT 60)
file(READ "${WORK}/last-good.rxas" _lastgood)
if(_status EQUAL 0 OR NOT _stderr MATCHES "LL_INLINE_LIMIT:" OR NOT _lastgood STREQUAL "last-good\n")
  message(FATAL_ERROR "Prefix resource refusal changed last-good output: ${_stdout}${_stderr}")
endif()
file(REMOVE_RECURSE "${WORK}")
message("RXAS_${_marker}|failures=0")

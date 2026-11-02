cmake_minimum_required(VERSION 3.24)
foreach(_required VM ASSEMBLER LINKER MUTATOR WORK)
  if(NOT DEFINED ${_required})
    message(FATAL_ERROR "Missing ${_required}")
  endif()
endforeach()
file(MAKE_DIRECTORY "${WORK}")
function(run_checked _name)
  execute_process(COMMAND ${ARGN} RESULT_VARIABLE _result
    OUTPUT_VARIABLE _stdout ERROR_VARIABLE _stderr TIMEOUT 30)
  if(NOT _result EQUAL 0)
    message(FATAL_ERROR "${_name}: ${_result}\n${_stdout}${_stderr}")
  endif()
endfunction()

foreach(_case direct nested stored failed)
  set(_case_dir "${WORK}/${_case}")
  file(MAKE_DIRECTORY "${_case_dir}")
  # Push the provider's procedure starts beyond the clone's physical range.
  # A metadata filter using the logical provider start would lose clone source.
  file(WRITE "${_case_dir}/provider.rxas" ".globals=0\nprefix() .locals=1\n")
  foreach(_padding RANGE 1 80)
    file(APPEND "${_case_dir}/provider.rxas" "  load r0,0\n")
  endforeach()
  file(APPEND "${_case_dir}/provider.rxas" "  ret\nhelper() .locals=0\n")
  if(_case STREQUAL "direct" OR _case STREQUAL "failed")
    file(APPEND "${_case_dir}/provider.rxas"
      "  .srcstep 1 1 17 \"helper-cause.rexx\" 31 1 12 \"raise helper\"\n  signal \"OTHER\",\"inline diagnostic fault\"\n  ret\n")
  else()
    file(APPEND "${_case_dir}/provider.rxas"
      "  .srcstep 1 1 17 \"helper-inner.rexx\" 32 1 10 \"call inner\"\n  call inner()\n  ret\n")
  endif()
  file(APPEND "${_case_dir}/provider.rxas" [=[child() .locals=0
  .srcstep 2 2 17 "child-cause.rexx" 51 1 11 "raise child"
  signal "OTHER","inline diagnostic fault"
  ret
inner() .locals=0
  .srcstep 3 3 17 "inner-child.rexx" 42 1 10 "call child"
  call child()
  ret
]=])
  run_checked("assemble ${_case} provider" "${ASSEMBLER}" -n "${_case_dir}/provider.rxas")
  set(_baseline_sources "")
  foreach(_lane baseline static)
    if(_case STREQUAL "failed" AND _lane STREQUAL "static")
      continue()
    endif()
    set(_dir "${_case_dir}/${_lane}")
    file(MAKE_DIRECTORY "${_dir}")
    file(WRITE "${_dir}/root.rxas" ".globals=1\nmain() .locals=3\n")
    if(_case STREQUAL "stored")
      file(APPEND "${_dir}/root.rxas" "  sigbrv captured,r1,\"OTHER\"\n")
    endif()
    file(APPEND "${_dir}/root.rxas"
      "  .srcstep 1 1 17 \"main-wrapper.rexx\" 10 1 12 \"call wrapper\"\n  call wrapper()\n  ret 99\n")
    if(_case STREQUAL "stored")
      file(APPEND "${_dir}/root.rxas" [=[captured:
  copy g0,r1
  null r1
  sighalt "OTHER"
  .srcstep 2 2 17 "rethrow-site.rexx" 90 1 7 "rethrow"
  linkattr1 r2,g0,5
  linkattr1 r0,r2,5
  signalorigin "OTHER",r0,g0
  ret 98
]=])
    endif()
    file(APPEND "${_dir}/root.rxas" "wrapper() .locals=12\n")
    if(_case STREQUAL "failed")
      file(APPEND "${_dir}/root.rxas"
        "  load r1,-1\n  sigignore \"FAILURE\"\n  sigignore \"OUT_OF_RANGE\"\n")
    endif()
    file(APPEND "${_dir}/root.rxas"
      "  .srcstep 3 3 17 \"wrapper-helper.rexx\" 20 1 11 \"call helper\"\n")
    if(_lane STREQUAL "baseline")
      if(_case STREQUAL "failed")
        file(APPEND "${_dir}/root.rxas" "  call r0,@1::1(),r1\n")
      else()
        file(APPEND "${_dir}/root.rxas" "  call @1::1()\n")
      endif()
    elseif(_lane STREQUAL "static")
      file(APPEND "${_dir}/root.rxas"
        "  .inlineframe 1 0 \"${_case_dir}/provider.rxas\" \"helper\" 3\nstatic_helper_begin:\n")
      if(_case STREQUAL "direct")
        file(APPEND "${_dir}/root.rxas"
          "  .srcstep 4 4 17 \"helper-cause.rexx\" 31 1 12 \"raise helper\"\n  signal \"OTHER\",\"inline diagnostic fault\"\n  br after_helper\nafter_helper:\n  .inlinemap static_helper_begin,after_helper,1\n")
      else()
        # The retained child returns into a physical static clone. Its saved
        # return PC, rather than a current raw event origin, selects frame 2.
        file(APPEND "${_dir}/root.rxas"
          "  .srcstep 4 4 17 \"helper-inner.rexx\" 32 1 10 \"call inner\"\n  .inlineframe 2 1 \"${_case_dir}/provider.rxas\" \"inner\" 4\nstatic_inner_begin:\n  .srcstep 5 5 17 \"inner-child.rexx\" 42 1 10 \"call child\"\n  call @1::2()\n  br after_inner\nafter_inner:\n  br after_helper\nafter_helper:\n  .inlinemap static_inner_begin,after_inner,2\n  .inlinemap after_inner,after_helper,1\n")
      endif()
    endif()
    if(_case STREQUAL "failed")
      file(APPEND "${_dir}/root.rxas"
        "  .srcstep 6 6 17 \"wrapper-failed.rexx\" 21 1 10 \"after fail\"\n  signal \"OTHER\",\"inline diagnostic fault\"\n")
    endif()
    file(APPEND "${_dir}/root.rxas" "  ret\n")
    run_checked("assemble ${_case} ${_lane}" "${ASSEMBLER}" -n "${_dir}/root.rxas")
    run_checked("seal ${_case} ${_lane}" "${MUTATOR}" join-seal "${_dir}/image.rxbin"
      "${_dir}/root.rxbin" "${_case_dir}/provider.rxbin")
    execute_process(COMMAND "${VM}" "${_dir}/image.rxbin"
      RESULT_VARIABLE _result OUTPUT_VARIABLE _stdout ERROR_VARIABLE _diagnostic TIMEOUT 30)
    string(REPLACE "\r\n" "\n" _diagnostic "${_diagnostic}")
    if(NOT "${_result}" STREQUAL "30" OR NOT _stdout STREQUAL "" OR
       NOT _diagnostic MATCHES "PANIC: inline diagnostic fault [(]SIGNAL OTHER[)]")
      message(FATAL_ERROR "${_case} ${_lane} signal mismatch: ${_result}\n${_stdout}${_diagnostic}")
    endif()
    string(REGEX MATCHALL "PANIC:" _panics "${_diagnostic}")
    list(LENGTH _panics _panic_count)
    string(REGEX MATCHALL "called from:" _callers "${_diagnostic}")
    list(LENGTH _callers _caller_count)
    string(REGEX MATCHALL "source: [^\n]*" _sources "${_diagnostic}")
    if(_case STREQUAL "direct")
      set(_expected_sources "helper-cause.rexx:31:1" "wrapper-helper.rexx:20:1" "main-wrapper.rexx:10:1")
      set(_expected_callers 2)
    elseif(_case STREQUAL "failed")
      set(_expected_sources "wrapper-failed.rexx:21:1" "main-wrapper.rexx:10:1")
      set(_expected_callers 1)
    else()
      set(_expected_sources "child-cause.rexx:51:1" "inner-child.rexx:42:1" "helper-inner.rexx:32:1"
        "wrapper-helper.rexx:20:1" "main-wrapper.rexx:10:1")
      set(_expected_callers 4)
    endif()
    list(LENGTH _sources _source_count)
    list(LENGTH _expected_sources _expected_source_count)
    if(NOT _panic_count EQUAL 1 OR NOT _caller_count EQUAL _expected_callers OR
       NOT _source_count EQUAL _expected_source_count OR _diagnostic MATCHES "rethrow-site[.]rexx|unavailable|FAIL:")
      message(FATAL_ERROR "${_case} ${_lane} duplicated/lost caller context\n${_diagnostic}")
    endif()
    set(_index 0)
    foreach(_source IN LISTS _expected_sources)
      list(GET _sources ${_index} _actual)
      string(FIND "${_actual}" "${_source}" _found)
      if(_found LESS 0)
        message(FATAL_ERROR "${_case} ${_lane} caller order mismatch: ${_actual}, wanted ${_source}")
      endif()
      math(EXPR _index "${_index} + 1")
    endforeach()
    if(_case STREQUAL "failed" AND
       _diagnostic MATCHES "logical origin: helper|logical caller: helper")
      message(FATAL_ERROR "failed ordinary entry invented a logical frame\n${_diagnostic}")
    endif()
    if(_lane STREQUAL "baseline")
      set(_baseline_sources "${_sources}")
    else()
      if(NOT _baseline_sources STREQUAL _sources)
        message(FATAL_ERROR "${_case} authored source chain differs from ordinary calls\n${_diagnostic}")
      endif()
      if(_case STREQUAL "direct")
        if(NOT _diagnostic MATCHES "at module 1 [(][^\n]*root[.]rxas[)]" OR
           NOT _diagnostic MATCHES "logical origin: helper [(]module [^\n]*provider[.]rxas[)]")
          message(FATAL_ERROR "raw physical origin and logical provider were conflated\n${_diagnostic}")
        endif()
      elseif(_case STREQUAL "nested" OR _case STREQUAL "stored")
        if(NOT _diagnostic MATCHES "logical caller: inner [(]module [^\n]*provider[.]rxas[)]" OR
           NOT _diagnostic MATCHES "logical caller: helper [(]module [^\n]*provider[.]rxas[)]")
          message(FATAL_ERROR "${_case} lost detached logical provider labels\n${_diagnostic}")
        endif()
      endif()
    endif()
  endforeach()
endforeach()

# Names/maps survive source stripping without resolving a provider by name.
# A second call after the mapped clone proves that its ended range contributes
# no logical frame; stored rethrow must keep the first event's detached chain.
foreach(_case direct retained ended stored)
  set(_dir "${WORK}/stripped-${_case}")
  file(MAKE_DIRECTORY "${_dir}")
  file(WRITE "${_dir}/root.rxas" ".globals=1\nmain() .locals=3\n")
  if(_case STREQUAL "stored")
    file(APPEND "${_dir}/root.rxas" "  sigbrv captured,r1,\"OTHER\"\n")
  endif()
  file(APPEND "${_dir}/root.rxas"
    "  .srcstep 1 1 17 \"strip-main.rexx\" 10 1 12 \"call wrapper\"\n  call wrapper()\n  ret 99\n")
  if(_case STREQUAL "stored")
    file(APPEND "${_dir}/root.rxas" [=[captured:
  copy g0,r1
  null r1
  sighalt "OTHER"
  .srcstep 2 2 17 "strip-rethrow.rexx" 90 1 7 "rethrow"
  linkattr1 r2,g0,5
  linkattr1 r0,r2,5
  signalorigin "OTHER",r0,g0
  ret 98
]=])
  endif()
  file(APPEND "${_dir}/root.rxas" [=[wrapper() .locals=1
  .srcstep 3 3 17 "strip-wrapper.rexx" 20 1 11 "call clone"
  .inlineframe 7 0 "diagnostic-only-provider" "clone" 3
clone_begin:
  .srcstep 4 4 17 "strip-clone.rexx" 30 1 10 "clone body"
]=])
  if(_case STREQUAL "direct" OR _case STREQUAL "stored")
    file(APPEND "${_dir}/root.rxas" "  signal \"OTHER\",\"inline diagnostic fault\"\n")
  elseif(_case STREQUAL "retained")
    file(APPEND "${_dir}/root.rxas" "  call child()\n")
  else()
    file(APPEND "${_dir}/root.rxas" "  load r0,1\n")
  endif()
  file(APPEND "${_dir}/root.rxas" "clone_end:\n  .inlinemap clone_begin,clone_end,7\n")
  if(_case STREQUAL "ended")
    file(APPEND "${_dir}/root.rxas" "  call child()\n")
  endif()
  file(APPEND "${_dir}/root.rxas" [=[  ret
child() .locals=0
  .srcstep 5 5 17 "strip-child.rexx" 40 1 11 "raise child"
  signal "OTHER","inline diagnostic fault"
  ret
]=])
  run_checked("assemble strip ${_case}" "${ASSEMBLER}" -n "${_dir}/root.rxas")
  file(WRITE "${_dir}/strip.control"
    "INPUT \"${_dir}/root.rxbin\"\nOUTPUT \"${_dir}/stripped.rxbin\"\nSTRIP SOURCE\nSTRIP INLINE\n")
  run_checked("strip ${_case}" "${LINKER}" -c "${_dir}/strip.control")
  execute_process(COMMAND "${VM}" "${_dir}/stripped.rxbin"
    RESULT_VARIABLE _result OUTPUT_VARIABLE _stdout ERROR_VARIABLE _diagnostic TIMEOUT 30)
  string(REPLACE "\r\n" "\n" _diagnostic "${_diagnostic}")
  string(REGEX MATCHALL "PANIC:" _panics "${_diagnostic}")
  list(LENGTH _panics _panic_count)
  string(REGEX MATCHALL "called from:" _callers "${_diagnostic}")
  list(LENGTH _callers _caller_count)
  if(_case STREQUAL "direct" OR _case STREQUAL "stored")
    set(_expected_callers 2)
    set(_logical_label "logical origin: clone")
  elseif(_case STREQUAL "retained")
    set(_expected_callers 3)
    set(_logical_label "logical caller: clone")
  else()
    set(_expected_callers 2)
    set(_logical_label "")
  endif()
  if(NOT "${_result}" STREQUAL "30" OR NOT _stdout STREQUAL "" OR
     NOT _panic_count EQUAL 1 OR NOT _caller_count EQUAL _expected_callers OR
     _diagnostic MATCHES "source:|unavailable|FAIL:")
    message(FATAL_ERROR "stripped ${_case} lost diagnostic structure: ${_result}\n${_stdout}${_diagnostic}")
  endif()
  if(_logical_label STREQUAL "")
    if(_diagnostic MATCHES "logical origin:|logical caller:")
      message(FATAL_ERROR "ended static range invented a logical frame\n${_diagnostic}")
    endif()
  elseif(NOT _diagnostic MATCHES "${_logical_label} [(]module diagnostic-only-provider[)]")
    message(FATAL_ERROR "stripped ${_case} lost static name\n${_diagnostic}")
  endif()
endforeach()
file(REMOVE_RECURSE "${WORK}")
message("INLINE_SIGNAL_DIAGNOSTICS|failures=0")

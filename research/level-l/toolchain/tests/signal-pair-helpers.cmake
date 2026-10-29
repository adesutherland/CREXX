# Shared ordinary RXAS signal test processes, pair checks and source observer.
include_guard(GLOBAL)

function(run_checked _description)
  execute_process(COMMAND ${ARGN} RESULT_VARIABLE _status
    OUTPUT_VARIABLE _stdout ERROR_VARIABLE _stderr TIMEOUT 60)
  if(NOT _status EQUAL 0)
    message(FATAL_ERROR "${_description}: ${_status}\n${_stdout}${_stderr}")
  endif()
  set(_last_stdout "${_stdout}" PARENT_SCOPE)
  set(_last_stderr "${_stderr}" PARENT_SCOPE)
endfunction()

function(expect_output _expected)
  string(REPLACE "\r\n" "\n" _normalized "${_last_stdout}")
  string(STRIP "${_normalized}" _actual)
  if(NOT _actual STREQUAL _expected)
    message(FATAL_ERROR "Expected '${_expected}', got '${_actual}'")
  endif()
endfunction()

function(expect_bound_policy _original _expanded)
  string(REGEX MATCHALL "(sigpush|sigbrv|sigpop)[ \t]+[^\n]+" _before "${_original}")
  string(REGEX MATCHALL "(sigpush|sigbrv|sigpop)[ \t]+[^\n]+" _after "${_expanded}")
  list(LENGTH _after _count)
  if(NOT _before STREQUAL _after OR NOT _count EQUAL 116)
    message(FATAL_ERROR "Bound scope changed its 116 executable policy rows")
  endif()
endfunction()

function(bound_pair _stem _original _expanded _expected)
  foreach(_kind original expanded)
    set(_product "${WORK}/${_stem}-${_kind}")
    file(WRITE "${_product}.rxas" "${_${_kind}}")
    run_checked("assemble ${_stem} ${_kind}" "${ASSEMBLER}" -n -o "${_product}" "${_product}.rxas")
    run_checked("execute ${_stem} ${_kind}" "${VM}" "${_product}.rxbin")
    expect_output("${_expected}")
  endforeach()
endfunction()

function(bound_keep _stem _source _reason)
  set(_input "${WORK}/bound-${_stem}.rxas")
  set(_output "${WORK}/bound-${_stem}-kept.rxas")
  file(WRITE "${_input}" "${_source}")
  run_checked("retain bound ${_stem}" "${VM}" "${CLI}" ${RUNTIME} -a
    --module bound.rxas --profile closed-main-default-halt.v1 --no-cleanup --work 12000000
    -o "${_output}" "${_input}")
  file(READ "${_output}" _kept)
  if(NOT _last_stderr MATCHES "${_reason}" OR NOT _kept MATCHES "call1 r1,checked_identity[(][)],r0")
    message(FATAL_ERROR "Bound ${_stem} lost its precise retained call: ${_last_stderr}")
  endif()
  set(_bound_kept "${_kept}" PARENT_SCOPE)
endfunction()

# Test-only metadata-owner lookup; never an inliner premise.
set(_observer "\nobserve_origin() .locals=5\nmetaloaddata r0,a1,a2\nload r1,1\norigin_next:\nigt r3,r1,r0\nbrt origin_missing,r3\nlinkattr1 r2,r0,r1\nseq r3,r2,\".meta_source_step\"\nbrt origin_found,r3\nunlink r2\niadd r1,r1,1\nbr origin_next\norigin_found:\nlinkattr1 r4,r2,4\nsay r4\nunlink r4\nlinkattr1 r4,r2,5\nicopy r3,r4\nitos r3\nsay r3\nunlink r4\nlinkattr1 r4,r2,6\nicopy r3,r4\nitos r3\nsay r3\nunlink r4\nlinkattr1 r4,r2,8\nsay r4\nunlink r4\nunlink r2\nret 0\norigin_missing:\nsignal \"FAILURE\",\"missing authored event origin\"\nret 1\n")

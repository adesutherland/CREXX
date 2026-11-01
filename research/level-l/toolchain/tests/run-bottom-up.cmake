cmake_minimum_required(VERSION 3.24)
foreach(_name VM ASSEMBLER CLI FIXTURES WORK)
  if(NOT DEFINED ${_name})
    message(FATAL_ERROR "Missing ${_name}")
  endif()
endforeach()
file(MAKE_DIRECTORY "${WORK}")
include("${FIXTURES}/signal-pair-helpers.cmake")

foreach(_case chain case recursive)
  set(_fixture "${FIXTURES}/io-bottom-up.rxas")
  if(_case STREQUAL case)
    set(_fixture "${FIXTURES}/io-bottom-up-case.rxas")
  endif()
  if(_case STREQUAL recursive)
    set(_fixture "${FIXTURES}/io-bottom-up-recursive.rxas")
  endif()
  set(_product "${WORK}/${_case}")
  run_checked("assemble ${_case} original" "${ASSEMBLER}" -n -o "${_product}-original" "${_fixture}")
  run_checked("execute ${_case} original" "${VM}" "${_product}-original.rxbin")
  run_checked("inline ${_case} bottom-up" "${VM}" "${CLI}" ${RUNTIME} -a
    --module "bottom-up-${_case}.rxas" --profile closed-main-default-halt.v1
    --no-cleanup --work 12000000 -o "${_product}.rxas" "${_fixture}")
  file(READ "${_product}.rxas" _expanded)
  if(_expanded MATCHES "(^|\n)[ \t]*inlenter[ \t]")
    message(FATAL_ERROR "${_case} default path introduced an allocating inline scope")
  endif()
  string(REGEX MATCHALL "(^|\n)[ \t]*call[0-9]*[ \t]+[^\n]+" _calls "${_expanded}")
  list(LENGTH _calls _count)
  if(_case STREQUAL chain OR _case STREQUAL case)
    if(NOT _count EQUAL 0)
      message(FATAL_ERROR "Bottom-up chain retained a call or copied a stale provider: ${_calls}\n${_expanded}")
    endif()
  else()
    if(NOT _count EQUAL 2 OR NOT _last_stderr MATCHES "policy[.]recursive[.]scc")
      message(FATAL_ERROR "Recursive SCC was expanded or lost its policy reason: ${_count}\n${_last_stderr}\n${_expanded}")
    endif()
  endif()
  run_checked("assemble ${_case} bottom-up" "${ASSEMBLER}" -n -o "${_product}" "${_product}.rxas")
  run_checked("execute ${_case} bottom-up" "${VM}" "${_product}.rxbin")
endforeach()

set(_lastgood "${WORK}/last-good.rxas")
file(WRITE "${_lastgood}" "last-good-bottom-up\n")
execute_process(COMMAND "${VM}" "${CLI}" ${RUNTIME} -a
  --module bottom-up-limit.rxas --profile closed-main-default-halt.v1
  --work 1 -o "${_lastgood}" "${FIXTURES}/io-bottom-up.rxas"
  RESULT_VARIABLE _status OUTPUT_VARIABLE _stdout ERROR_VARIABLE _stderr TIMEOUT 30)
file(READ "${_lastgood}" _after)
if(_status EQUAL 0 OR NOT _stderr MATCHES "inline[.]callgraph[.]" OR NOT _after STREQUAL "last-good-bottom-up\n")
  message(FATAL_ERROR "Bottom-up work limit changed last-good output: ${_status}\n${_stderr}\n${_after}")
endif()
# Disabled inlining reaches the original normalization budget without paying
# for the optional call graph, even when that graph would exceed the allowance.
execute_process(COMMAND "${VM}" "${CLI}" ${RUNTIME} -a
  --module bottom-up-disabled.rxas --no-inline --work 1
  -o "${_lastgood}" "${FIXTURES}/io-bottom-up.rxas"
  RESULT_VARIABLE _status OUTPUT_VARIABLE _stdout ERROR_VARIABLE _stderr TIMEOUT 30)
file(READ "${_lastgood}" _after)
if(_status EQUAL 0 OR NOT _stderr MATCHES "LL_INLINE_NORMALIZATION_LIMIT"
    OR _stderr MATCHES "inline[.]callgraph[.]" OR NOT _after STREQUAL "last-good-bottom-up\n")
  message(FATAL_ERROR "Disabled inlining paid for the call graph or changed last-good output: ${_status}\n${_stderr}\n${_after}")
endif()
message("RXAS_BOTTOM_UP|failures=0")

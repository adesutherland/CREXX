cmake_minimum_required(VERSION 3.24)
foreach(_name VM ASSEMBLER CLI FIXTURES WORK)
  if(NOT DEFINED ${_name})
    message(FATAL_ERROR "Missing ${_name}")
  endif()
endforeach()
file(MAKE_DIRECTORY "${WORK}")
include("${FIXTURES}/signal-pair-helpers.cmake")
file(READ "${FIXTURES}/io-loose-predicates.rxas" _original)
set(_inputs "01" "02.00 " "003.000" "banana")
set(_results 501 502 503 -500)
foreach(_case RANGE 0 3)
  list(GET _inputs ${_case} _input)
  list(GET _results ${_case} _result)
  string(REPLACE "load r0,\"01\"" "load r0,\"${_input}\"" _source "${_original}")
  set(_source_path "${WORK}/input-${_case}.rxas")
  file(WRITE "${_source_path}" "${_source}")
  foreach(_mode conservative cleaned)
    set(_option "")
    if(_mode STREQUAL conservative)
      set(_option --no-cleanup)
    endif()
    set(_product "${WORK}/loose-${_case}-${_mode}")
    run_checked("expand loose equality ${_case}/${_mode}" "${VM}" "${CLI}" ${RUNTIME} -a
      --module loose-predicates.rxas --profile closed-main-default-halt.v1 ${_option}
      -o "${_product}.rxas" "${_source_path}")
    file(READ "${_product}.rxas" _expanded)
    string(FIND "${_expanded}" "numeric_string() .locals=" _end)
    string(SUBSTRING "${_expanded}" 0 ${_end} _caller)
    if(_caller MATCHES "(^|\n)[ \t]*(call[1234]?|ichkrng|seq|sne|rseq)[ \t]")
      message(FATAL_ERROR "Loose equality retained a call or added a guard/strict comparator")
    endif()
    if(_mode STREQUAL conservative)
      string(REGEX MATCHALL "(^|\n)[ \t]*req " _req "${_caller}")
      string(REGEX MATCHALL "(^|\n)[ \t]*rne " _rne "${_caller}")
      list(LENGTH _req _req_count)
      list(LENGTH _rne _rne_count)
      if(NOT _req_count EQUAL 3 OR NOT _rne_count EQUAL 1)
        message(FATAL_ERROR "Loose predicates were folded or replaced before execution")
      endif()
      string(REGEX MATCH "[.]srcstep ([0-9]+) ([0-9]+) 17 \"select_dispatch_strings[.]crexx\" 83 3 17 [^\n]*\n[ \t]*[.]traceevent \"V\" 6 \"R\" \"S\" \"r\" ([0-9]+) ([0-9]+) ([0-9]+) 0 \"value\" \"\"\n[ \t]*req r([0-9]+),r([0-9]+),\"1\"" _comparison "${_caller}")
      if(NOT _comparison OR NOT CMAKE_MATCH_1 STREQUAL CMAKE_MATCH_4 OR
          NOT CMAKE_MATCH_2 STREQUAL CMAKE_MATCH_5 OR NOT CMAKE_MATCH_3 STREQUAL CMAKE_MATCH_7)
        message(FATAL_ERROR "Golden REQ lost source/TRACE or its incoming string slot")
      endif()
      set(_event ".traceevent \"O\" 4 \"R\" \"B\" \"r\" ${CMAKE_MATCH_6} ${CMAKE_MATCH_1} ${CMAKE_MATCH_2} 0 \"\" \"\"")
      string(FIND "${_caller}" "${_event}" _event_at)
      if(_event_at LESS 0)
        message(FATAL_ERROR "REQ result event lost the predicate destination")
      endif()
      string(REGEX MATCH "rne r([0-9]+),r([0-9]+),\"different\"" _alias "${_caller}")
      if(NOT _alias OR NOT CMAKE_MATCH_1 STREQUAL CMAKE_MATCH_2)
        message(FATAL_ERROR "RNE no longer writes through the original actual alias")
      endif()
    endif()
    bound_pair("loose-${_case}-${_mode}" "${_source}" "${_expanded}"
      "${_result}\n1\nkeep\n512\n1\nkeep\n512")
  endforeach()
endforeach()

# Independent booleans in pair order: trailing nonnumeric space 1/0,
# leading nonnumeric space 0/1, empty/spaces 1/0, byte-distinct Unicode 0/1,
# embedded NUL 0/1, and a 257-byte numeric candidate 1/0. The last input is
# longer than the converter's 128-byte local buffer, without forcing failure.
file(READ "${FIXTURES}/io-loose-boundaries.rxas" _boundaries)
foreach(_mode conservative cleaned)
  set(_option "")
  if(_mode STREQUAL conservative)
    set(_option --no-cleanup)
  endif()
  set(_product "${WORK}/boundaries-${_mode}")
  run_checked("expand loose boundary controls ${_mode}" "${VM}" "${CLI}" ${RUNTIME} -a
    --module loose-boundaries.rxas --profile closed-main-default-halt.v1 ${_option}
    -o "${_product}.rxas" "${FIXTURES}/io-loose-boundaries.rxas")
  file(READ "${_product}.rxas" _boundary_expanded)
  string(FIND "${_boundary_expanded}" "space_pairs() .locals=" _end)
  string(SUBSTRING "${_boundary_expanded}" 0 ${_end} _boundary_caller)
  if(_boundary_caller MATCHES "(^|\n)[ \t]*(call[1234]?|ichkrng|seq|sne|rseq)[ \t]")
    message(FATAL_ERROR "Boundary controls retained a selected call or changed comparison semantics")
  endif()
  if(_mode STREQUAL conservative)
    string(REGEX MATCHALL "(^|\n)[ \t]*(req|rne) " _boundary_predicates "${_boundary_caller}")
    list(LENGTH _boundary_predicates _boundary_count)
    if(NOT _boundary_count EQUAL 12)
      message(FATAL_ERROR "Boundary controls lost their twelve original comparison instructions")
    endif()
  endif()
  bound_pair("boundaries-${_mode}" "${_boundaries}" "${_boundary_expanded}"
    "1\n0\n0\n1\n1\n0\n0\n1\n0\n1\n1\n0")
endforeach()

# First activation defines the owned string; the second skips that definition.
# A fresh inline local cannot stand in for the original procedure's cache.
set(_recycled [=[.globals=0
main() .locals=3
load r0,1
call1 r1,recycled(),r0
load r0,0
call1 r2,recycled(),r0
ret 0
recycled() .locals=1
brf unreset,a1
load r0,"prior"
unreset:
req a1,r0,"prior"
ret a1
]=])
file(WRITE "${WORK}/recycled.rxas" "${_recycled}")
run_checked("retain recycled string predicate input" "${VM}" "${CLI}" ${RUNTIME} -a
  --module loose-recycled.rxas --profile closed-main-default-halt.v1 --no-cleanup
  -o "${WORK}/recycled-kept.rxas" "${WORK}/recycled.rxas")
file(READ "${WORK}/recycled-kept.rxas" _kept)
if(NOT _last_stderr MATCHES "LL_INLINE_KEEP: string[.]component[.]unproved" OR
    NOT _kept MATCHES "call1 r1,recycled[(][)],r0" OR
    NOT _kept MATCHES "call1 r2,recycled[(][)],r0")
  message(FATAL_ERROR "Recycled string predicate lost its exact refusal: ${_last_stderr}")
endif()
file(WRITE "${WORK}/last-good.rxas" "loose-last-good\n")
execute_process(COMMAND "${VM}" "${CLI}" ${RUNTIME} -a --module loose-limit.rxas
  --profile closed-main-default-halt.v1 --work 1 -o "${WORK}/last-good.rxas"
  "${FIXTURES}/io-loose-predicates.rxas"
  RESULT_VARIABLE _status OUTPUT_VARIABLE _stdout ERROR_VARIABLE _stderr TIMEOUT 60)
file(READ "${WORK}/last-good.rxas" _last_good)
if(_status EQUAL 0 OR NOT _stderr MATCHES "LL_INLINE_LIMIT:" OR NOT _last_good STREQUAL "loose-last-good\n")
  message(FATAL_ERROR "Loose-predicate work limit changed last-good output: ${_stdout}${_stderr}")
endif()
file(REMOVE_RECURSE "${WORK}")
message("RXAS_LOOSE_PREDICATES|failures=0")

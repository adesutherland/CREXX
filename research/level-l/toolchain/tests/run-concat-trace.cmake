cmake_minimum_required(VERSION 3.24)
foreach(_name VM ASSEMBLER CLI FIXTURES WORK)
  if(NOT DEFINED ${_name})
    message(FATAL_ERROR "Missing ${_name}")
  endif()
endforeach()
file(MAKE_DIRECTORY "${WORK}")
include("${FIXTURES}/signal-pair-helpers.cmake")
file(READ "${FIXTURES}/io-concat-trace.rxas" _original)
# Raw UTF-8, including embedded NUL and significant trailing/empty spaces:
# lambda/NUL/z:read; same:same; lambda-space/42/512/1 twice; [ /space/ ].
# The Windows console stream uses CRT text-mode CRLF; all payload bytes stay exact.
set(_line_end_hex "0a")
if(WIN32)
  set(_line_end_hex "0d0a")
endif()
set(_expected_lines
  "cebb007a3a72656164" "73616d653a73616d65" "cebb20" "3432"
  "353132" "31" "cebb20" "3432" "353132" "31" "5b" "20" "5d")
string(JOIN "${_line_end_hex}" _expected_hex ${_expected_lines})
string(APPEND _expected_hex "${_line_end_hex}")
function(concat_native _stem _source)
  set(_product "${WORK}/${_stem}")
  file(WRITE "${_product}.rxas" "${_source}")
  run_checked("assemble ${_stem}" "${ASSEMBLER}" -n -o "${_product}" "${_product}.rxas")
  execute_process(COMMAND "${VM}" "${_product}.rxbin" RESULT_VARIABLE _status
    OUTPUT_FILE "${_product}.out" ERROR_VARIABLE _stderr TIMEOUT 60)
  file(READ "${_product}.out" _observed HEX)
  file(REMOVE "${_product}.out")
  if(NOT _status EQUAL 0 OR NOT _observed STREQUAL _expected_hex)
    message(FATAL_ERROR "${_stem}: exact string/side-component output differs (${_status}): ${_observed}\n${_stderr}")
  endif()
endfunction()
concat_native(concat-original "${_original}")
foreach(_mode conservative cleaned)
  set(_option "")
  if(_mode STREQUAL conservative)
    set(_option --no-cleanup)
  endif()
  set(_product "${WORK}/concat-${_mode}")
  run_checked("expand concatenation/string TRACE ${_mode}" "${VM}" "${CLI}" ${RUNTIME} -a
    --module concat-trace.rxas --profile closed-main-default-halt.v1 ${_option}
    -o "${_product}.rxas" "${FIXTURES}/io-concat-trace.rxas")
  file(READ "${_product}.rxas" _expanded)
  string(FIND "${_expanded}" "readtext() .locals=" _end)
  if(_end LESS 0)
    string(FIND "${_expanded}" "readText() .locals=" _end)
  endif()
  if(_end LESS 0)
    message(FATAL_ERROR "Concatenation output lost the retained readText definition")
  endif()
  string(SUBSTRING "${_expanded}" 0 ${_end} _caller)
  if(_caller MATCHES "(^|\n)[ \t]*(call[1234]?|ichkrng)[ \t]")
    message(FATAL_ERROR "Concatenation retained a selected call or added a runtime helper/guard")
  endif()
  if(_mode STREQUAL conservative)
    string(REGEX MATCHALL "(^|\n)[ \t]*concat " _concats "${_caller}")
    string(REGEX MATCHALL "(^|\n)[ \t]*sconcat " _sconcats "${_caller}")
    list(LENGTH _concats _concat_count)
    list(LENGTH _sconcats _sconcat_count)
    if(NOT _concat_count EQUAL 3 OR NOT _sconcat_count EQUAL 2)
      message(FATAL_ERROR "Conservative string construction lost original instructions")
    endif()
    string(REGEX MATCH "[.]srcstep ([0-9]+) ([0-9]+) 17 \"arg_semantics_scalar[.]crexx\" 35 3 25 [^\n]*\n[ \t]*[.]traceevent \"V\" 6 \"R\" \"S\" \"r\" ([0-9]+) ([0-9]+) ([0-9]+) 0 \"text\" \"\"\n[ \t]*concat r([0-9]+),r([0-9]+),\":read\"" _trace "${_caller}")
    if(NOT _trace OR NOT CMAKE_MATCH_1 STREQUAL CMAKE_MATCH_4 OR
        NOT CMAKE_MATCH_2 STREQUAL CMAKE_MATCH_5 OR NOT CMAKE_MATCH_3 STREQUAL CMAKE_MATCH_7)
      message(FATAL_ERROR "Compiler-emitted string read lost source/TRACE mapping")
    endif()
    set(_result_event ".traceevent \"O\" 4 \"R\" \"S\" \"r\" ${CMAKE_MATCH_6} ${CMAKE_MATCH_1} ${CMAKE_MATCH_2} 0 \"\" \"\"")
    string(FIND "${_caller}" "${_result_event}" _event)
    if(_event LESS 0)
      message(FATAL_ERROR "String-only CONCAT result lost its typed S event")
    endif()
  endif()
  concat_native("concat-${_mode}-executed" "${_expanded}")
endforeach()

foreach(_case certificate source trace)
  set(_reason string.component.unproved)
  if(_case STREQUAL certificate)
    set(_first_call "call1 r1,unproved(),r0")
    set(_second_call "call1 r2,unproved(),r0")
    set(_negative [=[.globals=0
main() .locals=3
load r0,"lambda"
call1 r1,unproved(),r0
call1 r2,unproved(),r0
ret 0
unproved() .locals=1
concat r0,a1,":x"
ret a1
]=])
  else()
    set(_first_call "call2 r2,unproved(),r0,r1")
    set(_second_call "call2 r3,unproved(),r0,r1")
    set(_negative [=[.globals=0
main() .locals=4
load r0,1
load r1,"lambda"
call2 r2,unproved(),r0,r1
load r0,0
call2 r3,unproved(),r0,r1
ret 0
unproved() .locals=1
brf unreset,a1
load r0,"prior"
unreset:
]=])
    if(_case STREQUAL source)
      string(APPEND _negative "concat a2,r0,\":x\"\nret a2\n")
    else()
      string(APPEND _negative ".traceevent \"O\" 4 \"R\" \"S\" \"r\" 0 0 0 0 \"unproved-string\" \"\"\nret a2\n")
      set(_reason local.recycler.observation.unproved)
    endif()
  endif()
  file(WRITE "${WORK}/${_case}.rxas" "${_negative}")
  run_checked("retain ${_case} component premise" "${VM}" "${CLI}" ${RUNTIME} -a
    --module concat-negative.rxas --profile closed-main-default-halt.v1 --no-cleanup
    -o "${WORK}/${_case}-kept.rxas" "${WORK}/${_case}.rxas")
  file(READ "${WORK}/${_case}-kept.rxas" _kept)
  string(FIND "${_kept}" "${_first_call}" _first_at)
  string(FIND "${_kept}" "${_second_call}" _second_at)
  if(NOT _last_stderr MATCHES "LL_INLINE_KEEP: ${_reason}" OR _first_at LESS 0 OR _second_at LESS 0)
    message(FATAL_ERROR "${_case} lost its precise component refusal: ${_last_stderr}")
  endif()
endforeach()
file(WRITE "${WORK}/last-good.rxas" "concat-last-good\n")
execute_process(COMMAND "${VM}" "${CLI}" ${RUNTIME} -a --module concat-limit.rxas
  --profile closed-main-default-halt.v1 --work 1 -o "${WORK}/last-good.rxas"
  "${FIXTURES}/io-concat-trace.rxas"
  RESULT_VARIABLE _status OUTPUT_VARIABLE _stdout ERROR_VARIABLE _stderr TIMEOUT 60)
file(READ "${WORK}/last-good.rxas" _last_good)
if(_status EQUAL 0 OR NOT _stderr MATCHES "LL_INLINE_LIMIT:" OR NOT _last_good STREQUAL "concat-last-good\n")
  message(FATAL_ERROR "Concatenation work limit changed last-good publication: ${_stdout}${_stderr}")
endif()
file(REMOVE_RECURSE "${WORK}")
message("RXAS_CONCAT_TRACE|failures=0")

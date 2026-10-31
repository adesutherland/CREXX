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
  set(_wanted "${_expected_hex}")
  if(ARGC GREATER 2)
    set(_wanted "${ARGV2}")
  endif()
  if(NOT _status EQUAL 0 OR NOT _observed STREQUAL _wanted)
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

set(_ordinary_destination [=[.globals=0
main() .locals=3
load r0,"lambda"
call1 r1,ordinary_destination(),r0
call1 r2,ordinary_destination(),r0
say r1
say r2
ret 0
ordinary_destination() .locals=1
concat r0,a1,":x"
ret r0
]=])
# CONCAT defines its destination string and invalidates old normalization
# certificates itself. A fresh private destination needs no certificate proof.
# Read-before-write programs are outside this input contract; they do not
# justify an inliner gate that simulates callee recycler contents.
set(_ordinary_hex "6c616d6264613a78${_line_end_hex}6c616d6264613a78${_line_end_hex}")
concat_native(ordinary-destination-original "${_ordinary_destination}" "${_ordinary_hex}")
file(WRITE "${WORK}/ordinary-destination.rxas" "${_ordinary_destination}")
foreach(_mode conservative cleaned)
  set(_option "")
  if(_mode STREQUAL conservative)
    set(_option --no-cleanup)
  endif()
  run_checked("expand ordinary destination ${_mode}" "${VM}" "${CLI}" ${RUNTIME} -a
    --module concat-destination.rxas --profile closed-main-default-halt.v1
    ${_option} -o "${WORK}/ordinary-destination-${_mode}.rxas" "${WORK}/ordinary-destination.rxas")
  file(READ "${WORK}/ordinary-destination-${_mode}.rxas" _expanded)
  string(FIND "${_expanded}" "ordinary_destination() .locals=" _end)
  if(_end LESS 0)
    message(FATAL_ERROR "Ordinary destination output lost its retained definition")
  endif()
  string(SUBSTRING "${_expanded}" 0 ${_end} _caller)
  if(_caller MATCHES "(^|\n)[ \t]*call[1234]?[ \t]")
    message(FATAL_ERROR "Ordinary CONCAT destination retained a selected call")
  endif()
  if(_mode STREQUAL conservative)
    string(REGEX MATCHALL "(^|\n)[ \t]*concat " _concats "${_caller}")
    list(LENGTH _concats _count)
    if(NOT _count EQUAL 2)
      message(FATAL_ERROR "Ordinary destination lost an authored CONCAT")
    endif()
  endif()
  concat_native("ordinary-destination-${_mode}-executed" "${_expanded}" "${_ordinary_hex}")
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

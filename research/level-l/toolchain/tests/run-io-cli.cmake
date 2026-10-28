cmake_minimum_required(VERSION 3.24)
foreach(_name VM ASSEMBLER DISASSEMBLER CLI GENERATED FIXTURES WORK)
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
endfunction()

# This is the actual ordinary compiler -n output, not a handwritten substitute.
run_checked("read compiler output" "${VM}" "${CLI}" ${RUNTIME} -a
  --module io-generated.rxas --policy never -o "${WORK}/io-generated.rxas" "${GENERATED}")
run_checked("assemble compiler roundtrip without cleanup" "${ASSEMBLER}" -n
  -o "${WORK}/io-generated" "${WORK}/io-generated.rxas")
run_checked("execute compiler roundtrip" "${VM}" "${WORK}/io-generated.rxbin")
string(STRIP "${_last_stdout}" _result)
if(NOT _result STREQUAL "7")
  message(FATAL_ERROR "compiler roundtrip expected 7, got '${_result}'")
endif()

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

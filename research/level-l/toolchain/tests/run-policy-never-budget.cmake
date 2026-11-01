cmake_minimum_required(VERSION 3.24)
foreach(_required VM ASSEMBLER CLI FIXTURE WORK)
  if(NOT DEFINED ${_required})
    message(FATAL_ERROR "Missing ${_required}")
  endif()
endforeach()
file(MAKE_DIRECTORY "${WORK}")
set(_output "${WORK}/branch.rxas")
execute_process(COMMAND "${VM}" "${CLI}" ${RUNTIME} -a
  --module io-branch.rxas --policy never --no-cleanup --work 4000000
  -o "${_output}" "${FIXTURE}"
  RESULT_VARIABLE _status OUTPUT_VARIABLE _stdout ERROR_VARIABLE _stderr TIMEOUT 90)
if(NOT _status EQUAL 0)
  message(FATAL_ERROR "Default-budget normalization failed: ${_status}\n${_stdout}${_stderr}")
endif()
file(READ "${_output}" _source)
string(REGEX MATCHALL "[\r\n][ \t]*call3[ \t]" _calls "${_source}")
list(LENGTH _calls _call_count)
if(NOT _call_count EQUAL 2 OR _source MATCHES "[\r\n][ \t]*inlenter[ \t]")
  message(FATAL_ERROR "Policy-never changed the two ordinary CALL3 sites")
endif()
execute_process(COMMAND "${ASSEMBLER}" -n -o "${WORK}/branch" "${_output}"
  RESULT_VARIABLE _status OUTPUT_VARIABLE _stdout ERROR_VARIABLE _stderr TIMEOUT 60)
if(NOT _status EQUAL 0)
  message(FATAL_ERROR "Assemble normalized fixture: ${_status}\n${_stdout}${_stderr}")
endif()
execute_process(COMMAND "${VM}" "${WORK}/branch.rxbin"
  RESULT_VARIABLE _status OUTPUT_VARIABLE _stdout ERROR_VARIABLE _stderr TIMEOUT 30)
string(REPLACE "\r\n" "\n" _stdout "${_stdout}")
string(STRIP "${_stdout}" _stdout)
if(NOT _status EQUAL 0 OR NOT _stdout STREQUAL "9\n11\n99")
  message(FATAL_ERROR "Independent two-arm result changed: ${_status}\n${_stdout}${_stderr}")
endif()
set(_last_good "${WORK}/last-good.rxas")
file(WRITE "${_last_good}" "LAST_GOOD\n")
execute_process(COMMAND "${VM}" "${CLI}" ${RUNTIME} -a
  --module io-branch.rxas --policy never --no-cleanup --work 1
  -o "${_last_good}" "${FIXTURE}"
  RESULT_VARIABLE _status OUTPUT_VARIABLE _stdout ERROR_VARIABLE _stderr TIMEOUT 30)
file(READ "${_last_good}" _retained)
if(_status EQUAL 0 OR NOT _retained STREQUAL "LAST_GOOD\n" OR NOT _stderr MATCHES "LL_[A-Z_]*LIMIT")
  message(FATAL_ERROR "Tiny-work refusal lost its limit/last-good contract: ${_status}\n${_stdout}${_stderr}")
endif()
message(STATUS "RXAS_POLICY_NEVER_BUDGET|failures=0")

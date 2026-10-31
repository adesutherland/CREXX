cmake_minimum_required(VERSION 3.24)
foreach(_required VM ASSEMBLER DISASSEMBLER SERVICE WORK)
  if(NOT DEFINED ${_required})
    message(FATAL_ERROR "Missing ${_required}")
  endif()
endforeach()
file(MAKE_DIRECTORY "${WORK}")
set(_source "${WORK}/inline-service.rxas")
file(WRITE "${_source}" ".globals=0\nmain() .locals=2\n  call r0,helper()\n  ieq r1,r0,17\n  brf fail,r1\n  sayx \"PASS: rxas inline service\"\n  ret 0\nfail:\n  ret 1\nhelper() .locals=0\n  ret 17\n")
file(SHA256 "${_source}" _before)

function(run_checked _name)
  execute_process(COMMAND ${ARGN} RESULT_VARIABLE _result
    OUTPUT_VARIABLE _stdout ERROR_VARIABLE _stderr TIMEOUT 90)
  if(NOT _result EQUAL 0)
    message(FATAL_ERROR "${_name}: ${_result}\n${_stdout}${_stderr}")
  endif()
  set(_stdout "${_stdout}" PARENT_SCOPE)
endfunction()

run_checked("ordinary RXAS" "${ASSEMBLER}" "${_source}")
set(_result "${WORK}/inline-service.rxbin")
if(NOT EXISTS "${_result}")
  message(FATAL_ERROR "ordinary RXAS omitted default output stem")
endif()
file(RENAME "${_result}" "${WORK}/without-service.rxbin")
run_checked("ordinary disassembly" "${DISASSEMBLER}" "${WORK}/without-service.rxbin")
set(_ordinary "${_stdout}")
run_checked("ordinary execution" "${VM}" "${WORK}/without-service.rxbin")
if(NOT _stdout MATCHES "PASS: rxas inline service")
  message(FATAL_ERROR "ordinary execution missed PASS marker")
endif()

set(_runtime_args)
foreach(_image IN LISTS RUNTIME)
  list(APPEND _runtime_args --inline-runtime "${_image}")
endforeach()
run_checked("opt-in RXAS" "${ASSEMBLER}"
  --inline-service "${SERVICE}" --inline-vm "${VM}"
  --inline-module inline-service --inline-policy all-legal
  ${_runtime_args} "${_source}")
if(NOT EXISTS "${_result}")
  message(FATAL_ERROR "opt-in RXAS changed default output stem")
endif()
run_checked("opt-in disassembly" "${DISASSEMBLER}" "${_result}")
set(_inlined "${_stdout}")
string(REGEX MATCH "\\* MODULE - [^\n]*" _ordinary_module "${_ordinary}")
string(REGEX MATCH "\\* MODULE - [^\n]*" _inlined_module "${_inlined}")
string(REGEX MATCH "\\* DESCRIPTION - [^\n]*" _ordinary_description "${_ordinary}")
string(REGEX MATCH "\\* DESCRIPTION - [^\n]*" _inlined_description "${_inlined}")
if(NOT _ordinary_module STREQUAL _inlined_module OR
   NOT _ordinary_description STREQUAL _inlined_description)
  message(FATAL_ERROR "opt-in RXAS changed module identity")
endif()
if(_inlined MATCHES "[\n ]+call r0,helper\\(\\)")
  message(FATAL_ERROR "opt-in RXAS retained the simple local call")
endif()
run_checked("opt-in execution" "${VM}" "${_result}")
if(NOT _stdout MATCHES "PASS: rxas inline service")
  message(FATAL_ERROR "opt-in execution missed PASS marker")
endif()

file(WRITE "${WORK}/malformed-service.rxbin" "invalid RXBIN service")
execute_process(COMMAND "${ASSEMBLER}"
  --inline-service "${WORK}/malformed-service.rxbin" --inline-vm "${VM}"
  --inline-module inline-service ${_runtime_args}
  -o "${WORK}/must-not-exist" "${_source}"
  RESULT_VARIABLE _bad_result OUTPUT_VARIABLE _bad_stdout
  ERROR_VARIABLE _bad_stderr TIMEOUT 90)
file(SHA256 "${_source}" _after)
if(_bad_result EQUAL 0 OR EXISTS "${WORK}/must-not-exist.rxbin" OR
   NOT _before STREQUAL _after)
  message(FATAL_ERROR "failed inline service changed source or emitted RXBIN")
endif()
message("RXAS_INLINE_SERVICE|failures=0")

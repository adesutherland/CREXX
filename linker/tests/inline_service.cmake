cmake_minimum_required(VERSION 3.24)
foreach(_required VM ASSEMBLER LINKER DISASSEMBLER SERVICE BAD_SERVICE MUTATOR WORK)
  if(NOT DEFINED ${_required})
    message(FATAL_ERROR "Missing ${_required}")
  endif()
endforeach()
file(MAKE_DIRECTORY "${WORK}")
file(WRITE "${WORK}/root.rxas" ".globals=1\nhelper() .expose=linkinline.helper\nmain() .locals=4\n  load g0,100\n  load r0,5\n  call1 r1,helper(),r0\n  ieq r2,r1,12\n  brf failed,r2\n  ieq r3,g0,100\n  brf failed,r3\n  sayx \"PASS: selected private global\"\n  ret 0\nfailed:\n  ret 1\nprivate_child() .locals=0\n  ret 999\n")
file(WRITE "${WORK}/provider.rxas" ".globals=1\nhelper() .locals=2 .expose=linkinline.helper\n  load g0,7\n  call1 r0,private_child(),a1\n  iadd r1,r0,g0\n  ret r1\nprivate_child() .locals=1\n  iadd r0,a1,0\n  ret r0\n")

function(run_checked _name)
  execute_process(COMMAND ${ARGN} RESULT_VARIABLE _result
    OUTPUT_VARIABLE _stdout ERROR_VARIABLE _stderr TIMEOUT 120)
  if(NOT _result EQUAL 0)
    message(FATAL_ERROR "${_name}: ${_result}\n${_stdout}${_stderr}")
  endif()
  set(_stdout "${_stdout}" PARENT_SCOPE)
endfunction()

run_checked("assemble root" "${ASSEMBLER}" -n "${WORK}/root.rxas")
run_checked("assemble provider" "${ASSEMBLER}" -n "${WORK}/provider.rxas")
set(_inputs "${WORK}/root.rxbin" "${WORK}/provider.rxbin")
set(_options)
foreach(_runtime IN LISTS RUNTIME)
  list(APPEND _options --inline-runtime "${_runtime}")
endforeach()
run_checked("baseline link" "${LINKER}" -o "${WORK}/baseline" ${_inputs})
run_checked("baseline execution" "${VM}" "${WORK}/baseline.rxbin")
if(NOT _stdout MATCHES "PASS: selected private global")
  message(FATAL_ERROR "baseline result mismatch")
endif()
run_checked("selected service link" "${LINKER}" --inline-service "${SERVICE}"
  --inline-policy all-legal ${_options} -o "${WORK}/inlined" ${_inputs})
run_checked("selected service execution" "${VM}" "${WORK}/inlined.rxbin")
if(NOT _stdout MATCHES "PASS: selected private global")
  message(FATAL_ERROR "selected provider lost private/global identity")
endif()
run_checked("selected service disassembly" "${DISASSEMBLER}" "${WORK}/inlined.rxbin")
if(_stdout MATCHES "[\n ]+call1 [^\n]*helper\\(\\)" OR
   NOT _stdout MATCHES "inlenter" OR NOT _stdout MATCHES "qglobbind[^\n]*@1" OR
   NOT _stdout MATCHES "call1 [^\n]*@1::[0-9]+\\(\\)")
  message(FATAL_ERROR "selected call was retained or exact private-global binding missing\n${_stdout}")
endif()

# Mutate only one executable contract at a time. A valid wrong recipe ordinal
# disagrees with its P operand; QPR1's module/procedure bounds fail at prepare.
foreach(_mutation feature qproc-module qproc-procedure recipe-mismatch)
  run_checked("mutate ${_mutation}" "${MUTATOR}" "${_mutation}"
    "${WORK}/inlined.rxbin" "${WORK}/${_mutation}.rxbin")
  execute_process(COMMAND "${VM}" "${WORK}/${_mutation}.rxbin"
    RESULT_VARIABLE _rejected OUTPUT_VARIABLE _rejected_stdout
    ERROR_VARIABLE _rejected_stderr TIMEOUT 30)
  if(NOT "${_rejected}" MATCHES "^-?[0-9]+$" OR
     _rejected EQUAL 0 OR _rejected_stdout MATCHES "PASS: selected")
    message(FATAL_ERROR "${_mutation} executed or passed: ${_rejected}\n${_rejected_stdout}${_rejected_stderr}")
  endif()
  if(_mutation MATCHES "^qproc-" AND
     NOT _rejected_stderr MATCHES "Invalid or unsealed qualified procedure")
    message(FATAL_ERROR "${_mutation} lacked checked preparation rejection: ${_rejected_stderr}")
  endif()
  if(_mutation STREQUAL "recipe-mismatch" AND
     NOT _rejected_stderr MATCHES "RXBIN_CORRUPTION")
    message(FATAL_ERROR "${_mutation} lacked provider identity rejection: ${_rejected_stderr}")
  endif()
endforeach()

# The valid mock service returns an incomplete typed flat response. An existing
# output must remain byte-identical; no selected module may be swapped early.
file(SHA256 "${WORK}/inlined.rxbin" _before)
execute_process(COMMAND "${LINKER}" --inline-service "${BAD_SERVICE}"
  ${_options} -o "${WORK}/inlined" ${_inputs}
  RESULT_VARIABLE _bad OUTPUT_VARIABLE _bad_stdout ERROR_VARIABLE _bad_stderr TIMEOUT 120)
file(SHA256 "${WORK}/inlined.rxbin" _after)
if(_bad EQUAL 0 OR NOT _before STREQUAL _after)
  message(FATAL_ERROR "malformed response succeeded or damaged existing output\n${_bad_stdout}${_bad_stderr}")
endif()
run_checked("disabled service selection" "${LINKER}" --inline-service "${BAD_SERVICE}"
  --inline-policy never -o "${WORK}/disabled" ${_inputs})
run_checked("disabled disassembly" "${DISASSEMBLER}" "${WORK}/disabled.rxbin")
if(NOT _stdout MATCHES "call1 [^\n]*helper\\(\\)")
  message(FATAL_ERROR "disabled control did not retain ordinary call")
endif()

# A compact many-procedure input deterministically exhausts the request-local
# validation work allowance before invoking a service or publishing anything.
file(WRITE "${WORK}/budget.rxas" ".globals=0\nmain() .locals=0\n  ret 0\n")
foreach(_procedure RANGE 0 299)
  file(APPEND "${WORK}/budget.rxas" "budget${_procedure}() .locals=0\n  ret 0\n")
endforeach()
run_checked("assemble work-limit input" "${ASSEMBLER}" -n "${WORK}/budget.rxas")
file(COPY_FILE "${WORK}/baseline.rxbin" "${WORK}/budget-result.rxbin")
file(SHA256 "${WORK}/budget-result.rxbin" _before)
execute_process(COMMAND "${LINKER}" --inline-service "${BAD_SERVICE}"
  -o "${WORK}/budget-result" "${WORK}/budget.rxbin"
  RESULT_VARIABLE _limited OUTPUT_VARIABLE _limited_stdout ERROR_VARIABLE _limited_stderr TIMEOUT 120)
file(SHA256 "${WORK}/budget-result.rxbin" _after)
if(_limited EQUAL 0 OR NOT _limited_stderr MATCHES "selection work limit" OR
   NOT _before STREQUAL _after)
  message(FATAL_ERROR "selection work limit was not precise and atomic\n${_limited_stdout}${_limited_stderr}")
endif()
file(REMOVE_RECURSE "${WORK}")
message("RXLINK_INLINE_SERVICE|failures=0")

cmake_minimum_required(VERSION 3.24)
foreach(_required VM ASSEMBLER LINKER DISASSEMBLER SERVICE BAD_SERVICE MUTATOR WORK)
  if(NOT DEFINED ${_required})
    message(FATAL_ERROR "Missing ${_required}")
  endif()
endforeach()
file(MAKE_DIRECTORY "${WORK}")
# The call intentionally observes its physical caller. The service must retain
# that CALL while preserving the separately authored static private binding.
file(WRITE "${WORK}/root.rxas" [=[.globals=1
main() .locals=4
  .inlinedprivate r3,"@1",g0
  load g0,100
  load r3,7
  load r0,5
  call1 r1,@1::0(),r0
  ieq r2,r1,12
  brf failed,r2
  ieq r2,g0,100
  brf failed,r2
  sayx "PASS: selected private global"
  ret 0
failed:
  ret 1
private_child() .locals=0
  ret 999
]=])
file(WRITE "${WORK}/provider.rxas" [=[.globals=1
private_child() .locals=2
  metaloadcalleraddr r0
  linkattr1 r1,r0,2
  igt r1,r1,0
  brf missing_parent,r1
  iadd r1,a1,g0
  ret r1
missing_parent:
  ret -100
]=])

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
run_checked("seal exact call and private binding" "${MUTATOR}" join-seal
  "${WORK}/selected-input.rxbin" "${WORK}/root.rxbin" "${WORK}/provider.rxbin")
set(_inputs "${WORK}/selected-input.rxbin")
set(_options)
foreach(_runtime IN LISTS RUNTIME)
  list(APPEND _options --inline-runtime "${_runtime}")
endforeach()
# A same-module-only selection has no foreign provider-bundle attachment to
# incidentally execute lazy analyses. Its checked stage must finish the exact
# point/summary pair before lending either owner to the inliner context.
file(WRITE "${WORK}/same-owner.rxas" ".globals=0\nmain() .locals=0\n  call local_leaf()\n  ret 0\nlocal_leaf() .locals=0\n  ret 0\n")
run_checked("assemble same-owner proof bundle" "${ASSEMBLER}" -n "${WORK}/same-owner.rxas")
run_checked("same-owner service proof readiness" "${LINKER}" --inline-service "${SERVICE}"
  --inline-policy all-legal ${_options} -o "${WORK}/same-owner-inlined" "${WORK}/same-owner.rxbin")
run_checked("same-owner service execution" "${VM}" "${WORK}/same-owner-inlined.rxbin")
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
if(NOT _stdout MATCHES "[.]inlinedprivate r[0-9]+,\"@1\",g0" OR
   NOT _stdout MATCHES "call1 [^\n]*@1::0\\(\\)" OR
   _stdout MATCHES "[ \t](inlenter2?|linkargv|qglobbind)[ \t]")
  message(FATAL_ERROR "service lost retained physical CALL or static private binding\n${_stdout}")
endif()

# Reprocess existing sealed output after changing the selected module order.
# The old provider ordinal one now names the caller; its private_child shadow
# must never capture the retained QPR1 call during service discovery/snapshot.
file(WRITE "${WORK}/anchor.rxas" ".globals=0\nanchor() .locals=0\n  ret 0\nprivate_child() .locals=0\n  ret 888\n")
run_checked("assemble reprocessing root" "${ASSEMBLER}" -n "${WORK}/anchor.rxas")
file(SHA256 "${WORK}/inlined.rxbin" _sealed_input_before)
run_checked("service reprocess sealed image" "${LINKER}" --inline-service "${SERVICE}"
  --inline-policy all-legal ${_options} -r anchor -r root -o "${WORK}/reprocessed"
  "${WORK}/anchor.rxbin" "${WORK}/inlined.rxbin")
run_checked("reprocessed private provider execution" "${VM}" "${WORK}/reprocessed.rxbin")
if(NOT _stdout STREQUAL "PASS: selected private global")
  message(FATAL_ERROR "service reprocessing changed private target/global identity: ${_stdout}")
endif()
run_checked("reprocessed selected identities" "${DISASSEMBLER}" "${WORK}/reprocessed.rxbin")
if(NOT _stdout MATCHES "[.]inlinedprivate r[0-9]+,\"@2\",g0" OR
   NOT _stdout MATCHES "call1 [^\n]*@2::0\\(\\)" OR
   _stdout MATCHES "@1::[0-9]+\\(\\)" OR
   _stdout MATCHES "[ \t](inlenter2?|linkargv|qglobbind)[ \t]")
  message(FATAL_ERROR "sealed private child or static binding was not rebased before reprocessing\n${_stdout}")
endif()
file(SHA256 "${WORK}/inlined.rxbin" _sealed_input_after)
if(NOT _sealed_input_before STREQUAL _sealed_input_after)
  message(FATAL_ERROR "service reprocessing modified the sealed input")
endif()
file(SHA256 "${WORK}/reprocessed.rxbin" _reprocessed_before)

# Mutate one surviving executable contract at a time. QPR1's provider bounds
# fail at prepare; static binding seal/global failures are checked by the loader.
foreach(_mutation feature qproc-module qproc-procedure qproc-unsealed private-unsealed private-global)
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
  execute_process(COMMAND "${LINKER}" --inline-service "${SERVICE}"
    --inline-policy all-legal ${_options} -r anchor -r root -o "${WORK}/reprocessed"
    "${WORK}/anchor.rxbin" "${WORK}/${_mutation}.rxbin"
    RESULT_VARIABLE _rejected OUTPUT_VARIABLE _rejected_stdout
    ERROR_VARIABLE _rejected_stderr TIMEOUT 30)
  file(SHA256 "${WORK}/reprocessed.rxbin" _reprocessed_after)
  if(_rejected EQUAL 0 OR NOT _reprocessed_before STREQUAL _reprocessed_after)
    message(FATAL_ERROR "${_mutation} reprocessing repaired an invalid seal or changed last-good output\n${_rejected_stdout}${_rejected_stderr}")
  endif()
endforeach()

file(WRITE "${WORK}/omit-reprocess.ctl"
  "INPUT ${WORK}/anchor.rxbin\nINPUT ${WORK}/inlined.rxbin\nROOT anchor\nROOT root\nOMIT provider\nOUTPUT ${WORK}/reprocessed\n")
execute_process(COMMAND "${LINKER}" --inline-service "${SERVICE}"
  --inline-policy all-legal ${_options} -c "${WORK}/omit-reprocess.ctl"
  RESULT_VARIABLE _rejected OUTPUT_VARIABLE _rejected_stdout
  ERROR_VARIABLE _rejected_stderr TIMEOUT 30)
file(SHA256 "${WORK}/reprocessed.rxbin" _reprocessed_after)
if(_rejected EQUAL 0 OR NOT _reprocessed_before STREQUAL _reprocessed_after OR
   NOT "${_rejected_stdout}${_rejected_stderr}" MATCHES "unsealed or mismatched inline operand")
  message(FATAL_ERROR "missing exact provider was not refused atomically before service\n${_rejected_stdout}${_rejected_stderr}")
endif()

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
if(NOT _stdout MATCHES "call1 [^\n]*@1::0\\(\\)" OR
   NOT _stdout MATCHES "[.]inlinedprivate r[0-9]+,\"@1\",g0")
  message(FATAL_ERROR "disabled control did not retain the exact call and static binding")
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

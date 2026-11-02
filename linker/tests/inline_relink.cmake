cmake_minimum_required(VERSION 3.24)
foreach(_required VM ASSEMBLER LINKER DISASSEMBLER MUTATOR WORK)
  if(NOT DEFINED ${_required})
    message(FATAL_ERROR "Missing ${_required}")
  endif()
endforeach()
file(MAKE_DIRECTORY "${WORK}")

function(run_checked _name)
  execute_process(COMMAND ${ARGN} RESULT_VARIABLE _result
    OUTPUT_VARIABLE _stdout ERROR_VARIABLE _stderr TIMEOUT 30)
  if(NOT _result EQUAL 0)
    message(FATAL_ERROR "${_name}: ${_result}\n${_stdout}${_stderr}")
  endif()
  set(_stdout "${_stdout}" PARENT_SCOPE)
endfunction()

# The provider exports nothing. Only checked typed references can retain it.
# The rooted prefix and caller deliberately contain private same-name shadows.
file(WRITE "${WORK}/anchor.rxas" [=[.globals=1
anchor() .locals=0
  ret 0
hidden() .locals=0
  ret 888
]=])
file(WRITE "${WORK}/root.rxas" [=[.globals=1
main() .locals=4
  .inlinedprivate r3,"@1",g0
  load g0,100
  load r3,5
  call r0,@1::0()
  ieq r2,r0,12
  brf failed,r2
  ieq r2,r3,7
  brf failed,r2
  call r1,@1::0()
  ieq r2,r1,12
  brf failed,r2
  ieq r2,g0,100
  brf failed,r2
  sayx "PASS: private sealed relink"
  ret 0
failed:
  ret 1
hidden() .locals=0
  ret 999
]=])
file(WRITE "${WORK}/provider.rxas" [=[.globals=1
hidden() .locals=0
  load g0,7
  ret 12
decoy() .locals=0
  ret 777
]=])
foreach(_module anchor root provider)
  run_checked("assemble ${_module}" "${ASSEMBLER}" -n "${WORK}/${_module}.rxas")
endforeach()
run_checked("construct old exact image" "${MUTATOR}" join-seal "${WORK}/old.rxbin"
  "${WORK}/root.rxbin" "${WORK}/provider.rxbin")
run_checked("old image execution" "${VM}" "${WORK}/old.rxbin")
if(NOT _stdout STREQUAL "PASS: private sealed relink")
  message(FATAL_ERROR "old image failed independent value/global oracle: ${_stdout}")
endif()
file(SHA256 "${WORK}/old.rxbin" _old_hash)
run_checked("ordinary sealed relink" "${LINKER}" -r anchor -r root
  -o "${WORK}/relinked" "${WORK}/anchor.rxbin" "${WORK}/old.rxbin")
run_checked("relinked execution" "${VM}" "${WORK}/relinked.rxbin")
if(NOT _stdout STREQUAL "PASS: private sealed relink")
  message(FATAL_ERROR "relink rebound a private target/global: ${_stdout}")
endif()
run_checked("relinked structural oracle" "${DISASSEMBLER}" "${WORK}/relinked.rxbin")
if(NOT _stdout MATCHES "call [^\n]*@2::0\\(\\)" OR
   NOT _stdout MATCHES "[.]inlinedprivate r3,\"@2\",g0" OR
   _stdout MATCHES "@1::0\\(\\)" OR
   _stdout MATCHES "[ \t](inlenter2?|linkargv|qglobbind)[ \t]")
  message(FATAL_ERROR "typed references were not all rebased to exact provider two\n${_stdout}")
endif()

# Every malformed old identity is rejected before the last-good output opens.
file(SHA256 "${WORK}/relinked.rxbin" _before)
foreach(_mutation feature qproc-module qproc-procedure qproc-unsealed private-unsealed private-global)
  run_checked("mutate ${_mutation}" "${MUTATOR}" "${_mutation}"
    "${WORK}/old.rxbin" "${WORK}/${_mutation}.rxbin")
  execute_process(COMMAND "${LINKER}" -r anchor -r root -o "${WORK}/relinked"
    "${WORK}/anchor.rxbin" "${WORK}/${_mutation}.rxbin"
    RESULT_VARIABLE _rejected OUTPUT_VARIABLE _out ERROR_VARIABLE _err TIMEOUT 30)
  file(SHA256 "${WORK}/relinked.rxbin" _after)
  if(_rejected EQUAL 0 OR NOT _before STREQUAL _after)
    message(FATAL_ERROR "${_mutation} repaired an invalid identity or damaged last-good output: ${_out}${_err}")
  endif()
  if(NOT _mutation STREQUAL "feature" AND
     NOT "${_out}${_err}" MATCHES "unsealed or mismatched inline operand")
    message(FATAL_ERROR "${_mutation} lacked typed-owner refusal: ${_out}${_err}")
  endif()
endforeach()
file(WRITE "${WORK}/omit.ctl"
  "INPUT ${WORK}/anchor.rxbin\nINPUT ${WORK}/old.rxbin\nROOT anchor\nROOT root\nOMIT provider\nOUTPUT ${WORK}/relinked\n")
execute_process(COMMAND "${LINKER}" -c "${WORK}/omit.ctl"
  RESULT_VARIABLE _rejected OUTPUT_VARIABLE _out ERROR_VARIABLE _err TIMEOUT 30)
file(SHA256 "${WORK}/relinked.rxbin" _after)
if(_rejected EQUAL 0 OR NOT _before STREQUAL _after OR
   NOT "${_out}${_err}" MATCHES "unsealed or mismatched inline operand")
  message(FATAL_ERROR "omitted exact provider was not an atomic refusal: ${_out}${_err}")
endif()
file(SHA256 "${WORK}/old.rxbin" _after)
if(NOT _old_hash STREQUAL _after)
  message(FATAL_ERROR "relink modified the input image")
endif()

# Two standalone images both use sealed member zero. Their qualified CALLs
# and self-relative private bindings retain separate original graph identities.
foreach(_letter a b)
  if(_letter STREQUAL "a")
    set(_value 11)
  else()
    set(_value 22)
  endif()
  file(WRITE "${WORK}/${_letter}.rxas"
    ".globals=1\nentry${_letter}() .locals=3 .expose=relink.${_letter}\n  .inlinedprivate r2,\"self\",g0\n  call r0,@0::1()\n  ieq r1,r2,${_value}\n  brf failed,r1\n  ret r0\nfailed:\n  ret -100\nhidden() .locals=0\n  load g0,${_value}\n  ret ${_value}\n")
  run_checked("assemble independent image ${_letter}" "${ASSEMBLER}" -n "${WORK}/${_letter}.rxas")
  run_checked("seal independent image ${_letter}" "${MUTATOR}" join-seal
    "${WORK}/${_letter}-sealed.rxbin" "${WORK}/${_letter}.rxbin")
endforeach()
file(WRITE "${WORK}/collision.rxas" [=[.globals=1
entrya() .expose=relink.a
entryb() .expose=relink.b
main() .locals=4
  load g0,100
  call r0,entrya()
  call r1,entryb()
  iadd r2,r0,r1
  ieq r3,r2,33
  brf failed,r3
  ieq r3,g0,100
  brf failed,r3
  sayx "PASS: independent old ordinal zero"
  ret 0
failed:
  ret 1
hidden() .locals=0
  ret 999
]=])
run_checked("assemble collision root" "${ASSEMBLER}" -n "${WORK}/collision.rxas")
run_checked("relink colliding source ordinals" "${LINKER}" -r collision -o "${WORK}/collision-linked"
  "${WORK}/collision.rxbin" "${WORK}/a-sealed.rxbin" "${WORK}/b-sealed.rxbin")
run_checked("independent original graphs" "${VM}" "${WORK}/collision-linked.rxbin")
if(NOT _stdout STREQUAL "PASS: independent old ordinal zero")
  message(FATAL_ERROR "equal old ordinals collapsed original image identity: ${_stdout}")
endif()
file(REMOVE_RECURSE "${WORK}")
message("RXLINK_INLINE_RELINK|failures=0")

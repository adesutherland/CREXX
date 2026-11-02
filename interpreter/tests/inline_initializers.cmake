cmake_minimum_required(VERSION 3.24)
foreach(_required VM ASSEMBLER MUTATOR WORK)
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
  set(_stderr "${_stderr}" PARENT_SCOPE)
endfunction()

# A root initializer enters its provider before that provider's initializer.
# The provider changes the caller-owned exposed count and actual. Counted CALL
# observes the new count; fixed CALL retains its prior capture.
foreach(_mode counted fixed)
  set(_dir "${WORK}/${_mode}")
  file(MAKE_DIRECTORY "${_dir}")
  if(_mode STREQUAL "counted")
    set(_entry "  call r0,@1::0(),g0\n")
    set(_expected 0)
  else()
    set(_entry "  call1 r0,@1::0(),g1\n")
    set(_expected 1)
  endif()
  file(WRITE "${_dir}/root.rxas"
    ".globals=3\ng0 .expose=inlineinit.count\ng1 .expose=inlineinit.actual\ng2 .expose=inlineinit.starts\nboot() .locals=6\n  .meta \"inlineinit.root.boot\"=\"b\" \".void\" boot() \"\"\n  .meta \"inlineinit.root.boot\"=\".initializer\" \".void\" boot() \"\"\n  load g0,1\n  load g1,10\n${_entry}  ieq r1,r0,${_expected}\n  brf failed,r1\n  ieq r1,g0,0\n  brf failed,r1\n  ieq r1,g1,20\n  brf failed,r1\n  ieq r1,g2,1\n  brf failed,r1\n  call r0,@1::0(),g0\n  ieq r1,r0,0\n  brf failed,r1\n  ieq r1,g2,1\n  brf failed,r1\n  ret\nfailed:\n  signal \"OTHER\",\"initializer capture oracle\"\n  ret\nmain() .locals=0\n  sayx \"PASS: ${_mode} initializer capture and once-only state\"\n  ret 0\n")
  file(WRITE "${_dir}/provider.rxas" [=[.globals=3
g0 .expose=inlineinit.count
g1 .expose=inlineinit.actual
g2 .expose=inlineinit.starts
hidden() .locals=0
  ret a0
boot() .locals=0
  .meta "inlineinit.provider.boot"="b" ".void" boot() ""
  .meta "inlineinit.provider.boot"=".initializer" ".void" boot() ""
  iadd g2,g2,1
  load g0,0
  load g1,20
  ret
]=])
  run_checked("assemble ${_mode} root" "${ASSEMBLER}" -n "${_dir}/root.rxas")
  run_checked("assemble ${_mode} provider" "${ASSEMBLER}" -n "${_dir}/provider.rxas")
  run_checked("seal ${_mode} image" "${MUTATOR}" join-seal "${_dir}/image.rxbin"
    "${_dir}/root.rxbin" "${_dir}/provider.rxbin")
  run_checked("${_mode} initializer semantics" "${VM}" "${_dir}/image.rxbin")
  if(NOT _stdout STREQUAL "PASS: ${_mode} initializer capture and once-only state" OR NOT _stderr STREQUAL "")
    message(FATAL_ERROR "${_mode} did not preserve its capture phase: [${_stdout}][${_stderr}]")
  endif()
endforeach()

# A failing provider initializer is entered once and poisons its overlay.
# IGNORE applies to the caller's resulting FAILURE. Both ordinary call forms
# resume after each rejected call, retaining explicit caller preparation and
# leaving the result and unrelated backing unchanged without a provider retry.
foreach(_mode counted fixed)
  set(_dir "${WORK}/failure-${_mode}")
  file(MAKE_DIRECTORY "${_dir}")
  if(_mode STREQUAL "counted")
    set(_failure_call "  call r0,@1::0(),r1")
  else()
    set(_failure_call "  call1 r0,@1::0(),r2")
  endif()
  set(_root [=[.globals=1
g0 .expose=inlineinit.attempts
boot() .locals=8
  .meta "inlineinit.root.boot"="b" ".void" boot() ""
  .meta "inlineinit.root.boot"=".initializer" ".void" boot() ""
  load r0,77
  load r1,0
  load r2,100
  load r3,200
  load r4,44
  load r5,55
  settp r2,0
  settp r3,0
  sigignore "FAILURE"
  settp r2,256
  swap r2,r3
@FAILURE_CALL@
  ieq r6,r0,77
  brf failed,r6
  ieq r6,r2,200
  brf failed,r6
  ieq r6,r3,100
  brf failed,r6
  getandtp r6,r3,256
  ieq r6,r6,256
  brf failed,r6
  ieq r6,r4,44
  brf failed,r6
  ieq r6,r5,55
  brf failed,r6
@FAILURE_CALL@
  ieq r6,r0,77
  brf failed,r6
  ieq r6,r4,44
  brf failed,r6
  ieq r6,r5,55
  brf failed,r6
  ieq r6,g0,1
  brf failed,r6
  sayx "PASS: ignored initializer failure resumes without retry"
  ret
failed:
  signal "OTHER","initializer departure oracle"
  ret
main() .locals=0
  sayx "FAIL: poisoned initializer reached main"
  ret 93
]=])
  string(REPLACE "@FAILURE_CALL@" "${_failure_call}" _root "${_root}")
  file(WRITE "${_dir}/root.rxas" "${_root}")
  file(WRITE "${_dir}/provider.rxas" [=[.globals=1
g0 .expose=inlineinit.attempts
hidden() .locals=0
  ret 12
boot() .locals=0
  .meta "inlineinit.provider.boot"="b" ".void" boot() ""
  .meta "inlineinit.provider.boot"=".initializer" ".void" boot() ""
  iadd g0,g0,1
  signal "OTHER","provider initializer fails"
  ret
]=])
  run_checked("assemble failing ${_mode} ingress root" "${ASSEMBLER}" -n "${_dir}/root.rxas")
  run_checked("assemble failing ${_mode} ingress provider" "${ASSEMBLER}" -n "${_dir}/provider.rxas")
  run_checked("seal failing ${_mode} ingress image" "${MUTATOR}" join-seal "${_dir}/image.rxbin"
    "${_dir}/root.rxbin" "${_dir}/provider.rxbin")
  execute_process(COMMAND "${VM}" "${_dir}/image.rxbin"
    RESULT_VARIABLE _rejected OUTPUT_VARIABLE _stdout ERROR_VARIABLE _stderr TIMEOUT 30)
  if(NOT "${_rejected}" MATCHES "^-?[0-9]+$" OR _rejected EQUAL 0 OR
     NOT _stdout STREQUAL "PASS: ignored initializer failure resumes without retry" OR
     NOT _stderr MATCHES "initialization previously failed")
    message(FATAL_ERROR "${_mode} initializer failure did not preserve resume/poison/preparation: ${_rejected}\n${_stdout}${_stderr}")
  endif()
endforeach()
file(REMOVE_RECURSE "${WORK}")
message("INLINE_INITIALIZER_CONTRACT|failures=0")

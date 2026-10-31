cmake_minimum_required(VERSION 3.24)
foreach(_required PROBE ASSEMBLER LINKER WORK)
  if(NOT DEFINED ${_required})
    message(FATAL_ERROR "Missing ${_required}")
  endif()
endforeach()
file(MAKE_DIRECTORY "${WORK}")
function(run_checked _label)
  execute_process(COMMAND ${ARGN} RESULT_VARIABLE _status
    OUTPUT_VARIABLE _stdout ERROR_VARIABLE _stderr TIMEOUT 30)
  if(NOT _status EQUAL 0)
    message(FATAL_ERROR "${_label}: ${_status}\n${_stdout}${_stderr}")
  endif()
  set(_stdout "${_stdout}" PARENT_SCOPE)
endfunction()
file(WRITE "${WORK}/rxvml_inline_owner.rxas" [=[.globals=0
authority.echo() .locals=0
  .meta "authority.echo"="b" ".int" authority.echo() "x=.int"
  ret a1
authority.box.add() .locals=2
  .meta "authority.box.add"="b" ".int" authority.box.add() "x=.int"
  linkattr r0,a1,0
  iadd r1,r0,a2
  ret r1
authority.helper() .locals=0 .expose=authority.helper
  .meta "authority.helper"="b" ".int" authority.helper() ""
  ret 43
]=])
file(WRITE "${WORK}/rxvml_inline_import.rxas" [=[.globals=0
authority.imported() .locals=1
  .meta "authority.imported"="b" ".int" authority.imported() ""
  call r0,authority.helper()
  ret r0
authority.helper() .expose=authority.helper
  .meta "authority.helper"="b" ".int" authority.helper() ""
]=])
# A private META_FUNC with the same descriptor lives in the trusted owner;
# context-wide discovery would choose this earlier exported impostor instead.
file(WRITE "${WORK}/rxvml_inline_shadow.rxas" [=[.globals=0
authority.echo() .locals=0 .expose=authority.echo
  .meta "authority.echo"="b" ".int" authority.echo() "x=.int"
  ret 91
]=])
file(WRITE "${WORK}/rxvml_inline_collision.rxas" [=[.globals=0
authority.helper() .locals=0 .expose=authority.helper
  .meta "authority.helper"="b" ".int" authority.helper() ""
  ret 91
]=])
file(WRITE "${WORK}/rxvml_inline_later.rxas" [=[.globals=0
authority.later() .locals=0
  .meta "authority.later"="b" ".int" authority.later() ""
  ret 19
]=])
foreach(_member owner import shadow collision later)
  run_checked("assemble ${_member}" "${ASSEMBLER}" -n "${WORK}/rxvml_inline_${_member}.rxas")
endforeach()
run_checked("link exact multi-member image" "${LINKER}" -o "${WORK}/linked"
  -r rxvml_inline_owner -r rxvml_inline_import
  "${WORK}/rxvml_inline_owner.rxbin" "${WORK}/rxvml_inline_import.rxbin")
run_checked("owned entry and closure oracle" "${PROBE}"
  "${WORK}/rxvml_inline_shadow.rxbin" "${WORK}/linked.rxbin"
  "${WORK}/rxvml_inline_later.rxbin" "${WORK}/rxvml_inline_collision.rxbin")
if(NOT _stdout MATCHES "PRIVATE_INLINE_OWNED[|]failures=0")
  message(FATAL_ERROR "Missing owned-entry marker: ${_stdout}")
endif()
file(REMOVE_RECURSE "${WORK}")
message("PRIVATE_INLINE_OWNED|failures=0")

foreach(required_var RXC RXAS RXVM RXBVM IMPORT_DIR LIBRARY SOURCE WORK)
  if(NOT DEFINED ${required_var})
    message(FATAL_ERROR "Missing required variable: ${required_var}")
  endif()
endforeach()

file(REMOVE_RECURSE "${WORK}")
file(MAKE_DIRECTORY "${WORK}")
execute_process(
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${SOURCE}" "${WORK}/probe.crexx"
  RESULT_VARIABLE res)
if(NOT res EQUAL 0)
  message(FATAL_ERROR "Failed to stage nested-this receiver test")
endif()

execute_process(
  COMMAND "${RXC}" -i "${IMPORT_DIR}" -o probe probe.crexx
  WORKING_DIRECTORY "${WORK}"
  OUTPUT_VARIABLE out
  ERROR_VARIABLE out
  RESULT_VARIABLE res)
if(NOT res EQUAL 0)
  message(FATAL_ERROR "rxc failed on nested-this receiver test: ${out}")
endif()

file(READ "${WORK}/probe.rxas" rxas)
set(direct_header "§inline_test_mutating_method_scalar_attr_return.probe.directattr()")
set(next_header "§inline_test_mutating_method_scalar_attr_return.probe.localthenattr()")
set(branch_header "§inline_test_mutating_method_scalar_attr_return.probe.branchcaller()")
set(local_branch_header "§inline_test_mutating_method_scalar_attr_return.probe.localbranchcaller()")
set(single_callee_header "§inline_test_mutating_method_scalar_attr_return.probe.singleexitandmutate()")
set(single_caller_header "§inline_test_mutating_method_scalar_attr_return.probe.singleexitcaller()")
set(calling_callee_header "§inline_test_mutating_method_scalar_attr_return.probe.callingsingleexitandmutate()")
set(calling_caller_header "§inline_test_mutating_method_scalar_attr_return.probe.callingsingleexitcaller()")
set(fallthrough_callee_header "§inline_test_mutating_method_scalar_attr_return.probe.fallthroughmutate()")
set(fallthrough_caller_header "§inline_test_mutating_method_scalar_attr_return.probe.fallthroughcaller()")
string(FIND "${rxas}" "${direct_header} .locals=" direct_start)
string(FIND "${rxas}" "${next_header} .locals=" next_start)
string(FIND "${rxas}" "${branch_header} .locals=" branch_start)
string(FIND "${rxas}" "${local_branch_header} .locals=" local_branch_start)
string(FIND "${rxas}" "${single_callee_header} .locals=" single_callee_start)
string(FIND "${rxas}" "${single_caller_header} .locals=" single_caller_start)
string(FIND "${rxas}" "${calling_callee_header} .locals=" calling_callee_start)
string(FIND "${rxas}" "${calling_caller_header} .locals=" calling_caller_start)
string(FIND "${rxas}" "${fallthrough_callee_header} .locals=" fallthrough_callee_start)
string(FIND "${rxas}" "${fallthrough_caller_header} .locals=" fallthrough_caller_start)
if(direct_start LESS 0 OR next_start LESS 0 OR next_start LESS_EQUAL direct_start OR
   branch_start LESS 0 OR local_branch_start LESS_EQUAL branch_start OR
   single_callee_start LESS_EQUAL local_branch_start OR
   single_caller_start LESS_EQUAL single_callee_start OR
   calling_callee_start LESS_EQUAL single_caller_start OR
   calling_caller_start LESS_EQUAL calling_callee_start OR
   fallthrough_callee_start LESS_EQUAL calling_caller_start OR
   fallthrough_caller_start LESS_EQUAL fallthrough_callee_start)
  message(FATAL_ERROR "Could not isolate optimized receiver proof bodies:\n${rxas}")
endif()
math(EXPR direct_length "${next_start} - ${direct_start}")
math(EXPR branch_length "${local_branch_start} - ${branch_start}")
math(EXPR local_branch_length "${single_callee_start} - ${local_branch_start}")
math(EXPR single_caller_length "${calling_callee_start} - ${single_caller_start}")
math(EXPR calling_caller_length "${fallthrough_callee_start} - ${calling_caller_start}")
string(SUBSTRING "${rxas}" ${direct_start} ${direct_length} direct_body)
string(SUBSTRING "${rxas}" ${branch_start} ${branch_length} branch_body)
string(SUBSTRING "${rxas}" ${local_branch_start} ${local_branch_length} local_branch_body)
string(SUBSTRING "${rxas}" ${single_caller_start} ${single_caller_length} single_caller_body)
string(SUBSTRING "${rxas}" ${calling_caller_start} ${calling_caller_length} calling_caller_body)
string(SUBSTRING "${rxas}" ${fallthrough_caller_start} -1 fallthrough_caller_body)

if(direct_body MATCHES [=[\n[ \t]*call[ \t]+[^\n]*(reset|changeandreturndifferent)\(\)]=])
  message(FATAL_ERROR "Nested methods were not inlined:\n${direct_body}")
endif()
if(direct_body MATCHES "copy[ \t]+r[0-9]+,a1")
  message(FATAL_ERROR
    "Proved nested §this receiver retained a materialization copy:\n${direct_body}")
endif()
if(direct_body MATCHES "copy[ \t]+a1,r[0-9]+")
  message(FATAL_ERROR
    "Proved nested §this receiver retained a copyback copy:\n${direct_body}")
endif()
if(branch_body MATCHES "call[0-9]*[ \t]+[^\n]*branchandmutate\\(\\)")
  if(branch_body MATCHES "copy[ \t]+r[0-9]+,a1" OR
     branch_body MATCHES "copy[ \t]+a1,r[0-9]+")
    message(FATAL_ERROR
      "Bounded ordinary §this receiver fallback retained inline materialisation copies:\n${branch_body}")
  endif()
elseif(NOT branch_body MATCHES "copy[ \t]+r[0-9]+,a1" OR
       NOT branch_body MATCHES "copy[ \t]+a1,r[0-9]+")
  message(FATAL_ERROR
    "Inlined multiple-return nested §this receiver lost its conservative copy path:\n${branch_body}")
endif()
if(NOT local_branch_body MATCHES "call[0-9]*[ \t]+[^\n]*branchandmutate\\(\\)" OR
   local_branch_body MATCHES "copy[ \t]+r[0-9]+,r1")
  message(FATAL_ERROR
    "Bounded ordinary direct-object receiver fallback regressed:\n${local_branch_body}")
endif()
if(NOT single_caller_body MATCHES "brf" OR
   single_caller_body MATCHES "copy[ \t]+r[0-9]+,a1" OR
   single_caller_body MATCHES "copy[ \t]+a1,r[0-9]+")
  message(FATAL_ERROR
    "Single-return branching nested §this was not directly placed:\n${single_caller_body}")
endif()
if(calling_caller_body MATCHES [=[\n[ \t]*call[0-9]*[ \t]+[^\n]*(reset|changeandreturndifferent|callingsingleexitandmutate)\(\)]=] OR
   calling_caller_body MATCHES "copy[ \t]+r[0-9]+,a1" OR
   calling_caller_body MATCHES "copy[ \t]+a1,r[0-9]+")
  message(FATAL_ERROR
    "Single-return call-bearing nested §this or its cloned aliases were materialized:\n${calling_caller_body}")
endif()
if(fallthrough_caller_body MATCHES "copy[ \t]+r[0-9]+,a1" OR
   fallthrough_caller_body MATCHES "copy[ \t]+a1,r[0-9]+")
  message(FATAL_ERROR
    "Fallthrough nested §this was not directly placed:\n${fallthrough_caller_body}")
endif()

execute_process(
  COMMAND "${RXAS}" -o probe.rxbin probe
  WORKING_DIRECTORY "${WORK}"
  OUTPUT_VARIABLE out
  ERROR_VARIABLE out
  RESULT_VARIABLE res)
if(NOT res EQUAL 0)
  message(FATAL_ERROR "rxas failed on nested-this receiver test: ${out}")
endif()

foreach(vm RXVM RXBVM)
  execute_process(
    COMMAND "${${vm}}" "${LIBRARY}" "${WORK}/probe.rxbin"
    WORKING_DIRECTORY "${WORK}"
    OUTPUT_VARIABLE out
    ERROR_VARIABLE err
    RESULT_VARIABLE res)
  if(NOT res EQUAL 0)
    message(FATAL_ERROR "${vm} failed on nested-this receiver test: ${out}${err}")
  endif()
  string(REPLACE "\r\n" "\n" out "${out}")
  if(NOT out STREQUAL "115\n115\n15\n111\n212\n111\n212\n13\n14\n115\n16\n")
    message(FATAL_ERROR "${vm} output mismatch: ${out}${err}")
  endif()
endforeach()

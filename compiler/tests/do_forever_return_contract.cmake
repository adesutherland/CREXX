cmake_minimum_required(VERSION 3.24)

foreach(required IN ITEMS RXC RXAS RXVM RXBVM LIBRARY SOURCE_DIR WORK_ROOT)
  if(NOT DEFINED ${required} OR "${${required}}" STREQUAL "")
    message(FATAL_ERROR "Missing required -D${required}=...")
  endif()
endforeach()

file(REMOVE_RECURSE "${WORK_ROOT}")
file(MAKE_DIRECTORY "${WORK_ROOT}")

function(run_checked label)
  execute_process(
    COMMAND ${ARGN}
    OUTPUT_VARIABLE out
    ERROR_VARIABLE err
    RESULT_VARIABLE result
  )
  if(NOT result EQUAL 0)
    message(FATAL_ERROR "${label} failed (${result}):\n${out}${err}")
  endif()
endfunction()

foreach(mode IN ITEMS noopt opt)
  if(mode STREQUAL "noopt")
    set(mode_flag -n)
  else()
    set(mode_flag)
  endif()

  set(work "${WORK_ROOT}/${mode}")
  file(MAKE_DIRECTORY "${work}")

  foreach(source IN ITEMS
      do_forever_return_contract.crexx
      do_forever_nonterminating.crexx
      do_forever_reachable_leave_fail.crexx
      do_forever_conditional_leave_fail.crexx
      do_forever_fallthrough_fail.crexx)
    file(COPY_FILE "${SOURCE_DIR}/${source}" "${work}/${source}" ONLY_IF_DIFFERENT)
  endforeach()

  foreach(positive IN ITEMS do_forever_return_contract do_forever_nonterminating)
    run_checked(
      "${mode}/${positive} compile"
      "${RXC}" ${mode_flag} -i "${LIBRARY}" -o "${work}/${positive}"
      "${work}/${positive}.crexx"
    )
  endforeach()

  foreach(runnable IN ITEMS do_forever_return_contract)
    run_checked(
      "${mode}/${runnable} assemble"
      "${RXAS}" ${mode_flag} -o "${work}/${runnable}.rxbin"
      "${work}/${runnable}"
    )
  endforeach()

  foreach(runner IN ITEMS RXVM RXBVM)
    execute_process(
      COMMAND "${${runner}}" "${LIBRARY}/library.rxbin"
              "${work}/do_forever_return_contract.rxbin"
      WORKING_DIRECTORY "${work}"
      OUTPUT_VARIABLE out
      ERROR_VARIABLE err
      RESULT_VARIABLE result
    )
    if(NOT result EQUAL 0)
      message(FATAL_ERROR "${mode}/${runner} failed (${result}):\n${out}${err}")
    endif()
    if(NOT out STREQUAL "DO_FOREVER_RETURN_OK\n")
      message(FATAL_ERROR "${mode}/${runner} output mismatch:\n${out}${err}")
    endif()
  endforeach()

  foreach(negative IN ITEMS
      do_forever_reachable_leave_fail
      do_forever_conditional_leave_fail
      do_forever_fallthrough_fail)
    execute_process(
      COMMAND "${RXC}" ${mode_flag} -i "${LIBRARY}"
              -o "${work}/${negative}" "${work}/${negative}.crexx"
      WORKING_DIRECTORY "${work}"
      OUTPUT_VARIABLE out
      ERROR_VARIABLE err
      RESULT_VARIABLE result
    )
    if(result EQUAL 0)
      message(FATAL_ERROR "${mode}/${negative} unexpectedly compiled")
    endif()
    set(diagnostic "${out}${err}")
    if(NOT diagnostic MATCHES "RETVAL_MISSING")
      message(FATAL_ERROR
        "${mode}/${negative} lacked RETVAL_MISSING:\n${diagnostic}")
    endif()
  endforeach()
endforeach()

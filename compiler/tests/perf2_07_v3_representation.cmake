if(NOT DEFINED RXC OR NOT DEFINED RXAS OR NOT DEFINED RXVM OR
   NOT DEFINED RXBVM OR NOT DEFINED LIBRARY OR NOT DEFINED SOURCE OR
   NOT DEFINED WORK)
  message(FATAL_ERROR "PERF2-07 V3 test requires RXC, RXAS, RXVM, RXBVM, LIBRARY, SOURCE and WORK")
endif()

file(REMOVE_RECURSE "${WORK}")
file(MAKE_DIRECTORY "${WORK}")

foreach(mode IN ITEMS noopt opt)
  set(compiler_flags "")
  if(mode STREQUAL "noopt")
    list(APPEND compiler_flags -n)
  endif()

  set(base "${WORK}/perf2_07_v3_${mode}")
  execute_process(
    COMMAND "${RXC}" -i "${LIBRARY_DIR}" ${compiler_flags} -o "${base}" "${SOURCE}"
    RESULT_VARIABLE rc
    OUTPUT_VARIABLE out
    ERROR_VARIABLE err)
  if(NOT rc EQUAL 0)
    message(FATAL_ERROR "${mode} rxc failed (${rc}):\n${out}${err}")
  endif()

  file(READ "${base}.rxas" rxas_text)
  if(NOT rxas_text MATCHES "dcopy r[0-9]+,r[0-9]+" OR
     NOT rxas_text MATCHES "dtos r[0-9]+" OR
     NOT rxas_text MATCHES "strlen r[0-9]+,r[0-9]+")
    message(FATAL_ERROR "${mode} RXAS lost the distinguishing dcopy/dtos/strlen sequence")
  endif()

  execute_process(
    COMMAND "${RXAS}" -o "${base}.rxbin" "${base}"
    RESULT_VARIABLE rc
    OUTPUT_VARIABLE out
    ERROR_VARIABLE err)
  if(NOT rc EQUAL 0)
    message(FATAL_ERROR "${mode} rxas failed (${rc}):\n${out}${err}")
  endif()

  foreach(runner IN ITEMS RXVM RXBVM)
    execute_process(
      COMMAND "${${runner}}" "${LIBRARY}" "${base}.rxbin" -a "2.2"
      RESULT_VARIABLE rc
      OUTPUT_VARIABLE out
      ERROR_VARIABLE err)
    if(NOT rc EQUAL 0)
      message(FATAL_ERROR "${mode} ${runner} failed (${rc}):\n${out}${err}")
    endif()
    if(NOT out STREQUAL "PASS: PERF2-07 V3 representation validity\n")
      message(FATAL_ERROR "${mode} ${runner} output mismatch:\n${out}${err}")
    endif()
  endforeach()
endforeach()

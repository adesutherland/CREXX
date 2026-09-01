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
      parse_levelg_contract.crexx
      parse_levelb_contract.crexx
      levelg_authored_assembler_fail.crexx)
    file(COPY_FILE "${SOURCE_DIR}/${source}" "${work}/${source}" ONLY_IF_DIFFERENT)
  endforeach()

  foreach(level IN ITEMS levelg levelb)
    set(base "${work}/parse_${level}_contract")
    run_checked(
      "${mode}/${level} compile"
      "${RXC}" ${mode_flag} -i "${LIBRARY}" -o "${base}"
      "${work}/parse_${level}_contract.crexx"
    )
    file(READ "${base}.rxas" rxas_text)
    if(NOT rxas_text MATCHES "[\n\r][ \t]+parseplan ")
      message(FATAL_ERROR
        "${mode}/${level} did not retain the certified frozen PARSE lowering")
    endif()
    run_checked(
      "${mode}/${level} assemble"
      "${RXAS}" ${mode_flag} -o "${base}.rxbin" "${base}.rxas"
    )
  endforeach()

  foreach(runner IN ITEMS RXVM RXBVM)
    foreach(level IN ITEMS levelg levelb)
      execute_process(
        COMMAND "${${runner}}" "${LIBRARY}/library.rxbin"
                "${work}/parse_${level}_contract.rxbin"
        WORKING_DIRECTORY "${work}"
        OUTPUT_VARIABLE out
        ERROR_VARIABLE err
        RESULT_VARIABLE result
      )
      if(NOT result EQUAL 0)
        message(FATAL_ERROR
          "${mode}/${level}/${runner} failed (${result}):\n${out}${err}")
      endif()
      string(TOUPPER "${level}" level_upper)
      if(NOT out STREQUAL "PARSE_${level_upper}_OK\n")
        message(FATAL_ERROR
          "${mode}/${level}/${runner} output mismatch:\n${out}${err}")
      endif()
    endforeach()
  endforeach()

  execute_process(
    COMMAND "${RXC}" ${mode_flag} -i "${LIBRARY}"
            -o "${work}/levelg_authored_assembler_fail"
            "${work}/levelg_authored_assembler_fail.crexx"
    WORKING_DIRECTORY "${work}"
    OUTPUT_VARIABLE out
    ERROR_VARIABLE err
    RESULT_VARIABLE result
  )
  if(result EQUAL 0)
    message(FATAL_ERROR "${mode}/authored Level G assembler unexpectedly compiled")
  endif()
  set(diagnostic "${out}${err}")
  if(NOT diagnostic MATCHES "ASSEMBLER_ONLY_LEVELB")
    message(FATAL_ERROR
      "${mode}/authored Level G assembler lacked ASSEMBLER_ONLY_LEVELB:\n${diagnostic}")
  endif()
endforeach()

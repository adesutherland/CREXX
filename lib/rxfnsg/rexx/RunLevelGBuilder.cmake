# Keep successful dependency-wave chatter in logs, while failures remain visible.
foreach(required VM BUILDER SOURCE WORK_ROOT BIN RXC RXAS RXLINK CONFIG JOBS)
  if(NOT DEFINED ${required})
    message(FATAL_ERROR "Missing Level G builder argument: ${required}")
  endif()
endforeach()
file(MAKE_DIRECTORY "${WORK_ROOT}")
set(stdout_log "${WORK_ROOT}/builder.stdout.log")
set(stderr_log "${WORK_ROOT}/builder.stderr.log")
execute_process(
  COMMAND "${VM}" "${BUILDER}" -a "${SOURCE}" "${WORK_ROOT}" "${BIN}"
          "${RXC}" "${RXAS}" "${RXLINK}" "${VM}" "${CMAKE_COMMAND}"
          "${CONFIG}" "${JOBS}" all
  OUTPUT_FILE "${stdout_log}" ERROR_FILE "${stderr_log}"
  RESULT_VARIABLE result)

function(read_log_tail path out)
  file(SIZE "${path}" size)
  math(EXPR offset "${size} - 16384")
  if(offset LESS 0)
    set(offset 0)
  endif()
  file(READ "${path}" text OFFSET ${offset})
  set(${out} "${text}" PARENT_SCOPE)
endfunction()

if(NOT "${result}" STREQUAL "0")
  read_log_tail("${stdout_log}" output)
  read_log_tail("${stderr_log}" errors)
  message(FATAL_ERROR
    "Level G builder failed (${result}). Full logs: ${stdout_log}, ${stderr_log}\n"
    "${output}${errors}")
endif()
if(VERBOSE)
  file(READ "${stdout_log}" output)
  if(NOT output STREQUAL "")
    message("${output}")
  endif()
endif()
# Preserve unexpected stderr diagnostics even on success.
file(READ "${stderr_log}" errors)
if(NOT errors STREQUAL "")
  message("${errors}")
endif()

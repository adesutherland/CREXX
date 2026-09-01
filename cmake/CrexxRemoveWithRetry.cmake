if(NOT DEFINED REMOVE_PATHS)
  message(FATAL_ERROR "REMOVE_PATHS is required")
endif()

if(NOT DEFINED REMOVE_RETRIES)
  set(REMOVE_RETRIES 120)
endif()

if(NOT DEFINED REMOVE_DELAY)
  set(REMOVE_DELAY 0.25)
endif()

if(NOT DEFINED REMOVE_CONTEXT)
  set(REMOVE_CONTEXT "file removal")
endif()

foreach(_path IN LISTS REMOVE_PATHS)
  set(_attempt 1)
  while(EXISTS "${_path}" OR IS_SYMLINK "${_path}")
    execute_process(
      COMMAND "${CMAKE_COMMAND}" -E rm -f "${_path}"
      RESULT_VARIABLE _remove_result
      OUTPUT_VARIABLE _remove_output
      ERROR_VARIABLE _remove_error
    )
    if(NOT EXISTS "${_path}" AND NOT IS_SYMLINK "${_path}")
      break()
    endif()
    if(_attempt GREATER_EQUAL REMOVE_RETRIES)
      message(FATAL_ERROR
        "${REMOVE_CONTEXT} is still locked after ${REMOVE_RETRIES} remove attempts: ${_path}\n"
        "On Windows this usually means a running test, VM, driver, shell, debugger, antivirus scanner, or indexer still has the executable/plugin open.\n"
        "${_remove_output}${_remove_error}")
    endif()
    math(EXPR _attempt "${_attempt} + 1")
    execute_process(COMMAND "${CMAKE_COMMAND}" -E sleep "${REMOVE_DELAY}")
  endwhile()
endforeach()

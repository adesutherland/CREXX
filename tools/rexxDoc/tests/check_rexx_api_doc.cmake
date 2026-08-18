foreach(required RXC RXAS RXVM LIBRARY SOURCE FIXTURE WORK_DIR)
  if(NOT DEFINED ${required})
    message(FATAL_ERROR "Missing required variable ${required}")
  endif()
endforeach()

file(MAKE_DIRECTORY "${WORK_DIR}")

execute_process(
  COMMAND "${RXC}" -i "${LIBRARY}" -o rexxApiDoc "${SOURCE}"
  WORKING_DIRECTORY "${WORK_DIR}"
  RESULT_VARIABLE result
  OUTPUT_VARIABLE output
  ERROR_VARIABLE error
)
if(NOT result EQUAL 0)
  message(FATAL_ERROR "rxc failed for rexxApiDoc: ${output}${error}")
endif()

execute_process(
  COMMAND "${RXAS}" rexxApiDoc
  WORKING_DIRECTORY "${WORK_DIR}"
  RESULT_VARIABLE result
  OUTPUT_VARIABLE output
  ERROR_VARIABLE error
)
if(NOT result EQUAL 0)
  message(FATAL_ERROR "rxas failed for rexxApiDoc: ${output}${error}")
endif()

execute_process(
  COMMAND "${RXVM}" "${LIBRARY}/library.rxbin" "${WORK_DIR}/rexxApiDoc.rxbin"
          -a "${FIXTURE}" --exposed
  WORKING_DIRECTORY "${WORK_DIR}"
  RESULT_VARIABLE result
  OUTPUT_VARIABLE api
  ERROR_VARIABLE error
)
if(NOT result EQUAL 0)
  message(FATAL_ERROR "rxvm failed for rexxApiDoc: ${api}${error}")
endif()

foreach(required_text
    "\\begin{CrexxClass}{\\textit{.first}}{apidoc\\_fixture}"
    "\\begin{CrexxClass}{.second}{apidoc\\_fixture}"
    "{alpha}"
    "{launch}"
    "{beta}")
  string(FIND "${api}" "${required_text}" position)
  if(position EQUAL -1)
    message(FATAL_ERROR "Generated API is missing ${required_text}")
  endif()
endforeach()

foreach(forbidden_text hidden hidden_only _private_first _private_second)
  string(FIND "${api}" "${forbidden_text}" position)
  if(NOT position EQUAL -1)
    message(FATAL_ERROR "Generated API leaked ${forbidden_text}")
  endif()
endforeach()

foreach(member alpha launch beta)
  string(REGEX MATCHALL "\\{${member}\\}" hits "${api}")
  list(LENGTH hits count)
  if(NOT count EQUAL 1)
    message(FATAL_ERROR "Generated API emitted ${member} ${count} times")
  endif()
endforeach()

message(STATUS "rexxApiDoc exposed-only and task-member regression passed")

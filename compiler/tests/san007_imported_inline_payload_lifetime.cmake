foreach(required RXC IMPORT_DIR SOURCE WORK)
  if(NOT DEFINED ${required} OR "${${required}}" STREQUAL "")
    message(FATAL_ERROR "Missing required -D${required}=... argument")
  endif()
endforeach()

if(NOT EXISTS "${SOURCE}")
  message(FATAL_ERROR "Missing SAN-007 source fixture: ${SOURCE}")
endif()

file(REMOVE_RECURSE "${WORK}")
file(MAKE_DIRECTORY "${WORK}")

execute_process(
  COMMAND "${RXC}"
          -x
          -i "${IMPORT_DIR}"
          -o "${WORK}/main"
          "${SOURCE}"
  WORKING_DIRECTORY "${WORK}"
  OUTPUT_VARIABLE compiler_output
  ERROR_VARIABLE compiler_error
  RESULT_VARIABLE compiler_result)
if(NOT compiler_result EQUAL 0)
  message(FATAL_ERROR
          "SAN-007 focused compiler regression failed (${compiler_result}).\n"
          "stdout:\n${compiler_output}\n"
          "stderr:\n${compiler_error}")
endif()

if(NOT EXISTS "${WORK}/main.rxas")
  message(FATAL_ERROR "SAN-007 focused compiler regression did not emit main.rxas")
endif()

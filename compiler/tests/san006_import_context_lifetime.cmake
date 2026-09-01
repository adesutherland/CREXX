foreach(required RXC CLASS_AGGREGATE CLASS_MEMBER CLASS_SOURCE MAIN_SOURCE WORK)
  if(NOT DEFINED ${required} OR "${${required}}" STREQUAL "")
    message(FATAL_ERROR "Missing required -D${required}=... argument")
  endif()
endforeach()

foreach(input CLASS_AGGREGATE CLASS_MEMBER CLASS_SOURCE MAIN_SOURCE)
  if(NOT EXISTS "${${input}}")
    message(FATAL_ERROR "Missing SAN-006 fixture input ${input}: ${${input}}")
  endif()
endforeach()

file(REMOVE_RECURSE "${WORK}")
file(MAKE_DIRECTORY "${WORK}")

execute_process(
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different
          "${CLASS_AGGREGATE}" "${WORK}/classlib_native.rxbin"
  RESULT_VARIABLE copy_result)
if(NOT copy_result EQUAL 0)
  message(FATAL_ERROR "Failed to stage SAN-006 aggregate image")
endif()

execute_process(
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different
          "${CLASS_MEMBER}" "${WORK}/KeyDB.rxbin"
  RESULT_VARIABLE copy_result)
if(NOT copy_result EQUAL 0)
  message(FATAL_ERROR "Failed to stage SAN-006 individual class image")
endif()

execute_process(
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different
          "${CLASS_SOURCE}" "${WORK}/KeyDB.crexx"
  RESULT_VARIABLE copy_result)
if(NOT copy_result EQUAL 0)
  message(FATAL_ERROR "Failed to stage SAN-006 source class")
endif()

execute_process(
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different
          "${MAIN_SOURCE}" "${WORK}/main.crexx"
  RESULT_VARIABLE copy_result)
if(NOT copy_result EQUAL 0)
  message(FATAL_ERROR "Failed to stage SAN-006 primary source")
endif()

execute_process(
  COMMAND "${RXC}"
          -x --no-exe-import
          -i "${WORK}"
          -o "${WORK}/main"
          "${WORK}/main.crexx"
  WORKING_DIRECTORY "${WORK}"
  OUTPUT_VARIABLE compiler_output
  ERROR_VARIABLE compiler_error
  RESULT_VARIABLE compiler_result)
if(NOT compiler_result EQUAL 0)
  message(FATAL_ERROR
          "SAN-006 focused compiler regression failed (${compiler_result}).\n"
          "stdout:\n${compiler_output}\n"
          "stderr:\n${compiler_error}")
endif()

if(NOT EXISTS "${WORK}/main.rxas")
  message(FATAL_ERROR "SAN-006 focused compiler regression did not emit main.rxas")
endif()

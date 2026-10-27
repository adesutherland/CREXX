if(NOT DEFINED CREXX OR NOT DEFINED WORK_ROOT)
    message(FATAL_ERROR "CREXX and WORK_ROOT are required")
endif()

file(REMOVE_RECURSE "${WORK_ROOT}")
file(MAKE_DIRECTORY "${WORK_ROOT}")
set(plan "${WORK_ROOT}/malformed plan.build")
file(WRITE "${plan}" "not a build plan;\n")

# A single input reaches the real sibling runner; no compiler action is needed.
execute_process(COMMAND "${CREXX}" "${plan}"
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE errors TIMEOUT 20)
if(result STREQUAL "0" OR NOT "${output}${errors}" MATCHES "RXBUILD_PARSE")
    message(FATAL_ERROR "Single .build input did not reach rxbuild: ${result}\n${output}${errors}")
endif()

# Extra options and filenames must fail before the malformed plan is opened.
foreach(extra IN ITEMS "--help" "${WORK_ROOT}/second.build")
    execute_process(COMMAND "${CREXX}" "${plan}" "${extra}"
        RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE errors TIMEOUT 20)
    if(NOT result STREQUAL "2" OR
       NOT "${output}${errors}" MATCHES "exactly one [.][bB][uU][iI][lL][dD] input" OR
       "${output}${errors}" MATCHES "RXBUILD")
        message(FATAL_ERROR "Extra .build argument was not refused by the driver: ${result}\n${output}${errors}")
    endif()
endforeach()

file(REMOVE_RECURSE "${WORK_ROOT}")
message(STATUS "CREXX_BUILD_DISPATCH|PASS")

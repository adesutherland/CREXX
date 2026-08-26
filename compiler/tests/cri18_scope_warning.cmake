foreach(_required IN ITEMS RXC IMPORT_DIR WORK CRI18_SOURCE ASSIGN_SOURCE EXPLICIT_SOURCE)
    if(NOT DEFINED ${_required} OR "${${_required}}" STREQUAL "")
        message(FATAL_ERROR "Missing required variable ${_required}")
    endif()
endforeach()

file(MAKE_DIRECTORY "${WORK}")

function(_compile_and_check NAME SOURCE EXPECTED_WARNINGS)
    execute_process(
            COMMAND "${RXC}" -i "${IMPORT_DIR}" -n -o "${NAME}" "${SOURCE}"
            WORKING_DIRECTORY "${WORK}"
            OUTPUT_VARIABLE _stdout
            ERROR_VARIABLE _stderr
            RESULT_VARIABLE _result)
    set(_output "${_stdout}${_stderr}")
    if(NOT _result EQUAL 0)
        message(FATAL_ERROR "${NAME}: rxc failed with ${_result}\n${_output}")
    endif()
    string(REGEX MATCHALL "#NOT_IN_SAME_SCOPE" _matches "${_output}")
    list(LENGTH _matches _warning_count)
    if(NOT _warning_count EQUAL ${EXPECTED_WARNINGS})
        message(FATAL_ERROR
                "${NAME}: expected ${EXPECTED_WARNINGS} NOT_IN_SAME_SCOPE warning(s), got ${_warning_count}\n${_output}")
    endif()
endfunction()

_compile_and_check(cri18_disjoint_scope_warning "${CRI18_SOURCE}" 1)
_compile_and_check(disjoint_assignment_warning "${ASSIGN_SOURCE}" 2)
_compile_and_check(disjoint_explicit_intent "${EXPLICIT_SOURCE}" 0)

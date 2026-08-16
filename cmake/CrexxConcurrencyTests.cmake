include_guard(GLOBAL)

# Attach the enduring concurrency manifest labels to tests in the directory
# where they were created.  CTest's `concurrency` label is the complete initial
# closeout matrix; `concurrency-spNN` labels provide reviewable solution-point
# subsets without relying on historical stage names or test-name grep rules.
function(crexx_label_concurrency_tests SOLUTION_POINT)
    string(TOLOWER "${SOLUTION_POINT}" _crexx_concurrency_solution_point)
    if(NOT _crexx_concurrency_solution_point MATCHES "^sp0[1-9]$")
        message(FATAL_ERROR
                "Concurrency solution point must be SP01 through SP09, got '${SOLUTION_POINT}'")
    endif()
    if(NOT ARGN)
        message(FATAL_ERROR
                "No tests supplied for concurrency ${SOLUTION_POINT}")
    endif()
    foreach(_crexx_concurrency_test IN LISTS ARGN)
        if(NOT TEST "${_crexx_concurrency_test}")
            message(FATAL_ERROR
                    "Unknown concurrency test '${_crexx_concurrency_test}' in ${SOLUTION_POINT}")
        endif()
        set_property(TEST "${_crexx_concurrency_test}" APPEND PROPERTY LABELS
                concurrency
                "concurrency-${_crexx_concurrency_solution_point}")
    endforeach()
endfunction()

function(crexx_label_concurrency_stress_tests SOLUTION_POINT)
    crexx_label_concurrency_tests("${SOLUTION_POINT}" ${ARGN})
    foreach(_crexx_concurrency_test IN LISTS ARGN)
        set_property(TEST "${_crexx_concurrency_test}" APPEND PROPERTY LABELS
                concurrency-stress)
    endforeach()
endfunction()

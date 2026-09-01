foreach(_required IN ITEMS RXC IMPORT_DIR WORK CRI18_SOURCE ASSIGN_SOURCE EXPLICIT_SOURCE LEXICAL_SOURCE)
    if(NOT DEFINED ${_required} OR "${${_required}}" STREQUAL "")
        message(FATAL_ERROR "Missing required variable ${_required}")
    endif()
endforeach()

file(MAKE_DIRECTORY "${WORK}")

function(_compile_and_check NAME SOURCE)
    execute_process(
            COMMAND "${CMAKE_COMMAND}" -E env CREXX_DIAGNOSTICS=raw
                    "${RXC}" -i "${IMPORT_DIR}" -n -o "${NAME}" "${SOURCE}"
            WORKING_DIRECTORY "${WORK}"
            OUTPUT_VARIABLE _stdout
            ERROR_VARIABLE _stderr
            RESULT_VARIABLE _result)
    set(_output "${_stdout}${_stderr}")
    if(NOT _result EQUAL 0)
        message(FATAL_ERROR "${NAME}: rxc failed with ${_result}\n${_output}")
    endif()
    set(${NAME}_OUTPUT "${_output}" PARENT_SCOPE)
endfunction()

_compile_and_check(cri18_disjoint_scope_warning "${CRI18_SOURCE}")
set(_cri18_expected
    "Warning in cri18_disjoint_scope_warning\\.crexx @ 19:8 - #NOT_IN_SAME_SCOPE line=\"17\" column=\"5\", \"result\"")
if(NOT cri18_disjoint_scope_warning_OUTPUT MATCHES "${_cri18_expected}")
    message(FATAL_ERROR "CRI-18: exact result warning missing or moved\n${cri18_disjoint_scope_warning_OUTPUT}")
endif()
string(REGEX MATCHALL "#NOT_IN_SAME_SCOPE" _cri18_matches "${cri18_disjoint_scope_warning_OUTPUT}")
list(LENGTH _cri18_matches _cri18_count)
if(NOT _cri18_count EQUAL 1)
    message(FATAL_ERROR "CRI-18: expected exactly one result warning, got ${_cri18_count}\n${cri18_disjoint_scope_warning_OUTPUT}")
endif()

_compile_and_check(disjoint_assignment_warning "${ASSIGN_SOURCE}")
foreach(_expected IN ITEMS
        "Warning in disjoint_assignment_warning\\.crexx @ 7:3 - #NOT_IN_SAME_SCOPE line=\"5\" column=\"5\", \"i\""
        "Warning in disjoint_assignment_warning\\.crexx @ 12:7 - #NOT_IN_SAME_SCOPE line=\"10\" column=\"5\", \"j\"")
    if(NOT disjoint_assignment_warning_OUTPUT MATCHES "${_expected}")
        message(FATAL_ERROR "CRI-18: exact assignment/read warning missing or moved\n${disjoint_assignment_warning_OUTPUT}")
    endif()
endforeach()
string(REGEX MATCHALL "#NOT_IN_SAME_SCOPE" _assignment_matches "${disjoint_assignment_warning_OUTPUT}")
list(LENGTH _assignment_matches _assignment_count)
if(NOT _assignment_count EQUAL 2)
    message(FATAL_ERROR "CRI-18: expected exactly two assignment/read warnings, got ${_assignment_count}\n${disjoint_assignment_warning_OUTPUT}")
endif()

_compile_and_check(disjoint_explicit_intent "${EXPLICIT_SOURCE}")
if(disjoint_explicit_intent_OUTPUT MATCHES "#NOT_IN_SAME_SCOPE")
    message(FATAL_ERROR "CRI-18: explicit declarations must suppress disjoint-scope warnings\n${disjoint_explicit_intent_OUTPUT}")
endif()

_compile_and_check(disjoint_lexical_paths "${LEXICAL_SOURCE}")
foreach(_expected IN ITEMS
        "Warning in disjoint_lexical_paths\\.crexx @ 15:3 - #NOT_IN_SAME_SCOPE line=\"12\" column=\"5\", \"value\""
        "Warning in disjoint_lexical_paths\\.crexx @ 24:5 - #NOT_IN_SAME_SCOPE line=\"21\" column=\"5\", \"item\""
        "Warning in disjoint_lexical_paths\\.crexx @ 32:6 - #NOT_IN_SAME_SCOPE line=\"29\" column=\"6\", \"i\"")
    if(NOT disjoint_lexical_paths_OUTPUT MATCHES "${_expected}")
        message(FATAL_ERROR "CRI-18: lexical/path-independent warning missing or moved\n${disjoint_lexical_paths_OUTPUT}")
    endif()
endforeach()
string(REGEX MATCHALL "#NOT_IN_SAME_SCOPE" _lexical_matches "${disjoint_lexical_paths_OUTPUT}")
list(LENGTH _lexical_matches _lexical_count)
if(NOT _lexical_count EQUAL 3)
    message(FATAL_ERROR "CRI-18: expected exactly three lexical warnings, got ${_lexical_count}\n${disjoint_lexical_paths_OUTPUT}")
endif()

execute_process(
        COMMAND "${CMAKE_COMMAND}" -E env CREXX_DIAGNOSTICS=localized
                "${RXC}" --diagnostic-locale en_GB -i "${IMPORT_DIR}" -n
                -o disjoint_assignment_localized "${ASSIGN_SOURCE}"
        WORKING_DIRECTORY "${WORK}"
        OUTPUT_VARIABLE _localized_stdout
        ERROR_VARIABLE _localized_stderr
        RESULT_VARIABLE _localized_result)
set(_localized_output "${_localized_stdout}${_localized_stderr}")
if(NOT _localized_result EQUAL 0)
    message(FATAL_ERROR "CRI-18: localized rxc failed with ${_localized_result}\n${_localized_output}")
endif()
if(NOT _localized_output MATCHES
   "#NOT_IN_SAME_SCOPE: Implicit use creates a separate binding; the earlier same-named binding is at line 5, column 5\\.")
    message(FATAL_ERROR "CRI-18: actionable localized message missing prior binding location\n${_localized_output}")
endif()

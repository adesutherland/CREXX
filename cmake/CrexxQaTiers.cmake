include_guard(GLOBAL)

# Hand-written tests do not pass through the runtime-matrix helpers that can
# infer their generated artifact and runner prerequisites. Let their owning
# directory declare those producer targets explicitly while the test and the
# producer are still easy to understand together.
function(crexx_register_test_prep_targets)
    cmake_parse_arguments(CREXX "" "" "TESTS;TARGETS" ${ARGN})
    if(CREXX_UNPARSED_ARGUMENTS OR NOT CREXX_TESTS OR NOT CREXX_TARGETS)
        message(FATAL_ERROR
                "crexx_register_test_prep_targets requires TESTS and TARGETS")
    endif()

    foreach(_crexx_test IN LISTS CREXX_TESTS)
        if(TEST "${_crexx_test}")
            set_property(TEST "${_crexx_test}" APPEND PROPERTY
                    CREXX_PREP_TARGETS ${CREXX_TARGETS})
        endif()
    endforeach()
endfunction()

# Shared test helpers use this to request one final pass after the caller's
# CMakeLists has declared all of its direct and generated tests.
function(crexx_schedule_directory_qa_tier_finalization)
    cmake_language(DEFER GET_CALL_IDS _crexx_deferred_call_ids)
    if(NOT "crexx_qa_tier_finalization" IN_LIST _crexx_deferred_call_ids)
        cmake_language(DEFER ID crexx_qa_tier_finalization
                CALL crexx_finalize_directory_qa_tiers)
    endif()
endfunction()

# Finalize the tests declared in the caller's directory.  Existing topical
# labels remain intact; this adds exactly one scheduling tier so CTest
# selections do not have to infer intent from a component or test name.
function(crexx_finalize_directory_qa_tiers)
    get_property(_crexx_tests DIRECTORY PROPERTY TESTS)
    foreach(_crexx_test IN LISTS _crexx_tests)
        get_property(_crexx_labels TEST "${_crexx_test}" PROPERTY LABELS)
        if(NOT _crexx_labels OR _crexx_labels MATCHES "NOTFOUND$")
            set(_crexx_labels)
        endif()

        set(_crexx_explicit_tiers)
        foreach(_crexx_label IN LISTS _crexx_labels)
            if(_crexx_label STREQUAL "essential" OR
               _crexx_label STREQUAL "smoke" OR
               _crexx_label STREQUAL "comprehensive" OR
               _crexx_label STREQUAL "qualification" OR
               _crexx_label STREQUAL "stress" OR
               _crexx_label STREQUAL "performance-measurement")
                list(APPEND _crexx_explicit_tiers "${_crexx_label}")
            endif()
        endforeach()
        list(REMOVE_DUPLICATES _crexx_explicit_tiers)
        list(LENGTH _crexx_explicit_tiers _crexx_explicit_tier_count)
        if(_crexx_explicit_tier_count GREATER 1)
            message(FATAL_ERROR
                    "Test ${_crexx_test} declares multiple QA tiers: ${_crexx_explicit_tiers}")
        endif()

        if(_crexx_explicit_tier_count EQUAL 1)
            list(GET _crexx_explicit_tiers 0 _crexx_tier)
        else()
            string(TOLOWER "${_crexx_test}" _crexx_test_lower)
            set(_crexx_has_stress_label FALSE)
            foreach(_crexx_label IN LISTS _crexx_labels)
                string(TOLOWER "${_crexx_label}" _crexx_label_lower)
                if(_crexx_label_lower MATCHES "stress")
                    set(_crexx_has_stress_label TRUE)
                endif()
            endforeach()

            if(_crexx_test_lower MATCHES "stress" OR _crexx_has_stress_label)
                set(_crexx_tier stress)
            elseif("smoke" IN_LIST _crexx_labels)
                set(_crexx_tier smoke)
            elseif("qualification" IN_LIST _crexx_labels OR
                   "install" IN_LIST _crexx_labels OR
                   "package" IN_LIST _crexx_labels OR
                   "external-consumer" IN_LIST _crexx_labels OR
                   "reproducibility" IN_LIST _crexx_labels)
                set(_crexx_tier qualification)
            elseif("unit" IN_LIST _crexx_labels OR "contract" IN_LIST _crexx_labels)
                set(_crexx_tier essential)
            else()
                set(_crexx_tier comprehensive)
            endif()
            set_property(TEST "${_crexx_test}" APPEND PROPERTY LABELS "${_crexx_tier}")
        endif()

        if(_crexx_tier STREQUAL "performance-measurement")
            get_property(_crexx_run_serial TEST "${_crexx_test}" PROPERTY RUN_SERIAL)
            if(NOT _crexx_run_serial)
                message(FATAL_ERROR
                        "Measurement test ${_crexx_test} must declare RUN_SERIAL")
            endif()
        endif()

        get_property(_crexx_prep_targets TEST "${_crexx_test}" PROPERTY
                CREXX_PREP_TARGETS)
        if(_crexx_prep_targets AND
           NOT _crexx_prep_targets MATCHES "NOTFOUND$")
            if(_crexx_tier STREQUAL "performance-measurement")
                set(_crexx_prep_tier measurement)
            else()
                set(_crexx_prep_tier "${_crexx_tier}")
            endif()
            string(TOUPPER "${_crexx_prep_tier}" _crexx_prep_tier_upper)
            set_property(GLOBAL APPEND PROPERTY
                    "CREXX_QA_PREP_${_crexx_prep_tier_upper}_TARGETS"
                    ${_crexx_prep_targets})
        endif()
    endforeach()
endfunction()

# Test directories are configured before every possible producer target exists.
# Resolve and attach the collected names once the complete tree is declared.
function(crexx_finalize_qa_prep_targets)
    foreach(_crexx_prep_tier IN ITEMS
            essential smoke comprehensive qualification stress measurement)
        string(TOUPPER "${_crexx_prep_tier}" _crexx_prep_tier_upper)
        get_property(_crexx_prep_targets GLOBAL PROPERTY
                "CREXX_QA_PREP_${_crexx_prep_tier_upper}_TARGETS")
        if(NOT _crexx_prep_targets)
            continue()
        endif()
        list(REMOVE_DUPLICATES _crexx_prep_targets)
        set(_crexx_prep_aggregate "qa-prep-${_crexx_prep_tier}")
        if(NOT TARGET "${_crexx_prep_aggregate}")
            message(FATAL_ERROR
                    "Missing QA preparation aggregate: ${_crexx_prep_aggregate}")
        endif()
        foreach(_crexx_prep_target IN LISTS _crexx_prep_targets)
            if(NOT TARGET "${_crexx_prep_target}")
                message(FATAL_ERROR
                        "QA tier ${_crexx_prep_tier} has missing preparation dependency: ${_crexx_prep_target}")
            endif()
            add_dependencies("${_crexx_prep_aggregate}"
                    "${_crexx_prep_target}")
        endforeach()
    endforeach()
endfunction()

include_guard(GLOBAL)

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
    endforeach()
endfunction()

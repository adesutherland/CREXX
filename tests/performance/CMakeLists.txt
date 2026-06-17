set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

set(LIBRPATH ${CMAKE_BINARY_DIR}/bin/library)
set(CLASSLIBRPATH ${CMAKE_BINARY_DIR}/bin/classlib)

# These benchmarks are smoke-run under CTest so opt/noopt timing stays visible,
# but they do not assert performance thresholds because those vary by machine.
add_custom_target(performance_inline ALL)

crexx_add_rexx_opt_matrix(
        NAME inlining_simple
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/inlining_simple.crexx
        TARGET_GROUP performance_inline
        DEPENDS library compiler_exit_bin
        TEST_LABELS "performance;inlining"
)

add_custom_target(performance_reference_iterator ALL)
foreach(_crexx_runner IN ITEMS rxvm rxbvm)
    crexx_add_rexx_opt_matrix(
            NAME reference_iterator_compare_${_crexx_runner}
            SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/reference_iterator_compare.crexx
            TARGET_GROUP performance_reference_iterator
            RUNNER ${_crexx_runner}
            IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin
            DEPENDS library compiler_exit_bin
            RUNTIME_ARGS ${LIBRPATH}
            TEST_LABELS "performance;reference"
    )
    foreach(_crexx_mode IN ITEMS noopt opt)
        set_tests_properties(reference_iterator_compare_${_crexx_runner}_${_crexx_mode} PROPERTIES
                PASS_REGULAR_EXPRESSION "PASS: Reference Iterator Source Performance Smoke"
                FAIL_REGULAR_EXPRESSION "FAIL:|PANIC:"
        )
    endforeach()
endforeach()

add_custom_target(performance_classlib_iterator ALL)
foreach(_crexx_runner IN ITEMS rxvm rxbvm)
    crexx_add_rexx_opt_matrix(
            NAME stringarraylist_iterator_compare_${_crexx_runner}
            SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/stringarraylist_iterator_compare.crexx
            TARGET_GROUP performance_classlib_iterator
            RUNNER ${_crexx_runner}
            IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin
            DEPENDS library classlib compiler_exit_bin
            RUNTIME_ARGS ${LIBRPATH} ${CLASSLIBRPATH}
            TEST_LABELS "performance;reference;classlib"
    )
    foreach(_crexx_mode IN ITEMS noopt opt)
        set_tests_properties(stringarraylist_iterator_compare_${_crexx_runner}_${_crexx_mode} PROPERTIES
                PASS_REGULAR_EXPRESSION "PASS: StringArrayList StringIterator Performance Smoke"
                FAIL_REGULAR_EXPRESSION "FAIL:|PANIC:"
        )
    endforeach()
endforeach()

crexx_add_rexx_opt_matrix(
        NAME inlining_complex
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/inlining_complex.crexx
        TARGET_GROUP performance_inline
        DEPENDS library compiler_exit_bin
        TEST_LABELS "performance;inlining"
)

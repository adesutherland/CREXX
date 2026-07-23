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

# crexx_add_rexx_opt_matrix registers optimized images with the shared runtime
# fixture, while these NR-15 target groups are deliberately not part of ALL.
# Make the same serialized fixture own both groups so an isolated CTest run can
# generate the four noopt images instead of depending on a warm build tree.
add_dependencies(linked_opt_runtime_artifacts
        performance_nr15_stem_semantics
        performance_nr15_stem_access)

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

add_custom_target(performance_classlib_treemap ALL)
crexx_add_rexx_opt_matrix(
        NAME stringtreemap_insert_compare
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/stringtreemap_insert_compare.crexx
        TARGET_GROUP performance_classlib_treemap
        IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin
        DEPENDS library classlib compiler_exit_bin
        RUNTIME_ARGS ${LIBRPATH} ${CLASSLIBRPATH}
        TEST_LABELS "performance;classlib;treemap"
)
foreach(_crexx_mode IN ITEMS noopt opt)
    set_tests_properties(stringtreemap_insert_compare_${_crexx_mode} PROPERTIES
            PASS_REGULAR_EXPRESSION "PASS: StringTreeMap Insert Comparison"
            FAIL_REGULAR_EXPRESSION "FAIL:|PANIC:"
    )
endforeach()

add_custom_target(performance_binary_fastpath ALL)
crexx_add_rexx_opt_matrix(
        NAME binary_fastpath_compare
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/binary_fastpath_compare.crexx
        TARGET_GROUP performance_binary_fastpath
        IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin
        DEPENDS library compiler_exit_bin
        RUNTIME_ARGS ${LIBRPATH}
        TEST_LABELS "performance;binary"
)
foreach(_crexx_mode IN ITEMS noopt opt)
    set_tests_properties(binary_fastpath_compare_${_crexx_mode} PROPERTIES
            PASS_REGULAR_EXPRESSION "PASS: Binary Fastpath Compare"
            FAIL_REGULAR_EXPRESSION "FAIL:|PANIC:"
    )
endforeach()

add_custom_target(performance_rxjson_parser ALL)
crexx_add_rexx_opt_matrix(
        NAME rxjson_parser_compare
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/rxjson_parser_compare.crexx
        TARGET_GROUP performance_rxjson_parser
        IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin
        DEPENDS library compiler_exit_bin
        RUNTIME_ARGS ${LIBRPATH}
        TEST_LABELS "performance;json"
)
foreach(_crexx_mode IN ITEMS noopt opt)
    set_tests_properties(rxjson_parser_compare_${_crexx_mode} PROPERTIES
            PASS_REGULAR_EXPRESSION "PASS: rxjson parser compare"
            FAIL_REGULAR_EXPRESSION "FAIL:|PANIC:"
    )
endforeach()

add_custom_target(performance_tinyexpr_dispatch ALL)
crexx_add_rexx_opt_matrix(
        NAME tinyexpr_dispatch_compare
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/tinyexpr_dispatch_compare.crexx
        TARGET_GROUP performance_tinyexpr_dispatch
        IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin
        RXC_FLAGS -x
        DEPENDS library rxfnsl compiler_exit_bin
        RUNTIME_ARGS ${LIBRPATH} ${CMAKE_BINARY_DIR}/bin/rxfnsl
        TEST_LABELS "performance;dispatch;level-l"
)
foreach(_crexx_mode IN ITEMS noopt opt)
    set_tests_properties(tinyexpr_dispatch_compare_${_crexx_mode} PROPERTIES
            PASS_REGULAR_EXPRESSION "PASS: tinyexpr dispatch compare"
            FAIL_REGULAR_EXPRESSION "FAIL:|PANIC:"
    )
endforeach()

add_custom_target(performance_jump_dispatch ALL)
crexx_add_rexx_opt_matrix(
        NAME jump_dispatch_compare
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/jump_dispatch_compare.crexx
        TARGET_GROUP performance_jump_dispatch
        IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin
        DEPENDS library compiler_exit_bin
        RUNTIME_ARGS ${LIBRPATH}
        TEST_LABELS "performance;dispatch"
)
foreach(_crexx_mode IN ITEMS noopt opt)
    set_tests_properties(jump_dispatch_compare_${_crexx_mode} PROPERTIES
            PASS_REGULAR_EXPRESSION "PASS: jump dispatch compare"
            FAIL_REGULAR_EXPRESSION "FAIL:|PANIC:"
    )
endforeach()

add_custom_target(performance_runtime_lookup ALL)
crexx_add_rexx_opt_matrix(
        NAME runtime_interface_lookup_compare
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/runtime_interface_lookup_compare.crexx
        TARGET_GROUP performance_runtime_lookup
        IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin
        DEPENDS library compiler_exit_bin
        RUNTIME_ARGS ${LIBRPATH}
        TEST_LABELS "performance;vm;interfaces"
)
foreach(_crexx_mode IN ITEMS noopt opt)
    set_tests_properties(runtime_interface_lookup_compare_${_crexx_mode} PROPERTIES
            PASS_REGULAR_EXPRESSION "PASS: runtime interface lookup compare"
            FAIL_REGULAR_EXPRESSION "FAIL:|PANIC:"
    )
endforeach()

crexx_add_rexx_opt_matrix(
        NAME inlining_complex
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/inlining_complex.crexx
        TARGET_GROUP performance_inline
        DEPENDS library compiler_exit_bin
        TEST_LABELS "performance;inlining"
)

add_custom_target(performance_nr15_stem_semantics)
add_custom_target(performance_nr15_stem_access)
foreach(_crexx_runner IN ITEMS rxvm rxbvm)
    crexx_add_rexx_opt_matrix(
            NAME nr15_stem_semantics_${_crexx_runner}
            SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/nr15_stem_semantics.crexx
            TARGET_GROUP performance_nr15_stem_semantics
            RUNNER ${_crexx_runner}
            IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin
            DEPENDS library compiler_exit_bin
            RUNTIME_ARGS ${LIBRPATH}
            TEST_LABELS "performance;stem;nr15;semantics"
    )
    crexx_add_rexx_opt_matrix(
            NAME nr15_stem_access_${_crexx_runner}
            SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/nr15_stem_access_compare.crexx
            TARGET_GROUP performance_nr15_stem_access
            RUNNER ${_crexx_runner}
            IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin
            DEPENDS library compiler_exit_bin
            RUNTIME_ARGS ${LIBRPATH}
            TEST_LABELS "performance;stem;nr15"
    )
    foreach(_crexx_mode IN ITEMS noopt opt)
        set_tests_properties(nr15_stem_semantics_${_crexx_runner}_${_crexx_mode} PROPERTIES
                PASS_REGULAR_EXPRESSION "PASS: NR-15 stem semantic matrix"
                FAIL_REGULAR_EXPRESSION "FAIL:|PANIC:"
        )
        set_tests_properties(nr15_stem_access_${_crexx_runner}_${_crexx_mode} PROPERTIES
                PASS_REGULAR_EXPRESSION "PASS: NR-15 stem access compare"
                FAIL_REGULAR_EXPRESSION "FAIL:|PANIC:"
        )
    endforeach()
endforeach()

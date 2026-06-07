set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

add_custom_target(reference_iterator_system ALL)
foreach(_crexx_runner IN ITEMS rxvm rxbvm)
    crexx_add_rxas_opt_matrix(
            NAME reference_iterator_system_${_crexx_runner}
            SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/reference_iterator_system
            TARGET_GROUP reference_iterator_system
            RUNNER ${_crexx_runner}
            DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/reference_iterator_system.rxas rxas ${_crexx_runner} rxlink
            TEST_LABELS "reference;system;rxas"
    )
    foreach(_crexx_mode IN ITEMS noopt opt)
        set_tests_properties(reference_iterator_system_${_crexx_runner}_${_crexx_mode} PROPERTIES
                PASS_REGULAR_EXPRESSION "PASS: Reference Iterator Fixtures"
                FAIL_REGULAR_EXPRESSION "FAIL:|PANIC:"
        )
    endforeach()
endforeach()

add_custom_target(reference_iterator_perf ALL)
foreach(_crexx_runner IN ITEMS rxvm rxbvm)
    crexx_add_rxas_opt_matrix(
            NAME reference_iterator_perf_${_crexx_runner}
            SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/reference_iterator_perf
            TARGET_GROUP reference_iterator_perf
            RUNNER ${_crexx_runner}
            DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/reference_iterator_perf.rxas rxas ${_crexx_runner} rxlink
            TEST_LABELS "reference;performance;rxas"
    )
    foreach(_crexx_mode IN ITEMS noopt opt)
        set_tests_properties(reference_iterator_perf_${_crexx_runner}_${_crexx_mode} PROPERTIES
                PASS_REGULAR_EXPRESSION "PASS: Reference Iterator Performance Smoke"
                FAIL_REGULAR_EXPRESSION "FAIL:|PANIC:"
        )
    endforeach()
endforeach()

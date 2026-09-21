cmake_minimum_required(VERSION 3.24)
project(rxvector_plugin C)

set(CMAKE_C_STANDARD 99)

include(${CMAKE_SOURCE_DIR}/cmake/CrexxTestModes.cmake)
include(${CMAKE_SOURCE_DIR}/rxpa/RXPluginFunction.cmake)

add_dynamic_plugin_target(vector rxvector.c rxvector_index.c)
target_link_libraries(vector PRIVATE ${CREXX_MATH_LIBRARY})

add_static_plugin_target(vector rxvector.c rxvector_index.c)
target_link_libraries(vector_static PRIVATE ${CREXX_MATH_LIBRARY})

add_rxpa_provider_package(vector)

# The provider remains part of the default product build.  Keep it independent
# of targeted core-C compiler/VM builds so the bootstrap stage does not acquire
# a later Level B native-provider prerequisite.

if(BUILD_TESTING)
    add_executable(rxvector_index_core_test rxvector_index_test.c rxvector_index.c rxvector.c)
    target_compile_definitions(rxvector_index_core_test PRIVATE RXVECTOR_KERNEL_ONLY)
    target_link_libraries(rxvector_index_core_test PRIVATE crexx_rxpa_sdk ${CREXX_MATH_LIBRARY})
    add_test(NAME rxvector_index_core COMMAND rxvector_index_core_test)
    crexx_register_test_prep_targets(TESTS rxvector_index_core
            TARGETS rxvector_index_core_test)
    set_tests_properties(rxvector_index_core PROPERTIES LABELS "library;vector;exact" TIMEOUT 60)
    add_custom_target(rxvector_test ALL)
    set(_rxvector_runners rxbvm)
    if(TARGET rxtvm)
        list(APPEND _rxvector_runners rxtvm)
    endif()

    foreach(_rxvector_runner IN LISTS _rxvector_runners)
        crexx_add_rexx_opt_matrix(
                NAME rxvector_${_rxvector_runner}
                SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/rxvector_test.crexx
                TARGET_GROUP rxvector_test
                RUNNER ${_rxvector_runner}
                IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin
                DEPENDS library rxfnsg float float_static vector vector_static
                RUNTIME_ARGS ${CMAKE_BINARY_DIR}/bin/library
                        ${CMAKE_BINARY_DIR}/bin/rxfnsg
                TEST_LABELS "library;plugins;rxpa;vector;packed;exact"
                REQUIRED_OUTPUT_REGEX "PASS: rxvector exact packed contract"
                FORBIDDEN_OUTPUT_REGEX "FAIL:|PANIC:")
        crexx_add_rexx_opt_matrix(
                NAME rxvector_index_${_rxvector_runner}
                SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/rxvector_index_test.crexx
                TARGET_GROUP rxvector_test
                RUNNER ${_rxvector_runner}
                IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin
                DEPENDS library rxfnsg vector vector_static
                RUNTIME_ARGS ${CMAKE_BINARY_DIR}/bin/library ${CMAKE_BINARY_DIR}/bin/rxfnsg
                TEST_LABELS "library;plugins;rxpa;vector;owner"
                REQUIRED_OUTPUT_REGEX "RXVECTOR_INDEX_OK"
                FORBIDDEN_OUTPUT_REGEX "FAIL:|PANIC:")
    endforeach()
endif()

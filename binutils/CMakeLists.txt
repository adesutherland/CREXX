cmake_minimum_required(VERSION 3.24)

project(RXBIN C)

set(CMAKE_C_STANDARD 99)

include(${CMAKE_SOURCE_DIR}/cmake/CrexxConcurrencyTests.cmake)

add_library(rxbin STATIC
        rxbin.c
        rxbin007.c
        rxsignature.c
        rxsha256.c
        rxgraph.c
        include/rxsignature.h
        include/rxsha256.h
        include/rxgraph.h
        include/rxbin.h)
target_include_directories(rxbin
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/platform)
set_target_properties(rxbin PROPERTIES POSITION_INDEPENDENT_CODE ON)

if(BUILD_TESTING)
    add_executable(test_record_allocation tests/test_record_allocation.c)
    target_link_libraries(test_record_allocation PRIVATE rxbin)
    add_test(NAME record_allocation_failure COMMAND test_record_allocation)
    set_tests_properties(record_allocation_failure PROPERTIES
            LABELS "unit;binutils;allocation" TIMEOUT 30
            CREXX_PREP_TARGETS test_record_allocation)

    add_executable(test_compression_workspace tests/test_compression_workspace.c)
    target_link_libraries(test_compression_workspace PRIVATE rxbin)
    add_test(NAME compression_workspace COMMAND test_compression_workspace)
    set_tests_properties(compression_workspace PROPERTIES
            LABELS "unit;binutils;allocation" TIMEOUT 30
            CREXX_PREP_TARGETS test_compression_workspace)

    add_executable(test_rxsha256 tests/test_rxsha256.c)
    target_link_libraries(test_rxsha256 PRIVATE rxbin)
    add_test(NAME rxsha256_unit COMMAND test_rxsha256)
    set_tests_properties(rxsha256_unit PROPERTIES
            LABELS "unit;library;hash;sha256;file")
    set_property(TEST rxsha256_unit APPEND PROPERTY
            CREXX_PREP_TARGETS test_rxsha256)

    add_executable(test_rxgraph tests/test_rxgraph.c)
    target_link_libraries(test_rxgraph PRIVATE rxbin)
    add_test(NAME rxgraph_unit COMMAND test_rxgraph)
    crexx_label_concurrency_tests(SP03 rxgraph_unit)

    add_executable(rxgraph_bench EXCLUDE_FROM_ALL tools/rxgraph_bench.c)
    target_link_libraries(rxgraph_bench PRIVATE rxbin)
endif()

crexx_finalize_directory_qa_tiers()

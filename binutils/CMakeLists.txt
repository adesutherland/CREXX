cmake_minimum_required(VERSION 3.24)

project(RXBIN C)

set(CMAKE_C_STANDARD 99)

add_library(rxbin STATIC
        rxbin.c
        rxbin007.c
        rxsignature.c
        rxgraph.c
        include/rxsignature.h
        include/rxgraph.h
        include/rxbin.h)
target_include_directories(rxbin
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/platform)
set_target_properties(rxbin PROPERTIES POSITION_INDEPENDENT_CODE ON)

if(BUILD_TESTING)
    add_executable(test_rxgraph tests/test_rxgraph.c)
    target_link_libraries(test_rxgraph PRIVATE rxbin)
    add_test(NAME rxgraph_unit COMMAND test_rxgraph)

    add_executable(rxgraph_bench EXCLUDE_FROM_ALL tools/rxgraph_bench.c)
    target_link_libraries(rxgraph_bench PRIVATE rxbin)
endif()

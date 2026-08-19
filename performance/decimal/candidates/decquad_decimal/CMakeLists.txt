cmake_minimum_required(VERSION 3.24)

set(_d3_decnumber_root
    ${CMAKE_SOURCE_DIR}/interpreter/rxvmplugin/rxvmplugins/mc_decimal/decnumber)

include(${CMAKE_SOURCE_DIR}/interpreter/rxvmplugin/RxvmPluginFunction.cmake)

add_library(decquad_candidate STATIC
        ${_d3_decnumber_root}/decQuad.c
        ${_d3_decnumber_root}/decContext.c)
target_include_directories(decquad_candidate PUBLIC ${_d3_decnumber_root})
set_target_properties(decquad_candidate PROPERTIES POSITION_INDEPENDENT_CODE ON)

add_dynamic_rxvmplugin_target(decquad_decimal_dynamic decquad_decimal.c)
target_include_directories(decquad_decimal_dynamic PRIVATE ${_d3_decnumber_root})
target_link_libraries(decquad_decimal_dynamic decquad_candidate)
set_target_properties(decquad_decimal_dynamic PROPERTIES
        OUTPUT_NAME decquad_decimal)

add_executable(decquad_decimal_lifecycle_test
        decquad_decimal_lifecycle_test.c)
target_include_directories(decquad_decimal_lifecycle_test PRIVATE
        ${CMAKE_SOURCE_DIR}/interpreter
        ${CMAKE_SOURCE_DIR}/interpreter/rxvmplugin
        ${CMAKE_SOURCE_DIR}/platform
        ${CMAKE_SOURCE_DIR}/binutils/include
        ${CMAKE_SOURCE_DIR}/utf8)
target_link_libraries(decquad_decimal_lifecycle_test PRIVATE
        rxvmplugin ${CREXX_MATH_LIBRARY})
add_dependencies(decquad_decimal_lifecycle_test decquad_decimal_dynamic)
add_test(NAME decquad_decimal_lifecycle_test
         COMMAND decquad_decimal_lifecycle_test)
set_tests_properties(decquad_decimal_lifecycle_test PROPERTIES
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LABELS "performance;decimal;decimal01;candidate;d3;qualification"
    PASS_REGULAR_EXPRESSION "PASS: decQuad fixed-34 lifecycle"
    FAIL_REGULAR_EXPRESSION "FAIL:|PANIC:")

add_executable(decquad_decimal_core_comparison
        decquad_decimal_core_comparison.c)
target_include_directories(decquad_decimal_core_comparison PRIVATE
        ${_d3_decnumber_root})
target_link_libraries(decquad_decimal_core_comparison PRIVATE
        decquad_candidate decnumber ${CREXX_MATH_LIBRARY})
set_target_properties(decquad_decimal_core_comparison PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

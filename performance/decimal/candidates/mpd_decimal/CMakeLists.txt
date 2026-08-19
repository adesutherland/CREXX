cmake_minimum_required(VERSION 3.24)

set(LIBMPDEC_ROOT "" CACHE PATH
    "Root of an external, hash-pinned libmpdec build for DECIMAL-01")

find_path(LIBMPDEC_INCLUDE_DIR
          NAMES mpdecimal.h
          HINTS
              ${LIBMPDEC_ROOT}/libmpdec
              ${LIBMPDEC_ROOT}/include)
find_library(LIBMPDEC_LIBRARY
             NAMES mpdec libmpdec
             HINTS
                 ${LIBMPDEC_ROOT}/libmpdec
                 ${LIBMPDEC_ROOT}/lib)

if(NOT LIBMPDEC_INCLUDE_DIR OR NOT LIBMPDEC_LIBRARY)
    message(FATAL_ERROR
        "CREXX_BUILD_MPD_DECIMAL_CANDIDATE requires LIBMPDEC_ROOT to name "
        "an external libmpdec source/build containing mpdecimal.h and libmpdec")
endif()

include(${CMAKE_SOURCE_DIR}/interpreter/rxvmplugin/RxvmPluginFunction.cmake)

add_dynamic_rxvmplugin_target(mpd_decimal_dynamic mpd_decimal.c)
target_include_directories(mpd_decimal_dynamic PRIVATE ${LIBMPDEC_INCLUDE_DIR})
target_link_libraries(mpd_decimal_dynamic ${LIBMPDEC_LIBRARY})
set_target_properties(mpd_decimal_dynamic PROPERTIES OUTPUT_NAME mpd_decimal)

add_executable(mpd_decimal_lifecycle_test mpd_decimal_lifecycle_test.c)
target_include_directories(mpd_decimal_lifecycle_test PRIVATE
        ${CMAKE_SOURCE_DIR}/interpreter
        ${CMAKE_SOURCE_DIR}/interpreter/rxvmplugin
        ${CMAKE_SOURCE_DIR}/platform
        ${CMAKE_SOURCE_DIR}/binutils/include
        ${CMAKE_SOURCE_DIR}/utf8)
target_link_libraries(mpd_decimal_lifecycle_test PRIVATE
        rxvmplugin ${CREXX_MATH_LIBRARY})
add_dependencies(mpd_decimal_lifecycle_test mpd_decimal_dynamic)
add_test(NAME mpd_decimal_lifecycle_test COMMAND mpd_decimal_lifecycle_test)
set_tests_properties(mpd_decimal_lifecycle_test PROPERTIES
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LABELS "performance;decimal;decimal01;candidate;qualification"
    PASS_REGULAR_EXPRESSION "PASS: libmpdec pointer-free lifecycle"
    FAIL_REGULAR_EXPRESSION "FAIL:|PANIC:")

add_executable(mpd_decimal_core_comparison mpd_decimal_core_comparison.c)
target_include_directories(mpd_decimal_core_comparison PRIVATE
        ${LIBMPDEC_INCLUDE_DIR}
        ${CMAKE_SOURCE_DIR}/interpreter/rxvmplugin/rxvmplugins/mc_decimal/decnumber)
target_link_libraries(mpd_decimal_core_comparison PRIVATE
        ${LIBMPDEC_LIBRARY} decnumber ${CREXX_MATH_LIBRARY})
set_target_properties(mpd_decimal_core_comparison PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

if(TARGET mc_decimal_full_tests)
    add_test(NAME mpd_decimal_contract_tests
             COMMAND $<TARGET_FILE:mc_decimal_full_tests> rxvm_mpd_decimal)
    set_tests_properties(mpd_decimal_contract_tests PROPERTIES
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        LABELS "performance;decimal;decimal01;candidate;qualification"
        FAIL_REGULAR_EXPRESSION "Error -|Unable to load|No default|PANIC:")
endif()

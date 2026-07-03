cmake_minimum_required(VERSION 3.24)
project(preprocessor_tests C)

# Register rxpp smoke tests here.
# This test runs the native rxpp precompiler on a tiny .rxpp input and validates
# the generated Rexx snippet contains expected lines.

enable_testing()

add_test(NAME rxpp_smoke
         COMMAND ${CMAKE_COMMAND}
             -DRXPP_BIN=$<TARGET_FILE:rxpp>
             -DRXC_BIN=$<TARGET_FILE:rxc>
             -DINPUT_FILE=${CMAKE_CURRENT_SOURCE_DIR}/rxpp_smoke.rxpp
             -DOUTPUT_FILE=${CMAKE_CURRENT_BINARY_DIR}/rxpp_smoke.out.crexx
             -DMACLIB_FILE=${CMAKE_SOURCE_DIR}/preprocessor/maclib.rexx
             -P ${CMAKE_CURRENT_SOURCE_DIR}/run_rxpp_smoke.cmake
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

add_test(NAME rxpp_srcmap
         COMMAND ${CMAKE_COMMAND}
             -DRXPP_BIN=$<TARGET_FILE:rxpp>
             -DRXC_BIN=$<TARGET_FILE:rxc>
             -DINPUT_FILE=${CMAKE_CURRENT_SOURCE_DIR}/rxpp_srcmap.rxpp
             -DOUTPUT_FILE=${CMAKE_CURRENT_BINARY_DIR}/rxpp_srcmap.out.crexx
             -DMACLIB_FILE=${CMAKE_SOURCE_DIR}/preprocessor/maclib.rexx
             -P ${CMAKE_CURRENT_SOURCE_DIR}/run_rxpp_srcmap.cmake
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

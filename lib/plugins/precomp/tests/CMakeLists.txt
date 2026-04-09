cmake_minimum_required(VERSION 3.24)
project(precomp_tests C)

# Register rxpp smoke test here (precomp plugin area)
# This test runs the native rxpp precompiler on a tiny .rxpp input and validates
# the generated Rexx snippet contains expected lines.

enable_testing()

add_test(NAME rxpp_smoke_precomp
         COMMAND ${CMAKE_COMMAND}
             -DRXPP_BIN=$<TARGET_FILE:rxpp>
             -DINPUT_FILE=${CMAKE_CURRENT_SOURCE_DIR}/rxpp_smoke.rxpp
             -DOUTPUT_FILE=${CMAKE_CURRENT_BINARY_DIR}/rxpp_smoke.out.rexx
             -DMACLIB_FILE=${CMAKE_SOURCE_DIR}/lib/plugins/precomp/maclib.rexx
             -P ${CMAKE_CURRENT_SOURCE_DIR}/run_rxpp_smoke.cmake
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

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

add_test(NAME rxpp_diagnostics
         COMMAND ${CMAKE_COMMAND}
             -DRXPP_BIN=$<TARGET_FILE:rxpp>
             -DWORK=${CMAKE_CURRENT_BINARY_DIR}/rxpp_diagnostics
             -DMACLIB_FILE=${CMAKE_SOURCE_DIR}/preprocessor/maclib.rexx
             -DMESSAGE_PATH=${CMAKE_SOURCE_DIR}/messages
             -P ${CMAKE_CURRENT_SOURCE_DIR}/run_rxpp_diagnostics.cmake
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(rxpp_diagnostics PROPERTIES LABELS "preprocessor;diagnostics")

add_test(NAME rxpp_diagnostic_catalogs
         COMMAND ${CMAKE_COMMAND}
             -DRXC_BIN=$<TARGET_FILE:rxc>
             -DRXAS_BIN=$<TARGET_FILE:rxas>
             -DRXVM_BIN=$<TARGET_FILE:rxvm>
             -DBIN_DIR=${CMAKE_BINARY_DIR}/bin
             -DSOURCE_ROOT=${CMAKE_SOURCE_DIR}
             -DWORK=${CMAKE_CURRENT_BINARY_DIR}/rxpp_diagnostic_catalogs
             -P ${CMAKE_CURRENT_SOURCE_DIR}/run_rxpp_diagnostic_catalogs.cmake
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(rxpp_diagnostic_catalogs PROPERTIES LABELS "preprocessor;diagnostics")

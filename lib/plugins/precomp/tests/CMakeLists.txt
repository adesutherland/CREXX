cmake_minimum_required(VERSION 3.24)
project(precomp_tests C)

# Register rxpp smoke test here (precomp plugin area)
# This test runs the native rxpp precompiler on a tiny .rxpp input and validates
# the generated Rexx snippet contains expected lines.

enable_testing()

add_test(NAME rxpp_smoke_precomp
         COMMAND bash -c "${CMAKE_BINARY_DIR}/bin/rxpp rxprecomp -I ${CMAKE_CURRENT_SOURCE_DIR}/rxpp_smoke.rxpp -o rxpp_smoke.out.rexx -m ${CMAKE_SOURCE_DIR}/lib/plugins/precomp/maclib.rexx && grep -q 'say \"Hello\"' rxpp_smoke.out.rexx && grep -q 'say 3\\*3' rxpp_smoke.out.rexx"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

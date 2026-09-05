if(NOT DEFINED RXPP_BIN OR NOT DEFINED SOURCE_ROOT OR NOT DEFINED WORK)
    message(FATAL_ERROR "RXPP_BIN, SOURCE_ROOT and WORK are required")
endif()

file(REMOVE_RECURSE "${WORK}")
file(MAKE_DIRECTORY "${WORK}" "${WORK}/external")
foreach(support IN ITEMS maclib.rexx macsys.rexx mathlib.rexx syslib.rexx)
    file(COPY_FILE
            "${SOURCE_ROOT}/preprocessor/${support}"
            "${WORK}/${support}")
endforeach()

# The source/maclib directory registers first.  The explicit LOADMACRO root
# must replace this same-named package and supply the expansion.
file(WRITE "${WORK}/external_macro.rxpm" [=[##MACRO EXTERNAL_MACRO value
  .gen say "wrong &value"
##MEND
]=])
file(WRITE "${WORK}/external/external_macro.rxpm" [=[##MACRO EXTERNAL_MACRO value
  .gen say "external &value"
##MEND
]=])
file(WRITE "${WORK}/input.rxpp" [=[options levelb
##LOADMACRO external
##EXTERNAL_MACRO works
]=])

execute_process(
        COMMAND "${RXPP_BIN}" rxprecomp
                -I "${WORK}/input.rxpp"
                -o "${WORK}/output.crexx"
                -m "${WORK}/maclib.rexx"
        RESULT_VARIABLE rxpp_result
        OUTPUT_VARIABLE rxpp_output
        ERROR_VARIABLE rxpp_error)
if(NOT rxpp_result EQUAL 0)
    message(FATAL_ERROR
            "##LOADMACRO failed with ${rxpp_result}:\n${rxpp_output}${rxpp_error}")
endif()

file(READ "${WORK}/output.crexx" generated)
if(NOT generated MATCHES "say \"external works\"")
    message(FATAL_ERROR
            "##LOADMACRO did not select the requested macro directory:\n${generated}")
endif()
if(generated MATCHES "wrong works")
    message(FATAL_ERROR
            "##LOADMACRO retained the earlier same-named macro package:\n${generated}")
endif()

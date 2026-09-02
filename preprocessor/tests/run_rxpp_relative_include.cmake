if(NOT DEFINED RXPP_BIN OR NOT DEFINED SOURCE_ROOT OR NOT DEFINED WORK)
    message(FATAL_ERROR "RXPP_BIN, SOURCE_ROOT and WORK are required")
endif()

file(REMOVE_RECURSE "${WORK}")
file(MAKE_DIRECTORY "${WORK}")
foreach(support IN ITEMS maclib.rexx macsys.rexx mathlib.rexx syslib.rexx)
    file(COPY_FILE
            "${SOURCE_ROOT}/preprocessor/${support}"
            "${WORK}/${support}")
endforeach()

file(WRITE "${WORK}/bundle.rxpm" [=[##MACRO RELATIVE value
  .gen say "relative &value"
##MEND
]=])
file(WRITE "${WORK}/input.rxpp" [=[options levelb
##INCLUDE bundle.rxpm
##RELATIVE works
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
            "relative ##INCLUDE failed with ${rxpp_result}:\n${rxpp_output}${rxpp_error}")
endif()

file(READ "${WORK}/output.crexx" generated)
if(NOT generated MATCHES "say \"relative works\"")
    message(FATAL_ERROR
            "relative ##INCLUDE did not expand its macro:\n${generated}")
endif()

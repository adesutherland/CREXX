file(REMOVE_RECURSE "${WORK}")
file(MAKE_DIRECTORY "${WORK}")

execute_process(
        COMMAND "${RXC}" -i "${IMPORT_DIR}" -o inline_loop_bound_early_return_cleanup "${SOURCE}"
        WORKING_DIRECTORY "${WORK}"
        OUTPUT_VARIABLE out
        ERROR_VARIABLE err
        RESULT_VARIABLE rc)
if(NOT rc EQUAL 0)
    message(FATAL_ERROR "rxc failed on inline loop-bound early-return cleanup case:\n${out}${err}")
endif()

file(READ "${WORK}/inline_loop_bound_early_return_cleanup.rxas" image)
if(image MATCHES "[ \t]call[0-9]*[ \t][^\n]*find\\(\\)")
    message(FATAL_ERROR "optimized cleanup case retained the find() call:\n${image}")
endif()

# The block-expression result is live across the copied cleanup.  It must not
# be allocated to the linked loop-bound register that UNLINK restores.
string(REGEX MATCH
       "icopy r([0-9]+),r[0-9]+[\r\n]+[ \t]+unlinkbr r([0-9]+),l[0-9]+bexprend"
       result_cleanup_path "${image}")
if(NOT result_cleanup_path)
    string(REGEX MATCH
           "icopy r([0-9]+),r[0-9]+[\r\n]+[ \t]+unlink r([0-9]+)[\r\n]+[ \t]+br l[0-9]+bexprend"
           result_cleanup_path "${image}")
endif()
if(NOT result_cleanup_path)
    message(FATAL_ERROR
            "could not identify the inlined result-copy/cleanup path:\n${image}")
endif()
if(CMAKE_MATCH_1 STREQUAL CMAKE_MATCH_2)
    message(FATAL_ERROR
            "block result r${CMAKE_MATCH_1} overlaps crossed loop cleanup r${CMAKE_MATCH_2}:\n${image}")
endif()

# The early return is rewritten to a branch out of the inline block expression.
# It must release the linked counted-loop bound on that path.  Optimized output
# normally fuses the pair to unlinkbr; retain the unfused form in the contract
# so the test remains valid when superinstruction combination is disabled.
if(NOT image MATCHES "[\n\r][ \t]+unlinkbr r[0-9]+,l[0-9]+bexprend" AND
   NOT image MATCHES "[\n\r][ \t]+unlink r[0-9]+[\n\r]+[ \t]+br l[0-9]+bexprend")
    message(FATAL_ERROR
            "inlined early return did not clean up its crossed loop bound before leaving:\n${image}")
endif()

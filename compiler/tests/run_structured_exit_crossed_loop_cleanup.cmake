file(REMOVE_RECURSE "${WORK}")
file(MAKE_DIRECTORY "${WORK}")

execute_process(
        COMMAND "${RXC}" -i "${IMPORT_DIR}" -o structured_exit_crossed_loop_cleanup "${SOURCE}"
        WORKING_DIRECTORY "${WORK}"
        OUTPUT_VARIABLE out
        ERROR_VARIABLE err
        RESULT_VARIABLE rc)
if(NOT rc EQUAL 0)
    message(FATAL_ERROR "rxc failed on structured-exit cleanup case:\n${out}${err}")
endif()

file(READ "${WORK}/structured_exit_crossed_loop_cleanup.rxas" image)

# Both transfers target an outer loop.  The linked inner-loop bound must be
# restored before ITERATE reaches the outer increment and before LEAVE reaches
# the outer end.  Accept fused and unfused forms.
if(NOT image MATCHES "[\n\r][ \t]+unlinkbr r[0-9]+,l[0-9]+doinc" AND
   NOT image MATCHES "[\n\r][ \t]+unlink r[0-9]+[\n\r]+[ \t]+br l[0-9]+doinc")
    message(FATAL_ERROR
            "labelled ITERATE did not clean up its crossed inner loop:\n${image}")
endif()

if(NOT image MATCHES "[\n\r][ \t]+unlinkbr r[0-9]+,l[0-9]+doend" AND
   NOT image MATCHES "[\n\r][ \t]+unlink r[0-9]+[\n\r]+[ \t]+br l[0-9]+doend")
    message(FATAL_ERROR
            "labelled LEAVE did not clean up its crossed inner loop:\n${image}")
endif()

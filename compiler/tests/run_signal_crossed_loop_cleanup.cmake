file(REMOVE_RECURSE "${WORK}")
file(MAKE_DIRECTORY "${WORK}")

execute_process(
        COMMAND "${RXC}" -i "${IMPORT_DIR}" -o signal_crossed_loop_cleanup "${SOURCE}"
        WORKING_DIRECTORY "${WORK}"
        OUTPUT_VARIABLE out
        ERROR_VARIABLE err
        RESULT_VARIABLE rc)
if(NOT rc EQUAL 0)
    message(FATAL_ERROR "rxc failed on SIGNAL cleanup case:\n${out}${err}")
endif()

file(READ "${WORK}/signal_crossed_loop_cleanup.rxas" image)

if(NOT image MATCHES "signalhandler[0-9]+:[\n\r]+[ \t]+sigpop[^\n\r]*[\n\r]+[ \t]+unlink r[0-9]+")
    message(FATAL_ERROR
            "SIGNAL handler did not restore a descendant linked register on entry:\n${image}")
endif()
if(NOT image MATCHES "signalhandler[0-9]+:([^\n]*[\n\r]+)*[ \t]+endlife r[0-9]+")
    message(FATAL_ERROR
            "SIGNAL handler did not end descendant reference lifetimes on entry:\n${image}")
endif()

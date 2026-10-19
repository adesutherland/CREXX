foreach(required IN ITEMS CREXX RXC RXAS RXVM RXBVME RXVME BIN_DIR EXAMPLE_DIR SOURCE_DIR WORK_ROOT)
    if(NOT DEFINED ${required})
        message(FATAL_ERROR "Missing ${required}")
    endif()
endforeach()
file(MAKE_DIRECTORY "${WORK_ROOT}")

function(run_checked expected)
    execute_process(COMMAND ${ARGN}
        WORKING_DIRECTORY "${work}"
        RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error
        TIMEOUT 60)
    string(REPLACE "\r\n" "\n" output "${output}")
    if(NOT result STREQUAL "0" OR NOT output STREQUAL "${expected}\n")
        message(FATAL_ERROR "Process runtime regression (${ARGN}): ${result}\n${output}\n${error}")
    endif()
endfunction()

# These examples already exercise full compile/assemble/link and plain VMs in
# classlib QA. Also cover the product driver and both embedded VM variants:
# their complete runtime contains META_PROVIDER requirements even when the
# task itself only performs integer arithmetic.
foreach(example IN ITEMS concurrency_providers concurrency_taskwork concurrency_process_outcomes)
    if(example STREQUAL "concurrency_providers")
        set(expected "PASS: explicit concurrency provider example")
    elseif(example STREQUAL "concurrency_taskwork")
        set(expected "PASS: advanced taskwork example")
    else()
        set(expected "PASS: process providers and failure cleanup")
    endif()
    set(source "${EXAMPLE_DIR}/${example}.crexx")
    if(example STREQUAL "concurrency_process_outcomes")
        set(source "${SOURCE_DIR}/${example}.crexx")
    endif()
    foreach(mode IN ITEMS opt noopt)
        set(work "${WORK_ROOT}/${example}-${mode}")
        file(MAKE_DIRECTORY "${work}")
        file(COPY "${source}" DESTINATION "${work}")
        set(flags)
        if(mode STREQUAL "noopt")
            list(APPEND flags -n)
        endif()
        execute_process(COMMAND "${RXC}" ${flags} -i "${BIN_DIR}" "${example}.crexx"
            WORKING_DIRECTORY "${work}" RESULT_VARIABLE result
            OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 60)
        if(NOT result STREQUAL "0")
            message(FATAL_ERROR "Compile ${example}/${mode}: ${result}\n${output}\n${error}")
        endif()
        execute_process(COMMAND "${RXAS}" "${example}"
            WORKING_DIRECTORY "${work}" RESULT_VARIABLE result
            OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 60)
        if(NOT result STREQUAL "0")
            message(FATAL_ERROR "Assemble ${example}/${mode}: ${result}\n${output}\n${error}")
        endif()
        run_checked("${expected}" "${RXVM}" "${example}.rxbin"
            "${BIN_DIR}/library.rxbin" "${BIN_DIR}/classlib.rxbin"
            "${BIN_DIR}/classlib_native.rxbin")
        run_checked("${expected}" "${RXVME}" "${example}.rxbin")
        run_checked("${expected}" "${RXBVME}" "${example}.rxbin")
    endforeach()
    # Force a fresh compile through the actual driver rather than reusing the
    # explicit-compiler artifacts above.
    set(work "${WORK_ROOT}/${example}-driver")
    file(MAKE_DIRECTORY "${work}")
    file(COPY "${source}" DESTINATION "${work}")
    file(REMOVE "${work}/${example}.rxas" "${work}/${example}.rxbin")
    run_checked("${expected}" "${CREXX}" "${example}.crexx")
endforeach()

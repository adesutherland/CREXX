# Exact private dependencies must not open their namespace to unrelated source.
function(run_checked label)
    execute_process(COMMAND ${ARGN} WORKING_DIRECTORY "${work}"
        RESULT_VARIABLE rc OUTPUT_VARIABLE out ERROR_VARIABLE err)
    if(NOT rc EQUAL 0)
        message(FATAL_ERROR "${label}: ${rc}\n${out}\n${err}")
    endif()
endfunction()

function(run_rejected label diagnostic)
    execute_process(COMMAND ${ARGN} WORKING_DIRECTORY "${work}"
        RESULT_VARIABLE rc OUTPUT_VARIABLE out ERROR_VARIABLE err)
    if(NOT rc EQUAL 2 OR NOT "${out}${err}" MATCHES "${diagnostic}")
        message(FATAL_ERROR "${label}: expected ${diagnostic}, got ${rc}\n${out}\n${err}")
    endif()
endfunction()

file(REMOVE_RECURSE "${WORK_ROOT}")
file(MAKE_DIRECTORY "${WORK_ROOT}")
if(KIND STREQUAL provider)
    set(primary main)
    set(dependencies cleanup contract implementation)
else()
    set(primary model)
    set(dependencies cleanup)
endif()
foreach(mode IN ITEMS opt noopt)
    set(options)
    if(mode STREQUAL noopt)
        list(APPEND options -n)
    endif()
    set(work "${WORK_ROOT}/${mode}")
    file(MAKE_DIRECTORY "${work}")
    file(COPY "${FIXTURE}/${KIND}/" DESTINATION "${work}")
    set(arguments ${options} --no-exe-import -i "${BIN}" -s "${work}" -o "${primary}" "${work}/${primary}.crexx")
    set(snapshot "${work}/dependencies.snapshot")
    run_checked("${mode} source ${KIND}" "${RXC}" --project-dependencies "${snapshot}" ${arguments})
    file(APPEND "${work}/extension.crexx" "\n/* Unused private namespace extension body. */\n")
    run_checked("${mode} private dependency must not read extension" "${RXC}" --check-project-dependencies "${snapshot}" ${arguments})

    file(READ "${work}/cleanup.crexx" cleanup)
    string(REPLACE "call closefile" "call lineout" lineout "${cleanup}")
    file(WRITE "${work}/cleanup.crexx" "${lineout}")
    run_checked("${mode} lineout control" "${RXC}" ${arguments})
    file(WRITE "${work}/cleanup.crexx" "${cleanup}")

    if(KIND STREQUAL provider)
        file(READ "${work}/implementation.crexx" implementation)
        string(REPLACE " implements .providercontract" "" invalid "${implementation}")
        file(WRITE "${work}/implementation.crexx" "${invalid}")
        run_rejected("${mode} nominal interface control" TYPE_MISMATCH "${RXC}" ${arguments})
        file(WRITE "${work}/implementation.crexx" "${implementation}")
    endif()

    # Build declarations in dependency order in an isolated binary directory,
    # then compile the original primary against those RXBIN contracts.
    set(binary "${work}/binary")
    file(MAKE_DIRECTORY "${binary}")
    foreach(member IN LISTS dependencies)
        file(RENAME "${work}/${member}.crexx" "${binary}/${member}.crexx")
        run_checked("${mode} compile binary provider ${member}" "${RXC}" ${options}
            --no-exe-import -i "${binary}\;${BIN}" -o "${binary}/${member}" "${binary}/${member}.crexx")
        run_checked("${mode} assemble binary provider ${member}" "${RXAS}"
            -o "${binary}/${member}" "${binary}/${member}.rxas")
        file(RENAME "${binary}/${member}.crexx" "${binary}/${member}.source")
    endforeach()
    run_checked("${mode} binary ${KIND}" "${RXC}" ${options} --no-exe-import
        -i "${binary}\;${BIN}" -s "${work}" -o "${primary}" "${work}/${primary}.crexx")

    # A genuine source import must still load and validate the extension.
    file(WRITE "${work}/extension.crexx" "options levelb\nnamespace _rxsysb expose issue699extension\nissue699extension: procedure = .int\n  arg value = .int\n  return value + 1\n")
    file(WRITE "${work}/consumer.crexx" "options levelb\nimport _rxsysb\nmain: procedure = .int\n  return issue699extension(7)\n")
    set(consumer ${options} --no-exe-import -i "${BIN}" -s "${work}" -o consumer "${work}/consumer.crexx")
    run_checked("${mode} explicit extension" "${RXC}" --project-dependencies "${snapshot}" ${consumer})
    file(APPEND "${work}/extension.crexx" "\n/* A real dependency changes. */\n")
    execute_process(COMMAND "${RXC}" --check-project-dependencies "${snapshot}" ${consumer}
        WORKING_DIRECTORY "${work}" RESULT_VARIABLE stale OUTPUT_VARIABLE out ERROR_VARIABLE err)
    if(NOT stale EQUAL 2)
        message(FATAL_ERROR "${mode}: explicit extension edit must invalidate (${stale})\n${out}\n${err}")
    endif()
    file(WRITE "${work}/consumer.crexx" "options levelb\nimport _rxsysb\nmain: procedure = .int\n  return issue699extension(7, 8)\n")
    run_rejected("${mode} extension argument validation" UNEXPECTED_ARGUMENT "${RXC}" ${consumer})

    # Generated imports (ADDRESS) and the source's own namespace also retain
    # discovery, without relying on an explicit header IMPORT node.
    file(WRITE "${work}/consumer.crexx" "options levelb\nmain: procedure\n  address system 'echo unused'\n  return\n")
    run_checked("${mode} generated ADDRESS import" "${RXC}" --project-dependencies "${snapshot}" ${consumer})
    file(APPEND "${work}/extension.crexx" "\n/* Generated import dependency. */\n")
    execute_process(COMMAND "${RXC}" --check-project-dependencies "${snapshot}" ${consumer}
        WORKING_DIRECTORY "${work}" RESULT_VARIABLE stale OUTPUT_VARIABLE out ERROR_VARIABLE err)
    if(NOT stale EQUAL 2)
        message(FATAL_ERROR "${mode}: generated import did not retain extension (${stale})\n${out}\n${err}")
    endif()
    file(WRITE "${work}/consumer.crexx" "options levelb\nnamespace _rxsysb\nmain: procedure = .int\n  return issue699extension(7)\n")
    run_checked("${mode} own namespace extension" "${RXC}" ${consumer})
endforeach()

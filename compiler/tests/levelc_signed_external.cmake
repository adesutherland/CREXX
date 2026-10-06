foreach(required RXC RXAS RXLINK RXVM BINDIR SOURCE_DIR BUILD_DIR EXPECTED)
    if(NOT DEFINED ${required})
        message(FATAL_ERROR "Missing ${required}")
    endif()
endforeach()

if(NOT DEFINED NAME)
    set(NAME levelc_address_external)
endif()
if(NOT DEFINED PROVIDER)
    set(PROVIDER levelc_address_ext_provider)
endif()
if(NOT DEFINED CONSUMER)
    set(CONSUMER levelc_address_ext_consumer)
endif()

if(NOOPT)
    set(mode noopt)
    set(flags -n)
else()
    set(mode opt)
    set(flags)
endif()
set(workdir "${BUILD_DIR}/${NAME}/${mode}")
file(MAKE_DIRECTORY "${workdir}")

function(run_checked label)
    execute_process(COMMAND ${ARGN} WORKING_DIRECTORY "${workdir}"
            OUTPUT_VARIABLE output ERROR_VARIABLE error RESULT_VARIABLE result)
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "${label} failed (${result}): ${output}${error}")
    endif()
endfunction()

foreach(stem "${PROVIDER}" "${CONSUMER}")
    if(stem STREQUAL "${PROVIDER}")
        set(provider_flag --levelc-routine)
    else()
        set(provider_flag)
    endif()
    run_checked("compile ${stem}" "${RXC}" ${flags} ${provider_flag}
            -s "${SOURCE_DIR}" -i "${BINDIR}" -o "${stem}"
            "${SOURCE_DIR}/${stem}.rexx")
    run_checked("assemble ${stem}" "${RXAS}" -o "${stem}.rxbin" "${stem}")
endforeach()

run_checked("link ${NAME}" "${RXLINK}" -o "${NAME}_image.rxbin"
        "${CONSUMER}.rxbin" "${PROVIDER}.rxbin"
        "${BINDIR}/library.rxbin" "${BINDIR}/classlib.rxbin"
        "${BINDIR}/rxfnsc.rxbin")
execute_process(COMMAND "${RXVM}" "${NAME}_image.rxbin"
        WORKING_DIRECTORY "${workdir}"
        OUTPUT_VARIABLE output ERROR_VARIABLE error RESULT_VARIABLE result)
if(NOT result EQUAL 0 OR NOT error STREQUAL "")
    message(FATAL_ERROR "run ${NAME} failed (${result}): ${output}${error}")
endif()
file(READ "${EXPECTED}" expected)
string(REPLACE "\r\n" "\n" output "${output}")
if(NOT output STREQUAL expected)
    message(FATAL_ERROR "${NAME} output differs: ${output}")
endif()

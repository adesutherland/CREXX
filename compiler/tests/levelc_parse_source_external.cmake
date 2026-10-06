foreach(required RXC RXAS RXLINK RXVM BINDIR SOURCE_DIR BUILD_DIR)
    if(NOT DEFINED ${required})
        message(FATAL_ERROR "Missing ${required}")
    endif()
endforeach()

if(NOOPT)
    set(mode noopt)
    set(rxc_flags -n)
else()
    set(mode opt)
    set(rxc_flags)
endif()
set(work "${BUILD_DIR}/levelc_parse_source_external_${mode}")
file(MAKE_DIRECTORY "${work}")
set(source_path "${work}/source path with spaces")
file(MAKE_DIRECTORY "${source_path}")
file(COPY "${SOURCE_DIR}/levelc_parse_source_provider.rexx"
     DESTINATION "${source_path}")
file(COPY_FILE "${SOURCE_DIR}/levelc_parse_source_consumer.rexx"
               "${source_path}/consumer source file.rexx")

foreach(name IN ITEMS levelc_parse_source_provider levelc_parse_source_consumer)
    if(name STREQUAL "levelc_parse_source_consumer")
        set(source "${source_path}/consumer source file.rexx")
    else()
        set(source "${source_path}/${name}.rexx")
    endif()
    set(assembly "${work}/${name}.rxas")
    set(binary "${work}/${name}.rxbin")
    if(name STREQUAL "levelc_parse_source_provider")
        set(provider_flag --levelc-routine)
    else()
        set(provider_flag)
    endif()
    execute_process(
            COMMAND "${RXC}" ${rxc_flags} -s "${source_path}" -i "${BINDIR}"
                    ${provider_flag} -o "${assembly}" "${source}"
            WORKING_DIRECTORY "${work}"
            OUTPUT_VARIABLE output ERROR_VARIABLE error RESULT_VARIABLE result)
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "${name} compile failed: ${output}${error}")
    endif()
    execute_process(
            COMMAND "${RXAS}" -o "${binary}" "${assembly}"
            WORKING_DIRECTORY "${work}"
            OUTPUT_VARIABLE output ERROR_VARIABLE error RESULT_VARIABLE result)
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "${name} assemble failed: ${output}${error}")
    endif()
endforeach()

set(image "${work}/parse_source_image.rxbin")
execute_process(
        COMMAND "${RXLINK}" -o "${image}"
                "${work}/levelc_parse_source_provider.rxbin"
                "${work}/levelc_parse_source_consumer.rxbin"
                "${BINDIR}/library.rxbin" "${BINDIR}/classlib.rxbin"
                "${BINDIR}/rxfnsc.rxbin"
        WORKING_DIRECTORY "${work}"
        OUTPUT_VARIABLE output ERROR_VARIABLE error RESULT_VARIABLE result)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "PARSE SOURCE link failed: ${output}${error}")
endif()
execute_process(
        COMMAND "${RXVM}" "${image}"
        WORKING_DIRECTORY "${work}"
        OUTPUT_VARIABLE output ERROR_VARIABLE error RESULT_VARIABLE result)
if(NOT result EQUAL 0 OR NOT error STREQUAL "")
    message(FATAL_ERROR "PARSE SOURCE execution failed: ${output}${error}")
endif()
set(expected "main=COMMAND|1\nprovider=SUBROUTINE|1\nprovider-arg=410042|é🙂\nsubresult=ok\n")
if(NOT output STREQUAL expected)
    message(FATAL_ERROR "Unexpected PARSE SOURCE output: ${output}")
endif()

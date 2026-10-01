# Return conversions must survive nested and imported inline expansion (#689).
function(run_checked label)
    execute_process(COMMAND ${ARGN} WORKING_DIRECTORY "${work}"
        RESULT_VARIABLE result OUTPUT_VARIABLE out ERROR_VARIABLE err TIMEOUT 60)
    if(NOT "${result}" STREQUAL "0")
        message(FATAL_ERROR "${label}: exit ${result}\n${out}\n${err}")
    endif()
    set(last_output "${out}" PARENT_SCOPE)
endfunction()
file(READ "${FIXTURE}" provider)
set(main [=[main: procedure = .int
  arg arguments = .string[]
  value = 99
  value = Wrap("a b c")
  say "nested:" value
  value = Wrap("")
  say "empty:" value
  text = .string
  text = Canonical(arguments[1])
  say "canonical:" text
  sample = .Counter()
  text = sample.change(arguments[1])
  say "method:" text sample.read()
  return 0
]=])
set(expected "nested: 3\nempty: 0\ncanonical: 3\nmethod: 3 1\n")
file(REMOVE_RECURSE "${WORK_ROOT}")
foreach(kind IN ITEMS combined source binary)
    foreach(mode IN ITEMS opt noopt)
        set(work "${WORK_ROOT}/${kind}-${mode}")
        file(MAKE_DIRECTORY "${work}/imports" "${work}/provider" "${work}/runtime")
        set(options)
        if(mode STREQUAL noopt)
            list(APPEND options -n)
        endif()
        set(modules)
        if(kind STREQUAL combined)
            string(REPLACE "\nWrap: procedure" "\n${main}\nWrap: procedure" program "${provider}")
            file(WRITE "${work}/main.crexx" "${program}")
        else()
            file(WRITE "${work}/provider/provider.crexx" "${provider}")
            run_checked("${kind}/${mode} compile provider" "${RXC}" -i "${BIN}"
                -o runtime/provider provider/provider.crexx)
            run_checked("${kind}/${mode} assemble provider" "${RXAS}"
                -o runtime/provider runtime/provider.rxas)
            list(APPEND modules runtime/provider.rxbin)
            if(kind STREQUAL source)
                file(WRITE "${work}/imports/inline_return_conversions.crexx" "${provider}")
            else()
                file(COPY_FILE "${work}/runtime/provider.rxbin" "${work}/imports/inline_return_conversions.rxbin")
            endif()
            file(WRITE "${work}/main.crexx"
                "options levelb\nimport inline_return_conversions\nnamespace consumer\n${main}")
        endif()
        run_checked("${kind}/${mode} compile consumer" "${RXC}" ${options}
            -s imports -i imports -i "${BIN}" -o main main.crexx)
        if(mode STREQUAL opt)
            file(READ "${work}/main.rxas" assembly)
            string(TOLOWER "${assembly}" assembly)
            if(assembly MATCHES "call[0-9]* [^\n]*(canonical|counter\\.change)\\(\\)" OR
               (kind STREQUAL combined AND assembly MATCHES "call[0-9]* [^\n]*wrap\\(\\)"))
                message(FATAL_ERROR "${kind}: supported conversion calls must remain inlined")
            endif()
        endif()
        foreach(assembler_mode IN ITEMS opt noopt)
            set(assembler_options)
            if(assembler_mode STREQUAL noopt)
                list(APPEND assembler_options -n)
            endif()
            run_checked("assemble ${assembler_mode}" "${RXAS}" ${assembler_options} -o main main.rxas)
            run_checked("link ${kind}/${mode}/${assembler_mode}" "${RXLINK}" -s -o program
                main.rxbin ${modules} "${BIN}/library.rxbin" "${BIN}/classlib.rxbin")
            foreach(vm IN ITEMS "${RXBVM}" "${RXVM}")
                run_checked("run ${kind}/${mode}/${assembler_mode}/${vm}" "${vm}"
                    program.rxbin -a 003)
                string(REPLACE "\r\n" "\n" last_output "${last_output}")
                if(NOT last_output STREQUAL expected)
                    message(FATAL_ERROR "Incorrect conversion result: ${last_output}")
                endif()
                execute_process(COMMAND "${vm}" program.rxbin -a invalid
                    WORKING_DIRECTORY "${work}" RESULT_VARIABLE result
                    OUTPUT_VARIABLE out ERROR_VARIABLE err TIMEOUT 60)
                if(NOT "${result}" STREQUAL "6" OR
                    NOT "${out}${err}" MATCHES "CONVERSION_ERROR" OR
                    "${out}${err}" MATCHES "canonical:")
                    message(FATAL_ERROR "Missing declared return conversion failure: ${result}\n${out}\n${err}")
                endif()
            endforeach()
        endforeach()
    endforeach()
endforeach()

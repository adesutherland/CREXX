# Same-namespace imports must preserve complete exposed-global types (#686).
# Exercise source import contracts through compile/assemble/link/run.
function(run_checked label)
    execute_process(COMMAND ${ARGN} WORKING_DIRECTORY "${work}"
        RESULT_VARIABLE result OUTPUT_VARIABLE out ERROR_VARIABLE err
        TIMEOUT 60)
    if(NOT "${result}" STREQUAL "0")
        message(FATAL_ERROR "${label}: exit ${result}\n${out}\n${err}")
    endif()
    set(last_output "${out}" PARENT_SCOPE)
endfunction()

file(READ "${FIXTURE}" source)
file(REMOVE_RECURSE "${WORK_ROOT}")
foreach(mode IN ITEMS opt noopt)
    set(options)
    if(mode STREQUAL noopt)
        list(APPEND options -n)
    endif()

    # Keep the reported inference order: adding a main that calls Make first
    # can establish the local class before the imported global is considered.
    set(work "${WORK_ROOT}/${mode}-inferred")
    file(MAKE_DIRECTORY "${work}")
    set(reduced [=[options levelb
import rxfnsb
namespace shared expose Make Read
Make: procedure = .void expose S
  S = .stem()
  S["x"] = "hello"
  return
Read: procedure = .string expose S
  return S["x"]
]=])
    file(WRITE "${work}/a.crexx" "${reduced}")
    run_checked("${mode} compile inferred global alone" "${RXC}" ${options}
        -i "${BIN}" a.crexx)
    file(WRITE "${work}/b.crexx" "${reduced}")
    run_checked("${mode} compile inferred global with sibling" "${RXC}" ${options}
        -i "${BIN}" a.crexx)

    foreach(kind IN ITEMS alone source)
        set(work "${WORK_ROOT}/${mode}-${kind}")
        file(MAKE_DIRECTORY "${work}/imports")
        file(WRITE "${work}/a.crexx" "${source}")
        if(kind STREQUAL source)
            file(WRITE "${work}/b.crexx" "${source}")
        endif()
        run_checked("${mode}/${kind} compile consumer" "${RXC}" ${options}
            -i imports -i "${BIN}" -o a a.crexx)
        run_checked("${mode}/${kind} assemble consumer" "${RXAS}" ${options} -o a a.rxas)
        file(WRITE "${work}/runner.crexx" "options levelb\nimport global_import_types\ncall global_import_types..Make\nsay global_import_types..Read()\nexit 0\n")
        run_checked("${mode}/${kind} compile runner" "${RXC}" ${options}
            -i imports -i "${BIN}" -o runner runner.crexx)
        run_checked("${mode}/${kind} assemble runner" "${RXAS}" ${options}
            -o runner runner.rxas)
        run_checked("${mode}/${kind} link consumer" "${RXLINK}" -s -o program
            runner.rxbin a.rxbin "${BIN}/library.rxbin" "${BIN}/classlib.rxbin")
        run_checked("${mode}/${kind} execute consumer" "${RXVME}" program.rxbin)
        string(REPLACE "\r\n" "\n" last_output "${last_output}")
        if(NOT last_output STREQUAL "hello:value:42:7\n")
            message(FATAL_ERROR "${mode}/${kind}: incorrect global values: ${last_output}")
        endif()
    endforeach()

    # Keep scalar base types equal: a bounds mismatch must still be rejected.
    set(work "${WORK_ROOT}/${mode}-mismatch")
    file(MAKE_DIRECTORY "${work}")
    file(WRITE "${work}/a.crexx" "options levelb\nnamespace global_bounds expose Set\nSet: procedure = .void expose grid\n  grid = .int[-2 to 2]\n  return\n")
    file(WRITE "${work}/b.crexx" "options levelb\nnamespace global_bounds expose Set\nSet: procedure = .void expose grid\n  grid = .int[-1 to 1]\n  return\n")
    execute_process(COMMAND "${RXC}" ${options} -i "${BIN}" a.crexx
        WORKING_DIRECTORY "${work}" RESULT_VARIABLE result
        OUTPUT_VARIABLE out ERROR_VARIABLE err TIMEOUT 60)
    if(NOT "${result}" STREQUAL "2" OR
        NOT "${out}${err}" MATCHES "TYPE_MISMATCH" OR
        NOT "${out}${err}" MATCHES "grid")
        message(FATAL_ERROR "${mode}: incompatible global bounds accepted or wrong diagnostic: ${result}\n${out}\n${err}")
    endif()

endforeach()

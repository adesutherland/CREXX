cmake_minimum_required(VERSION 3.24)

foreach(required IN ITEMS RXC RXAS RXBVM SOURCE_DIR WORK_DIR)
    if(NOT DEFINED ${required} OR "${${required}}" STREQUAL "")
        message(FATAL_ERROR "Missing -D${required}=...")
    endif()
endforeach()

file(MAKE_DIRECTORY "${WORK_DIR}")
file(READ "${SOURCE_DIR}/implicit_constant_entry.expected" expected)

function(run_checked)
    execute_process(COMMAND ${ARGV} WORKING_DIRECTORY "${WORK_DIR}"
            OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE result
            TIMEOUT 60)
    if(NOT "${result}" STREQUAL "0")
        message(FATAL_ERROR "${ARGV} failed (${result}):\n${out}${err}")
    endif()
    if(NOT err STREQUAL "")
        message(FATAL_ERROR "${ARGV} emitted diagnostics:\n${err}")
    endif()
    set(last_output "${out}" PARENT_SCOPE)
endfunction()

# Two compiler routes isolate the changed mechanism. One nonoptimizing
# assembler and one VM keep downstream rewriting out of the correctness oracle.
foreach(mode IN ITEMS noopt opt)
    set(flags)
    if(mode STREQUAL "noopt")
        set(flags -n)
    endif()
    set(stem "${WORK_DIR}/${mode}")
    run_checked("${RXC}" -x --no-exe-import ${flags} -o "${stem}"
            "${SOURCE_DIR}/implicit_constant_entry.crexx")
    if(mode STREQUAL "opt")
        file(READ "${stem}.rxas" assembly)
        if(NOT assembly MATCHES "\n[ \t]+ret 37([\r\n]|$)")
            message(FATAL_ERROR "Unconditional entry initializer lost constant folding")
        endif()
    endif()
    run_checked("${RXAS}" -n -o "${stem}" "${stem}.rxas")
    run_checked("${RXBVM}" "${stem}.rxbin")
    string(REPLACE "\r\n" "\n" actual "${last_output}")
    if(NOT actual STREQUAL expected)
        message(FATAL_ERROR "${mode} value/state mismatch:\nExpected:\n${expected}Actual:\n${actual}")
    endif()
endforeach()

file(REMOVE_RECURSE "${WORK_DIR}")

# Shared by the two positive runtime paths and their synthetic ordinary check.
function(nr16_17_output_problem result output error mode problem_var)
    set(problem "")
    if(NOT "${result}" STREQUAL "0")
        set(problem "runner returned ${result}")
    elseif(NOT "${error}" STREQUAL "")
        set(problem "unexpected stderr: ${error}")
    elseif("${output}" MATCHES "FAIL:|PANIC:")
        set(problem "forbidden failure output")
    else()
        foreach(required IN ITEMS
                "(^|\n)PASS: NR-16/17 stable-state compare(\r?\n|$)"
                "(^|\n)mode=${mode}(\r?\n|$)"
                "(^|\n)iterations=10(\r?\n|$)"
                "(^|\n)checksum=55(\r?\n|$)")
            if(NOT "${output}" MATCHES "${required}")
                set(problem "missing required output: ${required}")
                break()
            endif()
        endforeach()
    endif()
    set(${problem_var} "${problem}" PARENT_SCOPE)
endfunction()

if(CMAKE_SCRIPT_MODE_FILE STREQUAL CMAKE_CURRENT_LIST_FILE)
    if(NOT NR16_MODE MATCHES "^(trace-off|address-same-static)$")
        message(FATAL_ERROR "Unknown NR-16/17 positive path: ${NR16_MODE}")
    endif()
    # Same stripped link, module order and VM arguments as the original linked
    # runtime registration; keep stdout/stderr distinct for the positive oracle.
    set(work "${NR16_WORKING_DIRECTORY}/linked_opt/${NR16_NAME}")
    file(REMOVE_RECURSE "${work}")
    file(MAKE_DIRECTORY "${work}")
    set(image "${work}/${NR16_NAME}")
    execute_process(COMMAND "${NR16_RXLINK}" -s -o "${image}"
        "${NR16_PROGRAM}" "${NR16_LIBRARY}" "${NR16_ORACLE}"
        WORKING_DIRECTORY "${work}"
        RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error)
    if(NOT result STREQUAL "0" OR NOT error STREQUAL "")
        file(REMOVE_RECURSE "${work}")
        message(FATAL_ERROR "NR-16/17 link failed (${result}): ${output}${error}")
    endif()
    execute_process(COMMAND "${NR16_RUNNER}" "${image}.rxbin" -a "${NR16_MODE}" 10
        WORKING_DIRECTORY "${NR16_WORKING_DIRECTORY}"
        RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error)
    nr16_17_output_problem("${result}" "${output}" "${error}" "${NR16_MODE}" problem)
    file(REMOVE_RECURSE "${work}")
    if(NOT problem STREQUAL "")
        message(FATAL_ERROR "NR-16/17 ${NR16_NAME}: ${problem}\n${output}${error}")
    endif()
    message("${output}")
endif()

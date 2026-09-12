foreach(_required IN ITEMS RXAS WORK_ROOT)
    if(NOT DEFINED ${_required} OR "${${_required}}" STREQUAL "")
        message(FATAL_ERROR "Missing required variable ${_required}")
    endif()
endforeach()

function(run_invalid_expose name source)
    set(_source "${WORK_ROOT}/${name}.rxas")
    set(_output "${WORK_ROOT}/${name}.rxbin")
    file(WRITE "${_source}" "${source}")
    execute_process(
            COMMAND "${RXAS}" -o "${_output}" "${_source}"
            WORKING_DIRECTORY "${WORK_ROOT}"
            RESULT_VARIABLE _result
            OUTPUT_VARIABLE _stdout
            ERROR_VARIABLE _stderr)
    set(_diagnostic "${_stdout}${_stderr}")
    if("${_result}" STREQUAL "0")
        message(FATAL_ERROR "${name}: invalid exposed global was accepted")
    endif()
    if(NOT "${_diagnostic}" MATCHES
       "global register number bigger than the number of globals")
        message(FATAL_ERROR
                "${name}: expected range diagnostic was not emitted (${_result})\n${_diagnostic}")
    endif()
    if(EXISTS "${_output}")
        message(FATAL_ERROR "${name}: invalid input produced ${_output}")
    endif()
endfunction()

file(REMOVE_RECURSE "${WORK_ROOT}")
file(MAKE_DIRECTORY "${WORK_ROOT}")

run_invalid_expose(
        missing_globals
        "g0 .expose=missing.value\nmain() .locals=0 .expose=missing.main\n  ret\n")
run_invalid_expose(
        zero_globals
        ".globals=0\ng0 .expose=zero.value\nmain() .locals=0 .expose=zero.main\n  ret\n")
run_invalid_expose(
        upper_bound
        ".globals=1\ng1 .expose=upper.value\nmain() .locals=0 .expose=upper.main\n  ret\n")
run_invalid_expose(
        large_register
        ".globals=1\ng2147483647 .expose=large.value\nmain() .locals=0 .expose=large.main\n  ret\n")

set(_valid_source "${WORK_ROOT}/valid_boundary.rxas")
set(_valid_output "${WORK_ROOT}/valid_boundary.rxbin")
file(WRITE "${_valid_source}"
     ".globals=1\ng0 .expose=valid.value\nmain() .locals=0 .expose=valid.main\n  ret\n")
execute_process(
        COMMAND "${RXAS}" -o "${_valid_output}" "${_valid_source}"
        WORKING_DIRECTORY "${WORK_ROOT}"
        RESULT_VARIABLE _valid_result
        OUTPUT_VARIABLE _valid_stdout
        ERROR_VARIABLE _valid_stderr)
if(NOT _valid_result EQUAL 0 OR NOT EXISTS "${_valid_output}")
    message(FATAL_ERROR
            "valid_boundary: valid exposed global failed (${_valid_result})\n${_valid_stdout}${_valid_stderr}")
endif()

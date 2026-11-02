cmake_minimum_required(VERSION 3.24)
foreach(required RXAS RUNNER WORK)
    if(NOT DEFINED ${required})
        message(FATAL_ERROR "Missing ${required}")
    endif()
endforeach()
file(MAKE_DIRECTORY "${WORK}")

function(check_case name source expected_error expected_result)
    file(WRITE "${WORK}/${name}.rxas" "${source}")
    execute_process(COMMAND "${RXAS}" -n -o "${name}" "${name}.rxas"
        WORKING_DIRECTORY "${WORK}" RESULT_VARIABLE assembled
        OUTPUT_VARIABLE assembly_out ERROR_VARIABLE assembly_err)
    if(NOT assembled EQUAL 0)
        message(FATAL_ERROR "${name}: assembly failed: ${assembly_out}${assembly_err}")
    endif()
    execute_process(COMMAND "${RUNNER}" "${name}"
        WORKING_DIRECTORY "${WORK}" RESULT_VARIABLE result
        OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15)
    if(NOT "${result}" MATCHES "^[0-9]+$")
        message(FATAL_ERROR "${name}: VM did not complete: ${result}: ${output}${error}")
    endif()
    if(expected_result STREQUAL "terminal")
        if(result EQUAL 0 OR NOT error MATCHES "${expected_error}")
            message(FATAL_ERROR "${name}: expected terminal metadata error, got ${result}: ${output}${error}")
        endif()
    elseif(NOT result EQUAL 0)
        message(FATAL_ERROR "${name}: expected silent terminal success, got ${result}: ${output}${error}")
    endif()
    if(output MATCHES "UNREACHABLE")
        message(FATAL_ERROR "${name}: resumed after terminal departure")
    endif()
endfunction()

check_case(sibling [[
.globals=0
main() .locals=0
left:
    .inlineregion 1,0
    .inlinerange left,left_end,1
    sigbr right,"OTHER"
    signal "OTHER","inactive sibling"
left_end:
    ret 1
right:
    .inlineregion 2,0
    .inlinerange right,right_end,2
    say "UNREACHABLE"
right_end:
    ret 0
]] "Invalid inline signal departure" terminal)

check_case(saved_numeric [[
.globals=0
main() .locals=1
    sigbr caught,"OTHER"
begin:
    .inlineregion 1,0
    .inlinerange begin,end,1
    .inlineexit 1,ready,end,0,"digits",r0
    load r0,0
ready:
    signal "OTHER","invalid saved digits"
end:
    ret 1
caught:
    say "UNREACHABLE"
    ret 0
]] "Invalid inline signal departure" terminal)

check_case(absent_argument [[
.globals=0
main() .locals=0
    call no_arguments()
    ret 1
no_arguments() .locals=0
    sigbr caught,"OTHER"
begin:
    .inlineregion 1,0
    .inlinerange begin,end,1
    .inlineexit 1,begin,end,0,"unlink",a1
    signal "OTHER","missing argument slot"
end:
    ret 1
caught:
    say "UNREACHABLE"
    ret 0
]] "Invalid inline signal departure" terminal)

check_case(terminal [[
.globals=0
main() .locals=1
begin:
    .inlineregion 1,0
    .inlinerange begin,end,1
    .inlineexit 1,begin,end,0,"endlife",r0
    sigshalt "OTHER"
    signal "OTHER","terminal region"
    say "UNREACHABLE"
end:
    ret 1
]] "" success)
file(REMOVE_RECURSE "${WORK}")
message(STATUS "PASS: inline cold-exit errors and terminal disposition")

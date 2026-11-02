if(NOT DEFINED RXAS OR NOT DEFINED WORKING_DIRECTORY)
    message(FATAL_ERROR "RXAS and WORKING_DIRECTORY are required")
endif()
file(MAKE_DIRECTORY "${WORKING_DIRECTORY}/inline_typed_operands")

function(reject_case name source expected)
    set(path "${WORKING_DIRECTORY}/inline_typed_operands/${name}.rxas")
    file(WRITE "${path}" "${source}")
    execute_process(COMMAND "${RXAS}" -o "${name}" "${path}"
            WORKING_DIRECTORY "${WORKING_DIRECTORY}/inline_typed_operands"
            RESULT_VARIABLE result OUTPUT_VARIABLE stdout ERROR_VARIABLE stderr)
    set(output "${stdout}${stderr}")
    if(result EQUAL 0 OR NOT "${output}" MATCHES "${expected}")
        message(FATAL_ERROR "${name}: expected ${expected}; exit=${result}\n${output}")
    endif()
endfunction()

reject_case(retired_inlenter [=[
.globals=0
main() .locals=4
    inlenter r0,helper(),"count=1;args=list(r1);a0=r2;proxies=();owned=(r2);prep=none",done
    ret
helper() .locals=0
    ret
done:
]=] "[Uu]nknown instruction|[Ii]nvalid instruction")

reject_case(retired_linkargv [=[
.globals=0
main() .locals=2
    linkargv r0,1
    ret
]=] "[Uu]nknown instruction|[Ii]nvalid instruction")

reject_case(retired_qglobbind [=[
.globals=1
main() .locals=3
    qglobbind r1,"@",0
    ret
]=] "[Uu]nknown instruction|[Ii]nvalid instruction")

reject_case(overflow_procedure [=[
.globals=0
main() .locals=1
    call @4294967296::0()
    ret
]=] "invalid qualified procedure ordinal")

reject_case(retired_sigret [=[
.globals=0
main() .locals=0
    sigret "OTHER"
    ret
]=] "[Uu]nknown instruction|[Ii]nvalid instruction")

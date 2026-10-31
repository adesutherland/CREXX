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

reject_case(malformed_recipe [=[
.globals=0
main() .locals=4
    inlenter r0,helper(),"count=1;args=list(r1);a0=r2;proxies=();owned=(r2);prep=unknown",done
done:
    ret
helper() .locals=0
    ret
]=] "invalid inline recipe")

reject_case(embedded_nul [=[
.globals=0
main() .locals=4
    inlenter r0,helper(),"count=1;args=list(r1);a0=r2;proxies=();owned=(r2);prep=none\0ignored",done
done:
    ret
helper() .locals=0
    ret
]=] "contains NUL")

reject_case(invalid_module [=[
.globals=1
main() .locals=3
    qglobbind r1,"@",0
    ret
]=] "invalid qualified module selector")

reject_case(overflow_procedure [=[
.globals=0
main() .locals=1
    call @4294967296::0()
    ret
]=] "invalid qualified procedure ordinal")

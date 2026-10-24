foreach(required IN ITEMS RXC RXAS RXLINK RXDAS SOURCE WORK)
    if(NOT DEFINED ${required})
        message(FATAL_ERROR "Missing ${required}")
    endif()
endforeach()
file(REMOVE_RECURSE "${WORK}")
file(MAKE_DIRECTORY "${WORK}")

function(run_checked)
    execute_process(COMMAND ${ARGV} WORKING_DIRECTORY "${WORK}"
        RESULT_VARIABLE result OUTPUT_VARIABLE out ERROR_VARIABLE err)
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "Core-provider command failed (${result}): ${ARGV}\n${out}${err}")
    endif()
endfunction()

# Exercise the real compiler declaration/emission path and keep the native
# identities in the linked RXBIN and map while projecting no external archive.
run_checked("${RXC}" -n -x --no-exe-import -o core "${SOURCE}")
run_checked("${RXAS}" -n -o core core)
run_checked("${RXLINK}" -m core.map -p core.rxproviders -o core-linked core.rxbin)
file(READ "${WORK}/core.rxproviders" requirements)
if(NOT requirements STREQUAL "CREXX-RXPA-REQUIREMENTS 1\n")
    message(FATAL_ERROR "Mandatory core became external archive requirements: ${requirements}")
endif()
file(READ "${WORK}/core.map" map)
if(NOT map MATCHES "required rxparse rxparse[.]cursor" OR
   NOT map MATCHES "required rxinvoke rxinvoke[.]integer")
    message(FATAL_ERROR "Native core identity disappeared from link map: ${map}")
endif()
run_checked("${RXDAS}" -o core-linked.rxas core-linked.rxbin)
file(READ "${WORK}/core-linked.rxas" linked)
if(NOT linked MATCHES "[.]provider\" \"rxparse\"" OR
   NOT linked MATCHES "[.]provider\" \"rxinvoke\"")
    message(FATAL_ERROR "Native core provider metadata disappeared from linked RXBIN")
endif()

function(probe label provider symbol result_type arguments options accepted)
    file(WRITE "${WORK}/${label}.rxas"
        ".globals=0\n"
        "native() .expose=${symbol}\n"
        "   .meta \"${symbol}\"=\"${options}\" \"${result_type}\" native() \"${arguments}\"\n"
        "   .meta \"${symbol}\"=\".provider\" \"${provider}\"\n"
        "main() .locals=2\n   load r1,0\n   call r0,native(),r1\n   ret 0\n")
    run_checked("${RXAS}" -n -o "${label}" "${label}.rxas")
    execute_process(COMMAND "${RXLINK}" -p "${label}.rxproviders" -o "${label}-linked" "${label}.rxbin"
        WORKING_DIRECTORY "${WORK}" RESULT_VARIABLE result OUTPUT_VARIABLE out ERROR_VARIABLE err)
    if(accepted)
        if(NOT result EQUAL 0)
            message(FATAL_ERROR "${label} was rejected (${result}): ${out}${err}")
        endif()
        file(READ "${WORK}/${label}.rxproviders" projected)
        if(NOT projected MATCHES "required\tother\tother[.]call\t[.]int\t\t")
            message(FATAL_ERROR "External provider requirement disappeared: ${projected}")
        endif()
    elseif(result EQUAL 0 OR NOT "${out}${err}" MATCHES "conflicting mandatory core provider contract")
        message(FATAL_ERROR "${label} did not reject a conflicting core contract (${result}): ${out}${err}")
    endif()
endfunction()

probe(wrong-result rxparse rxparse.cursor.next .string "" b FALSE)
probe(renamed-argument rxparse rxparse.cursor.supply .void "renamed=.string" b FALSE)
probe(reference-argument rxparse rxparse.cursor.supply .void "expose text=.string" b FALSE)
probe(optional-argument rxparse rxparse.cursor.supply .void "?text=.string" b FALSE)
probe(wrong-options rxparse rxparse.cursor.next .int "" g FALSE)
probe(wrong-provider other rxparse.cursor.next .int "" b FALSE)
probe(unknown-symbol rxparse other.call .int "" b FALSE)
probe(wrong-core-provider rxinvoke rxparse.cursor.next .int "" b FALSE)
probe(external-provider other other.call .int "" b TRUE)
file(REMOVE_RECURSE "${WORK}")
message(STATUS "Mandatory core metadata, external projection and conflicts passed")

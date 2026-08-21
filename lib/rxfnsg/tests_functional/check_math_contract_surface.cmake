foreach(required_variable IN ITEMS
        INTEGER_SOURCE INTEGER_CONTRACT DECIMAL_SOURCE DECIMAL_CONTRACT)
    if(NOT DEFINED ${required_variable})
        message(FATAL_ERROR "${required_variable} is required")
    endif()
endforeach()

function(require_namespace_contract namespace_name source_path contract_path)
    file(READ "${source_path}" source_text)
    file(READ "${contract_path}" contract_text)

    string(REGEX MATCH
           "namespace[ \t]+${namespace_name}[ \t]+expose[ \t]+([^\r\n]+)"
           namespace_declaration "${source_text}")
    if(NOT namespace_declaration)
        message(FATAL_ERROR
                "No exposed ${namespace_name} namespace declaration found")
    endif()

    set(exposed_text "${CMAKE_MATCH_1}")
    string(REGEX MATCHALL "[A-Za-z_][A-Za-z0-9_]*"
           exposed_names "${exposed_text}")
    if(NOT exposed_names)
        message(FATAL_ERROR "No exposed ${namespace_name} procedures found")
    endif()

    foreach(exposed_name IN LISTS exposed_names)
        string(REGEX MATCHALL
               "${namespace_name}\\.\\.${exposed_name}\\("
               contract_calls "${contract_text}")
        list(LENGTH contract_calls contract_count)
        if(contract_count LESS 1)
            message(FATAL_ERROR
                    "${namespace_name}.${exposed_name} has no public contract call")
        endif()
    endforeach()

    list(LENGTH exposed_names exposed_count)
    message(STATUS
            "${namespace_name} contract covers ${exposed_count} exposed procedures")
endfunction()

require_namespace_contract(rxint "${INTEGER_SOURCE}" "${INTEGER_CONTRACT}")
require_namespace_contract(rxdecimal "${DECIMAL_SOURCE}" "${DECIMAL_CONTRACT}")

file(READ "${DECIMAL_CONTRACT}" decimal_contract_text)
foreach(required_digits IN ITEMS 9 10 18 19 32 33 64 65 96 97 128)
    string(REGEX MATCH
           "numeric[ \t]+digits[ \t]+${required_digits}([^0-9]|$)"
           context_match "${decimal_contract_text}")
    if(NOT context_match)
        message(FATAL_ERROR
                "Decimal contract has no numeric digits ${required_digits} context")
    endif()
endforeach()

message(STATUS "RCC-5C decimal contract covers all required caller contexts")

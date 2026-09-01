include_guard(GLOBAL)
include(CMakeParseArguments)

function(crexx_add_operation_contract)
    set(options)
    set(oneValueArgs TARGET RXBIN OPERATION CONTRACT_VERSION OUTPUT PREVIOUS)
    set(multiValueArgs NULLABLE OPTIONAL_FIELDS ERROR_TYPES)
    cmake_parse_arguments(CONTRACT "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    foreach(_required IN ITEMS TARGET RXBIN OPERATION CONTRACT_VERSION OUTPUT)
        if(NOT CONTRACT_${_required})
            message(FATAL_ERROR "crexx_add_operation_contract requires ${_required}")
        endif()
    endforeach()
    if(CONTRACT_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR
                "crexx_add_operation_contract received unknown arguments: ${CONTRACT_UNPARSED_ARGUMENTS}")
    endif()

    if(TARGET crexx-contract)
        set(_contract_tool "$<TARGET_FILE:crexx-contract>")
        set(_contract_tool_dependency crexx-contract)
    elseif(TARGET CREXX::crexx-contract)
        set(_contract_tool "$<TARGET_FILE:CREXX::crexx-contract>")
        set(_contract_tool_dependency CREXX::crexx-contract)
    else()
        message(FATAL_ERROR "crexx-contract executable target is unavailable")
    endif()

    set(_command
            "${_contract_tool}"
            --rxbin "${CONTRACT_RXBIN}"
            --operation "${CONTRACT_OPERATION}"
            --contract-version "${CONTRACT_CONTRACT_VERSION}"
            --output "${CONTRACT_OUTPUT}")
    set(_dependencies "${CONTRACT_RXBIN}" "${_contract_tool_dependency}")
    if(CONTRACT_PREVIOUS)
        list(APPEND _command --previous "${CONTRACT_PREVIOUS}")
        list(APPEND _dependencies "${CONTRACT_PREVIOUS}")
    endif()
    foreach(_field IN LISTS CONTRACT_NULLABLE)
        list(APPEND _command --nullable "${_field}")
    endforeach()
    foreach(_field IN LISTS CONTRACT_OPTIONAL_FIELDS)
        list(APPEND _command --optional-field "${_field}")
    endforeach()
    foreach(_type IN LISTS CONTRACT_ERROR_TYPES)
        list(APPEND _command --error "${_type}")
    endforeach()

    add_custom_command(
            OUTPUT "${CONTRACT_OUTPUT}"
            COMMAND ${_command}
            DEPENDS ${_dependencies}
            VERBATIM
            COMMENT "Generate CREXX operation contract ${CONTRACT_TARGET}")
    add_custom_target("${CONTRACT_TARGET}" DEPENDS "${CONTRACT_OUTPUT}")
    set("${CONTRACT_TARGET}_OUTPUT" "${CONTRACT_OUTPUT}" PARENT_SCOPE)
endfunction()

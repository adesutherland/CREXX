if(NOT DEFINED PROVIDER_SOURCE OR NOT DEFINED CONTRACT_SOURCE)
    message(FATAL_ERROR
            "PROVIDER_SOURCE and CONTRACT_SOURCE are required")
endif()

file(READ "${PROVIDER_SOURCE}" provider_source)
file(READ "${CONTRACT_SOURCE}" contract_source)

string(REGEX MATCHALL
       "RXFLOAT_ADD_(UNARY|BINARY|NULLARY)\\([A-Za-z0-9_]+,"
       registrations "${provider_source}")

set(public_names)
foreach(registration IN LISTS registrations)
    string(REGEX REPLACE
           "RXFLOAT_ADD_(UNARY|BINARY|NULLARY)\\(([A-Za-z0-9_]+),"
           "\\2" public_name "${registration}")
    if(NOT public_name STREQUAL "name")
        list(APPEND public_names "${public_name}")
    endif()
endforeach()
list(REMOVE_DUPLICATES public_names)

if(NOT public_names)
    message(FATAL_ERROR "No rxfloat registrations found")
endif()

foreach(public_name IN LISTS public_names)
    string(REGEX MATCHALL
           "rxfloat\\.\\.${public_name}\\("
           canonical_calls "${contract_source}")
    list(LENGTH canonical_calls canonical_count)
    if(canonical_count LESS 2)
        message(FATAL_ERROR
                "rxfloat.${public_name} needs an expected-value contract call and a compatibility comparison")
    endif()

    string(REGEX MATCHALL
           "rxmath\\.\\.${public_name}\\("
           compatibility_calls "${contract_source}")
    list(LENGTH compatibility_calls compatibility_count)
    if(compatibility_count LESS 1)
        message(FATAL_ERROR
                "rxmath.${public_name} needs a compatibility comparison")
    endif()
endforeach()

list(LENGTH public_names public_count)
message(STATUS
        "rxfloat contract suite covers ${public_count} canonical and compatibility procedures")

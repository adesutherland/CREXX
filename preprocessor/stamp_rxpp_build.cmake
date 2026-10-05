if(NOT DEFINED RXPP_INPUT OR NOT DEFINED RXPP_OUTPUT)
    message(FATAL_ERROR "RXPP_INPUT and RXPP_OUTPUT are required")
endif()

file(READ "${RXPP_INPUT}" rxpp_source)
string(TIMESTAMP rxpp_build_timestamp "%Y-%m-%d %H:%M:%S UTC" UTC)
string(REPLACE
    "rxpp_build_timestamp = 'RXPP_BUILD_TIMESTAMP'"
    "rxpp_build_timestamp = '${rxpp_build_timestamp}'"
    stamped_source "${rxpp_source}")

if(stamped_source STREQUAL rxpp_source)
    message(FATAL_ERROR "RXPP build timestamp marker was not found in ${RXPP_INPUT}")
endif()

file(WRITE "${RXPP_OUTPUT}" "${stamped_source}")

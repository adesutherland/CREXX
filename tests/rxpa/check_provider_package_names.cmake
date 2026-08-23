if(NOT DEFINED PROVIDER_DIR OR NOT DEFINED PROVIDER_ID OR
   NOT DEFINED STATIC_SUFFIX)
    message(FATAL_ERROR "provider package-name test arguments are incomplete")
endif()

set(_dynamic "${PROVIDER_DIR}/${PROVIDER_ID}.rxplugin")
set(_canonical_static "${PROVIDER_DIR}/${PROVIDER_ID}${STATIC_SUFFIX}")
set(_legacy_static "${PROVIDER_DIR}/${PROVIDER_ID}_static${STATIC_SUFFIX}")
set(_retired_sidecar "${PROVIDER_DIR}/${PROVIDER_ID}.rxprovider")

foreach(_required IN ITEMS "${_dynamic}" "${_canonical_static}" "${_legacy_static}")
    if(NOT EXISTS "${_required}")
        message(FATAL_ERROR "provider package artifact is missing: ${_required}")
    endif()
endforeach()

if(EXISTS "${_retired_sidecar}")
    message(FATAL_ERROR "retired provider sidecar was published: ${_retired_sidecar}")
endif()

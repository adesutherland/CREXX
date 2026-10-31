include_guard(GLOBAL)

# One fingerprint owner for the generated B/L catalogue and native adapters.
set(_crexx_catalog_inputs
  "${CMAKE_SOURCE_DIR}/binutils/include/rxops.h"
  "${CMAKE_SOURCE_DIR}/binutils/include/rxopeffects.h"
  "${CMAKE_SOURCE_DIR}/binutils/include/rxopsignals.h"
  "${CMAKE_SOURCE_DIR}/binutils/include/rxdefs.h"
  "${CMAKE_SOURCE_DIR}/binutils/include/rxflags.h"
  "${CMAKE_SOURCE_DIR}/binutils/rxopmeta.c"
  "${CMAKE_SOURCE_DIR}/binutils/include/opdata.c")
set(_crexx_catalog_parts "")
foreach(_crexx_catalog_input IN LISTS _crexx_catalog_inputs)
  file(SHA256 "${_crexx_catalog_input}" _crexx_catalog_part)
  string(APPEND _crexx_catalog_parts "${_crexx_catalog_part}")
endforeach()
string(SHA256 _crexx_catalog_fingerprint "${_crexx_catalog_parts}")
set_property(GLOBAL PROPERTY CREXX_RXAS_CATALOG_FINGERPRINT "${_crexx_catalog_fingerprint}")
set_property(GLOBAL PROPERTY CREXX_RXAS_CATALOG_INPUTS "${_crexx_catalog_inputs}")

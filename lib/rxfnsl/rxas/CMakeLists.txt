cmake_minimum_required(VERSION 3.24)

# Extract the owning native metadata once at build time, never in a request.
set_property(GLOBAL PROPERTY RXFNSL_RXAS_SELECTORS "llrxascatalog;llrxas")
add_executable(llrxascataloggen EXCLUDE_FROM_ALL
  "${CMAKE_SOURCE_DIR}/tools/generate-rxas-catalog.c"
  "${CMAKE_SOURCE_DIR}/binutils/include/opdata.c"
  "${CMAKE_SOURCE_DIR}/binutils/rxopmeta.c")
target_include_directories(llrxascataloggen PRIVATE "${CMAKE_SOURCE_DIR}/binutils/include")
set(_catalog_inputs
  "${CMAKE_SOURCE_DIR}/binutils/include/rxops.h"
  "${CMAKE_SOURCE_DIR}/binutils/include/rxopeffects.h"
  "${CMAKE_SOURCE_DIR}/binutils/include/rxopsignals.h"
  "${CMAKE_SOURCE_DIR}/binutils/include/rxdefs.h"
  "${CMAKE_SOURCE_DIR}/binutils/rxopmeta.c"
  "${CMAKE_SOURCE_DIR}/binutils/include/opdata.c")
set(_fingerprints "")
foreach(_input IN LISTS _catalog_inputs)
  file(SHA256 "${_input}" _sha)
  string(APPEND _fingerprints "${_sha}")
endforeach()
string(SHA256 _fingerprint "${_fingerprints}")
target_compile_definitions(llrxascataloggen PRIVATE LL_RXAS_CATALOG_FINGERPRINT="${_fingerprint}")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${_catalog_inputs})
if(BUILD_TESTING)
  add_executable(test_llrxascataloggen_no_tmpfile EXCLUDE_FROM_ALL
    "${CMAKE_SOURCE_DIR}/tools/tests/generate_rxas_catalog_no_tmpfile.c"
    "${CMAKE_SOURCE_DIR}/binutils/include/opdata.c"
    "${CMAKE_SOURCE_DIR}/binutils/rxopmeta.c")
  target_include_directories(test_llrxascataloggen_no_tmpfile PRIVATE
    "${CMAKE_SOURCE_DIR}/binutils/include")
  target_compile_definitions(test_llrxascataloggen_no_tmpfile PRIVATE
    LL_RXAS_CATALOG_FINGERPRINT="${_fingerprint}")
  add_test(NAME llrxascataloggen_scratch
    COMMAND ${CMAKE_COMMAND}
      "-DGENERATOR=$<TARGET_FILE:llrxascataloggen>"
      "-DDENIED_GENERATOR=$<TARGET_FILE:test_llrxascataloggen_no_tmpfile>"
      "-DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/catalog scratch test"
      -P "${CMAKE_SOURCE_DIR}/tools/tests/check_rxas_catalog_scratch.cmake")
  set_tests_properties(llrxascataloggen_scratch PROPERTIES
    LABELS "unit;build;rxas;smoke" TIMEOUT 30)
  crexx_register_test_prep_targets(TESTS llrxascataloggen_scratch
    TARGETS llrxascataloggen test_llrxascataloggen_no_tmpfile)
  crexx_finalize_directory_qa_tiers()
endif()
set(_catalog "${CMAKE_CURRENT_BINARY_DIR}/llrxascatalog.crexx")
add_custom_command(OUTPUT "${_catalog}"
  COMMAND $<TARGET_FILE:llrxascataloggen> "${_catalog}"
  DEPENDS llrxascataloggen ${_catalog_inputs} VERBATIM)
set(_imports "${CMAKE_CURRENT_BINARY_DIR}/imports")
crexx_add_import_root(rxfnsl_rxas_imports DIRECTORY "${_imports}"
  OUTPUT_VARIABLE _import_files FILES "${CMAKE_BINARY_DIR}/bin/library.rxbin"
  DEPENDS library)
set(_flags)
if(CREXX_LEVEL_L_NOOPT)
  list(APPEND _flags -n)
endif()
set(_bins)
foreach(_module llrxascatalog llrxas)
  set(_source "${CMAKE_CURRENT_SOURCE_DIR}/${_module}.crexx")
  set(_deps)
  if(_module STREQUAL "llrxascatalog")
    set(_source "${_catalog}")
  else()
    list(APPEND _deps "${CMAKE_CURRENT_BINARY_DIR}/llrxascatalog.rxbin")
  endif()
  crexx_add_fixture_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${_module}.rxbin"
    COMMAND $<TARGET_FILE:rxc> ${_flags} -x --no-exe-import
      -i "${_imports};${CMAKE_CURRENT_BINARY_DIR}" -o "${_module}" "${_source}"
    COMMAND $<TARGET_FILE:rxas> ${_flags} "${_module}"
    DEPENDS rxc rxas rxfnsl_rxas_imports ${_import_files} ${_deps} "${_source}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}" VERBATIM)
  list(APPEND _bins "${CMAKE_CURRENT_BINARY_DIR}/${_module}.rxbin")
endforeach()
set(_control "${CMAKE_CURRENT_BINARY_DIR}/rxas.ctl")
file(GENERATE OUTPUT "${_control}" CONTENT
  "INPUT ${CMAKE_CURRENT_BINARY_DIR}/llrxascatalog.rxbin\nINPUT ${CMAKE_CURRENT_BINARY_DIR}/llrxas.rxbin\nINCLUDE llrxascatalog\nINCLUDE llrxas\nPRESERVE INLINE\nOUTPUT ${CMAKE_BINARY_DIR}/bin/rxfnsl-rxas\n")
crexx_add_fixture_command(OUTPUT "${CMAKE_BINARY_DIR}/bin/rxfnsl-rxas.rxbin"
  COMMAND $<TARGET_FILE:rxlink> -c "${_control}"
  DEPENDS rxlink ${_bins} "${_control}" VERBATIM)
add_custom_target(rxfnsl_rxas ALL DEPENDS "${CMAKE_BINARY_DIR}/bin/rxfnsl-rxas.rxbin")
crexx_install_bytecode(rxfnsl_rxas "${CMAKE_BINARY_DIR}/bin/rxfnsl-rxas.rxbin")

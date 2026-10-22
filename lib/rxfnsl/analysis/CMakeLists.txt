cmake_minimum_required(VERSION 3.24)

# Neutral analyses are a separate image: grammar bootstrap selects only rxfnsl.
set(_modules llirgraph llireffects)
set_property(GLOBAL PROPERTY RXFNSL_ANALYSIS_SELECTORS "${_modules}")
set(_imports "${CMAKE_CURRENT_BINARY_DIR}/imports")
crexx_add_import_root(rxfnsl_analysis_imports DIRECTORY "${_imports}"
  OUTPUT_VARIABLE _import_files
  FILES "${CMAKE_BINARY_DIR}/bin/library.rxbin" "${CMAKE_BINARY_DIR}/bin/rxfnsl.rxbin"
  DEPENDS library rxfnsl)
set(_flags)
if(CREXX_LEVEL_L_NOOPT)
  list(APPEND _flags -n)
endif()
set(_bins)
foreach(_module IN LISTS _modules)
  set(_source "${CMAKE_CURRENT_SOURCE_DIR}/${_module}.crexx")
  set(_deps)
  if(_module STREQUAL "llireffects")
    list(APPEND _deps "${CMAKE_CURRENT_BINARY_DIR}/llirgraph.rxbin")
  endif()
  crexx_add_fixture_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${_module}.rxbin"
    COMMAND $<TARGET_FILE:rxc> ${_flags} -x --no-exe-import
      -i "${_imports};${CMAKE_CURRENT_BINARY_DIR}" -o "${_module}" "${_source}"
    COMMAND $<TARGET_FILE:rxas> ${_flags} "${_module}"
    DEPENDS rxc rxas rxfnsl_analysis_imports ${_import_files} ${_deps} "${_source}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}" VERBATIM)
  list(APPEND _bins "${CMAKE_CURRENT_BINARY_DIR}/${_module}.rxbin")
endforeach()
set(_control "${CMAKE_CURRENT_BINARY_DIR}/analysis.ctl")
file(GENERATE OUTPUT "${_control}" CONTENT
  "INPUT ${CMAKE_CURRENT_BINARY_DIR}/llirgraph.rxbin\nINPUT ${CMAKE_CURRENT_BINARY_DIR}/llireffects.rxbin\nINCLUDE llirgraph\nINCLUDE llireffects\nPRESERVE INLINE\nOUTPUT ${CMAKE_BINARY_DIR}/bin/rxfnsl-analysis\n")
crexx_add_fixture_command(OUTPUT "${CMAKE_BINARY_DIR}/bin/rxfnsl-analysis.rxbin"
  COMMAND $<TARGET_FILE:rxlink> -c "${_control}"
  DEPENDS rxlink ${_bins} "${_control}" VERBATIM)
add_custom_target(rxfnsl_analysis ALL DEPENDS "${CMAKE_BINARY_DIR}/bin/rxfnsl-analysis.rxbin")
crexx_install_bytecode(rxfnsl_analysis "${CMAKE_BINARY_DIR}/bin/rxfnsl-analysis.rxbin")

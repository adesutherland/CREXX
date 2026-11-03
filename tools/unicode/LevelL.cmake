set(CREXX_ROOT "${CMAKE_CURRENT_LIST_DIR}/../..")
if(TARGET rxtvm)
  set(_ll_vm rxtvm)
else()
  set(_ll_vm rxbvm)
endif()
# The Unicode builder is a downstream consumer of the completed L2 tool.
set(_gennorm_spec "${CMAKE_CURRENT_LIST_DIR}/gennorm2.levell")
set(_gennorm_rxas "${CMAKE_CURRENT_BINARY_DIR}/gennorm2_language.rxas")
set(_gennorm_stamp "${_gennorm_rxas}.generation.qa-output")
set(_gennorm_generate_args
  -DLEVEL_L_VM=$<TARGET_FILE:${_ll_vm}>
  -DLEVEL_L_BYTECODE=${CMAKE_BINARY_DIR}/langtools/bootstrap/L2/rxlang-linked.rxbin
  -DSPEC=${_gennorm_spec} -DOUTPUT=${_gennorm_rxas} -DFORMAT=rxas
  -DLEXER_WORKERS=${CREXX_LEVEL_L_LEXER_WORKERS})
crexx_add_fixture_command(OUTPUT "${_gennorm_stamp}" BYPRODUCTS "${_gennorm_rxas}"
  COMMAND ${CMAKE_COMMAND} ${_gennorm_generate_args}
    -P "${CREXX_ROOT}/langtools/cmake/generate.cmake"
  COMMAND ${CMAKE_COMMAND} -E touch "${_gennorm_stamp}"
  DEPENDS ${_ll_vm} "${CMAKE_BINARY_DIR}/langtools/bootstrap/verified.stamp" "${CMAKE_BINARY_DIR}/langtools/bootstrap/L2/rxlang-linked.rxbin" "${_gennorm_spec}" "${CREXX_ROOT}/langtools/cmake/generate.cmake" VERBATIM)
if(CMAKE_GENERATOR MATCHES "^Ninja")
  add_custom_target(levell_unicode_source DEPENDS "${_gennorm_stamp}")
else()
  add_custom_target(levell_unicode_source
    COMMAND ${CMAKE_COMMAND} ${_gennorm_generate_args}
      -P "${CREXX_ROOT}/langtools/cmake/ensure-generated.cmake"
    DEPENDS "${_gennorm_stamp}" VERBATIM)
endif()
add_dependencies(levell_unicode_source rxlang_bootstrap)
crexx_add_fixture_command(OUTPUT "${CMAKE_BINARY_DIR}/bin/gennorm2_language.rxbin"
  COMMAND $<TARGET_FILE:rxas> -o "${CMAKE_BINARY_DIR}/bin/gennorm2_language" "${_gennorm_rxas}"
  DEPENDS rxas levell_unicode_source "${_gennorm_rxas}" VERBATIM)
add_custom_target(levell_unicode_lexer DEPENDS "${CMAKE_BINARY_DIR}/bin/gennorm2_language.rxbin")
add_dependencies(levell_unicode_lexer rxlang_bootstrap levell_unicode_source)

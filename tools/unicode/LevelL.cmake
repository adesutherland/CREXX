set(CREXX_ROOT "${CMAKE_CURRENT_LIST_DIR}/../..")
if(TARGET rxtvm)
  set(_ll_vm rxtvm)
else()
  set(_ll_vm rxbvm)
endif()
# The Unicode builder is a downstream consumer of the completed L2 tool.
set(_gennorm_spec "${CMAKE_CURRENT_LIST_DIR}/gennorm2.levell")
set(_gennorm_rxas "${CMAKE_CURRENT_BINARY_DIR}/gennorm2_language.rxas")
crexx_add_fixture_command(OUTPUT "${_gennorm_rxas}"
  COMMAND ${CMAKE_COMMAND} -DLEVEL_L_VM=$<TARGET_FILE:${_ll_vm}>
    -DLEVEL_L_BYTECODE=${CMAKE_BINARY_DIR}/langtools/bootstrap/L2/rxlang-linked.rxbin
    -DSPEC=${_gennorm_spec} -DOUTPUT=${_gennorm_rxas} -DFORMAT=rxas
    -P "${CREXX_ROOT}/langtools/cmake/generate.cmake"
  DEPENDS ${_ll_vm} "${CMAKE_BINARY_DIR}/langtools/bootstrap/verified.stamp" "${CMAKE_BINARY_DIR}/langtools/bootstrap/L2/rxlang-linked.rxbin" "${_gennorm_spec}" "${CREXX_ROOT}/langtools/cmake/generate.cmake" VERBATIM)
crexx_add_fixture_command(OUTPUT "${CMAKE_BINARY_DIR}/bin/gennorm2_language.rxbin"
  COMMAND $<TARGET_FILE:rxas> -o "${CMAKE_BINARY_DIR}/bin/gennorm2_language" "${_gennorm_rxas}"
  DEPENDS rxas "${_gennorm_rxas}" VERBATIM)
add_custom_target(levell_unicode_lexer DEPENDS "${CMAKE_BINARY_DIR}/bin/gennorm2_language.rxbin")
add_dependencies(levell_unicode_lexer rxlang_bootstrap)

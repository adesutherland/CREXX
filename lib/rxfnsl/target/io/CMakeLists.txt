cmake_minimum_required(VERSION 3.24)

# The prepared L grammar is build input; no parsing request invokes a tool.
set(_grammar "${CMAKE_CURRENT_BINARY_DIR}/llrxasgrammar.rxas")
# Full Unicode RXAS uses the maintained Classic grammar's finite DFA allowance.
# Keep this build profile local and reuse the canonical output-owned lexer cache.
set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_CLEAN_FILES
  "${_grammar}.lexer" "${_grammar}.lexer.manifest" "${_grammar}.lexer.lock")
add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/llrxasgrammar.rxbin"
  COMMAND ${CMAKE_COMMAND} -DLEVEL_L=$<TARGET_FILE:rxlang>
    -DSPEC=${CMAKE_CURRENT_SOURCE_DIR}/rxas.rxl -DOUTPUT=${_grammar} -DFORMAT=rxas
    -DGENERATOR_LIMITS=dfa.states=16384 -DREUSE_LEXER=ON
    -P "${CMAKE_SOURCE_DIR}/langtools/cmake/generate.cmake"
  COMMAND $<TARGET_FILE:rxas> "${CMAKE_CURRENT_BINARY_DIR}/llrxasgrammar"
  DEPENDS rxlang rxlanglib rxas "${CMAKE_CURRENT_SOURCE_DIR}/rxas.rxl"
    "${CMAKE_SOURCE_DIR}/langtools/cmake/generate.cmake" VERBATIM)
add_custom_target(llrxasgrammar
  DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/llrxasgrammar.rxbin")

crexx_add_import_root(rxfnsl_target_io_imports
  DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/imports" OUTPUT_VARIABLE _imports
  FILES "${CMAKE_BINARY_DIR}/bin/library.rxbin"
    "${CMAKE_BINARY_DIR}/bin/rxfnsl.rxbin"
    "${CMAKE_BINARY_DIR}/bin/rxfnsl-rxas.rxbin"
    "${CMAKE_BINARY_DIR}/bin/rxfnsl-target.rxbin"
    "${CMAKE_BINARY_DIR}/langtools/common/llrxastext.rxbin"
  DEPENDS library rxfnsl rxfnsl_rxas rxfnsl_target llrxastext)
set(_flags)
if(CREXX_LEVEL_L_NOOPT)
  list(APPEND _flags -n)
endif()
set(_bins "${CMAKE_CURRENT_BINARY_DIR}/llrxasgrammar.rxbin")
foreach(_module llrxasinput llrxasoutput)
  crexx_add_fixture_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${_module}.rxbin"
    COMMAND $<TARGET_FILE:rxc> ${_flags} -x --no-exe-import
      -i "${CMAKE_CURRENT_BINARY_DIR}/imports;${CMAKE_CURRENT_BINARY_DIR}"
      -o "${CMAKE_CURRENT_BINARY_DIR}/${_module}"
      "${CMAKE_CURRENT_SOURCE_DIR}/${_module}.crexx"
    COMMAND $<TARGET_FILE:rxas> ${_flags} "${CMAKE_CURRENT_BINARY_DIR}/${_module}"
    DEPENDS rxc rxas rxfnsl_target_io_imports ${_imports} llrxasgrammar
      "${CMAKE_CURRENT_SOURCE_DIR}/${_module}.crexx" VERBATIM)
  list(APPEND _bins "${CMAKE_CURRENT_BINARY_DIR}/${_module}.rxbin")
endforeach()
set(_control "${CMAKE_CURRENT_BINARY_DIR}/target-io.ctl")
file(GENERATE OUTPUT "${_control}" CONTENT
  "INPUT ${CMAKE_CURRENT_BINARY_DIR}/llrxasgrammar.rxbin\nINPUT ${CMAKE_CURRENT_BINARY_DIR}/llrxasinput.rxbin\nINPUT ${CMAKE_CURRENT_BINARY_DIR}/llrxasoutput.rxbin\nINCLUDE llrxasgrammar\nINCLUDE llrxasinput\nINCLUDE llrxasoutput\nPRESERVE INLINE\nOUTPUT ${CMAKE_BINARY_DIR}/bin/rxfnsl-target-io\n")
crexx_add_fixture_command(OUTPUT "${CMAKE_BINARY_DIR}/bin/rxfnsl-target-io.rxbin"
  COMMAND $<TARGET_FILE:rxlink> -c "${_control}"
  DEPENDS rxlink ${_bins} "${_control}" VERBATIM)
add_custom_target(rxfnsl_target_io
  DEPENDS "${CMAKE_BINARY_DIR}/bin/rxfnsl-target-io.rxbin")
crexx_install_bytecode(rxfnsl_target_io
  "${CMAKE_BINARY_DIR}/bin/rxfnsl-target-io.rxbin")

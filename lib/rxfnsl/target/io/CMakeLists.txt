cmake_minimum_required(VERSION 3.24)
set(CREXX_RXAS_GRAMMAR_LEXER_WORKERS "4" CACHE STRING
  "Maintained RXAS grammar lexer shards: 0 serial, or 1/2/4/8 local task workers")
set_property(CACHE CREXX_RXAS_GRAMMAR_LEXER_WORKERS PROPERTY STRINGS 0 1 2 4 8)
if(NOT CREXX_RXAS_GRAMMAR_LEXER_WORKERS MATCHES "^(0|1|2|4|8)$")
  message(FATAL_ERROR "CREXX_RXAS_GRAMMAR_LEXER_WORKERS must be 0, 1, 2, 4 or 8")
endif()

# The prepared L grammar is build input; no parsing request invokes a tool.
set(_grammar "${CMAKE_CURRENT_BINARY_DIR}/llrxasgrammar.rxas")
set(_grammar_stamp "${_grammar}.generation.stamp")
set(_generator_identity "${CMAKE_BINARY_DIR}/langtools/bootstrap/source/generator.identity")
# Full Unicode RXAS uses the maintained Classic grammar's finite DFA allowance.
# Keep this build profile local and reuse the canonical output-owned lexer cache.
set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_CLEAN_FILES
  "${_grammar}.lexer" "${_grammar}.lexer.manifest" "${_grammar}.lexer.lock")
set(_generate_args
  -DLEVEL_L=$<TARGET_FILE:rxlang>
  -DSPEC=${CMAKE_CURRENT_SOURCE_DIR}/rxas.rxl -DOUTPUT=${_grammar} -DFORMAT=rxas
  -DGENERATOR_LIMITS=dfa.states=16384 -DREUSE_LEXER=ON
  -DLEXER_WORKERS=${CREXX_RXAS_GRAMMAR_LEXER_WORKERS}
  -DGENERATOR_IDENTITY=${_generator_identity})
add_custom_command(OUTPUT "${_grammar_stamp}" BYPRODUCTS "${_grammar}"
  COMMAND ${CMAKE_COMMAND} ${_generate_args}
    -P "${CMAKE_SOURCE_DIR}/langtools/cmake/generate.cmake"
  COMMAND ${CMAKE_COMMAND} -E touch "${_grammar_stamp}"
  DEPENDS rxlang rxlanglib "${_generator_identity}" "${CMAKE_CURRENT_SOURCE_DIR}/rxas.rxl"
    "${CMAKE_SOURCE_DIR}/langtools/cmake/generate.cmake" VERBATIM)
if(CMAKE_GENERATOR MATCHES "^Ninja")
  add_custom_target(llrxasgrammar_source DEPENDS "${_grammar_stamp}")
else()
  # Make-style generators do not rebuild a missing BYPRODUCT when its success
  # stamp remains. The guard is cheap on no-op builds and uses the same owner.
  add_custom_target(llrxasgrammar_source
    COMMAND ${CMAKE_COMMAND} ${_generate_args}
      -P "${CMAKE_SOURCE_DIR}/langtools/cmake/ensure-generated.cmake"
    DEPENDS "${_grammar_stamp}" VERBATIM)
endif()
add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/llrxasgrammar.rxbin"
  COMMAND $<TARGET_FILE:rxas> "${CMAKE_CURRENT_BINARY_DIR}/llrxasgrammar"
  DEPENDS rxas "${_grammar}" VERBATIM)
add_custom_target(llrxasgrammar
  DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/llrxasgrammar.rxbin")
add_dependencies(llrxasgrammar llrxasgrammar_source)

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
set(_link_root "${CMAKE_CURRENT_BINARY_DIR}/linked")
set(_link_image "${_link_root}/rxfnsl-target-io.rxbin")
set(_publish_image "${CMAKE_BINARY_DIR}/bin/rxfnsl-target-io.rxbin")
set(_publish_temp "${CMAKE_CURRENT_BINARY_DIR}/publish/rxfnsl-target-io.rxbin.tmp")
file(GENERATE OUTPUT "${_control}" CONTENT
  "INPUT ${CMAKE_CURRENT_BINARY_DIR}/llrxasgrammar.rxbin\nINPUT ${CMAKE_CURRENT_BINARY_DIR}/llrxasinput.rxbin\nINPUT ${CMAKE_CURRENT_BINARY_DIR}/llrxasoutput.rxbin\nINCLUDE llrxasgrammar\nINCLUDE llrxasinput\nINCLUDE llrxasoutput\nPRESERVE INLINE\nOUTPUT ${_link_root}/rxfnsl-target-io\n")
crexx_add_fixture_command(OUTPUT "${_link_image}"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${_link_root}"
  COMMAND $<TARGET_FILE:rxlink> -c "${_control}"
  DEPENDS rxlink ${_bins} "${_control}" VERBATIM)
add_custom_command(OUTPUT "${_publish_image}"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/publish"
  COMMAND ${CMAKE_COMMAND} -E copy "${_link_image}" "${_publish_temp}"
  COMMAND ${CMAKE_COMMAND} -E rename "${_publish_temp}" "${_publish_image}"
  DEPENDS "${_link_image}" VERBATIM)
add_custom_target(rxfnsl_target_io
  DEPENDS "${_publish_image}")
crexx_install_bytecode(rxfnsl_target_io "${_publish_image}")

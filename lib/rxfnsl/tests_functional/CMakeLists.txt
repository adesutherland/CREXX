cmake_minimum_required(VERSION 3.24)

project(test_rxfnsl)

enable_testing()

include(${CMAKE_SOURCE_DIR}/cmake/CrexxTestModes.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/CrexxSmokeTests.cmake)

set(REXXBINS
  ts_tinyexpr
)

set(LIBRPATH ${CMAKE_BINARY_DIR}/bin/library)
set(RXFNSL_RPATH ${CMAKE_BINARY_DIR}/bin/rxfnsl)

add_custom_target(testrxfnsl ALL)

foreach(_binfile IN LISTS REXXBINS)
  get_filename_component(_basename "${_binfile}" NAME_WE)

  set(_rexx_file "${CMAKE_CURRENT_SOURCE_DIR}/${_basename}.crexx")

  add_custom_target("${_basename}")
  add_dependencies(testrxfnsl "${_basename}")
  crexx_add_rexx_opt_matrix(
    NAME "${_basename}"
    SOURCE "${_rexx_file}"
    TARGET_GROUP "${_basename}"
    IMPORT_PATHS "${CMAKE_BINARY_DIR}/bin"
    RXC_FLAGS -x
    DEPENDS library rxfnsl "${CMAKE_BINARY_DIR}/bin/rxfnsl.rxbin" "${_rexx_file}"
    RUNTIME_ARGS ${LIBRPATH} ${RXFNSL_RPATH}
  )
endforeach()

set_tests_properties(ts_tinyexpr_noopt ts_tinyexpr_opt
  PROPERTIES PASS_REGULAR_EXPRESSION "PASS: rxfnsl tinyexpr generated-shape demo")

crexx_label_smoke_tests(
        ts_tinyexpr_noopt
)

# A second consumer, packed-image validation, ownership and source diagnostics.
set(_generated_spec "${CMAKE_CURRENT_SOURCE_DIR}/../tools/example.levell")
set(_generated_grammar "${CMAKE_CURRENT_BINARY_DIR}/levell_example.rxas")
add_custom_command(OUTPUT "${_generated_grammar}"
  COMMAND ${CMAKE_COMMAND} -DLEVEL_L=$<TARGET_FILE:levell_cli>
    -DSPEC=${_generated_spec} -DOUTPUT=${_generated_grammar} -DFORMAT=rxas
    -P "${CMAKE_CURRENT_SOURCE_DIR}/../tools/generate.cmake"
  DEPENDS levell_cli "${_generated_spec}" "${CMAKE_CURRENT_SOURCE_DIR}/../tools/generate.cmake"
  VERBATIM)
add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/levell_example.rxbin"
  COMMAND $<TARGET_FILE:rxas> levell_example
  DEPENDS rxas "${_generated_grammar}"
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}" VERBATIM)
crexx_add_import_root(levell_generated_test_imports
  DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/imports" OUTPUT_VARIABLE _generated_imports
  FILES "${CMAKE_BINARY_DIR}/bin/library.rxbin" "${CMAKE_BINARY_DIR}/bin/rxfnsl.rxbin"
    "${CMAKE_CURRENT_BINARY_DIR}/levell_example.rxbin"
  DEPENDS library rxfnsl)
add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/testGenerated-linked.rxbin"
  COMMAND $<TARGET_FILE:rxc> -x --no-exe-import
    -i "${CMAKE_CURRENT_BINARY_DIR}/imports"
    -o testGenerated "${CMAKE_CURRENT_SOURCE_DIR}/testGenerated.crexx"
  COMMAND $<TARGET_FILE:rxas> testGenerated
  COMMAND $<TARGET_FILE:rxlink> -s -o testGenerated-linked testGenerated levell_example
    "${CMAKE_BINARY_DIR}/bin/rxfnsl" "${CMAKE_BINARY_DIR}/bin/library"
  DEPENDS rxc rxas rxlink rxfnsl levell_generated_test_imports ${_generated_imports} "${CMAKE_BINARY_DIR}/bin/rxfnsl.rxbin"
    "${CMAKE_BINARY_DIR}/bin/library.rxbin" "${_generated_grammar}" testGenerated.crexx
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}" VERBATIM)
add_custom_target(testlevell_generated ALL DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/testGenerated-linked.rxbin")
add_dependencies(testrxfnsl testlevell_generated)
foreach(_vm rxtvm rxbvm)
  if(TARGET ${_vm})
    add_test(NAME levell_generated_${_vm}
      COMMAND $<TARGET_FILE:${_vm}> "${CMAKE_CURRENT_BINARY_DIR}/testGenerated-linked.rxbin" -a "${_generated_spec}")
    set_tests_properties(levell_generated_${_vm} PROPERTIES
      PASS_REGULAR_EXPRESSION "generated_failures=0" TIMEOUT 300 LABELS "library;level-l")
    crexx_label_smoke_tests(levell_generated_${_vm}
      PREP_TARGETS testlevell_generated ${_vm})
  endif()
endforeach()

# Compile and link AST fixtures during the build. CTests launch only one VM;
# they do not rebuild tools or run a nested qualification matrix.
crexx_add_import_root(levell_ast_test_imports
  DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/ast-imports" OUTPUT_VARIABLE _ast_imports
  FILES "${CMAKE_BINARY_DIR}/bin/library.rxbin" "${CMAKE_BINARY_DIR}/bin/rxfnsl.rxbin"
  DEPENDS library rxfnsl)
foreach(_mode noopt opt)
  set(_ast_flags)
  if(_mode STREQUAL "noopt")
    list(APPEND _ast_flags -n)
  endif()
  set(_ast_name "ts_llast_${_mode}")
  add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${_ast_name}-linked.rxbin"
    COMMAND $<TARGET_FILE:rxc> ${_ast_flags} -x --no-exe-import
      -i "${CMAKE_CURRENT_BINARY_DIR}/ast-imports"
      -o "${_ast_name}" "${CMAKE_CURRENT_SOURCE_DIR}/ts_llast.crexx"
    COMMAND $<TARGET_FILE:rxas> ${_ast_flags} "${_ast_name}"
    COMMAND $<TARGET_FILE:rxlink> -s -o "${_ast_name}-linked" "${_ast_name}"
      "${CMAKE_BINARY_DIR}/bin/rxfnsl" "${CMAKE_BINARY_DIR}/bin/library"
    DEPENDS rxc rxas rxlink rxfnsl levell_ast_test_imports ${_ast_imports} ts_llast.crexx
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}" VERBATIM)
  add_custom_target(${_ast_name} ALL DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/${_ast_name}-linked.rxbin")
  add_dependencies(testrxfnsl ${_ast_name})
  foreach(_vm rxtvm rxbvm)
    if(TARGET ${_vm})
      add_test(NAME ${_ast_name}_${_vm}
        COMMAND $<TARGET_FILE:${_vm}> "${CMAKE_CURRENT_BINARY_DIR}/${_ast_name}-linked.rxbin")
      set_tests_properties(${_ast_name}_${_vm} PROPERTIES
        PASS_REGULAR_EXPRESSION "ast_failures=0" TIMEOUT 300 LABELS "library;level-l;optimizer-parity")
      crexx_register_test_prep_targets(TESTS ${_ast_name}_${_vm} TARGETS ${_ast_name} ${_vm})
    endif()
  endforeach()
endforeach()

# Keep generated mappings under normal CI on every general parser backend.
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/ast-fixture.rxl")
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/ast-fixture.rxl" _ast_spec)
foreach(_backend ll1 lr1 packrat earley)
  set(_ast_dir "${CMAKE_CURRENT_BINARY_DIR}/ast-${_backend}")
  file(MAKE_DIRECTORY "${_ast_dir}")
  set(_ast_backend "backend ${_backend}")
  if(_backend STREQUAL "packrat")
    string(APPEND _ast_backend " semantics ordered")
  endif()
  string(REPLACE "backend lr1" "${_ast_backend}" _ast_text "${_ast_spec}")
  file(GENERATE OUTPUT "${_ast_dir}/grammar.rxl" CONTENT "${_ast_text}")
  add_custom_command(OUTPUT "${_ast_dir}/ast_fixture.rxbin"
    COMMAND ${CMAKE_COMMAND} -DLEVEL_L=$<TARGET_FILE:levell_cli>
      -DSPEC=${_ast_dir}/grammar.rxl -DOUTPUT=${_ast_dir}/ast_fixture.rxas -DFORMAT=rxas
      -P "${CMAKE_CURRENT_SOURCE_DIR}/../tools/generate.cmake"
    COMMAND $<TARGET_FILE:rxas> "${_ast_dir}/ast_fixture"
    DEPENDS levell_cli rxas "${_ast_dir}/grammar.rxl" "${CMAKE_CURRENT_SOURCE_DIR}/../tools/generate.cmake"
    VERBATIM)
  crexx_add_import_root(levell_ast_${_backend}_imports
    DIRECTORY "${_ast_dir}/grammar-imports" OUTPUT_VARIABLE _ast_grammar_imports
    FILES "${_ast_dir}/ast_fixture.rxbin" DEPENDS "${_ast_dir}/ast_fixture.rxbin")
  foreach(_mode noopt opt)
    set(_ast_flags)
    if(_mode STREQUAL "noopt")
      list(APPEND _ast_flags -n)
    endif()
    set(_ast_name "ast_${_backend}_${_mode}")
    add_custom_command(OUTPUT "${_ast_dir}/${_mode}-linked.rxbin"
      COMMAND $<TARGET_FILE:rxc> ${_ast_flags} -x --no-exe-import
        -i "${CMAKE_CURRENT_BINARY_DIR}/ast-imports;${_ast_dir}/grammar-imports"
        -o "${_mode}" "${CMAKE_CURRENT_SOURCE_DIR}/ast-probe.crexx"
      COMMAND $<TARGET_FILE:rxas> ${_ast_flags} "${_mode}"
      COMMAND $<TARGET_FILE:rxlink> -s -o "${_mode}-linked" "${_mode}" ast_fixture
        "${CMAKE_BINARY_DIR}/bin/rxfnsl" "${CMAKE_BINARY_DIR}/bin/library"
      DEPENDS rxc rxas rxlink levell_ast_test_imports ${_ast_imports} ast-probe.crexx
        levell_ast_${_backend}_imports ${_ast_grammar_imports}
        "${_ast_dir}/ast_fixture.rxbin"
      WORKING_DIRECTORY "${_ast_dir}" VERBATIM)
    add_custom_target(${_ast_name} ALL DEPENDS "${_ast_dir}/${_mode}-linked.rxbin")
    add_dependencies(testrxfnsl ${_ast_name})
    foreach(_vm rxtvm rxbvm)
      if(TARGET ${_vm})
        add_test(NAME ${_ast_name}_${_vm} COMMAND $<TARGET_FILE:${_vm}> "${_ast_dir}/${_mode}-linked.rxbin")
        set_tests_properties(${_ast_name}_${_vm} PROPERTIES
          PASS_REGULAR_EXPRESSION "AST_CHECK[|]OK" TIMEOUT 300 LABELS "library;level-l;optimizer-parity")
        crexx_register_test_prep_targets(TESTS ${_ast_name}_${_vm} TARGETS ${_ast_name} ${_vm})
      endif()
    endforeach()
  endforeach()
endforeach()

crexx_finalize_directory_qa_tiers()

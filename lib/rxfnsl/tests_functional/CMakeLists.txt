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
foreach(_test llast llanalysis llir llirservices)
  foreach(_mode noopt opt)
    set(_ast_flags)
    if(_mode STREQUAL "noopt")
      list(APPEND _ast_flags -n)
    endif()
    set(_ast_name "ts_${_test}_${_mode}")
    add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${_ast_name}-linked.rxbin"
      COMMAND $<TARGET_FILE:rxc> ${_ast_flags} -x --no-exe-import
        -i "${CMAKE_CURRENT_BINARY_DIR}/ast-imports"
        -o "${_ast_name}" "${CMAKE_CURRENT_SOURCE_DIR}/ts_${_test}.crexx"
      COMMAND $<TARGET_FILE:rxas> ${_ast_flags} "${_ast_name}"
      COMMAND $<TARGET_FILE:rxlink> -s -o "${_ast_name}-linked" "${_ast_name}"
        "${CMAKE_BINARY_DIR}/bin/rxfnsl" "${CMAKE_BINARY_DIR}/bin/library"
      DEPENDS rxc rxas rxlink rxfnsl levell_ast_test_imports ${_ast_imports} ts_${_test}.crexx
      WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}" VERBATIM)
    add_custom_target(${_ast_name} ALL DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/${_ast_name}-linked.rxbin")
    add_dependencies(testrxfnsl ${_ast_name})
    foreach(_vm rxtvm rxbvm)
      if(TARGET ${_vm})
        add_test(NAME ${_ast_name}_${_vm}
          COMMAND $<TARGET_FILE:${_vm}> "${CMAKE_CURRENT_BINARY_DIR}/${_ast_name}-linked.rxbin")
        set_tests_properties(${_ast_name}_${_vm} PROPERTIES
          PASS_REGULAR_EXPRESSION "failures=0" TIMEOUT 300 LABELS "library;level-l;optimizer-parity")
        crexx_register_test_prep_targets(TESTS ${_ast_name}_${_vm} TARGETS ${_ast_name} ${_vm})
      endif()
    endforeach()
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


# Compile the properties/flow lesson on each general backend. Tests execute
# prelinked consumers; no CTest launches a compiler or nested matrix.
set(_flow_spec "${CMAKE_CURRENT_SOURCE_DIR}/../examples/flow/rules.rxl")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${_flow_spec}")
file(READ "${_flow_spec}" _flow_text)
foreach(_backend ll1 lr1 packrat earley)
  set(_flow_dir "${CMAKE_CURRENT_BINARY_DIR}/flow-${_backend}")
  file(MAKE_DIRECTORY "${_flow_dir}")
  string(REPLACE "backend ll1" "backend ${_backend}" _flow_backend_text "${_flow_text}")
  if(_backend STREQUAL "packrat")
    string(REPLACE "semantics contextfree" "semantics ordered" _flow_backend_text "${_flow_backend_text}")
  endif()
  file(GENERATE OUTPUT "${_flow_dir}/rules.rxl" CONTENT "${_flow_backend_text}")
  set(_flow_provider "${_flow_dir}/flowdemo.rxas")
  add_custom_command(OUTPUT "${_flow_provider}"
    COMMAND ${CMAKE_COMMAND} -DLEVEL_L=$<TARGET_FILE:levell_cli>
      -DSPEC=${_flow_dir}/rules.rxl -DOUTPUT=${_flow_provider} -DFORMAT=rxas
      -P "${CMAKE_CURRENT_SOURCE_DIR}/../tools/generate.cmake"
    DEPENDS levell_cli "${_flow_dir}/rules.rxl" "${CMAKE_CURRENT_SOURCE_DIR}/../tools/generate.cmake"
    VERBATIM)
  add_custom_command(OUTPUT "${_flow_dir}/flowdemo.rxbin"
    COMMAND $<TARGET_FILE:rxas> flowdemo
    DEPENDS rxas "${_flow_provider}"
    WORKING_DIRECTORY "${_flow_dir}" VERBATIM)
  crexx_add_import_root(levell_flow_${_backend}_imports
    DIRECTORY "${_flow_dir}/imports" OUTPUT_VARIABLE _flow_imports
    FILES "${CMAKE_BINARY_DIR}/bin/library.rxbin" "${CMAKE_BINARY_DIR}/bin/rxfnsl.rxbin"
      "${_flow_dir}/flowdemo.rxbin"
    DEPENDS library rxfnsl)
  foreach(_mode noopt opt)
    set(_flow_flags)
    if(_mode STREQUAL "noopt")
      list(APPEND _flow_flags -n)
    endif()
    set(_flow_name "ts_llflow_${_backend}_${_mode}")
    add_custom_command(OUTPUT "${_flow_dir}/${_flow_name}-linked.rxbin"
      COMMAND $<TARGET_FILE:rxc> ${_flow_flags} -x --no-exe-import
        -i "${_flow_dir}/imports"
        -o "${_flow_name}" "${CMAKE_CURRENT_SOURCE_DIR}/ts_llflow.crexx"
      COMMAND $<TARGET_FILE:rxas> ${_flow_flags} "${_flow_name}"
      COMMAND $<TARGET_FILE:rxlink> -s -o "${_flow_name}-linked" "${_flow_name}" flowdemo
        "${CMAKE_BINARY_DIR}/bin/rxfnsl" "${CMAKE_BINARY_DIR}/bin/library"
      DEPENDS rxc rxas rxlink rxfnsl levell_flow_${_backend}_imports ${_flow_imports} ts_llflow.crexx
      WORKING_DIRECTORY "${_flow_dir}" VERBATIM)
    add_custom_target(${_flow_name} ALL DEPENDS "${_flow_dir}/${_flow_name}-linked.rxbin")
    add_dependencies(testrxfnsl ${_flow_name})
    foreach(_vm rxtvm rxbvm)
      if(TARGET ${_vm})
        add_test(NAME ${_flow_name}_${_vm}
          COMMAND $<TARGET_FILE:${_vm}> "${_flow_dir}/${_flow_name}-linked.rxbin")
        set_tests_properties(${_flow_name}_${_vm} PROPERTIES
          PASS_REGULAR_EXPRESSION "failures=0" TIMEOUT 300 LABELS "library;level-l;optimizer-parity")
        crexx_register_test_prep_targets(TESTS ${_flow_name}_${_vm} TARGETS ${_flow_name} ${_vm})
      endif()
    endforeach()
  endforeach()
endforeach()

# Structured-flow consumers also run directly, without nested builds.
set(_structured_flow_spec "${CMAKE_CURRENT_SOURCE_DIR}/../examples/flow/loops.rxl")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${_structured_flow_spec}")
file(READ "${_structured_flow_spec}" _structured_flow_text)
foreach(_backend ll1 lr1 packrat earley)
  set(_structured_flow_dir "${CMAKE_CURRENT_BINARY_DIR}/structured-flow-${_backend}")
  file(MAKE_DIRECTORY "${_structured_flow_dir}")
  string(REPLACE "backend ll1" "backend ${_backend}" _structured_flow_backend_text "${_structured_flow_text}")
  if(_backend STREQUAL "packrat")
    string(REPLACE "semantics contextfree" "semantics ordered" _structured_flow_backend_text "${_structured_flow_backend_text}")
  endif()
  file(GENERATE OUTPUT "${_structured_flow_dir}/rules.rxl" CONTENT "${_structured_flow_backend_text}")
  set(_structured_flow_provider "${_structured_flow_dir}/flowloops.rxas")
  add_custom_command(OUTPUT "${_structured_flow_provider}"
    COMMAND ${CMAKE_COMMAND} -DLEVEL_L=$<TARGET_FILE:levell_cli>
      -DSPEC=${_structured_flow_dir}/rules.rxl -DOUTPUT=${_structured_flow_provider} -DFORMAT=rxas
      -P "${CMAKE_CURRENT_SOURCE_DIR}/../tools/generate.cmake"
    DEPENDS levell_cli "${_structured_flow_dir}/rules.rxl" "${CMAKE_CURRENT_SOURCE_DIR}/../tools/generate.cmake"
    VERBATIM)
  add_custom_command(OUTPUT "${_structured_flow_dir}/flowloops.rxbin"
    COMMAND $<TARGET_FILE:rxas> flowloops
    DEPENDS rxas "${_structured_flow_provider}"
    WORKING_DIRECTORY "${_structured_flow_dir}" VERBATIM)
  crexx_add_import_root(levell_structured_flow_${_backend}_imports
    DIRECTORY "${_structured_flow_dir}/imports" OUTPUT_VARIABLE _structured_flow_imports
    FILES "${CMAKE_BINARY_DIR}/bin/library.rxbin" "${CMAKE_BINARY_DIR}/bin/rxfnsl.rxbin"
      "${_structured_flow_dir}/flowloops.rxbin"
    DEPENDS library rxfnsl)
  foreach(_flowcase structuredflow conditions)
  foreach(_mode noopt opt)
    set(_structured_flow_flags)
    if(_mode STREQUAL "noopt")
      list(APPEND _structured_flow_flags -n)
    endif()
    set(_structured_flow_name "ts_ll${_flowcase}_${_backend}_${_mode}")
    add_custom_command(OUTPUT "${_structured_flow_dir}/${_structured_flow_name}-linked.rxbin"
      COMMAND $<TARGET_FILE:rxc> ${_structured_flow_flags} -x --no-exe-import
        -i "${_structured_flow_dir}/imports"
        -o "${_structured_flow_name}" "${CMAKE_CURRENT_SOURCE_DIR}/ts_ll${_flowcase}.crexx"
      COMMAND $<TARGET_FILE:rxas> ${_structured_flow_flags} "${_structured_flow_name}"
      COMMAND $<TARGET_FILE:rxlink> -s -o "${_structured_flow_name}-linked" "${_structured_flow_name}" flowloops
        "${CMAKE_BINARY_DIR}/bin/rxfnsl" "${CMAKE_BINARY_DIR}/bin/library"
      DEPENDS rxc rxas rxlink rxfnsl levell_structured_flow_${_backend}_imports ${_structured_flow_imports} ts_ll${_flowcase}.crexx
      WORKING_DIRECTORY "${_structured_flow_dir}" VERBATIM)
    add_custom_target(${_structured_flow_name} ALL DEPENDS "${_structured_flow_dir}/${_structured_flow_name}-linked.rxbin")
    add_dependencies(testrxfnsl ${_structured_flow_name})
    foreach(_vm rxtvm rxbvm)
      if(TARGET ${_vm})
        add_test(NAME ${_structured_flow_name}_${_vm}
          COMMAND $<TARGET_FILE:${_vm}> "${_structured_flow_dir}/${_structured_flow_name}-linked.rxbin")
        set_tests_properties(${_structured_flow_name}_${_vm} PROPERTIES
          PASS_REGULAR_EXPRESSION "failures=0" TIMEOUT 300 LABELS "library;level-l;optimizer-parity")
        crexx_register_test_prep_targets(TESTS ${_structured_flow_name}_${_vm} TARGETS ${_structured_flow_name} ${_vm})
      endif()
    endforeach()
  endforeach()
  endforeach()
endforeach()

# Checked target-schema lowering, using the same module as the worked example.
set(_lowering_spec "${CMAKE_CURRENT_SOURCE_DIR}/../examples/lowering/unless.rxl")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${_lowering_spec}")
file(READ "${_lowering_spec}" _lowering_text)
foreach(_backend ll1 lr1 packrat earley)
  set(_lowering_dir "${CMAKE_CURRENT_BINARY_DIR}/lowering-${_backend}")
  file(MAKE_DIRECTORY "${_lowering_dir}")
  string(REPLACE "backend ll1" "backend ${_backend}" _lowering_backend_text "${_lowering_text}")
  if(_backend STREQUAL "packrat")
    string(REPLACE "semantics contextfree" "semantics ordered" _lowering_backend_text "${_lowering_backend_text}")
  endif()
  file(GENERATE OUTPUT "${_lowering_dir}/rules.rxl" CONTENT "${_lowering_backend_text}")
  set(_lowering_provider "${_lowering_dir}/unlesslang.rxas")
  add_custom_command(OUTPUT "${_lowering_provider}"
    COMMAND ${CMAKE_COMMAND} -DLEVEL_L=$<TARGET_FILE:levell_cli>
      -DSPEC=${_lowering_dir}/rules.rxl -DOUTPUT=${_lowering_provider} -DFORMAT=rxas
      -P "${CMAKE_CURRENT_SOURCE_DIR}/../tools/generate.cmake"
    DEPENDS levell_cli "${_lowering_dir}/rules.rxl" "${CMAKE_CURRENT_SOURCE_DIR}/../tools/generate.cmake"
    VERBATIM)
  add_custom_command(OUTPUT "${_lowering_dir}/unlesslang.rxbin"
    COMMAND $<TARGET_FILE:rxas> unlesslang
    DEPENDS rxas "${_lowering_provider}"
    WORKING_DIRECTORY "${_lowering_dir}" VERBATIM)
  add_custom_command(OUTPUT "${_lowering_dir}/lowerdemo.rxbin"
    COMMAND $<TARGET_FILE:rxc> -x --no-exe-import -i "${CMAKE_BINARY_DIR}/bin"
      -o lowerdemo "${CMAKE_CURRENT_SOURCE_DIR}/../examples/lowering/lowering.crexx"
    COMMAND $<TARGET_FILE:rxas> lowerdemo
    DEPENDS rxc rxas rxfnsl "${CMAKE_CURRENT_SOURCE_DIR}/../examples/lowering/lowering.crexx"
    WORKING_DIRECTORY "${_lowering_dir}" VERBATIM)
  crexx_add_import_root(levell_lowering_${_backend}_imports
    DIRECTORY "${_lowering_dir}/imports" OUTPUT_VARIABLE _lowering_imports
    FILES "${CMAKE_BINARY_DIR}/bin/library.rxbin" "${CMAKE_BINARY_DIR}/bin/rxfnsl.rxbin"
      "${_lowering_dir}/unlesslang.rxbin" "${_lowering_dir}/lowerdemo.rxbin"
    DEPENDS library rxfnsl)
  foreach(_flowcase lowering)
  foreach(_mode noopt opt)
    set(_lowering_flags)
    if(_mode STREQUAL "noopt")
      list(APPEND _lowering_flags -n)
    endif()
    set(_lowering_name "ts_ll${_flowcase}_${_backend}_${_mode}")
    add_custom_command(OUTPUT "${_lowering_dir}/${_lowering_name}-linked.rxbin"
      COMMAND $<TARGET_FILE:rxc> ${_lowering_flags} -x --no-exe-import
        -i "${_lowering_dir}/imports"
        -o "${_lowering_name}" "${CMAKE_CURRENT_SOURCE_DIR}/ts_ll${_flowcase}.crexx"
      COMMAND $<TARGET_FILE:rxas> ${_lowering_flags} "${_lowering_name}"
      COMMAND $<TARGET_FILE:rxlink> -s -o "${_lowering_name}-linked" "${_lowering_name}" unlesslang lowerdemo
        "${CMAKE_BINARY_DIR}/bin/rxfnsl" "${CMAKE_BINARY_DIR}/bin/library"
      DEPENDS rxc rxas rxlink rxfnsl levell_lowering_${_backend}_imports ${_lowering_imports} ts_ll${_flowcase}.crexx
      WORKING_DIRECTORY "${_lowering_dir}" VERBATIM)
    add_custom_target(${_lowering_name} ALL DEPENDS "${_lowering_dir}/${_lowering_name}-linked.rxbin")
    add_dependencies(testrxfnsl ${_lowering_name})
    foreach(_vm rxtvm rxbvm)
      if(TARGET ${_vm})
        add_test(NAME ${_lowering_name}_${_vm}
          COMMAND $<TARGET_FILE:${_vm}> "${_lowering_dir}/${_lowering_name}-linked.rxbin")
        set_tests_properties(${_lowering_name}_${_vm} PROPERTIES
          PASS_REGULAR_EXPRESSION "failures=0" TIMEOUT 300 LABELS "library;level-l;optimizer-parity")
        crexx_register_test_prep_targets(TESTS ${_lowering_name}_${_vm} TARGETS ${_lowering_name} ${_vm})
      endif()
    endforeach()
  endforeach()
  endforeach()
endforeach()

# Source origins and owned recovery inspection across parser backends.
set(_origins_spec "${CMAKE_CURRENT_SOURCE_DIR}/../examples/origins/records.rxl")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${_origins_spec}")
file(READ "${_origins_spec}" _origins_text)
foreach(_backend ll1 lr1 packrat earley regional)
  set(_origins_dir "${CMAKE_CURRENT_BINARY_DIR}/origins-${_backend}")
  file(MAKE_DIRECTORY "${_origins_dir}")
  string(REPLACE "backend ll1" "backend ${_backend}" _origins_backend_text "${_origins_text}")
  if(_backend STREQUAL "packrat")
    string(REPLACE "semantics contextfree" "semantics ordered" _origins_backend_text "${_origins_backend_text}")
  endif()
  if(_backend STREQUAL "regional")
    string(REPLACE "backend regional" "backend lr1" _origins_backend_text "${_origins_backend_text}")
    string(REPLACE "variable | literal" "variable | literal | regionexpr" _origins_backend_text "${_origins_backend_text}")
    string(REPLACE "scan: lexer input utf8" "regionexpr: token\nplus: token\nlparen: token\nrparen: token\nlbracket: token\nrbracket: token\nscan: lexer input utf8" _origins_backend_text "${_origins_backend_text}")
    string(REPLACE "  spaces: rule" "  plus.lex: rule = '+' then emit plus\n  lparen.lex: rule = '(' then emit lparen\n  rparen.lex: rule = ')' then emit rparen\n  lbracket.lex: rule = '[' then emit lbracket\n  rbracket.lex: rule = ']' then emit rbracket\n  spaces: rule" _origins_backend_text "${_origins_backend_text}")
    string(APPEND _origins_backend_text "\nmath: operators atom number group lparen rparen\nsum: infix math plus precedence 10 left\n  then tree Add(left = left, right = right)\nregion: region token regionexpr using math open lbracket close rbracket\n")
    # Every AST kind needs a flow role, including expressions inside the region.
    string(REPLACE "  Number: value" "  Number: value\n  Add: evaluate left right" _origins_backend_text "${_origins_backend_text}")
    # Operator atoms are token-level mappings in the region.
    string(REPLACE "number: token" "number: token then tree Number(spelling = text)" _origins_backend_text "${_origins_backend_text}")
  endif()
  file(GENERATE OUTPUT "${_origins_dir}/rules.rxl" CONTENT "${_origins_backend_text}")
  set(_origins_provider "${_origins_dir}/originlang.rxas")
  add_custom_command(OUTPUT "${_origins_provider}"
    COMMAND ${CMAKE_COMMAND} -DLEVEL_L=$<TARGET_FILE:levell_cli>
      -DSPEC=${_origins_dir}/rules.rxl -DOUTPUT=${_origins_provider} -DFORMAT=rxas
      -P "${CMAKE_CURRENT_SOURCE_DIR}/../tools/generate.cmake"
    DEPENDS levell_cli "${_origins_dir}/rules.rxl" "${CMAKE_CURRENT_SOURCE_DIR}/../tools/generate.cmake"
    VERBATIM)
  add_custom_command(OUTPUT "${_origins_dir}/originlang.rxbin"
    COMMAND $<TARGET_FILE:rxas> originlang
    DEPENDS rxas "${_origins_provider}"
    WORKING_DIRECTORY "${_origins_dir}" VERBATIM)
  crexx_add_import_root(levell_origins_${_backend}_imports
    DIRECTORY "${_origins_dir}/imports" OUTPUT_VARIABLE _origins_imports
    FILES "${CMAKE_BINARY_DIR}/bin/library.rxbin" "${CMAKE_BINARY_DIR}/bin/rxfnsl.rxbin"
      "${_origins_dir}/originlang.rxbin"
    DEPENDS library rxfnsl)
  foreach(_flowcase origins)
  foreach(_mode noopt opt)
    set(_origins_flags)
    if(_mode STREQUAL "noopt")
      list(APPEND _origins_flags -n)
    endif()
    set(_origins_name "ts_ll${_flowcase}_${_backend}_${_mode}")
    add_custom_command(OUTPUT "${_origins_dir}/${_origins_name}-linked.rxbin"
      COMMAND $<TARGET_FILE:rxc> ${_origins_flags} -x --no-exe-import
        -i "${_origins_dir}/imports"
        -o "${_origins_name}" "${CMAKE_CURRENT_SOURCE_DIR}/ts_ll${_flowcase}.crexx"
      COMMAND $<TARGET_FILE:rxas> ${_origins_flags} "${_origins_name}"
      COMMAND $<TARGET_FILE:rxlink> -s -o "${_origins_name}-linked" "${_origins_name}" originlang
        "${CMAKE_BINARY_DIR}/bin/rxfnsl" "${CMAKE_BINARY_DIR}/bin/library"
      DEPENDS rxc rxas rxlink rxfnsl levell_origins_${_backend}_imports ${_origins_imports} ts_ll${_flowcase}.crexx
      WORKING_DIRECTORY "${_origins_dir}" VERBATIM)
    add_custom_target(${_origins_name} ALL DEPENDS "${_origins_dir}/${_origins_name}-linked.rxbin")
    add_dependencies(testrxfnsl ${_origins_name})
    foreach(_vm rxtvm rxbvm)
      if(TARGET ${_vm})
        add_test(NAME ${_origins_name}_${_vm}
          COMMAND $<TARGET_FILE:${_vm}> "${_origins_dir}/${_origins_name}-linked.rxbin")
        set_tests_properties(${_origins_name}_${_vm} PROPERTIES
          PASS_REGULAR_EXPRESSION "failures=0" TIMEOUT 300 LABELS "library;level-l;optimizer-parity")
        crexx_register_test_prep_targets(TESTS ${_origins_name}_${_vm} TARGETS ${_origins_name} ${_vm})
      endif()
    endforeach()
  endforeach()
  endforeach()
endforeach()


crexx_finalize_directory_qa_tiers()

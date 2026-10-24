cmake_minimum_required(VERSION 3.24)
include(${CMAKE_SOURCE_DIR}/cmake/CrexxTestModes.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/CrexxSmokeTests.cmake)
add_custom_target(testlangtools ALL)
add_subdirectory(library)
add_dependencies(testlangtools testrxfnsl rxdas)
# A second consumer, packed-image validation, ownership and source diagnostics.
set(_generated_spec "${CMAKE_CURRENT_SOURCE_DIR}/example.levell")
set(_generated_grammar "${CMAKE_CURRENT_BINARY_DIR}/levell_example.rxas")
add_custom_command(OUTPUT "${_generated_grammar}"
  COMMAND ${CMAKE_COMMAND} -DLEVEL_L=$<TARGET_FILE:rxlang>
    -DSPEC=${_generated_spec} -DOUTPUT=${_generated_grammar} -DFORMAT=rxas
    -P "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/generate.cmake"
  DEPENDS rxlang "${_generated_spec}" "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/generate.cmake"
  VERBATIM)
# Share one producer between generated-module and callable-service preparation.
# Target ordering prevents independent parallel Make consumers racing this file.
add_custom_target(levell_generated_grammar DEPENDS "${_generated_grammar}")
add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/levell_example.rxbin"
  COMMAND $<TARGET_FILE:rxas> levell_example
  DEPENDS rxas "${_generated_grammar}"
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}" VERBATIM)
crexx_add_import_root(levell_generated_test_imports
  DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/imports" OUTPUT_VARIABLE _generated_imports
  FILES "${CMAKE_BINARY_DIR}/bin/library.rxbin" "${CMAKE_BINARY_DIR}/bin/rxfnsl.rxbin" "${CMAKE_BINARY_DIR}/bin/rxlanglib.rxbin"
    "${CMAKE_CURRENT_BINARY_DIR}/levell_example.rxbin"
  DEPENDS library rxfnsl rxlanglib levell_generated_grammar)
add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/testGenerated-linked.rxbin"
  COMMAND $<TARGET_FILE:rxc> -x --no-exe-import
    -i "${CMAKE_CURRENT_BINARY_DIR}/imports"
    -o testGenerated "${CMAKE_CURRENT_SOURCE_DIR}/testGenerated.crexx"
  COMMAND $<TARGET_FILE:rxas> testGenerated
  COMMAND $<TARGET_FILE:rxlink> -s -o testGenerated-linked testGenerated levell_example
    "${CMAKE_BINARY_DIR}/bin/rxfnsl" "${CMAKE_BINARY_DIR}/bin/rxlanglib" "${CMAKE_BINARY_DIR}/bin/library"
  DEPENDS rxc rxas rxlink rxfnsl levell_generated_test_imports ${_generated_imports} "${CMAKE_BINARY_DIR}/bin/rxfnsl.rxbin" "${CMAKE_BINARY_DIR}/bin/rxlanglib.rxbin"
    "${CMAKE_BINARY_DIR}/bin/library.rxbin" "${_generated_grammar}" testGenerated.crexx
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}" VERBATIM)
add_custom_target(testlevell_generated ALL DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/testGenerated-linked.rxbin")
add_dependencies(testlangtools testlevell_generated)
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

crexx_add_import_root(langtools_ast_test_imports
  DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/ast-imports" OUTPUT_VARIABLE _ast_imports
  FILES "${CMAKE_BINARY_DIR}/bin/library.rxbin" "${CMAKE_BINARY_DIR}/bin/rxfnsl.rxbin"
  DEPENDS library rxfnsl)
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
    COMMAND ${CMAKE_COMMAND} -DLEVEL_L=$<TARGET_FILE:rxlang>
      -DSPEC=${_ast_dir}/grammar.rxl -DOUTPUT=${_ast_dir}/ast_fixture.rxas -DFORMAT=rxas
      -P "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/generate.cmake"
    COMMAND $<TARGET_FILE:rxas> "${_ast_dir}/ast_fixture"
    DEPENDS rxlang rxas "${_ast_dir}/grammar.rxl" "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/generate.cmake"
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
      DEPENDS rxc rxas rxlink langtools_ast_test_imports ${_ast_imports} ast-probe.crexx
        levell_ast_${_backend}_imports ${_ast_grammar_imports}
        "${_ast_dir}/ast_fixture.rxbin"
      WORKING_DIRECTORY "${_ast_dir}" VERBATIM)
    add_custom_target(${_ast_name} ALL DEPENDS "${_ast_dir}/${_mode}-linked.rxbin")
    add_dependencies(testlangtools ${_ast_name})
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
set(_flow_runtime_workload semantic)
if(CREXX_QA_AUDIT_NATIVE_SOURCES)
  # Keep the original instrumented workload until its native-scope review.
  set(_flow_runtime_workload full)
endif()
set(_flow_spec "${CMAKE_SOURCE_DIR}/langtools/examples/flow/rules.rxl")
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
    COMMAND ${CMAKE_COMMAND} -DLEVEL_L=$<TARGET_FILE:rxlang>
      -DSPEC=${_flow_dir}/rules.rxl -DOUTPUT=${_flow_provider} -DFORMAT=rxas
      -P "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/generate.cmake"
    DEPENDS rxlang "${_flow_dir}/rules.rxl" "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/generate.cmake"
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
    add_dependencies(testlangtools ${_flow_name})
    foreach(_vm rxtvm rxbvm)
      if(TARGET ${_vm})
        add_test(NAME ${_flow_name}_${_vm}
          COMMAND $<TARGET_FILE:${_vm}> "${_flow_dir}/${_flow_name}-linked.rxbin" -a ${_flow_runtime_workload})
        set_tests_properties(${_flow_name}_${_vm} PROPERTIES
          PASS_REGULAR_EXPRESSION "failures=0" TIMEOUT 300 LABELS "library;level-l;optimizer-parity")
        if(_backend STREQUAL "earley" AND NOT CREXX_QA_AUDIT_NATIVE_SOURCES)
          # Reserve memory admission slots for the dense Earley index;
          # this is a CTest scheduling weight, not six execution threads.
          set_tests_properties(${_flow_name}_${_vm} PROPERTIES PROCESSORS 6)
        endif()
        crexx_register_test_prep_targets(TESTS ${_flow_name}_${_vm} TARGETS ${_flow_name} ${_vm})
      endif()
    endforeach()
  endforeach()
endforeach()

if(NOT CREXX_QA_AUDIT_NATIVE_SOURCES)
  add_test(NAME ts_llflow_resources
    COMMAND $<TARGET_FILE:${CREXX_DEFAULT_VM_TARGET}>
      "${CMAKE_CURRENT_BINARY_DIR}/flow-ll1/ts_llflow_ll1_opt-linked.rxbin" -a resources)
  set_tests_properties(ts_llflow_resources PROPERTIES
    PASS_REGULAR_EXPRESSION "FLOW_RESOURCES[|]width=200[|]depth=40[|]graph_sweeps=[1-9][0-9]*[|]remaining=0[|]failures=0"
    FAIL_REGULAR_EXPRESSION "FAIL:|PANIC:" TIMEOUT 300 LABELS "library;level-l;resource-limit")
  crexx_register_test_prep_targets(TESTS ts_llflow_resources
    TARGETS ts_llflow_ll1_opt ${CREXX_DEFAULT_VM_TARGET})
endif()

# Structured-flow consumers also run directly, without nested builds.
set(_structured_flow_spec "${CMAKE_SOURCE_DIR}/langtools/examples/flow/loops.rxl")
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
    COMMAND ${CMAKE_COMMAND} -DLEVEL_L=$<TARGET_FILE:rxlang>
      -DSPEC=${_structured_flow_dir}/rules.rxl -DOUTPUT=${_structured_flow_provider} -DFORMAT=rxas
      -P "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/generate.cmake"
    DEPENDS rxlang "${_structured_flow_dir}/rules.rxl" "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/generate.cmake"
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
    add_dependencies(testlangtools ${_structured_flow_name})
    foreach(_vm rxtvm rxbvm)
      if(TARGET ${_vm})
        set(_structured_flow_runtime_args)
        if(_flowcase STREQUAL "structuredflow")
          set(_structured_flow_runtime_args -a ${_flow_runtime_workload})
        endif()
        add_test(NAME ${_structured_flow_name}_${_vm}
          COMMAND $<TARGET_FILE:${_vm}> "${_structured_flow_dir}/${_structured_flow_name}-linked.rxbin" ${_structured_flow_runtime_args})
        set_tests_properties(${_structured_flow_name}_${_vm} PROPERTIES
          PASS_REGULAR_EXPRESSION "failures=0" TIMEOUT 300 LABELS "library;level-l;optimizer-parity")
        if(_backend STREQUAL "earley" AND NOT CREXX_QA_AUDIT_NATIVE_SOURCES)
          set_tests_properties(${_structured_flow_name}_${_vm} PROPERTIES PROCESSORS 6)
        endif()
        if(_flowcase STREQUAL "structuredflow")
          # SAN-QA-019 requires isolation under instrumentation. Ordinary
          # consumers have private VM state and read-only prepared inputs.
          set_tests_properties(${_structured_flow_name}_${_vm} PROPERTIES
            TIMEOUT 900)
          if(CREXX_QA_AUDIT_NATIVE_SOURCES)
            set_tests_properties(${_structured_flow_name}_${_vm} PROPERTIES RUN_SERIAL TRUE)
          endif()
        endif()
        crexx_register_test_prep_targets(TESTS ${_structured_flow_name}_${_vm} TARGETS ${_structured_flow_name} ${_vm})
      endif()
    endforeach()
  endforeach()
  endforeach()
endforeach()

if(NOT CREXX_QA_AUDIT_NATIVE_SOURCES)
  add_test(NAME ts_llstructuredflow_resources
    COMMAND $<TARGET_FILE:${CREXX_DEFAULT_VM_TARGET}>
      "${CMAKE_CURRENT_BINARY_DIR}/structured-flow-ll1/ts_llstructuredflow_ll1_opt-linked.rxbin" -a resources)
  set_tests_properties(ts_llstructuredflow_resources PROPERTIES
    PASS_REGULAR_EXPRESSION "STRUCTURED_RESOURCES[|]depth=30[|]cyclic_sweeps=[1-9][0-9]*[|]cyclic_remaining=0[|]port_sweeps=[1-9][0-9]*[|]port_remaining=0[|]failures=0"
    FAIL_REGULAR_EXPRESSION "FAIL:|PANIC:" TIMEOUT 900 LABELS "library;level-l;resource-limit")
  crexx_register_test_prep_targets(TESTS ts_llstructuredflow_resources
    TARGETS ts_llstructuredflow_ll1_opt ${CREXX_DEFAULT_VM_TARGET})
endif()

# Checked target-schema lowering, using the same module as the worked example.
# Its helper imports only the SDK, so one producer serves every parser backend.
set(_lowering_example_dir "${CMAKE_CURRENT_BINARY_DIR}/lowering-shared")
crexx_add_import_root(levell_lowering_example_imports
  DIRECTORY "${_lowering_example_dir}/imports" OUTPUT_VARIABLE _lowering_example_imports
  FILES "${CMAKE_BINARY_DIR}/bin/library.rxbin" "${CMAKE_BINARY_DIR}/bin/rxfnsl.rxbin"
  DEPENDS library rxfnsl)
crexx_add_fixture_command(OUTPUT "${_lowering_example_dir}/lowerdemo.rxbin"
  BYPRODUCTS "${_lowering_example_dir}/lowerdemo.rxas"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${_lowering_example_dir}"
  COMMAND $<TARGET_FILE:rxc> -x --no-exe-import -i "${_lowering_example_dir}/imports"
    -o "${_lowering_example_dir}/lowerdemo" "${CMAKE_SOURCE_DIR}/langtools/examples/lowering/lowering.crexx"
  COMMAND $<TARGET_FILE:rxas> "${_lowering_example_dir}/lowerdemo"
  DEPENDS rxc rxas levell_lowering_example_imports ${_lowering_example_imports}
    "${CMAKE_SOURCE_DIR}/langtools/examples/lowering/lowering.crexx"
  VERBATIM)
add_custom_target(levell_lowering_example DEPENDS "${_lowering_example_dir}/lowerdemo.rxbin")
set(_lowering_spec "${CMAKE_SOURCE_DIR}/langtools/examples/lowering/unless.rxl")
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
    COMMAND ${CMAKE_COMMAND} -DLEVEL_L=$<TARGET_FILE:rxlang>
      -DSPEC=${_lowering_dir}/rules.rxl -DOUTPUT=${_lowering_provider} -DFORMAT=rxas
      -P "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/generate.cmake"
    DEPENDS rxlang "${_lowering_dir}/rules.rxl" "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/generate.cmake"
    VERBATIM)
  add_custom_command(OUTPUT "${_lowering_dir}/unlesslang.rxbin"
    COMMAND $<TARGET_FILE:rxas> unlesslang
    DEPENDS rxas "${_lowering_provider}"
    WORKING_DIRECTORY "${_lowering_dir}" VERBATIM)
  crexx_add_import_root(levell_lowering_${_backend}_imports
    DIRECTORY "${_lowering_dir}/imports" OUTPUT_VARIABLE _lowering_imports
    FILES "${CMAKE_BINARY_DIR}/bin/library.rxbin" "${CMAKE_BINARY_DIR}/bin/rxfnsl.rxbin"
      "${_lowering_dir}/unlesslang.rxbin" "${_lowering_example_dir}/lowerdemo.rxbin"
    DEPENDS library rxfnsl levell_lowering_example)
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
      COMMAND $<TARGET_FILE:rxlink> -s -o "${_lowering_name}-linked" "${_lowering_name}" unlesslang
        "${_lowering_example_dir}/lowerdemo"
        "${CMAKE_BINARY_DIR}/bin/rxfnsl" "${CMAKE_BINARY_DIR}/bin/library"
      DEPENDS rxc rxas rxlink rxfnsl levell_lowering_${_backend}_imports ${_lowering_imports} ts_ll${_flowcase}.crexx
      WORKING_DIRECTORY "${_lowering_dir}" VERBATIM)
    add_custom_target(${_lowering_name} ALL DEPENDS "${_lowering_dir}/${_lowering_name}-linked.rxbin")
    add_dependencies(testlangtools ${_lowering_name})
    foreach(_vm rxtvm rxbvm)
      if(TARGET ${_vm})
        add_test(NAME ${_lowering_name}_${_vm}
          COMMAND $<TARGET_FILE:${_vm}> "${_lowering_dir}/${_lowering_name}-linked.rxbin" -a ${_flow_runtime_workload})
        set_tests_properties(${_lowering_name}_${_vm} PROPERTIES
          PASS_REGULAR_EXPRESSION "failures=0" TIMEOUT 300 LABELS "library;level-l;optimizer-parity")
        if(_backend STREQUAL "earley" AND NOT CREXX_QA_AUDIT_NATIVE_SOURCES)
          set_tests_properties(${_lowering_name}_${_vm} PROPERTIES PROCESSORS 6)
        endif()
        crexx_register_test_prep_targets(TESTS ${_lowering_name}_${_vm} TARGETS ${_lowering_name} ${_vm})
      endif()
    endforeach()
  endforeach()
  endforeach()
endforeach()

# The large raw-tree control never uses a generated parser. Existing AST
# depth/width controls retain both modes and engines; reuse this one image.
if(NOT CREXX_QA_AUDIT_NATIVE_SOURCES)
  add_test(NAME ts_lllowering_deep
    COMMAND $<TARGET_FILE:${CREXX_DEFAULT_VM_TARGET}>
      "${CMAKE_CURRENT_BINARY_DIR}/lowering-lr1/ts_lllowering_lr1_opt-linked.rxbin" -a deep)
  set_tests_properties(ts_lllowering_deep PROPERTIES
    PASS_REGULAR_EXPRESSION "deep nodes=2001 failures=0" FAIL_REGULAR_EXPRESSION "FAIL:|PANIC:"
    TIMEOUT 300 LABELS "library;level-l;resource-limit")
  crexx_register_test_prep_targets(TESTS ts_lllowering_deep
    TARGETS ts_lllowering_lr1_opt ${CREXX_DEFAULT_VM_TARGET})
endif()

# Source origins and owned recovery inspection across parser backends.
set(_origins_spec "${CMAKE_SOURCE_DIR}/langtools/examples/origins/records.rxl")
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
    COMMAND ${CMAKE_COMMAND} -DLEVEL_L=$<TARGET_FILE:rxlang>
      -DSPEC=${_origins_dir}/rules.rxl -DOUTPUT=${_origins_provider} -DFORMAT=rxas
      -P "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/generate.cmake"
    DEPENDS rxlang "${_origins_dir}/rules.rxl" "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/generate.cmake"
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
    add_dependencies(testlangtools ${_origins_name})
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

# Exercise the public Earley chart allowance with the existing recovery-capable
# image. No generator, nested compilation or additional backend/mode matrix.
add_test(NAME ts_llearley_budget
  COMMAND $<TARGET_FILE:${CREXX_DEFAULT_VM_TARGET}>
    "${CMAKE_CURRENT_BINARY_DIR}/origins-earley/ts_llorigins_earley_opt-linked.rxbin" -a earley-budget)
set_tests_properties(ts_llearley_budget PROPERTIES
  PASS_REGULAR_EXPRESSION "EARLEY_BUDGET[|]minimum=[1-9][0-9]*[|]strict_gap=1[|]search_steps=[1-9][0-9]*[|]recovery_positive=0[|]recovery_short=2[|]failures=0"
  FAIL_REGULAR_EXPRESSION "FAIL:|PANIC:" TIMEOUT 300 LABELS "library;level-l;resource-limit")
crexx_register_test_prep_targets(TESTS ts_llearley_budget
  TARGETS ts_llorigins_earley_opt ${CREXX_DEFAULT_VM_TARGET})

crexx_finalize_directory_qa_tiers()

# Public callable tooling, independent of both research compilers.
set(_services "${CMAKE_CURRENT_BINARY_DIR}/services")
# Independent metadata-reader linkage regression, not executable application code.
foreach(_part definition import)
  add_custom_command(OUTPUT "${_services}/metadata-${_part}.rxbin"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${_services}"
    COMMAND $<TARGET_FILE:rxas> -n -o "${_services}/metadata-${_part}"
      "${CMAKE_CURRENT_SOURCE_DIR}/metadata-${_part}.rxas"
    DEPENDS rxas "${CMAKE_CURRENT_SOURCE_DIR}/metadata-${_part}.rxas" VERBATIM)
endforeach()
add_custom_command(OUTPUT "${_services}/metadata-linked.rxbin"
  COMMAND $<TARGET_FILE:rxlink> -s -o "${_services}/metadata-linked"
    "${_services}/metadata-definition.rxbin" "${_services}/metadata-import.rxbin"
  DEPENDS rxlink "${_services}/metadata-definition.rxbin" "${_services}/metadata-import.rxbin" VERBATIM)
add_custom_target(levell_metadata_fixtures DEPENDS "${_services}/metadata-linked.rxbin")
set(_attribute_files "${CMAKE_BINARY_DIR}/bin/library.rxbin"
  "${CMAKE_BINARY_DIR}/langtools/common/llbinmeta.rxbin")
crexx_add_import_root(levell_metadata_attribute_imports
  DIRECTORY "${_services}/attribute-imports" OUTPUT_VARIABLE _attribute_imports
  FILES ${_attribute_files} DEPENDS library llbinmeta)
foreach(_attribute_mode opt noopt)
  set(_attribute_flags)
  if(_attribute_mode STREQUAL "noopt")
    set(_attribute_flags -n)
  endif()
  set(_attribute_output "${_services}/attributes-${_attribute_mode}")
  add_custom_command(OUTPUT "${_attribute_output}-linked.rxbin"
    COMMAND $<TARGET_FILE:rxc> ${_attribute_flags} -x --no-exe-import
      -i "${_services}/attribute-imports" -o "${_attribute_output}"
      "${CMAKE_CURRENT_SOURCE_DIR}/metadata-attributes.crexx"
    COMMAND $<TARGET_FILE:rxas> ${_attribute_flags} "${_attribute_output}"
    COMMAND $<TARGET_FILE:rxlink> -s -o "${_attribute_output}-linked"
      "${_attribute_output}" ${_attribute_files}
    DEPENDS rxc rxas rxlink levell_metadata_attribute_imports ${_attribute_imports}
      "${CMAKE_CURRENT_SOURCE_DIR}/metadata-attributes.crexx" VERBATIM)
  add_custom_target(levell_metadata_attributes_${_attribute_mode}
    DEPENDS "${_attribute_output}-linked.rxbin"
      "${_services}/metadata-definition.rxbin")
  set(_attribute_vm rxbvm)
  if(_attribute_mode STREQUAL "noopt" AND TARGET rxtvm)
    set(_attribute_vm rxtvm)
  endif()
  add_test(NAME levell_metadata_attributes_${_attribute_mode}
    COMMAND $<TARGET_FILE:${_attribute_vm}> "${_attribute_output}-linked.rxbin"
      -a "${_services}/metadata-definition.rxbin")
  set_tests_properties(levell_metadata_attributes_${_attribute_mode} PROPERTIES
    PASS_REGULAR_EXPRESSION "META_ATTRIBUTES[|]checks=[0-9]+[|]failures=0"
    TIMEOUT 60 LABELS "level-l;optimizer-parity")
  crexx_register_test_prep_targets(TESTS levell_metadata_attributes_${_attribute_mode}
    TARGETS levell_metadata_attributes_${_attribute_mode} ${_attribute_vm})
endforeach()
foreach(_classify_part metadata-classify metadata-classify-import)
  add_custom_command(OUTPUT "${_services}/${_classify_part}.rxbin"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${_services}"
    COMMAND $<TARGET_FILE:rxas> -n -o "${_services}/${_classify_part}"
      "${CMAKE_CURRENT_SOURCE_DIR}/${_classify_part}.rxas"
    DEPENDS rxas "${CMAKE_CURRENT_SOURCE_DIR}/${_classify_part}.rxas" VERBATIM)
endforeach()
add_custom_command(OUTPUT "${_services}/metadata-classify-linked.rxbin"
  COMMAND $<TARGET_FILE:rxlink> -s -o "${_services}/metadata-classify-linked"
    "${_services}/metadata-classify.rxbin" "${_services}/metadata-classify-import.rxbin"
  DEPENDS rxlink "${_services}/metadata-classify.rxbin"
    "${_services}/metadata-classify-import.rxbin" VERBATIM)
crexx_add_import_root(levell_metadata_classify_imports
  DIRECTORY "${_services}/classify-imports" OUTPUT_VARIABLE _classify_imports
  FILES ${_attribute_files} DEPENDS library llbinmeta)
foreach(_classify_mode opt noopt)
  set(_classify_flags)
  if(_classify_mode STREQUAL "noopt")
    set(_classify_flags -n)
  endif()
  set(_classify_output "${_services}/classify-${_classify_mode}")
  add_custom_command(OUTPUT "${_classify_output}-linked.rxbin"
    COMMAND $<TARGET_FILE:rxc> ${_classify_flags} -x --no-exe-import
      -i "${_services}/classify-imports" -o "${_classify_output}"
      "${CMAKE_CURRENT_SOURCE_DIR}/metadata-classify.crexx"
    COMMAND $<TARGET_FILE:rxas> ${_classify_flags} "${_classify_output}"
    COMMAND $<TARGET_FILE:rxlink> -s -o "${_classify_output}-linked"
      "${_classify_output}" ${_attribute_files}
    DEPENDS rxc rxas rxlink levell_metadata_classify_imports ${_classify_imports}
      "${CMAKE_CURRENT_SOURCE_DIR}/metadata-classify.crexx" VERBATIM)
  add_custom_target(levell_metadata_classify_${_classify_mode}
    DEPENDS "${_classify_output}-linked.rxbin"
      "${_services}/metadata-classify.rxbin" "${_services}/metadata-classify-linked.rxbin")
  set(_classify_vm rxbvm)
  if(_classify_mode STREQUAL "noopt" AND TARGET rxtvm)
    set(_classify_vm rxtvm)
  endif()
  add_test(NAME levell_metadata_classify_${_classify_mode}
    COMMAND $<TARGET_FILE:${_classify_vm}> "${_classify_output}-linked.rxbin"
      -a "${_services}/metadata-classify.rxbin" "${_services}/metadata-classify-linked.rxbin")
  set_tests_properties(levell_metadata_classify_${_classify_mode} PROPERTIES
    PASS_REGULAR_EXPRESSION "META_CLASSIFY[|]checks=[0-9]+[|]failures=0"
    RUN_SERIAL TRUE TIMEOUT 300 LABELS "level-l;optimizer-parity")
  crexx_register_test_prep_targets(TESTS levell_metadata_classify_${_classify_mode}
    TARGETS levell_metadata_classify_${_classify_mode} ${_classify_vm})
endforeach()
set(_service_files "${CMAKE_BINARY_DIR}/bin/library.rxbin" "${CMAKE_BINARY_DIR}/bin/rxfnsl.rxbin"
  "${CMAKE_BINARY_DIR}/bin/rxlanglib.rxbin" "${CMAKE_BINARY_DIR}/langtools/common/llrxastext.rxbin"
  "${CMAKE_BINARY_DIR}/langtools/common/llbinmeta.rxbin" "${CMAKE_BINARY_DIR}/langtools/common/llfileoutput.rxbin")
crexx_add_import_root(levell_service_test_imports DIRECTORY "${_services}/imports"
  OUTPUT_VARIABLE _service_imports FILES ${_service_files} "${CMAKE_BINARY_DIR}/bin/rxfs.rxplugin"
  DEPENDS library rxfnsl rxlanglib llrxastext llbinmeta llfileoutput fs)
foreach(_mode opt noopt)
  set(_flags)
  if(_mode STREQUAL "noopt")
    set(_flags -n)
  endif()
  add_custom_command(OUTPUT "${_services}/${_mode}-linked.rxbin"
    COMMAND $<TARGET_FILE:rxc> ${_flags} -x --no-exe-import -i "${_services}/imports"
      -o "${_services}/${_mode}" "${CMAKE_CURRENT_SOURCE_DIR}/services.crexx"
    COMMAND $<TARGET_FILE:rxas> ${_flags} "${_services}/${_mode}"
    COMMAND $<TARGET_FILE:rxlink> -s -o "${_services}/${_mode}-linked" "${_services}/${_mode}" ${_service_files}
    DEPENDS rxc rxas rxlink levell_service_test_imports ${_service_imports} services.crexx VERBATIM)
  add_custom_target(levell_services_${_mode} ALL DEPENDS "${_services}/${_mode}-linked.rxbin" "${_services}/metadata-linked.rxbin")
  add_dependencies(levell_services_${_mode} levell_generated_grammar levell_metadata_fixtures)
  add_dependencies(testlangtools levell_services_${_mode})
  foreach(_vm rxtvm rxbvm)
    if(TARGET ${_vm})
      add_test(NAME levell_services_${_mode}_${_vm} COMMAND $<TARGET_FILE:${_vm}>
        "${_services}/${_mode}-linked.rxbin" -a "${_generated_spec}" "${CMAKE_BINARY_DIR}/bin/rxfnsl.rxbin" "${_services}/${_mode}-${_vm}.bin" "${_generated_grammar}" "${_services}/metadata-linked.rxbin" "${_services}/metadata-import.rxbin")
      set_tests_properties(levell_services_${_mode}_${_vm} PROPERTIES
        PASS_REGULAR_EXPRESSION "TOOL_SERVICES[|]failures=0"
        TIMEOUT 900 LABELS "level-l;optimizer-parity")
      if(CREXX_QA_AUDIT_NATIVE_SOURCES)
        # Preserve instrumented generation's established contention policy.
        set_tests_properties(levell_services_${_mode}_${_vm} PROPERTIES RUN_SERIAL TRUE)
      endif()
      crexx_register_test_prep_targets(TESTS levell_services_${_mode}_${_vm} TARGETS levell_services_${_mode} ${_vm})
      crexx_label_smoke_tests(levell_services_${_mode}_${_vm})
    endif()
  endforeach()
endforeach()
crexx_schedule_directory_qa_tier_finalization()

find_package(Python3 REQUIRED COMPONENTS Interpreter)
add_test(NAME rxlang_tinyexpr_binary_surface_smoke
  COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/check-tinyexpr.py"
    $<TARGET_FILE:rxdas> "${CMAKE_BINARY_DIR}/langtools/examples/tinyexpr.rxbin")
set_tests_properties(rxlang_tinyexpr_binary_surface_smoke PROPERTIES
  PASS_REGULAR_EXPRESSION "TINYEXPR_BINARY_SURFACE[|]PASS" TIMEOUT 120 LABELS "level-l;smoke")
crexx_register_test_prep_targets(TESTS rxlang_tinyexpr_binary_surface_smoke
  TARGETS rxlang_example_tinyexpr rxdas)
set(_layout_arguments)
if(CREXX_BUILD_RESEARCH_COMPILERS)
  list(APPEND _layout_arguments --research-compilers)
endif()
add_test(NAME levell_repository_layout COMMAND ${Python3_EXECUTABLE}
  "${CMAKE_CURRENT_SOURCE_DIR}/check-layout.py" ${_layout_arguments})
set_tests_properties(levell_repository_layout PROPERTIES PASS_REGULAR_EXPRESSION "LEVEL_L_LAYOUT[|]PASS" TIMEOUT 30 LABELS "level-l;essential")

add_test(NAME levell_product_inventory COMMAND ${CMAKE_COMMAND}
  -DBUILD=${CMAKE_BINARY_DIR} -DSUFFIX=${CMAKE_EXECUTABLE_SUFFIX}
  -P "${CMAKE_CURRENT_SOURCE_DIR}/check-products.cmake")
set_tests_properties(levell_product_inventory PROPERTIES PASS_REGULAR_EXPRESSION "LEVEL_L_PRODUCTS[|]PASS" TIMEOUT 30 LABELS "level-l;essential")
crexx_register_test_prep_targets(TESTS levell_product_inventory
  TARGETS rxfnsl rxfnsl_analysis rxfnsl_rxas rxfnsl_execute
    rxlang rxlanglib llrxastext llfileoutput llbinmeta)

# Inspect real prepared package/import manifests; this check never rebuilds.
add_test(NAME levell_package_closures COMMAND ${Python3_EXECUTABLE}
  "${CMAKE_CURRENT_SOURCE_DIR}/check-package-closures.py"
  --build "${CMAKE_BINARY_DIR}" --config "$<CONFIG>")
set_tests_properties(levell_package_closures PROPERTIES
  PASS_REGULAR_EXPRESSION "LEVEL_L_PACKAGE_CLOSURES[|]PASS" TIMEOUT 60
  LABELS "level-l;essential")
crexx_register_test_prep_targets(TESTS levell_package_closures
  TARGETS rxfnsl rxfnsl_analysis rxfnsl_rxas rxfnsl_execute
    rxlang llrxastext llfileoutput llbinmeta)

if(TARGET rxbvm)
  # One guide-derived producer and its independent static target, on one VM.
  add_test(NAME levell_public_packages_opt_rxbvm COMMAND ${Python3_EXECUTABLE}
    "${CMAKE_CURRENT_SOURCE_DIR}/check-public-packages.py" --build "${CMAKE_BINARY_DIR}")
  set_tests_properties(levell_public_packages_opt_rxbvm PROPERTIES
    PASS_REGULAR_EXPRESSION "LEVEL_L_PUBLIC_PACKAGES[|]PASS" TIMEOUT 300 RUN_SERIAL TRUE
    LABELS "level-l;component")
  crexx_register_test_prep_targets(TESTS levell_public_packages_opt_rxbvm
    TARGETS library rxfnsl rxfnsl_rxas rxc rxas rxlink rxbvm)
endif()

# Independent lexical reuse: one ordinary prepared service and one intentionally
# unprepared manual service. Each image has one producer and a private import root.
set(_reuse "${CMAKE_CURRENT_BINARY_DIR}/lexical-reuse")
file(MAKE_DIRECTORY "${_reuse}/unstamped")
set(_reuse_base "${CMAKE_BINARY_DIR}/bin/library.rxbin" "${CMAKE_BINARY_DIR}/bin/rxfnsl.rxbin")
set(_reuse_bootstrap "${CMAKE_BINARY_DIR}/langtools/bootstrap/L2")
crexx_add_import_root(levell_lexical_base_imports DIRECTORY "${_reuse}/base"
  OUTPUT_VARIABLE _reuse_base_imports FILES ${_reuse_base}
    "${_reuse_bootstrap}/llfront.rxbin" "${_reuse_bootstrap}/llsyntax.rxbin"
  DEPENDS library rxfnsl rxlang_bootstrap)
add_custom_command(OUTPUT "${_reuse}/llgenerate.rxbin" BYPRODUCTS "${_reuse}/llgenerate.rxas"
  COMMAND $<TARGET_FILE:rxc> -x --no-exe-import -i "${_reuse}/base"
    -o "${_reuse}/llgenerate" "${CMAKE_BINARY_DIR}/langtools/bootstrap/source/llgenerate.crexx"
  COMMAND $<TARGET_FILE:rxas> "${_reuse}/llgenerate"
  DEPENDS rxc rxas levell_generator_source levell_lexical_base_imports ${_reuse_base_imports}
    "${CMAKE_BINARY_DIR}/langtools/bootstrap/source/llgenerate.crexx" VERBATIM)
add_custom_target(levell_lexical_service DEPENDS "${_reuse}/llgenerate.rxbin")
crexx_add_import_root(levell_lexical_imports DIRECTORY "${_reuse}/imports"
  OUTPUT_VARIABLE _reuse_imports FILES ${_reuse_base} "${_reuse}/llgenerate.rxbin"
    "${_reuse_bootstrap}/llfront.rxbin" "${_reuse_bootstrap}/llsyntax.rxbin"
  DEPENDS library rxfnsl rxlang_bootstrap levell_lexical_service)
add_custom_command(OUTPUT "${_reuse}/service-linked.rxbin"
  COMMAND $<TARGET_FILE:rxc> -x --no-exe-import -i "${_reuse}/imports"
    -o "${_reuse}/service" "${CMAKE_CURRENT_SOURCE_DIR}/lexical-reuse.crexx"
  COMMAND $<TARGET_FILE:rxas> "${_reuse}/service"
  COMMAND $<TARGET_FILE:rxlink> -s -o "${_reuse}/service-linked" "${_reuse}/service"
    "${_reuse}/llgenerate" "${_reuse_bootstrap}/llfront" "${_reuse_bootstrap}/llsyntax" ${_reuse_base}
  DEPENDS rxc rxas rxlink levell_lexical_imports ${_reuse_imports} lexical-reuse.crexx VERBATIM)
add_custom_command(OUTPUT "${_reuse}/unstamped/llgenerate.rxbin"
  COMMAND $<TARGET_FILE:rxc> -x --no-exe-import -i "${_reuse}/base"
    -o "${_reuse}/unstamped/llgenerate" "${CMAKE_CURRENT_SOURCE_DIR}/../rexx/llgenerate.crexx"
  COMMAND $<TARGET_FILE:rxas> "${_reuse}/unstamped/llgenerate"
  DEPENDS rxc rxas levell_lexical_base_imports ${_reuse_base_imports}
    "${CMAKE_CURRENT_SOURCE_DIR}/../rexx/llgenerate.crexx" VERBATIM)
crexx_add_import_root(levell_unstamped_imports DIRECTORY "${_reuse}/unstamped/imports"
  OUTPUT_VARIABLE _unstamped_imports FILES ${_reuse_base}
    "${_reuse_bootstrap}/llfront.rxbin" "${_reuse_bootstrap}/llsyntax.rxbin"
    "${_reuse}/unstamped/llgenerate.rxbin"
  DEPENDS library rxfnsl rxlang_bootstrap "${_reuse}/unstamped/llgenerate.rxbin")
add_custom_command(OUTPUT "${_reuse}/unstamped/linked.rxbin"
  COMMAND $<TARGET_FILE:rxc> -x --no-exe-import -i "${_reuse}/unstamped/imports"
    -o "${_reuse}/unstamped/probe" "${CMAKE_CURRENT_SOURCE_DIR}/lexical-reuse.crexx"
  COMMAND $<TARGET_FILE:rxas> "${_reuse}/unstamped/probe"
  COMMAND $<TARGET_FILE:rxlink> -s -o "${_reuse}/unstamped/linked" "${_reuse}/unstamped/probe"
    "${_reuse}/unstamped/llgenerate" "${_reuse_bootstrap}/llfront" "${_reuse_bootstrap}/llsyntax" ${_reuse_base}
  DEPENDS rxc rxas rxlink levell_unstamped_imports ${_unstamped_imports} lexical-reuse.crexx VERBATIM)
add_custom_target(levell_lexical_reuse ALL DEPENDS "${_reuse}/service-linked.rxbin" "${_reuse}/unstamped/linked.rxbin")
add_dependencies(testlangtools levell_lexical_reuse)
add_test(NAME levell_lexical_reuse_service COMMAND $<TARGET_FILE:rxbvm>
  --no-autoload "${_reuse}/service-linked.rxbin" -a service)
add_test(NAME levell_lexical_reuse_unstamped COMMAND $<TARGET_FILE:rxbvm>
  --no-autoload "${_reuse}/unstamped/linked.rxbin" -a unstamped)
# Service probes use private requests and read-only inputs. Reserve ordinary
# memory admission slots; retain the established instrumented isolation.
set_tests_properties(levell_lexical_reuse_service levell_lexical_reuse_unstamped PROPERTIES
  PASS_REGULAR_EXPRESSION "LEXICAL_REUSE[|].*[|]failures=0" TIMEOUT 900 LABELS "level-l;essential")
if(CREXX_QA_AUDIT_NATIVE_SOURCES)
  set_tests_properties(levell_lexical_reuse_service levell_lexical_reuse_unstamped PROPERTIES RUN_SERIAL TRUE)
else()
  set_tests_properties(levell_lexical_reuse_service levell_lexical_reuse_unstamped PROPERTIES PROCESSORS 6)
endif()
crexx_register_test_prep_targets(TESTS levell_lexical_reuse_service levell_lexical_reuse_unstamped
  TARGETS levell_lexical_reuse rxbvm)
# Keep the nested producer contract on the owning generator and build driver.
set(_lexical_cli_build_args --generator "${CMAKE_GENERATOR}")
if(NOT "${CMAKE_MAKE_PROGRAM}" STREQUAL "")
  list(APPEND _lexical_cli_build_args --make-program "${CMAKE_MAKE_PROGRAM}")
endif()
add_test(NAME rxlang_lexical_reuse_cli COMMAND ${Python3_EXECUTABLE}
  "${CMAKE_CURRENT_SOURCE_DIR}/check-lexical-reuse.py" --bin "${CMAKE_BINARY_DIR}/bin"
  --cmake "${CMAKE_COMMAND}" --service-rxas "${_reuse}/llgenerate.rxas" ${_lexical_cli_build_args})
set_tests_properties(rxlang_lexical_reuse_cli PROPERTIES
  PASS_REGULAR_EXPRESSION "LEXICAL_REUSE_CLI[|]commands=.*[|]failures=0" RUN_SERIAL TRUE TIMEOUT 1800 LABELS "level-l;essential")
if(NOT CREXX_QA_AUDIT_NATIVE_SOURCES)
  # Cache/publication controls own a private temporary root.
  set_tests_properties(rxlang_lexical_reuse_cli PROPERTIES RUN_SERIAL FALSE PROCESSORS 6)
endif()
crexx_register_test_prep_targets(TESTS rxlang_lexical_reuse_cli TARGETS rxlang levell_lexical_service)

# Bounded persistent-VM measurement targets; register after isolated Debug timing.
include("${CMAKE_SOURCE_DIR}/interpreter/tests/sustained_vm.cmake")
# Isolated ordinary Debug fixes the bounded workloads; deadline is hang protection.
crexx_register_sustained_vm_tests(1000 1000 10 1800)

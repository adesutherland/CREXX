cmake_minimum_required(VERSION 3.24)

project(test_rxfnsl)

enable_testing()

include(${CMAKE_SOURCE_DIR}/cmake/CrexxTestModes.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/CrexxSmokeTests.cmake)

set(REXXBINS
  ts_tinyexpr
)

set(LIBRPATH ${CMAKE_BINARY_DIR}/bin/library)
set(TINYEXPR_RPATH ${CMAKE_BINARY_DIR}/langtools/examples/tinyexpr)

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
    IMPORT_PATHS "${CMAKE_BINARY_DIR}/bin" "${CMAKE_BINARY_DIR}/langtools/examples"
    RXC_FLAGS -x
    DEPENDS library rxlang_example_tinyexpr "${TINYEXPR_RPATH}.rxbin" "${_rexx_file}"
    RUNTIME_ARGS ${LIBRPATH} ${TINYEXPR_RPATH}
  )
endforeach()

set_tests_properties(ts_tinyexpr_noopt ts_tinyexpr_opt
  PROPERTIES PASS_REGULAR_EXPRESSION "PASS: TinyExpr generated-shape demo")

crexx_label_smoke_tests(
        ts_tinyexpr_noopt
)

# Compile and link AST fixtures during the build. CTests launch only one VM;
# they do not rebuild tools or run a nested qualification matrix.
crexx_add_import_root(levell_ast_test_imports
  DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/ast-imports" OUTPUT_VARIABLE _ast_imports
  FILES "${CMAKE_BINARY_DIR}/bin/library.rxbin" "${CMAKE_BINARY_DIR}/bin/rxfnsl.rxbin"
  DEPENDS library rxfnsl)
foreach(_test llast llanalysis llir llirservices llirflow llsource lloptimise)
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


# Read the already-built SDK RXAS; diagnostic noopt SDK builds intentionally
# disable the inline resize shape checked here.
if(NOT CREXX_LEVEL_L_NOOPT)
  find_package(Python3 REQUIRED COMPONENTS Interpreter)
  add_test(NAME levell_ir_flow_rxas COMMAND ${Python3_EXECUTABLE}
    "${CMAKE_CURRENT_SOURCE_DIR}/../check-ir-flow-rxas.py"
    "${CMAKE_BINARY_DIR}/lib/rxfnsl/rexx/llirservices.rxas")
  set_tests_properties(levell_ir_flow_rxas PROPERTIES
    PASS_REGULAR_EXPRESSION "IR_FLOW_RXAS[|]PASS" TIMEOUT 30
    LABELS "library;level-l;optimizer-parity")
  crexx_register_test_prep_targets(TESTS levell_ir_flow_rxas TARGETS rxfnsl)
endif()

crexx_finalize_directory_qa_tiers()

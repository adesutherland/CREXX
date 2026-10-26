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
foreach(_test llast llanalysis llir llirservices llirflow llsource lloptimise llborrowed)
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


# Prepared-definition format/ownership proof uses one small normal SDK lane.
add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/ts_llprepared_opt-linked.rxbin"
  COMMAND $<TARGET_FILE:rxc> -x --no-exe-import
    -i "${CMAKE_CURRENT_BINARY_DIR}/ast-imports"
    -o ts_llprepared_opt "${CMAKE_CURRENT_SOURCE_DIR}/ts_llprepared.crexx"
  COMMAND $<TARGET_FILE:rxas> ts_llprepared_opt
  COMMAND $<TARGET_FILE:rxlink> -s -o ts_llprepared_opt-linked ts_llprepared_opt
    "${CMAKE_BINARY_DIR}/bin/rxfnsl" "${CMAKE_BINARY_DIR}/bin/library"
  DEPENDS rxc rxas rxlink rxfnsl levell_ast_test_imports ${_ast_imports} ts_llprepared.crexx
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}" VERBATIM)
add_custom_target(ts_llprepared_opt
  DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/ts_llprepared_opt-linked.rxbin")
add_dependencies(testrxfnsl ts_llprepared_opt)
if(TARGET rxbvm)
  add_test(NAME ts_llprepared_opt_rxbvm
    COMMAND $<TARGET_FILE:rxbvm> "${CMAKE_CURRENT_BINARY_DIR}/ts_llprepared_opt-linked.rxbin")
  set_tests_properties(ts_llprepared_opt_rxbvm PROPERTIES
    PASS_REGULAR_EXPRESSION "PREPARED_DEFINITION[|]checks=[0-9]+[|]failures=0"
    TIMEOUT 300 LABELS "library;level-l")
  crexx_register_test_prep_targets(TESTS ts_llprepared_opt_rxbvm TARGETS ts_llprepared_opt rxbvm)
endif()


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

# Inspect the built readers through their nested metadata/expected-token path.
find_package(Python3 REQUIRED COMPONENTS Interpreter)
add_test(NAME levell_borrowed_rxas COMMAND ${Python3_EXECUTABLE}
  "${CMAKE_CURRENT_SOURCE_DIR}/../check-borrowed-rxas.py"
  "${CMAKE_BINARY_DIR}/lib/rxfnsl/rexx")
set_tests_properties(levell_borrowed_rxas PROPERTIES
  PASS_REGULAR_EXPRESSION "BORROWED_RXAS[|]PASS" TIMEOUT 30
  LABELS "library;level-l;optimizer-parity")
# RXAS names are UTF-8 even when Python's default file encoding is not.
set_property(TEST levell_borrowed_rxas APPEND PROPERTY ENVIRONMENT
  "PYTHONUTF8=0" "PYTHONCOERCECLOCALE=0" "LC_ALL=C")
crexx_register_test_prep_targets(TESTS levell_borrowed_rxas TARGETS rxfnsl)

# Optional IR analyses, execution and RXAS constraints use one focused normal lane. These
# packages are deliberately absent from the generic AST/bootstrap import root.
foreach(_test llirgraph llrxas llirexecute llirexecute_dynamic)
  set(_name "ts_${_test}_opt")
  set(_images "${CMAKE_BINARY_DIR}/bin/library.rxbin")
  set(_producers library)
  if(_test STREQUAL "llirgraph" OR _test MATCHES "^llirexecute")
    list(APPEND _images "${CMAKE_BINARY_DIR}/bin/rxfnsl.rxbin"
      "${CMAKE_BINARY_DIR}/bin/rxfnsl-analysis.rxbin")
    list(APPEND _producers rxfnsl rxfnsl_analysis)
    if(_test MATCHES "^llirexecute")
      list(APPEND _images "${CMAKE_BINARY_DIR}/bin/rxfnsl-execute.rxbin")
      list(APPEND _producers rxfnsl_execute)
    endif()
  else()
    list(APPEND _images "${CMAKE_BINARY_DIR}/bin/rxfnsl-rxas.rxbin")
    list(APPEND _producers rxfnsl_rxas)
  endif()
  crexx_add_import_root(${_name}_imports
    DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${_name}-imports"
    OUTPUT_VARIABLE _infrastructure_imports FILES ${_images} DEPENDS ${_producers})
  add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${_name}-linked.rxbin"
    COMMAND $<TARGET_FILE:rxc> -x --no-exe-import
      -i "${CMAKE_CURRENT_BINARY_DIR}/${_name}-imports"
      -o "${_name}" "${CMAKE_CURRENT_SOURCE_DIR}/ts_${_test}.crexx"
    COMMAND $<TARGET_FILE:rxas> "${_name}"
    COMMAND $<TARGET_FILE:rxlink> -s -o "${_name}-linked" "${_name}"
      ${_images}
    DEPENDS rxc rxas rxlink ${_name}_imports ${_infrastructure_imports}
      "${CMAKE_CURRENT_SOURCE_DIR}/ts_${_test}.crexx"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}" VERBATIM)
  add_custom_target(${_name} DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/${_name}-linked.rxbin")
  if(TARGET rxbvm)
    add_test(NAME ${_name}_rxbvm
      COMMAND $<TARGET_FILE:rxbvm> "${CMAKE_CURRENT_BINARY_DIR}/${_name}-linked.rxbin")
    set_tests_properties(${_name}_rxbvm PROPERTIES
      PASS_REGULAR_EXPRESSION "failures=0" TIMEOUT 300 LABELS "library;level-l")
    crexx_register_test_prep_targets(TESTS ${_name}_rxbvm TARGETS ${_name} rxbvm)
  endif()
endforeach()

crexx_finalize_directory_qa_tiers()

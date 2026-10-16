cmake_minimum_required(VERSION 3.24)
# Finalize after this directory has declared every test, including frontends.
# Preparation targets are collected by the parent after subdirectories return.
crexx_schedule_directory_qa_tier_finalization()

# Compiler semantics are a separate consumer of the generic Level L library.
crexx_add_import_root(llcompiler_ir_imports
  DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/imports" OUTPUT_VARIABLE _compiler_ir_imports
  FILES "${CMAKE_BINARY_DIR}/bin/library.rxbin" "${CMAKE_BINARY_DIR}/bin/rxfnsl.rxbin"
  DEPENDS library rxfnsl)
foreach(_mode opt noopt)
  set(_flags)
  if(_mode STREQUAL "noopt")
    list(APPEND _flags -n)
  endif()
  set(_dir "${CMAKE_CURRENT_BINARY_DIR}/${_mode}")
  file(MAKE_DIRECTORY "${_dir}")
  add_custom_command(OUTPUT "${_dir}/llcrexxir.rxbin"
    COMMAND $<TARGET_FILE:rxc> ${_flags} -x --no-exe-import
      -i "${CMAKE_CURRENT_BINARY_DIR}/imports" -o llcrexxir "${CMAKE_CURRENT_SOURCE_DIR}/common/llcrexxir.crexx"
    COMMAND $<TARGET_FILE:rxas> ${_flags} llcrexxir
    DEPENDS rxc rxas llcompiler_ir_imports ${_compiler_ir_imports} common/llcrexxir.crexx
    WORKING_DIRECTORY "${_dir}" VERBATIM)
  foreach(_example demo contracts)
    if(_example STREQUAL "demo")
      set(_source "${CMAKE_CURRENT_SOURCE_DIR}/../examples/ir/demo.crexx")
      set(_success "IR_DEMO[|]OK")
    else()
      set(_source "${CMAKE_CURRENT_SOURCE_DIR}/tests/ir-contracts.crexx")
      set(_success "failures=0")
    endif()
    set(_name "llcompiler_ir_${_example}_${_mode}")
    add_custom_command(OUTPUT "${_dir}/${_example}-linked.rxbin"
      COMMAND $<TARGET_FILE:rxc> ${_flags} -x --no-exe-import
        -i "${CMAKE_CURRENT_BINARY_DIR}/imports;${_dir}" -o ${_example} "${_source}"
      COMMAND $<TARGET_FILE:rxas> ${_flags} ${_example}
      COMMAND $<TARGET_FILE:rxlink> -s -o ${_example}-linked ${_example} llcrexxir
        "${CMAKE_BINARY_DIR}/bin/rxfnsl" "${CMAKE_BINARY_DIR}/bin/library"
      DEPENDS rxc rxas rxlink llcompiler_ir_imports ${_compiler_ir_imports}
        "${_dir}/llcrexxir.rxbin" "${_source}"
      WORKING_DIRECTORY "${_dir}" VERBATIM)
    add_custom_target(${_name} ALL DEPENDS "${_dir}/${_example}-linked.rxbin")
    foreach(_vm rxtvm rxbvm)
      if(TARGET ${_vm})
        add_test(NAME ${_name}_${_vm} COMMAND $<TARGET_FILE:${_vm}> "${_dir}/${_example}-linked.rxbin")
        set_tests_properties(${_name}_${_vm} PROPERTIES PASS_REGULAR_EXPRESSION "${_success}"
          TIMEOUT 300 LABELS "library;level-l;optimizer-parity")
        crexx_register_test_prep_targets(TESTS ${_name}_${_vm} TARGETS ${_name} ${_vm})
      endif()
    endforeach()
  endforeach()
endforeach()

# Backend targets follow the frontend declarations below.

# STEP03 grammars are generated once; compiler hosts import only B and Level L.
set(_front "${CMAKE_CURRENT_BINARY_DIR}/frontends")
file(MAKE_DIRECTORY "${_front}")
foreach(_language c g)
  set(_grammar "${_front}/ll${_language}grammar")
  add_custom_command(OUTPUT "${_grammar}.rxas"
    COMMAND ${CMAKE_COMMAND} -DLEVEL_L=$<TARGET_FILE:levell_cli>
      -DSPEC=${CMAKE_CURRENT_SOURCE_DIR}/level${_language}/level${_language}.rxl
      -DOUTPUT=${_grammar}.rxas -DFORMAT=rxas
      -P "${CMAKE_CURRENT_SOURCE_DIR}/../tools/generate.cmake"
    DEPENDS levell_cli level${_language}/level${_language}.rxl ../tools/generate.cmake VERBATIM)
  add_custom_command(OUTPUT "${_grammar}.rxbin"
    COMMAND $<TARGET_FILE:rxas> "${_grammar}"
    DEPENDS rxas "${_grammar}.rxas" VERBATIM)
  list(APPEND _front_grammars "${_grammar}.rxbin")
endforeach()

foreach(_mode opt noopt)
  set(_flags)
  if(_mode STREQUAL "noopt")
    list(APPEND _flags -n)
  endif()
  set(_dir "${_front}/${_mode}")
  file(MAKE_DIRECTORY "${_dir}")
  set(_previous ${_front_grammars})
  foreach(_module llcompilercontracts llfrontendparse llfrontend)
    if(_module STREQUAL "llcompilercontracts")
      set(_source "${CMAKE_CURRENT_SOURCE_DIR}/common/${_module}.crexx")
    else()
      set(_source "${CMAKE_CURRENT_SOURCE_DIR}/driver/${_module}.crexx")
    endif()
    add_custom_command(OUTPUT "${_dir}/${_module}.rxbin"
      COMMAND $<TARGET_FILE:rxc> ${_flags} -x --no-exe-import
        -i "${CMAKE_CURRENT_BINARY_DIR}/imports;${_dir};${_front};${CMAKE_CURRENT_BINARY_DIR}/${_mode}"
        -o ${_module} "${_source}"
      COMMAND $<TARGET_FILE:rxas> ${_flags} ${_module}
      DEPENDS rxc rxas llcompiler_ir_imports ${_compiler_ir_imports} ${_previous}
        "${CMAKE_CURRENT_BINARY_DIR}/${_mode}/llcrexxir.rxbin" "${_source}"
      WORKING_DIRECTORY "${_dir}" VERBATIM)
    list(APPEND _previous "${_dir}/${_module}.rxbin")
  endforeach()
  foreach(_consumer frontend-tests frontend-probe parse-probe contracts-probe)
    add_custom_command(OUTPUT "${_dir}/${_consumer}-linked.rxbin"
      COMMAND $<TARGET_FILE:rxc> ${_flags} -x --no-exe-import
        -i "${CMAKE_CURRENT_BINARY_DIR}/imports;${_dir}"
        -o ${_consumer} "${CMAKE_CURRENT_SOURCE_DIR}/tests/${_consumer}.crexx"
      COMMAND $<TARGET_FILE:rxas> ${_flags} ${_consumer}
      COMMAND $<TARGET_FILE:rxlink> -s -o ${_consumer}-linked ${_consumer}
        llfrontend llcompilercontracts llfrontendparse "${_front}/llcgrammar" "${_front}/llggrammar"
        "${CMAKE_CURRENT_BINARY_DIR}/${_mode}/llcrexxir"
        "${CMAKE_BINARY_DIR}/bin/rxfnsl" "${CMAKE_BINARY_DIR}/bin/library"
      DEPENDS rxc rxas rxlink ${_previous} "${CMAKE_CURRENT_SOURCE_DIR}/tests/${_consumer}.crexx"
      WORKING_DIRECTORY "${_dir}" VERBATIM)
    add_custom_target(llcompiler_${_consumer}_${_mode} ALL DEPENDS "${_dir}/${_consumer}-linked.rxbin")
  endforeach()
  add_dependencies(llcompiler_frontend-tests_${_mode} rxfnsc)
  foreach(_group syntax grammar mapping calls contracts lifecycle)
    foreach(_vm rxtvm rxbvm)
      if(TARGET ${_vm})
        set(_test "llcompiler_frontend_${_group}_${_mode}_${_vm}")
        add_test(NAME ${_test} COMMAND $<TARGET_FILE:${_vm}> "${_dir}/frontend-tests-linked.rxbin"
          -a ${_group} "${CMAKE_BINARY_DIR}/bin" "${CMAKE_CURRENT_SOURCE_DIR}/examples/step01")
        set_tests_properties(${_test} PROPERTIES PASS_REGULAR_EXPRESSION "failures=0"
          TIMEOUT 300 LABELS "library;level-l;compiler-frontends;optimizer-parity")
        crexx_register_test_prep_targets(TESTS ${_test} TARGETS llcompiler_frontend-tests_${_mode} ${_vm})
      endif()
    endforeach()
  endforeach()
endforeach()

include("${CMAKE_CURRENT_SOURCE_DIR}/backend.cmake")

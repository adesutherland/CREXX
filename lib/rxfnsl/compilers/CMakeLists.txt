cmake_minimum_required(VERSION 3.24)
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

# RexxScript 1 remains explicitly selected while its language/API is reviewed.
set(_rs1_root "${CMAKE_CURRENT_BINARY_DIR}/modern")
file(MAKE_DIRECTORY "${_rs1_root}")
set(_rs1_grammar "${_rs1_root}/rexxscriptgrammar.rxas")
set(_rs1_level_l "${CMAKE_BINARY_DIR}/lib/rxfnsl/tools/bootstrap/L2/levell-linked.rxbin")
# Construction is pure cREXX bytecode. Use the supported threaded VM for this
# build tool when available; the installed runner selection is unchanged.
set(_rs1_generation_vm "${CREXX_RXVM_EXECUTABLE}")
set(_rs1_generation_target rxvm)
if(TARGET rxtvm)
  set(_rs1_generation_vm "$<TARGET_FILE:rxtvm>")
  set(_rs1_generation_target rxtvm)
endif()
add_custom_command(OUTPUT "${_rs1_grammar}"
  COMMAND ${CMAKE_COMMAND} -DLEVEL_L=$<TARGET_FILE:levell_cli>
    -DLEVEL_L_VM=${_rs1_generation_vm}
    -DLEVEL_L_BYTECODE=${_rs1_level_l}
    -DSPEC=${CMAKE_CURRENT_SOURCE_DIR}/grammar/rexxscript.levell
    -DOUTPUT=${_rs1_grammar} -DFORMAT=rxas -P ${CMAKE_SOURCE_DIR}/lib/rxfnsl/tools/generate.cmake
  DEPENDS levell_cli ${_rs1_generation_target} "${_rs1_level_l}"
    grammar/rexxscript.levell ${CMAKE_SOURCE_DIR}/lib/rxfnsl/tools/generate.cmake
  COMMENT "Level L: admit RexxScript grammar and generate native DFA/LR tables"
  VERBATIM)
crexx_add_import_root(rexxscript1_imports DIRECTORY "${_rs1_root}/imports"
  OUTPUT_VARIABLE _rs1_imports
  FILES "${CMAKE_BINARY_DIR}/bin/library.rxbin" "${CMAKE_BINARY_DIR}/bin/classlib.rxbin"
    "${CMAKE_BINARY_DIR}/bin/rxfnsc.rxbin" "${CMAKE_BINARY_DIR}/bin/rxfnsl.rxbin"
  DEPENDS library classlib rxfnsc rxfnsl)
set(_rs1_modules rexxscriptgrammar RexxScriptBifs RexxScriptOperators RexxScriptProgram RexxScriptExecution)
set(_rs1_deps_RexxScriptProgram rexxscriptgrammar)
set(_rs1_deps_RexxScriptExecution RexxScriptProgram RexxScriptBifs RexxScriptOperators)
set(_rs1_bins)
foreach(_module IN LISTS _rs1_modules)
  set(_source "${CMAKE_CURRENT_SOURCE_DIR}/${_module}.crexx")
  if(_module STREQUAL "rexxscriptgrammar")
    add_custom_command(OUTPUT "${_rs1_root}/${_module}.rxbin"
      COMMAND $<TARGET_FILE:rxas> "${_module}"
      DEPENDS rxas "${_rs1_grammar}"
      WORKING_DIRECTORY "${_rs1_root}" VERBATIM)
    list(APPEND _rs1_bins "${_rs1_root}/${_module}.rxbin")
    continue()
  endif()
  set(_deps)
  foreach(_dep IN LISTS _rs1_deps_${_module})
    list(APPEND _deps "${_rs1_root}/${_dep}.rxbin")
  endforeach()
  # Import only current direct dependencies, never linked drivers/tests left
  # in the output directory: their embedded prototypes can shadow API changes.
  set(_module_imports "${_rs1_root}/imports")
  set(_module_import_files)
  set(_module_import_target)
  if(_deps)
    set(_module_import_target "rexxscript1_module_imports_${_module}")
    crexx_add_import_root(${_module_import_target}
      DIRECTORY "${_rs1_root}/module-imports/${_module}"
      OUTPUT_VARIABLE _module_import_files FILES ${_deps} DEPENDS ${_deps})
    string(APPEND _module_imports ";${_rs1_root}/module-imports/${_module}")
  endif()
  add_custom_command(OUTPUT "${_rs1_root}/${_module}.rxbin"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${_rs1_root}/source/${_module}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${_source}" "${_rs1_root}/source/${_module}/${_module}.crexx"
    COMMAND $<TARGET_FILE:rxc> -x --no-exe-import -i "${_module_imports}"
      -o "${_module}" "${_rs1_root}/source/${_module}/${_module}.crexx"
    COMMAND $<TARGET_FILE:rxas> "${_module}"
    DEPENDS rxc rxas rexxscript1_imports ${_rs1_imports} "${_source}" ${_deps} ${_module_import_target} ${_module_import_files}
    WORKING_DIRECTORY "${_rs1_root}" VERBATIM)
  list(APPEND _rs1_bins "${_rs1_root}/${_module}.rxbin")
endforeach()
set(_rs1_control "${_rs1_root}/library.ctl")
set(_rs1_text "* Generated RexxScript 1 library\n")
foreach(_bin IN LISTS _rs1_bins)
  string(APPEND _rs1_text "INPUT ${_bin}\n")
endforeach()
foreach(_module IN LISTS _rs1_modules)
  string(APPEND _rs1_text "INCLUDE ${_module}\n")
endforeach()
string(APPEND _rs1_text "PRESERVE INLINE\nOUTPUT ${CMAKE_BINARY_DIR}/bin/rexxscript1\n")
file(GENERATE OUTPUT "${_rs1_control}" CONTENT "${_rs1_text}")
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/bin/rexxscript1.rxbin"
  COMMAND $<TARGET_FILE:rxlink> -c "${_rs1_control}"
  DEPENDS rxlink ${_rs1_bins} "${_rs1_control}" VERBATIM)
add_custom_target(rexxscript1 ALL DEPENDS "${CMAKE_BINARY_DIR}/bin/rexxscript1.rxbin")
set(_rs1_runtime_bins)
foreach(_library rexxscript1 rxfnsl rxfnsc rxfnsg classlib library)
  list(APPEND _rs1_runtime_bins "${CMAKE_BINARY_DIR}/bin/${_library}.rxbin")
endforeach()
crexx_add_import_root(rexxscript1_driver_imports
  DIRECTORY "${_rs1_root}/driver-imports" OUTPUT_VARIABLE _rs1_driver_imports
  FILES "${CMAKE_BINARY_DIR}/bin/rexxscript1.rxbin" DEPENDS rexxscript1)
add_custom_command(OUTPUT "${_rs1_root}/RexxScript1Runner.c"
  BYPRODUCTS "${_rs1_root}/RexxScript1Runner.rxbin" "${_rs1_root}/RexxScript1Runner-linked.rxbin"
  COMMAND $<TARGET_FILE:rxc> -x --no-exe-import -i "${_rs1_root}/imports;${_rs1_root}/driver-imports"
    -o RexxScript1Runner "${CMAKE_CURRENT_SOURCE_DIR}/RexxScript1Runner.crexx"
  COMMAND $<TARGET_FILE:rxas> RexxScript1Runner
  COMMAND $<TARGET_FILE:rxlink> -s -o RexxScript1Runner-linked RexxScript1Runner
    "${CMAKE_BINARY_DIR}/bin/rexxscript1" "${CMAKE_BINARY_DIR}/bin/rxfnsl"
    "${CMAKE_BINARY_DIR}/bin/rxfnsc" "${CMAKE_BINARY_DIR}/bin/rxfnsg" "${CMAKE_BINARY_DIR}/bin/classlib" "${CMAKE_BINARY_DIR}/bin/library"
  COMMAND $<TARGET_FILE:rxcpack> -o RexxScript1Runner RexxScript1Runner-linked
  DEPENDS rxc rxas rxlink rxcpack rexxscript1_driver_imports ${_rs1_driver_imports} rxfnsg rexxscript1 ${_rs1_runtime_bins} RexxScript1Runner.crexx
  WORKING_DIRECTORY "${_rs1_root}" VERBATIM)
add_executable(rexxscript1_cli "${_rs1_root}/RexxScript1Runner.c")
set_target_properties(rexxscript1_cli PROPERTIES OUTPUT_NAME rexxscript1)
target_link_libraries(rexxscript1_cli rxvml rxpashim)
include(${CMAKE_SOURCE_DIR}/rxpa/RXPluginFunction.cmake)
configure_linker_for_static_lib(rexxscript1_cli cstream)
configure_linker_for_static_lib(rexxscript1_cli fs)
add_dependencies(rexxscript1_cli rxvme rxvml rxpa avl_tree platform)

# Prebuilt functional program: CTest launches one VM, never a nested build.
add_custom_command(OUTPUT "${_rs1_root}/testRexxScript1-linked.rxbin"
  COMMAND $<TARGET_FILE:rxc> -x --no-exe-import -i "${_rs1_root}/imports;${_rs1_root}/driver-imports"
    -o testRexxScript1 "${CMAKE_CURRENT_SOURCE_DIR}/tests_functional/testRexxScript1.crexx"
  COMMAND $<TARGET_FILE:rxas> testRexxScript1
  COMMAND $<TARGET_FILE:rxlink> -s -o testRexxScript1-linked testRexxScript1
    "${CMAKE_BINARY_DIR}/bin/rexxscript1" "${CMAKE_BINARY_DIR}/bin/rxfnsl"
    "${CMAKE_BINARY_DIR}/bin/rxfnsc" "${CMAKE_BINARY_DIR}/bin/rxfnsg" "${CMAKE_BINARY_DIR}/bin/classlib" "${CMAKE_BINARY_DIR}/bin/library"
  DEPENDS rxc rxas rxlink rexxscript1_driver_imports ${_rs1_driver_imports} rxfnsg rexxscript1 ${_rs1_runtime_bins} tests_functional/testRexxScript1.crexx
  WORKING_DIRECTORY "${_rs1_root}" VERBATIM)
add_custom_target(testrexxscript1 ALL DEPENDS "${_rs1_root}/testRexxScript1-linked.rxbin")
foreach(_vm rxtvm rxbvm)
  if(TARGET ${_vm})
    add_test(NAME rexxscript1_${_vm}
      COMMAND $<TARGET_FILE:${_vm}> "${_rs1_root}/testRexxScript1-linked.rxbin")
    set_tests_properties(rexxscript1_${_vm} PROPERTIES
      PASS_REGULAR_EXPRESSION "rexxscript1_failures=0" TIMEOUT 600
      RESOURCE_LOCK rexxscript1_functional LABELS "rexxscript;level-l")
  endif()
endforeach()

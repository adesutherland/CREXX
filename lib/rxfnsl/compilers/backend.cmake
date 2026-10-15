# The compiler host is B/L only. Classic runtime metadata and its target adapter
# are separate build products; they are never linked into the source compiler.
crexx_add_import_root(llcompiler_cli_imports
  DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/cli-imports" OUTPUT_VARIABLE _cli_imports
  FILES "${CMAKE_BINARY_DIR}/bin/library.rxbin" "${CMAKE_BINARY_DIR}/bin/rxfnsl.rxbin"
    "${CMAKE_BINARY_DIR}/bin/rxfs.rxplugin"
  DEPENDS library rxfnsl fs)
crexx_add_import_root(llcompiler_target_imports
  DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/target-imports" OUTPUT_VARIABLE _target_imports
  FILES "${CMAKE_BINARY_DIR}/bin/library.rxbin" "${CMAKE_BINARY_DIR}/bin/rxfnsc.rxbin"
  DEPENDS library rxfnsc)
foreach(_mode opt noopt)
  set(_flags)
  if(_mode STREQUAL "noopt")
    list(APPEND _flags -n)
  endif()
  set(_dir "${CMAKE_CURRENT_BINARY_DIR}/backend/${_mode}")
  file(MAKE_DIRECTORY "${_dir}")
  set(_frontdir "${CMAKE_CURRENT_BINARY_DIR}/frontends/${_mode}")
  set(_host "${_frontdir}/llfrontend.rxbin" "${_frontdir}/llfrontendparse.rxbin"
    "${_frontdir}/llcompilercontracts.rxbin" "${_front}/llcgrammar.rxbin" "${_front}/llggrammar.rxbin"
    "${CMAKE_CURRENT_BINARY_DIR}/${_mode}/llcrexxir.rxbin")
  foreach(_module llrxas llcompiler llcompilercli)
    set(_source "${CMAKE_CURRENT_SOURCE_DIR}/driver/${_module}.crexx")
    if(_module STREQUAL "llrxas")
      set(_source "${CMAKE_CURRENT_SOURCE_DIR}/rxas/llrxas.crexx")
    endif()
    crexx_add_import_root(llcompiler_${_module}_imports_${_mode}
      DIRECTORY "${_dir}/imports-${_module}" OUTPUT_VARIABLE _module_imports
      FILES ${_cli_imports} ${_host} DEPENDS llcompiler_cli_imports ${_host})
    add_custom_command(OUTPUT "${_dir}/${_module}.rxbin"
      COMMAND $<TARGET_FILE:rxc> ${_flags} -x --no-exe-import
        -i "${_dir}/imports-${_module}"
        -o ${_module} "${_source}"
      COMMAND $<TARGET_FILE:rxas> ${_flags} ${_module}
      DEPENDS rxc rxas llcompiler_${_module}_imports_${_mode} ${_module_imports} ${_host} "${_source}"
      WORKING_DIRECTORY "${_dir}" VERBATIM)
    list(APPEND _host "${_dir}/${_module}.rxbin")
  endforeach()
  add_custom_command(OUTPUT "${_dir}/llctarget.rxbin"
    COMMAND $<TARGET_FILE:rxc> ${_flags} -x --no-exe-import
      -i "${CMAKE_CURRENT_BINARY_DIR}/target-imports" -o llctarget "${CMAKE_CURRENT_SOURCE_DIR}/rxas/llctarget.crexx"
    COMMAND $<TARGET_FILE:rxas> ${_flags} llctarget
    DEPENDS rxc rxas llcompiler_target_imports ${_target_imports} rxas/llctarget.crexx
    WORKING_DIRECTORY "${_dir}" VERBATIM)
  add_custom_target(llcompiler_target_${_mode} ALL DEPENDS "${_dir}/llctarget.rxbin")
  crexx_add_import_root(llcompiler_consumers_imports_${_mode}
    DIRECTORY "${_dir}/imports-consumers" OUTPUT_VARIABLE _consumer_imports
    FILES ${_cli_imports} ${_host} DEPENDS llcompiler_cli_imports ${_host})
  if(_mode STREQUAL "opt")
    # Install the reusable service without the command's native file I/O provider.
    set(_service_files ${_host})
    list(REMOVE_ITEM _service_files "${_dir}/llcompilercli.rxbin")
    set(_service_control "")
    foreach(_file IN LISTS _service_files)
      get_filename_component(_name "${_file}" NAME_WE)
      string(APPEND _service_control "INPUT ${_file}\nINCLUDE ${_name}\n")
    endforeach()
    string(APPEND _service_control "PRESERVE INLINE\nOUTPUT ${CMAKE_BINARY_DIR}/bin/llcompilers.rxbin\n")
    file(GENERATE OUTPUT "${_dir}/llcompilers.ctl" CONTENT "${_service_control}")
    add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/bin/llcompilers.rxbin"
      COMMAND $<TARGET_FILE:rxlink> -c "${_dir}/llcompilers.ctl"
      DEPENDS rxlink ${_service_files} "${_dir}/llcompilers.ctl" VERBATIM)
    add_custom_target(llcompilers ALL DEPENDS "${CMAKE_BINARY_DIR}/bin/llcompilers.rxbin")
    install(FILES "${CMAKE_BINARY_DIR}/bin/llcompilers.rxbin" DESTINATION bin)
  endif()
  foreach(_consumer backend-probe backend-tests backend-optimise backend-poc llrxc-c llrxc-g)
    set(_source "${CMAKE_CURRENT_SOURCE_DIR}/driver/${_consumer}.crexx")
    if(_consumer MATCHES "^backend-")
      set(_source "${CMAKE_CURRENT_SOURCE_DIR}/tests/${_consumer}.crexx")
    endif()
    add_custom_command(OUTPUT "${_dir}/${_consumer}-linked.rxbin"
      COMMAND $<TARGET_FILE:rxc> ${_flags} -x --no-exe-import
        -i "${_dir}/imports-consumers" -o ${_consumer} "${_source}"
      COMMAND $<TARGET_FILE:rxas> ${_flags} ${_consumer}
      COMMAND $<TARGET_FILE:rxlink> -s -o ${_consumer}-linked ${_consumer} ${_host}
        "${CMAKE_BINARY_DIR}/bin/rxfnsl" "${CMAKE_BINARY_DIR}/bin/library"
      DEPENDS rxc rxas rxlink llcompiler_consumers_imports_${_mode} ${_consumer_imports} ${_host} "${_source}"
      WORKING_DIRECTORY "${_dir}" VERBATIM)
    add_custom_target(llcompiler_${_consumer}_${_mode} ALL DEPENDS "${_dir}/${_consumer}-linked.rxbin")
    if(_mode STREQUAL "opt" AND _consumer MATCHES "^llrxc-")
      # Standard generated VM launcher, not compiler logic written in C.
      add_custom_command(OUTPUT "${_dir}/${_consumer}-launcher.c"
        COMMAND $<TARGET_FILE:rxcpack> -o "${_dir}/${_consumer}-launcher" "${_dir}/${_consumer}-linked"
        DEPENDS rxcpack "${_dir}/${_consumer}-linked.rxbin" VERBATIM)
      add_executable(${_consumer} "${_dir}/${_consumer}-launcher.c")
      configure_linker_for_static_lib(${_consumer} fs)
      target_link_libraries(${_consumer} rxvml rxpashim)
      install(TARGETS ${_consumer} RUNTIME DESTINATION bin)
    endif()
  endforeach()
endforeach()
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/bin/llctarget.rxbin"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/backend/opt/llctarget.rxbin" "${CMAKE_BINARY_DIR}/bin/llctarget.rxbin"
  DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/backend/opt/llctarget.rxbin" VERBATIM)
add_custom_target(llctarget ALL DEPENDS "${CMAKE_BINARY_DIR}/bin/llctarget.rxbin")
install(FILES "${CMAKE_BINARY_DIR}/bin/llctarget.rxbin" DESTINATION bin)
add_test(NAME llcompiler_product_inventory
  COMMAND ${CMAKE_COMMAND} "-DBIN_DIRECTORY=${CMAKE_BINARY_DIR}/bin"
    "-DEXECUTABLE_SUFFIX=${CMAKE_EXECUTABLE_SUFFIX}"
    -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/check-product.cmake")
set_tests_properties(llcompiler_product_inventory PROPERTIES
  LABELS "essential;library;level-l;packaging" TIMEOUT 30)
# Keep this tied to the public product selection. Listing the individual
# producers here would hide a missing stage dependency on a clean CI build.
crexx_register_test_prep_targets(TESTS llcompiler_product_inventory TARGETS stage-product)
find_package(Python3 REQUIRED COMPONENTS Interpreter)
add_custom_target(llcompiler_poc_measure
  COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/tests/check-poc.py"
    --build "${CMAKE_BINARY_DIR}" --work "${CMAKE_CURRENT_BINARY_DIR}/poc-measure"
    --mode opt --vm rxbvm
  DEPENDS llcompiler_backend-poc_opt llcompiler_llrxc-c_opt llcompiler_llrxc-g_opt
    llcompiler_target_opt library rxfnsc classlib rxbvm rxas rxlink
  VERBATIM)
add_custom_target(llcompiler_backend_measure
  COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/tests/check-backend.py"
    --build "${CMAKE_BINARY_DIR}" --work "${CMAKE_CURRENT_BINARY_DIR}/backend-measure"
    --mode noopt --vm rxbvm
  DEPENDS llcompiler_llrxc-c_noopt llcompiler_llrxc-g_noopt llcompiler_target_noopt llcompiler_backend-tests_noopt rxbvm rxas rxlink
  VERBATIM)

# Retained isolated provenance measurement target; normal CTest lanes follow.
add_custom_target(llcompiler_provenance_measure
  COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/tests/check-provenance.py"
    --build "${CMAKE_BINARY_DIR}" --work "${CMAKE_CURRENT_BINARY_DIR}/provenance-measure"
    --mode noopt --vm rxbvm
  DEPENDS llcompiler_llrxc-c_noopt llcompiler_llrxc-g_noopt llcompiler_target_noopt llcompiler_backend-tests_noopt rxbvm rxas rxlink
  VERBATIM)

# Isolated measurements: 89s Debug (29 corpus cases), 263s maintained Apple
# ASan (31 cases), including nested tools. Serialize whole aggregates; 1800s
# is a hang guard with room for slower hosted machines, not a speed criterion.
foreach(_mode opt noopt)
  foreach(_vm rxbvm rxtvm)
    if(TARGET ${_vm})
      set(_test "llcompiler_backend_${_mode}_${_vm}")
      add_test(NAME ${_test}
        COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/tests/check-backend.py"
          --build "${CMAKE_BINARY_DIR}" --work "${CMAKE_CURRENT_BINARY_DIR}/checks/${_test}"
          --mode ${_mode} --vm ${_vm})
      set_tests_properties(${_test} PROPERTIES
        PASS_REGULAR_EXPRESSION "BACKEND_E2E[|]cases=31[|]commands=232[|]failures=0"
        TIMEOUT 1800 RUN_SERIAL TRUE RESOURCE_LOCK llcompiler_backend
        LABELS "library;level-l;compiler-backend;optimizer-parity")
      crexx_register_test_prep_targets(TESTS ${_test}
        TARGETS llcompiler_llrxc-c_${_mode} llcompiler_llrxc-g_${_mode}
          llcompiler_backend-tests_${_mode} llcompiler_target_${_mode}
          library rxfnsc rxas rxlink ${_vm})

      # Isolated 90-command provenance proof: 28s Debug, 90s Apple ASan.
      # Keep nested tools serialized, with generous whole/inner hang guards.
      set(_provenance "llcompiler_provenance_${_mode}_${_vm}")
      add_test(NAME ${_provenance}
        COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/tests/check-provenance.py"
          --build "${CMAKE_BINARY_DIR}" --work "${CMAKE_CURRENT_BINARY_DIR}/checks/${_provenance}"
          --mode ${_mode} --vm ${_vm})
      set_tests_properties(${_provenance} PROPERTIES
        PASS_REGULAR_EXPRESSION "PROVENANCE_E2E[|]failures=0"
        TIMEOUT 900 RUN_SERIAL TRUE RESOURCE_LOCK llcompiler_backend
        LABELS "library;level-l;compiler-backend;optimizer-parity")
      crexx_register_test_prep_targets(TESTS ${_provenance}
        TARGETS llcompiler_llrxc-c_${_mode} llcompiler_llrxc-g_${_mode}
          llcompiler_backend-tests_${_mode} llcompiler_target_${_mode}
          library rxfnsc rxas rxlink ${_vm})

      # STEP05 isolated 33-command aggregate: 32s Debug, 65s Apple ASan.
      # Compiler IR on/off is independent of the B host's opt/noopt build.
      set(_optimise "llcompiler_optimise_${_mode}_${_vm}")
      add_test(NAME ${_optimise}
        COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/tests/check-optimise.py"
          --build "${CMAKE_BINARY_DIR}" --work "${CMAKE_CURRENT_BINARY_DIR}/checks/${_optimise}"
          --mode ${_mode} --vm ${_vm})
      set_tests_properties(${_optimise} PROPERTIES
        PASS_REGULAR_EXPRESSION "OPTIMISE_E2E[|]commands=33[|]failures=0"
        TIMEOUT 900 RUN_SERIAL TRUE RESOURCE_LOCK llcompiler_backend
        LABELS "library;level-l;compiler-backend;optimizer-parity")
      crexx_register_test_prep_targets(TESTS ${_optimise}
        TARGETS llcompiler_llrxc-c_${_mode} llcompiler_llrxc-g_${_mode}
          llcompiler_backend-optimise_${_mode} llcompiler_target_${_mode}
          library rxfnsc classlib rxas rxlink ${_vm})

      # STEP06: 54 nested commands, isolated Debug 32s / Apple ASan 118s.
      # Serial whole tests and 300s inner / 900s outer limits are hang guards.
      set(_poc "llcompiler_poc_${_mode}_${_vm}")
      add_test(NAME ${_poc}
        COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/tests/check-poc.py"
          --build "${CMAKE_BINARY_DIR}" --work "${CMAKE_CURRENT_BINARY_DIR}/checks/${_poc}"
          --mode ${_mode} --vm ${_vm})
      set_tests_properties(${_poc} PROPERTIES
        PASS_REGULAR_EXPRESSION "POC_E2E[|]commands=54[|]failures=0"
        TIMEOUT 900 RUN_SERIAL TRUE RESOURCE_LOCK llcompiler_backend
        LABELS "library;level-l;compiler-backend;optimizer-parity")
      crexx_register_test_prep_targets(TESTS ${_poc}
        TARGETS llcompiler_llrxc-c_${_mode} llcompiler_llrxc-g_${_mode}
          llcompiler_backend-poc_${_mode} llcompiler_target_${_mode}
          library rxfnsc classlib rxas rxlink ${_vm})
    endif()
  endforeach()
endforeach()

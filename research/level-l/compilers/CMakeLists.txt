cmake_minimum_required(VERSION 3.24)
crexx_schedule_directory_qa_tier_finalization()
set(_products "${CMAKE_CURRENT_BINARY_DIR}/products")
file(MAKE_DIRECTORY "${_products}")
set(_languages)
foreach(_language IN LISTS CREXX_RESEARCH_LANGUAGES)
  string(TOLOWER "${_language}" _language)
  if(NOT _language MATCHES "^[cg]$")
    message(FATAL_ERROR "Research compiler profile must be C or G; Level B remains unimplemented")
  endif()
  list(APPEND _languages "${_language}")
endforeach()
list(REMOVE_DUPLICATES _languages)
if(NOT _languages)
  message(FATAL_ERROR "Select at least one research compiler")
endif()
function(ll_compile NAME SOURCE DIRECTORY)
  set(_files ${ARGN})
  # Cross-directory generated imports need their producer target as well as
  # the file dependency, so Unix Make builds them before staging metadata.
  set(_producers)
  foreach(_file IN LISTS _files)
    if(_file STREQUAL "${CMAKE_BINARY_DIR}/bin/library.rxbin")
      list(APPEND _producers library)
    elseif(_file STREQUAL "${CMAKE_BINARY_DIR}/bin/rxfnsl.rxbin")
      list(APPEND _producers rxfnsl)
    elseif(_file STREQUAL "${CMAKE_BINARY_DIR}/bin/rxfnsc.rxbin")
      list(APPEND _producers rxfnsc)
    elseif(_file STREQUAL "${CMAKE_BINARY_DIR}/langtools/common/llrxastext.rxbin")
      list(APPEND _producers llrxastext)
    elseif(_file STREQUAL "${CMAKE_BINARY_DIR}/langtools/common/llbinmeta.rxbin")
      list(APPEND _producers llbinmeta)
    elseif(_file STREQUAL "${CMAKE_BINARY_DIR}/langtools/common/llfileoutput.rxbin")
      list(APPEND _producers llfileoutput)
    elseif(_file STREQUAL "${CMAKE_BINARY_DIR}/bin/rxfs.rxplugin")
      list(APPEND _producers fs)
    endif()
  endforeach()
  list(REMOVE_DUPLICATES _producers)
  file(MAKE_DIRECTORY "${DIRECTORY}")
  string(MAKE_C_IDENTIFIER "${DIRECTORY}_${NAME}" _key)
  crexx_add_import_root(${_key}_imports DIRECTORY "${DIRECTORY}/imports-${NAME}"
    OUTPUT_VARIABLE _imports FILES ${_files} DEPENDS ${_files} ${_producers})
  add_custom_command(OUTPUT "${DIRECTORY}/${NAME}.rxbin"
    COMMAND $<TARGET_FILE:rxc> ${_flags} -x --no-exe-import -i "${DIRECTORY}/imports-${NAME}"
      -o "${DIRECTORY}/${NAME}" "${SOURCE}"
    COMMAND $<TARGET_FILE:rxas> ${_flags} "${DIRECTORY}/${NAME}"
    DEPENDS rxc rxas ${_key}_imports ${_imports} "${SOURCE}" VERBATIM)
endfunction()
set(_base "${CMAKE_BINARY_DIR}/bin/library.rxbin" "${CMAKE_BINARY_DIR}/bin/rxfnsl.rxbin")
set(_text "${CMAKE_BINARY_DIR}/langtools/common/llrxastext.rxbin" "${CMAKE_BINARY_DIR}/langtools/common/llbinmeta.rxbin")
set(_io "${CMAKE_BINARY_DIR}/langtools/common/llfileoutput.rxbin")
set(_front "${CMAKE_CURRENT_BINARY_DIR}/frontends")
file(MAKE_DIRECTORY "${_front}")
foreach(_language IN LISTS _languages)
  set(_spec "${CMAKE_CURRENT_SOURCE_DIR}/levelc/levelc.rxl")
  if(_language STREQUAL "g")
    set(_spec "${CMAKE_CURRENT_SOURCE_DIR}/typed/typed.rxl")
  endif()
  add_custom_command(OUTPUT "${_front}/ll${_language}grammar.rxbin"
    COMMAND ${CMAKE_COMMAND} -DLEVEL_L=$<TARGET_FILE:rxlang> -DSPEC=${_spec}
      -DOUTPUT=${_front}/ll${_language}grammar.rxas -DFORMAT=rxas
      -P "${CMAKE_SOURCE_DIR}/langtools/cmake/generate.cmake"
    COMMAND $<TARGET_FILE:rxas> "${_front}/ll${_language}grammar"
    DEPENDS rxlang rxas "${_spec}" "${CMAKE_SOURCE_DIR}/langtools/cmake/generate.cmake" VERBATIM)
endforeach()
foreach(_mode opt noopt)
  set(_flags)
  if(_mode STREQUAL "noopt")
    set(_flags -n)
  endif()
  set(_common "${CMAKE_CURRENT_BINARY_DIR}/common/${_mode}")
  ll_compile(llscalar "${CMAKE_CURRENT_SOURCE_DIR}/common/llscalar.crexx" "${_common}" ${_base})
  ll_compile(llcompilercontracts "${CMAKE_CURRENT_SOURCE_DIR}/common/llcompilercontracts.crexx" "${_common}" ${_base} ${_text})
  set(_shared "${_common}/llscalar.rxbin" "${_common}/llcompilercontracts.rxbin" ${_text})
  set(_allhosts)
  set(_dir "${CMAKE_CURRENT_BINARY_DIR}/backend/${_mode}")
  foreach(_language IN LISTS _languages)
    set(_owner levelc)
    set(_prefix llc)
    if(_language STREQUAL "g")
      set(_owner typed)
      set(_prefix llt)
    endif()
    set(_langdir "${CMAKE_CURRENT_BINARY_DIR}/${_owner}/${_mode}")
    set(_host ${_shared} "${_front}/ll${_language}grammar.rxbin")
    foreach(_part ir parse frontend rxas)
      set(_module "${_prefix}${_part}")
      ll_compile(${_module} "${CMAKE_CURRENT_SOURCE_DIR}/${_owner}/${_module}.crexx" "${_langdir}" ${_base} ${_host})
      list(APPEND _host "${_langdir}/${_module}.rxbin")
    endforeach()
    set(_service "ll${_language}compiler")
    ll_compile(${_service} "${CMAKE_CURRENT_SOURCE_DIR}/level${_language}/${_service}.crexx" "${_langdir}" ${_base} ${_host})
    list(APPEND _host "${_langdir}/${_service}.rxbin")
    list(APPEND _allhosts ${_host})
    set(_cli "llrxc-${_language}")
    ll_compile(${_cli} "${CMAKE_CURRENT_SOURCE_DIR}/level${_language}/${_cli}.crexx" "${_dir}" ${_base} ${_host} "${_io}" "${CMAKE_BINARY_DIR}/bin/rxfs.rxplugin")
    add_custom_command(OUTPUT "${_dir}/${_cli}-linked.rxbin"
      COMMAND $<TARGET_FILE:rxlink> -s -o "${_dir}/${_cli}-linked" "${_dir}/${_cli}" ${_host} "${_io}" ${_base}
      DEPENDS rxlink "${_dir}/${_cli}.rxbin" ${_host} "${_io}" ${_base} VERBATIM)
    add_custom_target(llcompiler_${_cli}_${_mode} ALL DEPENDS "${_dir}/${_cli}-linked.rxbin")
    if(_mode STREQUAL "opt")
      set(_control "")
      foreach(_file IN LISTS _host)
        get_filename_component(_name "${_file}" NAME_WE)
        string(APPEND _control "INPUT ${_file}\nINCLUDE ${_name}\n")
      endforeach()
      string(APPEND _control "PRESERVE INLINE\nOUTPUT ${_products}/${_service}\n")
      file(GENERATE OUTPUT "${_langdir}/service.ctl" CONTENT "${_control}")
      add_custom_command(OUTPUT "${_products}/${_service}.rxbin"
        COMMAND $<TARGET_FILE:rxlink> -c "${_langdir}/service.ctl"
        DEPENDS rxlink ${_host} "${_langdir}/service.ctl" VERBATIM)
      add_custom_target(${_service} ALL DEPENDS "${_products}/${_service}.rxbin")
      install(FILES "${_products}/${_service}.rxbin" DESTINATION bin COMPONENT ResearchCompilers EXCLUDE_FROM_ALL)
      add_custom_command(OUTPUT "${_dir}/${_cli}-launcher.c"
        COMMAND $<TARGET_FILE:rxcpack> -o "${_dir}/${_cli}-launcher" "${_dir}/${_cli}-linked"
        DEPENDS rxcpack "${_dir}/${_cli}-linked.rxbin" VERBATIM)
      add_executable(${_cli} "${_dir}/${_cli}-launcher.c")
      set_target_properties(${_cli} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${_products}")
      configure_linker_for_static_lib(${_cli} fs)
      target_link_libraries(${_cli} rxvml rxpashim)
      install(TARGETS ${_cli} RUNTIME DESTINATION bin COMPONENT ResearchCompilers EXCLUDE_FROM_ALL)
    endif()
  endforeach()
  if("c" IN_LIST _languages)
    # Keep the installed image and legacy exports stable while adding the
    # borrowed-context namespace. Members are private, not separate products.
    set(_target_members "${_dir}/target-members")
    foreach(_adapter llctarget llctarget2)
      ll_compile(${_adapter} "${CMAKE_CURRENT_SOURCE_DIR}/levelc/runtime/${_adapter}.crexx" "${_target_members}"
        "${CMAKE_BINARY_DIR}/bin/library.rxbin" "${CMAKE_BINARY_DIR}/bin/rxfnsc.rxbin")
    endforeach()
    set(_target_control "${_target_members}/target.ctl")
    file(GENERATE OUTPUT "${_target_control}" CONTENT
      "INPUT ${_target_members}/llctarget.rxbin\nINPUT ${_target_members}/llctarget2.rxbin\nINCLUDE llctarget\nINCLUDE llctarget2\nPRESERVE INLINE\nOUTPUT ${_dir}/llctarget\n")
    add_custom_command(OUTPUT "${_dir}/llctarget.rxbin"
      COMMAND $<TARGET_FILE:rxlink> -c "${_target_control}"
      DEPENDS rxlink "${_target_control}" "${_target_members}/llctarget.rxbin"
        "${_target_members}/llctarget2.rxbin" VERBATIM)
    add_custom_target(llcompiler_target_${_mode} ALL DEPENDS "${_dir}/llctarget.rxbin")

    ll_compile(target-context "${CMAKE_CURRENT_SOURCE_DIR}/tests/target-context.crexx" "${_dir}"
      "${CMAKE_BINARY_DIR}/bin/library.rxbin" "${CMAKE_BINARY_DIR}/bin/rxfnsc.rxbin"
      "${_dir}/llctarget.rxbin")
    set(_target_runtime "${CMAKE_BINARY_DIR}/bin/library.rxbin"
      "${CMAKE_BINARY_DIR}/bin/classlib.rxbin" "${CMAKE_BINARY_DIR}/bin/rxfnsc.rxbin"
      "${CMAKE_BINARY_DIR}/bin/rxunicode-core.rxbin")
    add_custom_command(OUTPUT "${_dir}/target-context-linked.rxbin"
      COMMAND $<TARGET_FILE:rxlink> -o "${_dir}/target-context-linked"
        "${_dir}/target-context.rxbin" "${_dir}/llctarget.rxbin" ${_target_runtime}
      DEPENDS rxlink classlib rxunicode_core "${_dir}/target-context.rxbin"
        "${_dir}/llctarget.rxbin" ${_target_runtime} VERBATIM)
    add_custom_target(llcompiler_target_context_${_mode}
      DEPENDS "${_dir}/target-context-linked.rxbin")
    find_package(Python3 REQUIRED COMPONENTS Interpreter)
    set(_context_shape "llcompiler_target_context_shape_${_mode}")
    add_test(NAME ${_context_shape} COMMAND ${Python3_EXECUTABLE}
      "${CMAKE_CURRENT_SOURCE_DIR}/tests/check-target-context.py"
      --context "${CMAKE_BINARY_DIR}/lib/rxfnsc/members/RexxClassicContext/RexxClassicContext.rxas"
      --adapter "${_target_members}/llctarget2.rxas")
    set_tests_properties(${_context_shape} PROPERTIES TIMEOUT 60
      LABELS "library;levelc;compiler-backend;optimizer-parity")
    crexx_register_test_prep_targets(TESTS ${_context_shape}
      TARGETS rxfnsc llcompiler_target_${_mode})
    foreach(_vm rxbvm rxtvm)
      if(TARGET ${_vm})
        set(_context_test "llcompiler_target_context_${_mode}_${_vm}")
        add_test(NAME ${_context_test} COMMAND $<TARGET_FILE:${_vm}>
          --no-autoload "${_dir}/target-context-linked.rxbin")
        set_tests_properties(${_context_test} PROPERTIES PASS_REGULAR_EXPRESSION "TARGET_CONTEXT[|]checks=[0-9]+[|]failures=0"
          TIMEOUT 300 LABELS "library;levelc;compiler-backend;optimizer-parity")
        crexx_register_test_prep_targets(TESTS ${_context_test}
          TARGETS llcompiler_target_context_${_mode} fs cstream ${_vm})
      endif()
    endforeach()
  endif()
  # Mixed-language regression adapters are never part of the products.
  if("c" IN_LIST _languages AND "g" IN_LIST _languages)
    list(REMOVE_DUPLICATES _allhosts)
    set(_tests "${CMAKE_CURRENT_BINARY_DIR}/test-support/${_mode}")
    foreach(_adapter lltestfamilies lltestparse lltestfrontend lltestrxas lltestcompiler)
      ll_compile(${_adapter} "${CMAKE_CURRENT_SOURCE_DIR}/tests/${_adapter}.crexx" "${_tests}" ${_base} ${_allhosts})
      list(APPEND _allhosts "${_tests}/${_adapter}.rxbin")
    endforeach()
    foreach(_consumer frontend-tests frontend-probe parse-probe contracts-probe backend-probe backend-tests backend-optimise backend-poc backend-step07 demo contracts)
      set(_dest "${CMAKE_CURRENT_BINARY_DIR}/frontends/${_mode}")
      set(_source "${CMAKE_CURRENT_SOURCE_DIR}/tests/${_consumer}.crexx")
      if(_consumer MATCHES "^backend")
        set(_dest "${_dir}")
      elseif(_consumer STREQUAL "demo")
        set(_source "${CMAKE_CURRENT_SOURCE_DIR}/examples/ir-demo.crexx")
      elseif(_consumer STREQUAL "contracts")
        set(_source "${CMAKE_CURRENT_SOURCE_DIR}/tests/ir-contracts.crexx")
      endif()
      ll_compile(${_consumer} "${_source}" "${_dest}" ${_base} ${_allhosts})
      add_custom_command(OUTPUT "${_dest}/${_consumer}-linked.rxbin"
        COMMAND $<TARGET_FILE:rxlink> -s -o "${_dest}/${_consumer}-linked" "${_dest}/${_consumer}" ${_allhosts} ${_base}
        DEPENDS rxlink "${_dest}/${_consumer}.rxbin" ${_allhosts} ${_base} VERBATIM)
      add_custom_target(llcompiler_${_consumer}_${_mode} ALL DEPENDS "${_dest}/${_consumer}-linked.rxbin")
      if(_consumer STREQUAL "demo" OR _consumer STREQUAL "contracts")
        set(_success "failures=0")
        if(_consumer STREQUAL "demo")
          set(_success "IR_DEMO[|]OK")
        endif()
        foreach(_vm rxtvm rxbvm)
          if(TARGET ${_vm})
            set(_test "llcompiler_ir_${_consumer}_${_mode}_${_vm}")
            add_test(NAME ${_test} COMMAND $<TARGET_FILE:${_vm}> "${_dest}/${_consumer}-linked.rxbin")
            set_tests_properties(${_test} PROPERTIES PASS_REGULAR_EXPRESSION "${_success}" TIMEOUT 300 LABELS "level-l;optimizer-parity")
            crexx_register_test_prep_targets(TESTS ${_test} TARGETS llcompiler_${_consumer}_${_mode} ${_vm})
          endif()
        endforeach()
      endif()
    endforeach()
    add_dependencies(llcompiler_frontend-tests_${_mode} rxfnsc)
    set(_dir "${CMAKE_CURRENT_BINARY_DIR}/frontends/${_mode}")
  foreach(_group syntax grammar mapping calls contracts lifecycle optional literals)
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
  endif()
endforeach()
if("c" IN_LIST _languages)
  add_custom_command(OUTPUT "${_products}/llctarget.rxbin"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/backend/opt/llctarget.rxbin" "${_products}/llctarget.rxbin"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/backend/opt/llctarget.rxbin" VERBATIM)
  add_custom_target(llctarget ALL DEPENDS "${_products}/llctarget.rxbin")
  install(FILES "${_products}/llctarget.rxbin" DESTINATION bin COMPONENT ResearchCompilers EXCLUDE_FROM_ALL)
endif()
if("c" IN_LIST _languages)
  find_package(Python3 REQUIRED COMPONENTS Interpreter)
  # Retained isolated timing entrypoint for this nested aggregate.
  add_custom_target(llcompiler_c_only_measure
    COMMAND ${CMAKE_COMMAND} -E time ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/tests/check-say.py"
      --build "${CMAKE_BINARY_DIR}" --work "${CMAKE_CURRENT_BINARY_DIR}/c-only-measure"
      --mode opt --vm rxbvm --c-only
    DEPENDS llcompiler_llrxc-c_opt llcompiler_target_opt library classlib rxfnsc rxunicode_core
      fs cstream rxas rxlink rxbvm
    USES_TERMINAL VERBATIM)
  add_custom_target(llcompiler_c_calls_measure
    COMMAND ${CMAKE_COMMAND} -E time ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/tests/check-step07.py"
      --build "${CMAKE_BINARY_DIR}" --work "${CMAKE_CURRENT_BINARY_DIR}/c-calls-measure"
      --mode opt --vm rxbvm --c-only
    DEPENDS llcompiler_llrxc-c_opt llcompiler_target_opt library classlib rxfnsc rxunicode_core
      fs cstream rxas rxlink rxbvm
    USES_TERMINAL VERBATIM)
  # 55 commands: isolated Debug 32s / maintained Apple ASan 97s. This reuses
  # existing call/numeric fixtures; serialize nested tools with a hang guard.
  foreach(_mode opt noopt)
    foreach(_vm rxbvm rxtvm)
      if(TARGET ${_vm})
        set(_test "llcompiler_c_calls_${_mode}_${_vm}")
        add_test(NAME ${_test}
          COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/tests/check-step07.py"
            --build "${CMAKE_BINARY_DIR}" --work "${CMAKE_CURRENT_BINARY_DIR}/checks/${_test}"
            --mode ${_mode} --vm ${_vm} --c-only)
        set_tests_properties(${_test} PROPERTIES
          PASS_REGULAR_EXPRESSION "STEP07_C_ONLY[|]commands=55[|]failures=0"
          TIMEOUT 900 RUN_SERIAL TRUE RESOURCE_LOCK llcompiler_backend
          LABELS "library;level-l;levelc;compiler-backend;optimizer-parity")
        crexx_register_test_prep_targets(TESTS ${_test}
          TARGETS llcompiler_llrxc-c_${_mode} llcompiler_target_${_mode}
            library classlib rxfnsc rxunicode_core fs cstream rxas rxlink ${_vm})
      endif()
    endforeach()
  endforeach()
  # 32 commands: isolated Debug 19s, maintained Apple ASan 55s. Serialize
  # nested tools; 900s is a generous hang guard, not a performance criterion.
  foreach(_mode opt noopt)
    foreach(_vm rxbvm rxtvm)
      if(TARGET ${_vm})
        set(_test "llcompiler_c_only_${_mode}_${_vm}")
        add_test(NAME ${_test}
          COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/tests/check-say.py"
            --build "${CMAKE_BINARY_DIR}" --work "${CMAKE_CURRENT_BINARY_DIR}/checks/${_test}"
            --mode ${_mode} --vm ${_vm} --c-only)
        set_tests_properties(${_test} PROPERTIES
          PASS_REGULAR_EXPRESSION "SAY C-ONLY PASS: 32 commands"
          TIMEOUT 900 RUN_SERIAL TRUE RESOURCE_LOCK llcompiler_backend
          LABELS "library;level-l;levelc;compiler-backend;optimizer-parity")
        crexx_register_test_prep_targets(TESTS ${_test}
          TARGETS llcompiler_llrxc-c_${_mode} llcompiler_target_${_mode}
            library classlib rxfnsc rxunicode_core fs cstream rxas rxlink ${_vm})
      endif()
    endforeach()
  endforeach()
endif()
if("c" IN_LIST _languages AND "g" IN_LIST _languages)
  include("${CMAKE_CURRENT_SOURCE_DIR}/tests/BackendTests.cmake")
endif()

add_test(NAME llcompiler_product_inventory COMMAND ${CMAKE_COMMAND}
  -DBUILD=${CMAKE_BINARY_DIR} -DSUFFIX=${CMAKE_EXECUTABLE_SUFFIX} -DRESEARCH=ON
  "-DLANGUAGES=${CREXX_RESEARCH_LANGUAGES}" -P "${CMAKE_SOURCE_DIR}/langtools/tests/check-products.cmake")
set_tests_properties(llcompiler_product_inventory PROPERTIES PASS_REGULAR_EXPRESSION "LEVEL_L_PRODUCTS[|]PASS" TIMEOUT 30 LABELS "level-l;essential")
set(_selected)
foreach(_language IN LISTS _languages)
  list(APPEND _selected ll${_language}compiler llrxc-${_language})
endforeach()
if("c" IN_LIST _languages)
  list(APPEND _selected llctarget)
endif()
crexx_register_test_prep_targets(TESTS llcompiler_product_inventory TARGETS ${_selected} rxfnsl rxlang rxlanglib llrxastext llfileoutput llbinmeta)

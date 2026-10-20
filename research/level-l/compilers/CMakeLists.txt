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
    elseif(_file STREQUAL "${CMAKE_BINARY_DIR}/lib/rxfnsc/members/RexxClassicUnicodeProperties/RexxClassicUnicodeProperties.rxbin")
      list(APPEND _producers rxfnsc_unicode_properties_member)
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
  add_custom_command(OUTPUT "${DIRECTORY}/${NAME}.rxbin" "${DIRECTORY}/${NAME}.rxas"
    COMMAND $<TARGET_FILE:rxc> ${_flags} -x --no-exe-import -i "${DIRECTORY}/imports-${NAME}"
      -o "${DIRECTORY}/${NAME}" "${SOURCE}"
    COMMAND $<TARGET_FILE:rxas> ${_flags} "${DIRECTORY}/${NAME}"
    DEPENDS rxc rxas ${_key}_imports ${_imports} "${SOURCE}" VERBATIM)
endfunction()
set(_base "${CMAKE_BINARY_DIR}/bin/library.rxbin" "${CMAKE_BINARY_DIR}/bin/rxfnsl.rxbin")
set(_text "${CMAKE_BINARY_DIR}/langtools/common/llrxastext.rxbin" "${CMAKE_BINARY_DIR}/langtools/common/llbinmeta.rxbin")
set(_io "${CMAKE_BINARY_DIR}/langtools/common/llfileoutput.rxbin")
set(_classic_names "${CMAKE_BINARY_DIR}/lib/rxfnsc/members/RexxClassicUnicodeProperties/RexxClassicUnicodeProperties.rxbin")
set(_front "${CMAKE_CURRENT_BINARY_DIR}/frontends")
file(MAKE_DIRECTORY "${_front}")
foreach(_language IN LISTS _languages)
  set(_spec "${CMAKE_CURRENT_SOURCE_DIR}/levelc/levelc.rxl")
  set(_grammar_limits)
  if(_language STREQUAL "g")
    set(_spec "${CMAKE_CURRENT_SOURCE_DIR}/typed/typed.rxl")
  else()
    # The Unicode variable lexer exceeds the generic 4096-state build default.
    # Keep this compiler's allowance separate from global Level L policy.
    list(APPEND _grammar_limits -DGENERATOR_LIMITS=dfa.states=16384 -DREUSE_LEXER=ON)
    set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_CLEAN_FILES
      "${_front}/llcgrammar.rxas.lexer" "${_front}/llcgrammar.rxas.lexer.manifest"
      "${_front}/llcgrammar.rxas.lexer.lock")
  endif()
  add_custom_command(OUTPUT "${_front}/ll${_language}grammar.rxbin"
    COMMAND ${CMAKE_COMMAND} -DLEVEL_L=$<TARGET_FILE:rxlang> -DSPEC=${_spec}
      -DOUTPUT=${_front}/ll${_language}grammar.rxas -DFORMAT=rxas
      ${_grammar_limits}
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
    set(_parts ir parse frontend rxas)
    if(_language STREQUAL "c")
      list(APPEND _host "${_classic_names}")
      set(_parts ir doprotocol parseprogram parse frontend rxas)
    endif()
    foreach(_part IN LISTS _parts)
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
    # The approved C ABI is one clean namespace and one target product.
    ll_compile(llctarget "${CMAKE_CURRENT_SOURCE_DIR}/levelc/runtime/llctarget.crexx" "${_dir}"
      "${CMAKE_BINARY_DIR}/bin/library.rxbin" "${CMAKE_BINARY_DIR}/bin/rxfnsc.rxbin")
    add_custom_target(llcompiler_target_${_mode} ALL DEPENDS "${_dir}/llctarget.rxbin")

    set(_c_contract_inputs ${_base} ${_shared}
      "${_front}/llcgrammar.rxbin"
      "${CMAKE_CURRENT_BINARY_DIR}/levelc/${_mode}/llcir.rxbin"
      "${CMAKE_CURRENT_BINARY_DIR}/levelc/${_mode}/llcdoprotocol.rxbin"
      "${CMAKE_CURRENT_BINARY_DIR}/levelc/${_mode}/llcparseprogram.rxbin"
      "${CMAKE_CURRENT_BINARY_DIR}/levelc/${_mode}/llcrxas.rxbin")
    list(REMOVE_DUPLICATES _c_contract_inputs)
    ll_compile(c-context-contracts "${CMAKE_CURRENT_SOURCE_DIR}/tests/c-context-contracts.crexx"
      "${_dir}" ${_c_contract_inputs})
    add_custom_command(OUTPUT "${_dir}/c-context-contracts-linked.rxbin"
      COMMAND $<TARGET_FILE:rxlink> -s -o "${_dir}/c-context-contracts-linked"
        "${_dir}/c-context-contracts.rxbin" ${_c_contract_inputs}
      DEPENDS rxlink "${_dir}/c-context-contracts.rxbin" ${_c_contract_inputs} VERBATIM)
    crexx_add_import_root(llcompiler_c_context_contract_runtime_${_mode}
      DIRECTORY "${_dir}/context-contract-runtime"
      OUTPUT_VARIABLE _c_contract_runtime
      FILES "${CMAKE_BINARY_DIR}/bin/library.rxbin" "${CMAKE_BINARY_DIR}/bin/rxfnsc.rxbin" "${_dir}/llctarget.rxbin"
      DEPENDS library rxfnsc llcompiler_target_${_mode}
        "${CMAKE_BINARY_DIR}/bin/library.rxbin" "${CMAKE_BINARY_DIR}/bin/rxfnsc.rxbin" "${_dir}/llctarget.rxbin")
    add_custom_target(llcompiler_c_context_contracts_${_mode}
      DEPENDS "${_dir}/c-context-contracts-linked.rxbin" "${_c_contract_runtime}")
    # Independent packet and selector fixtures, separate from source grammar.
    foreach(_parse_fixture program-contracts admission core-backend)
      ll_compile(parse-${_parse_fixture}
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/parse/${_parse_fixture}.crexx"
        "${_dir}" ${_c_contract_inputs})
      add_custom_command(OUTPUT "${_dir}/parse-${_parse_fixture}-linked.rxbin"
        COMMAND $<TARGET_FILE:rxlink> -s -o "${_dir}/parse-${_parse_fixture}-linked"
          "${_dir}/parse-${_parse_fixture}.rxbin" ${_c_contract_inputs}
        DEPENDS rxlink "${_dir}/parse-${_parse_fixture}.rxbin" ${_c_contract_inputs} VERBATIM)
    endforeach()
    get_target_property(_c_runtime_rxas rxfnsc CREXX_RXFNSC_MEMBER_RXAS)
    add_custom_target(llcompiler_c_parse_core_${_mode}
      DEPENDS "${_dir}/parse-program-contracts-linked.rxbin"
        "${_dir}/parse-admission-linked.rxbin"
        "${_dir}/parse-core-backend-linked.rxbin" "${_c_contract_runtime}"
        "${_dir}/llctarget.rxas" ${_c_runtime_rxas})
    add_dependencies(llcompiler_c_parse_core_${_mode} rxfnsc)
    set(_c_drop_inputs ${_c_contract_inputs}
      "${_classic_names}"
      "${CMAKE_CURRENT_BINARY_DIR}/levelc/${_mode}/llcparse.rxbin"
      "${CMAKE_CURRENT_BINARY_DIR}/levelc/${_mode}/llcfrontend.rxbin")
    ll_compile(drop-contracts "${CMAKE_CURRENT_SOURCE_DIR}/tests/drop/contracts.crexx"
      "${_dir}" ${_c_drop_inputs})
    add_custom_command(OUTPUT "${_dir}/drop-contracts-linked.rxbin"
      COMMAND $<TARGET_FILE:rxlink> -s -o "${_dir}/drop-contracts-linked"
        "${_dir}/drop-contracts.rxbin" ${_c_drop_inputs}
      DEPENDS rxlink "${_dir}/drop-contracts.rxbin" ${_c_drop_inputs} VERBATIM)
    add_custom_target(llcompiler_c_drop_contracts_${_mode}
      DEPENDS "${_dir}/drop-contracts-linked.rxbin")
    ll_compile(assignment-contracts "${CMAKE_CURRENT_SOURCE_DIR}/tests/assignment/contracts.crexx"
      "${_dir}" ${_c_drop_inputs})
    add_custom_command(OUTPUT "${_dir}/assignment-contracts-linked.rxbin"
      COMMAND $<TARGET_FILE:rxlink> -s -o "${_dir}/assignment-contracts-linked"
        "${_dir}/assignment-contracts.rxbin" ${_c_drop_inputs}
      DEPENDS rxlink "${_dir}/assignment-contracts.rxbin" ${_c_drop_inputs} VERBATIM)
    add_custom_target(llcompiler_c_assignment_contracts_${_mode}
      DEPENDS "${_dir}/assignment-contracts-linked.rxbin")
    foreach(_vm rxbvm rxtvm)
      if(TARGET ${_vm})
        set(_contracts_test "llcompiler_c_context_contracts_${_mode}_${_vm}")
        add_test(NAME ${_contracts_test} COMMAND $<TARGET_FILE:${_vm}>
          --no-autoload "${_dir}/c-context-contracts-linked.rxbin"
          -a "${_dir}/context-contract-runtime")
        set_tests_properties(${_contracts_test} PROPERTIES
          PASS_REGULAR_EXPRESSION "C_CONTEXT_CONTRACTS[|]checks=[0-9]+[|]failures=0"
          TIMEOUT 300 LABELS "level-l;levelc;compiler-backend;optimizer-parity")
        crexx_register_test_prep_targets(TESTS ${_contracts_test}
          TARGETS llcompiler_c_context_contracts_${_mode} llcompiler_target_${_mode} rxfnsc ${_vm})
      endif()
    endforeach()

    ll_compile(target-parse-source "${CMAKE_CURRENT_SOURCE_DIR}/tests/target-parse-source.crexx" "${_dir}"
      "${CMAKE_BINARY_DIR}/bin/library.rxbin" "${CMAKE_BINARY_DIR}/bin/rxfnsc.rxbin"
      "${_dir}/llctarget.rxbin")
    set(_parse_source_runtime "${CMAKE_BINARY_DIR}/bin/library.rxbin"
      "${CMAKE_BINARY_DIR}/bin/classlib.rxbin" "${CMAKE_BINARY_DIR}/bin/rxfnsc.rxbin"
      "${CMAKE_BINARY_DIR}/bin/rxunicode-core.rxbin")
    add_custom_command(OUTPUT "${_dir}/target-parse-source-linked.rxbin"
      COMMAND $<TARGET_FILE:rxlink> -o "${_dir}/target-parse-source-linked"
        "${_dir}/target-parse-source.rxbin" "${_dir}/llctarget.rxbin" ${_parse_source_runtime}
      DEPENDS rxlink classlib rxunicode_core "${_dir}/target-parse-source.rxbin"
        "${_dir}/llctarget.rxbin" ${_parse_source_runtime} VERBATIM)
    add_custom_target(llcompiler_target_parse_source_${_mode}
      DEPENDS "${_dir}/target-parse-source-linked.rxbin")
    foreach(_vm rxbvm rxtvm)
      if(TARGET ${_vm})
        set(_test "llcompiler_target_parse_source_${_mode}_${_vm}")
        add_test(NAME ${_test} COMMAND $<TARGET_FILE:${_vm}>
          --no-autoload "${_dir}/target-parse-source-linked.rxbin")
        set_tests_properties(${_test} PROPERTIES
          PASS_REGULAR_EXPRESSION "TARGET_PARSE_SOURCE[|]checks=[0-9]+[|]failures=0"
          TIMEOUT 300 LABELS "library;levelc;compiler-backend;optimizer-parity")
        crexx_register_test_prep_targets(TESTS ${_test}
          TARGETS llcompiler_target_parse_source_${_mode} fs cstream ${_vm})
      endif()
    endforeach()

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
      --adapter "${_dir}/llctarget.rxas")
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
    foreach(_vm rxbvm rxtvm)
      if(TARGET ${_vm})
        foreach(_fixture program-contracts admission)
          set(_test "llcompiler_c_parse_${_fixture}_${_mode}_${_vm}")
          set(_arguments)
          if(_fixture STREQUAL "admission")
            list(APPEND _arguments -a "${_dir}/context-contract-runtime")
          endif()
          add_test(NAME ${_test} COMMAND $<TARGET_FILE:${_vm}>
            --no-autoload "${_dir}/parse-${_fixture}-linked.rxbin" ${_arguments})
          set_tests_properties(${_test} PROPERTIES
            PASS_REGULAR_EXPRESSION "failures=0" TIMEOUT 300
            LABELS "level-l;levelc;compiler-backend;optimizer-parity")
          crexx_register_test_prep_targets(TESTS ${_test}
            TARGETS llcompiler_c_parse_core_${_mode} ${_vm})
        endforeach()
        # Isolated opt/rxbvm CORE: 42 commands in 34.361s. The checkpoint-only
        # phase qualifies its added lifecycle control without replaying others.
        # Serialize nested tool use; 600s is a generous hang guard.
        set(_test "llcompiler_c_parse_core_${_mode}_${_vm}")
        add_test(NAME ${_test} COMMAND ${Python3_EXECUTABLE}
          "${CMAKE_CURRENT_SOURCE_DIR}/tests/parse/check-core.py"
          --bin "${CMAKE_BINARY_DIR}/bin"
          --host "${_dir}/parse-core-backend-linked.rxbin"
          --artifacts "${_dir}/context-contract-runtime" --vm $<TARGET_FILE:${_vm}>)
        set_tests_properties(${_test} PROPERTIES
          PASS_REGULAR_EXPRESSION "PARSE_CORE[|]checks=[0-9]+[|]commands=[0-9]+[|]failures=0"
          TIMEOUT 600 RUN_SERIAL TRUE RESOURCE_LOCK llcompiler_backend
          LABELS "library;level-l;levelc;compiler-backend;optimizer-parity")
        crexx_register_test_prep_targets(TESTS ${_test} TARGETS
          llcompiler_c_parse_core_${_mode} library classlib rxfnsc rxunicode_core
          compiler_exit_bin fs cstream rxc rxas rxlink ${_vm})
      endif()
    endforeach()
  endif()
  # Independent IR contracts use generic Level L only, including in C-only
  # configurations. Their test vocabulary does not load either compiler.
  set(_tests "${CMAKE_CURRENT_BINARY_DIR}/test-support/${_mode}")
  set(_contracts_dir "${CMAKE_CURRENT_BINARY_DIR}/frontends/${_mode}")
  ll_compile(lltestfamilies "${CMAKE_CURRENT_SOURCE_DIR}/tests/lltestfamilies.crexx" "${_tests}" ${_base})
  ll_compile(contracts "${CMAKE_CURRENT_SOURCE_DIR}/tests/ir-contracts.crexx"
    "${_contracts_dir}" ${_base} "${_tests}/lltestfamilies.rxbin")
  add_custom_command(OUTPUT "${_contracts_dir}/contracts-linked.rxbin"
    COMMAND $<TARGET_FILE:rxlink> -s -o "${_contracts_dir}/contracts-linked"
      "${_contracts_dir}/contracts.rxbin" "${_tests}/lltestfamilies.rxbin" ${_base}
    DEPENDS rxlink "${_contracts_dir}/contracts.rxbin" "${_tests}/lltestfamilies.rxbin" ${_base} VERBATIM)
  add_custom_target(llcompiler_contracts_${_mode} ALL DEPENDS "${_contracts_dir}/contracts-linked.rxbin")
  foreach(_vm rxtvm rxbvm)
    if(TARGET ${_vm})
      set(_test "llcompiler_ir_contracts_${_mode}_${_vm}")
      add_test(NAME ${_test} COMMAND $<TARGET_FILE:${_vm}> "${_contracts_dir}/contracts-linked.rxbin")
      set_tests_properties(${_test} PROPERTIES PASS_REGULAR_EXPRESSION "failures=0"
        TIMEOUT 300 LABELS "level-l;optimizer-parity")
      crexx_register_test_prep_targets(TESTS ${_test} TARGETS llcompiler_contracts_${_mode} ${_vm})
    endif()
  endforeach()
  # Mixed-language regression adapters are never part of the products.
  if("c" IN_LIST _languages AND "g" IN_LIST _languages)
    list(APPEND _allhosts "${_tests}/lltestfamilies.rxbin")
    list(REMOVE_DUPLICATES _allhosts)
    foreach(_adapter lltestparse lltestfrontend lltestrxas lltestcompiler)
      ll_compile(${_adapter} "${CMAKE_CURRENT_SOURCE_DIR}/tests/${_adapter}.crexx" "${_tests}" ${_base} ${_allhosts})
      list(APPEND _allhosts "${_tests}/${_adapter}.rxbin")
    endforeach()
    foreach(_consumer frontend-tests frontend-probe parse-probe contracts-probe backend-probe backend-tests backend-optimise backend-poc backend-step07 demo)
      set(_dest "${CMAKE_CURRENT_BINARY_DIR}/frontends/${_mode}")
      set(_source "${CMAKE_CURRENT_SOURCE_DIR}/tests/${_consumer}.crexx")
      if(_consumer MATCHES "^backend")
        set(_dest "${_dir}")
      elseif(_consumer STREQUAL "demo")
        set(_source "${CMAKE_CURRENT_SOURCE_DIR}/examples/ir-demo.crexx")
      endif()
      ll_compile(${_consumer} "${_source}" "${_dest}" ${_base} ${_allhosts})
      add_custom_command(OUTPUT "${_dest}/${_consumer}-linked.rxbin"
        COMMAND $<TARGET_FILE:rxlink> -s -o "${_dest}/${_consumer}-linked" "${_dest}/${_consumer}" ${_allhosts} ${_base}
        DEPENDS rxlink "${_dest}/${_consumer}.rxbin" ${_allhosts} ${_base} VERBATIM)
      add_custom_target(llcompiler_${_consumer}_${_mode} ALL DEPENDS "${_dest}/${_consumer}-linked.rxbin")
      if(_consumer STREQUAL "demo")
        foreach(_vm rxtvm rxbvm)
          if(TARGET ${_vm})
            set(_test "llcompiler_ir_${_consumer}_${_mode}_${_vm}")
            add_test(NAME ${_test} COMMAND $<TARGET_FILE:${_vm}> "${_dest}/${_consumer}-linked.rxbin")
            set_tests_properties(${_test} PROPERTIES PASS_REGULAR_EXPRESSION "IR_DEMO[|]OK" TIMEOUT 300 LABELS "level-l;optimizer-parity")
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
        if(_group STREQUAL "grammar")
          # IF adds 44s Debug, SELECT 50s, DO 66s, PARSE 65.86s and ARG 28.7s measured
          # components. PARSE's focused selector remains available without a
          # duplicate CTest registration; grammar already invokes it.
          # Serialize the expanded grammar workload with a generous hang guard.
          set_tests_properties(${_test} PROPERTIES TIMEOUT 900
            RUN_SERIAL TRUE RESOURCE_LOCK llcompiler_backend)
        endif()
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
  set(_drop_vms rxbvm)
  set(_drop_vm_args --vm rxbvm)
  if(TARGET rxtvm)
    list(APPEND _drop_vms rxtvm)
    list(APPEND _drop_vm_args --vm rxtvm)
  endif()
  # Isolated opt/rxbvm source PARSE: 14 commands in 37.670s IR-on / 38.288s
  # IR-off, each including both assembler modes. Preserve serial scheduling.
  foreach(_mode opt noopt)
    foreach(_vm IN LISTS _drop_vms)
      foreach(_ir on off)
        set(_test "llcompiler_c_parse_frontend_${_mode}_${_ir}_${_vm}")
        add_test(NAME ${_test} COMMAND ${Python3_EXECUTABLE}
          "${CMAKE_CURRENT_SOURCE_DIR}/tests/check-parse-frontend.py"
          --build "${CMAKE_BINARY_DIR}"
          --work "${CMAKE_CURRENT_BINARY_DIR}/checks/${_test}"
          --mode ${_mode} --vm ${_vm} --ir ${_ir})
        set_tests_properties(${_test} PROPERTIES
          PASS_REGULAR_EXPRESSION "PARSE_FRONTEND[|]commands=[0-9]+[|]failures=0"
          TIMEOUT 600 RUN_SERIAL TRUE RESOURCE_LOCK llcompiler_backend
          LABELS "library;level-l;levelc;compiler-frontends;optimizer-parity")
        crexx_register_test_prep_targets(TESTS ${_test} TARGETS
          llcompiler_llrxc-c_${_mode} llcompiler_target_${_mode}
          library classlib rxfnsc rxunicode_core fs cstream rxas rxlink ${_vm})
      endforeach()
    endforeach()
  endforeach()
  # Isolated ARG: 29 nested commands in 121.816s opt/rxbvm/IR-off Debug.
  # Pair compiler modes/VMs; unchanged shared-executor cross-cell proof is reused.
  # Windows has only rxbvm, so its noopt lane uses that available engine too.
  foreach(_mode opt noopt)
    set(_arg_vm rxbvm)
    if(_mode STREQUAL "noopt" AND TARGET rxtvm)
      set(_arg_vm rxtvm)
    endif()
    foreach(_ir on off)
      set(_test "llcompiler_c_arg_${_mode}_${_ir}_${_arg_vm}")
      add_test(NAME ${_test} COMMAND ${Python3_EXECUTABLE}
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/arg/check-arg.py"
        --build "${CMAKE_BINARY_DIR}"
        --work "${CMAKE_CURRENT_BINARY_DIR}/checks/${_test}"
        --mode ${_mode} --vm ${_arg_vm} --ir ${_ir})
      set_tests_properties(${_test} PROPERTIES
        PASS_REGULAR_EXPRESSION "ARG[|]phase=all[|].*commands=29[|]failures=0"
        TIMEOUT 900 RUN_SERIAL TRUE RESOURCE_LOCK llcompiler_backend
        LABELS "library;level-l;levelc;compiler-frontends;optimizer-parity")
      crexx_register_test_prep_targets(TESTS ${_test} TARGETS
        llcompiler_llrxc-c_${_mode} llcompiler_target_${_mode}
        library classlib rxfnsc rxunicode_core fs cstream rxas rxlink ${_arg_vm})
    endforeach()
  endforeach()
  add_custom_target(llcompiler_c_drop_measure
    COMMAND ${CMAKE_COMMAND} -E time ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/tests/check-drop.py"
      --build "${CMAKE_BINARY_DIR}" --work "${CMAKE_CURRENT_BINARY_DIR}/drop-measure"
      ${_drop_vm_args}
    DEPENDS llcompiler_llrxc-c_opt llcompiler_llrxc-c_noopt
      llcompiler_c_drop_contracts_opt llcompiler_c_drop_contracts_noopt
      llcompiler_target_opt llcompiler_target_noopt library classlib rxfnsc rxunicode_core
      fs cstream rxc rxas rxlink ${_drop_vms}
    USES_TERMINAL VERBATIM)
  # Isolated Debug components: positive 40 commands <125s (including one
  # refused source), final 15 refusals 19.5s; no single full-run timing claimed.
  # Serialize nested tools; 1800s is a generous hang guard. See AGENTS.md.
  add_test(NAME llcompiler_c_drop COMMAND ${Python3_EXECUTABLE}
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/check-drop.py"
    --build "${CMAKE_BINARY_DIR}" --work "${CMAKE_CURRENT_BINARY_DIR}/checks/llcompiler_c_drop"
    ${_drop_vm_args})
  set_tests_properties(llcompiler_c_drop PROPERTIES
    PASS_REGULAR_EXPRESSION "DROP[|]commands=[0-9]+[|]failures=0"
    TIMEOUT 1800 RUN_SERIAL TRUE RESOURCE_LOCK llcompiler_backend
    LABELS "library;level-l;levelc;compiler-backend;optimizer-parity")
  crexx_register_test_prep_targets(TESTS llcompiler_c_drop TARGETS
    llcompiler_llrxc-c_opt llcompiler_llrxc-c_noopt
    llcompiler_c_drop_contracts_opt llcompiler_c_drop_contracts_noopt
    llcompiler_target_opt llcompiler_target_noopt library classlib rxfnsc rxunicode_core
    fs cstream rxc rxas rxlink ${_drop_vms})
  # Isolated Debug components: 36 positives + 5 refusals + one fatal compile
  # completed within 77s; the remaining fatal component took 5.65s. No single
  # successful full-run timing is claimed. Source-only policy is in AGENTS.md.
  add_test(NAME llcompiler_c_assignment COMMAND ${Python3_EXECUTABLE}
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/check-assignment.py"
    --build "${CMAKE_BINARY_DIR}" --work "${CMAKE_CURRENT_BINARY_DIR}/checks/llcompiler_c_assignment"
    ${_drop_vm_args})
  set_tests_properties(llcompiler_c_assignment PROPERTIES
    PASS_REGULAR_EXPRESSION "ASSIGN[|]phase=all[|]commands=[0-9]+[|]failures=0"
    TIMEOUT 1800 RUN_SERIAL TRUE RESOURCE_LOCK llcompiler_backend
    LABELS "library;level-l;levelc;compiler-backend;optimizer-parity")
  crexx_register_test_prep_targets(TESTS llcompiler_c_assignment TARGETS
    llcompiler_llrxc-c_opt llcompiler_llrxc-c_noopt
    llcompiler_c_assignment_contracts_opt llcompiler_c_assignment_contracts_noopt
    llcompiler_target_opt llcompiler_target_noopt library classlib rxfnsc rxunicode_core
    fs cstream rxc rxas rxlink ${_drop_vms})
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
  # C calls: 145 commands in the representative cell, 68 in the other cells;
  # existing 110-command Debug baseline 96s; focused SELECT metadata adds
  # 11 default commands (its independent 12-command control measured 13s).
  # DO adds one valid metadata compile and seven single-field refusals (15).
  # LEAVE adds one valid compile and four single-field refusals (9); its
  # independent 10-command metadata component measured 16s in Debug.
  # This is component evidence, not a fresh full-aggregate measurement.
  # Source-only qualification uses Debug; sanitizer scheduling follows AGENTS.md.
  # Serialize nested tools; 1800s is a hang guard for slower hosts.
  foreach(_mode opt noopt)
    foreach(_vm rxbvm rxtvm)
      if(TARGET ${_vm})
        set(_calls "llcompiler_c_calls_${_mode}_${_vm}")
        set(_commands 68)
        if(_mode STREQUAL "opt" AND _vm STREQUAL "rxbvm")
          set(_commands 145)
        endif()
        add_test(NAME ${_calls}
          COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/tests/check-step07.py"
            --build "${CMAKE_BINARY_DIR}" --work "${CMAKE_CURRENT_BINARY_DIR}/checks/${_calls}"
            --mode ${_mode} --vm ${_vm} --c-only)
        set_tests_properties(${_calls} PROPERTIES
          PASS_REGULAR_EXPRESSION "STEP07_C_ONLY[|]commands=${_commands}[|]failures=0"
          TIMEOUT 1800 RUN_SERIAL TRUE RESOURCE_LOCK llcompiler_backend
          LABELS "library;level-l;levelc;compiler-backend;optimizer-parity")
        crexx_register_test_prep_targets(TESTS ${_calls}
          TARGETS llcompiler_llrxc-c_${_mode} llcompiler_target_${_mode}
            library classlib rxfnsc rxunicode_core fs cstream rxas rxlink ${_vm})
        # 32 commands: isolated Debug 19s, maintained Apple ASan 55s. Serialize
        # nested tools; 900s is a generous hang guard, not a performance criterion.
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

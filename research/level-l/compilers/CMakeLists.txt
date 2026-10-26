cmake_minimum_required(VERSION 3.24)
crexx_schedule_directory_qa_tier_finalization()
find_package(Python3 REQUIRED COMPONENTS Interpreter)
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
if("c" IN_LIST _languages)
  add_executable(g2_process_fixture tests/group2/process-fixture.c)
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
    elseif(_file STREQUAL "${CMAKE_BINARY_DIR}/bin/rxfnsl-analysis.rxbin")
      list(APPEND _producers rxfnsl_analysis)
    elseif(_file STREQUAL "${CMAKE_BINARY_DIR}/bin/rxfnsl-rxas.rxbin")
      list(APPEND _producers rxfnsl_rxas)
    elseif(_file STREQUAL "${CMAKE_BINARY_DIR}/bin/rxfnsl-execute.rxbin")
      list(APPEND _producers rxfnsl_execute)
    elseif(_file STREQUAL "${CMAKE_BINARY_DIR}/bin/rxfnsc.rxbin")
      list(APPEND _producers rxfnsc)
    elseif(_file STREQUAL "${CMAKE_BINARY_DIR}/bin/classlib.rxbin")
      list(APPEND _producers classlib)
    elseif(_file STREQUAL "${CMAKE_BINARY_DIR}/bin/rxunicode-core.rxbin")
      list(APPEND _producers rxunicode_core)
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
    elseif(_file STREQUAL "${CMAKE_BINARY_DIR}/bin/providers/rxcontract.rxplugin")
      list(APPEND _producers llcontract)
    elseif(_file STREQUAL "${_front}/llcgrammar.rxbin")
      list(APPEND _producers llcompiler_c_grammar)
    elseif(_file STREQUAL "${_front}/llggrammar.rxbin")
      list(APPEND _producers llcompiler_g_grammar)
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
# Internal compiler bytecode keeps its compiler-only import contract. The public
# packed C command gains source-run support in a separate image below.
function(ll_command _language _mode _dir)
  set(_cli "llrxc-${_language}")
  set(_command_inputs ${ARGN})
  ll_compile(${_cli} "${CMAKE_CURRENT_SOURCE_DIR}/level${_language}/${_cli}.crexx"
    "${_dir}" ${_command_inputs} "${CMAKE_BINARY_DIR}/bin/rxfs.rxplugin")
  set(_link_inputs ${_command_inputs})
  if(_language STREQUAL "g")
    # Preserve the existing G command's exact link-provider ordering.
    list(REMOVE_ITEM _link_inputs ${_base})
    list(APPEND _link_inputs ${_base})
  endif()
  add_custom_command(OUTPUT "${_dir}/${_cli}-linked.rxbin"
    COMMAND $<TARGET_FILE:rxlink> -s -o "${_dir}/${_cli}-linked"
      "${_dir}/${_cli}" ${_link_inputs}
    DEPENDS rxlink "${_dir}/${_cli}.rxbin" ${_link_inputs} VERBATIM)
  add_custom_target(llcompiler_${_cli}_${_mode} ALL DEPENDS "${_dir}/${_cli}-linked.rxbin")
  if(_mode STREQUAL "opt" AND _language STREQUAL "g")
    add_custom_command(OUTPUT "${_dir}/${_cli}-launcher.c"
      COMMAND $<TARGET_FILE:rxcpack> -o "${_dir}/${_cli}-launcher" "${_dir}/${_cli}-linked"
      DEPENDS rxcpack "${_dir}/${_cli}-linked.rxbin" VERBATIM)
    add_executable(${_cli} "${_dir}/${_cli}-launcher.c")
    set_target_properties(${_cli} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${_products}")
    configure_linker_for_static_lib(${_cli} fs)
    if(_language STREQUAL "c")
      configure_linker_for_static_lib(${_cli} llcontract PROVIDER_ID rxcontract)
      configure_linker_for_static_lib(${_cli} cstream)
    endif()
    target_link_libraries(${_cli} rxvml rxpashim)
    install(TARGETS ${_cli} RUNTIME DESTINATION bin COMPONENT ResearchCompilers EXCLUDE_FROM_ALL)
  endif()
endfunction()
set(_base "${CMAKE_BINARY_DIR}/bin/library.rxbin" "${CMAKE_BINARY_DIR}/bin/rxfnsl.rxbin")
set(_infrastructure "${CMAKE_BINARY_DIR}/bin/rxfnsl-analysis.rxbin"
  "${CMAKE_BINARY_DIR}/bin/rxfnsl-rxas.rxbin")
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
    # This declared Unicode/Classic grammar exceeds generic build defaults.
    # Keep its finite DFA/clause allowances separate from Level L policy.
    list(APPEND _grammar_limits "-DGENERATOR_LIMITS=dfa.states=16384\;grammar.clauses=1024" -DREUSE_LEXER=ON)
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
  # Opt/noopt consumers share this grammar, including its assembly step.
  add_custom_target(llcompiler_${_language}_grammar
    DEPENDS "${_front}/ll${_language}grammar.rxbin")
endforeach()
set(_ll_private_nested_tests)
if(BUILD_TESTING)
  math(EXPR _ll_private_nested_processors "${CREXX_QA_CTEST_JOBS} / 5 + 1")
endif()
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
      set(_parts ir language symbols bifs bindings srcmap diagnostics parseprogram procedureprotocol signalprotocol doprotocol clauseprotocol commandprotocol parse frontend graph traceemit callemit effects rxas)
    endif()
    foreach(_part IN LISTS _parts)
      # Optional packet facts reuse neutral analysis. Target RXAS support still
      # enters after the frontend-only closure, at its first actual consumer.
      if(_language STREQUAL "c" AND _part STREQUAL "procedureprotocol")
        list(APPEND _host "${CMAKE_BINARY_DIR}/bin/rxfnsl-analysis.rxbin")
      elseif(_part STREQUAL "traceemit")
        list(APPEND _host "${CMAKE_BINARY_DIR}/bin/rxfnsl-rxas.rxbin")
      elseif(_language STREQUAL "g" AND _part STREQUAL "rxas")
        list(APPEND _host ${_infrastructure})
      endif()
      set(_module "${_prefix}${_part}")
      set(_native_imports)
      if(_language STREQUAL "c" AND (_part STREQUAL "bindings" OR _part STREQUAL "rxas"))
        # Compiler-host metadata only: never a bootstrap or application input.
        set(_native_imports "${CMAKE_BINARY_DIR}/bin/providers/rxcontract.rxplugin")
      endif()
      ll_compile(${_module} "${CMAKE_CURRENT_SOURCE_DIR}/${_owner}/${_module}.crexx" "${_langdir}" ${_base} ${_host} ${_native_imports})
      list(APPEND _host "${_langdir}/${_module}.rxbin")
      if(_language STREQUAL "c" AND _part STREQUAL "graph")
        # Optional execution reuses this frontend-only closure. RXAS selection
        # and compiler CLI services enter strictly downstream.
        set(_c_frontend_host ${_host})
      endif()
    endforeach()
    set(_service "ll${_language}compiler")
    ll_compile(${_service} "${CMAKE_CURRENT_SOURCE_DIR}/level${_language}/${_service}.crexx" "${_langdir}" ${_base} ${_host})
    list(APPEND _host "${_langdir}/${_service}.rxbin")
    list(APPEND _allhosts ${_host})
    set(_cli "llrxc-${_language}")
    if(_language STREQUAL "c")
      ll_compile(llccompilecli "${CMAKE_CURRENT_SOURCE_DIR}/levelc/llccompilecli.crexx"
        "${_langdir}" ${_base} ${_host} "${_io}" "${CMAKE_BINARY_DIR}/bin/rxfs.rxplugin")
      set(_c_compiler_host ${_host} "${_langdir}/llccompilecli.rxbin")
      ll_command(c ${_mode} "${_dir}" ${_base} ${_c_compiler_host} "${_io}")
    else()
      ll_command(${_language} ${_mode} "${_dir}" ${_base} ${_host} "${_io}")
    endif()
    if(_mode STREQUAL "opt")
      set(_control "")
      foreach(_file IN LISTS _host)
        get_filename_component(_name "${_file}" NAME_WE)
        string(APPEND _control "INPUT ${_file}\n")
        set(_selectors "${_name}")
        if(_name STREQUAL "rxfnsl-analysis")
          get_property(_selectors GLOBAL PROPERTY RXFNSL_ANALYSIS_SELECTORS)
        elseif(_name STREQUAL "rxfnsl-rxas")
          get_property(_selectors GLOBAL PROPERTY RXFNSL_RXAS_SELECTORS)
        endif()
        foreach(_selector IN LISTS _selectors)
          string(APPEND _control "INCLUDE ${_selector}\n")
        endforeach()
      endforeach()
      string(APPEND _control "PRESERVE INLINE\nOUTPUT ${_products}/${_service}\n")
      file(GENERATE OUTPUT "${_langdir}/service.ctl" CONTENT "${_control}")
      add_custom_command(OUTPUT "${_products}/${_service}.rxbin"
        COMMAND $<TARGET_FILE:rxlink> -c "${_langdir}/service.ctl"
        DEPENDS rxlink ${_host} "${_langdir}/service.ctl" VERBATIM)
      add_custom_target(${_service} ALL DEPENDS "${_products}/${_service}.rxbin")
      install(FILES "${_products}/${_service}.rxbin" DESTINATION bin COMPONENT ResearchCompilers EXCLUDE_FROM_ALL)
    endif()
  endforeach()
  if("c" IN_LIST _languages)
    # The approved C ABI is one clean namespace and one target product.
    ll_compile(llctarget "${CMAKE_CURRENT_SOURCE_DIR}/levelc/runtime/llctarget.crexx" "${_dir}"
      "${CMAKE_BINARY_DIR}/bin/library.rxbin" "${CMAKE_BINARY_DIR}/bin/rxfnsc.rxbin")
    add_custom_target(llcompiler_target_${_mode} ALL DEPENDS "${_dir}/llctarget.rxbin")

    set(_dynamic_dir "${CMAKE_CURRENT_BINARY_DIR}/dynamic/${_mode}")
    set(_dynamic_inputs ${_base} ${_c_frontend_host}
      "${CMAKE_BINARY_DIR}/bin/rxfnsl-execute.rxbin"
      "${CMAKE_BINARY_DIR}/bin/rxfnsc.rxbin"
      "${_dir}/llctarget.rxbin"
      "${CMAKE_CURRENT_BINARY_DIR}/levelc/${_mode}/llceffects.rxbin"
      "${CMAKE_BINARY_DIR}/bin/providers/rxcontract.rxplugin"
      "${CMAKE_BINARY_DIR}/bin/rxfs.rxplugin")
    # The consolidated Classic runtime already owns this generated property
    # member. Retaining its standalone compiler-host image duplicates symbols.
    list(REMOVE_ITEM _dynamic_inputs "${_classic_names}")
    foreach(_part llcsource llcdynamicbudget llcdynamicbifs llcdynamicobserve llcdynamic)
      set(_part_inputs ${_dynamic_inputs})
      if(_part STREQUAL "llcsource")
        set(_part_inputs "${CMAKE_BINARY_DIR}/bin/library.rxbin" "${_classic_names}"
          "${CMAKE_BINARY_DIR}/bin/rxfs.rxplugin")
      elseif(_part STREQUAL "llcdynamicbudget")
        set(_part_inputs "${CMAKE_BINARY_DIR}/bin/library.rxbin")
      endif()
      ll_compile(${_part} "${CMAKE_CURRENT_SOURCE_DIR}/levelc/dynamic/${_part}.crexx"
        "${_dynamic_dir}" ${_part_inputs})
      list(APPEND _dynamic_inputs "${_dynamic_dir}/${_part}.rxbin")
    endforeach()
    set(_dynamic_runtime ${_dynamic_inputs}
      "${CMAKE_BINARY_DIR}/bin/classlib.rxbin"
      "${CMAKE_BINARY_DIR}/bin/rxunicode-core.rxbin")
    list(FILTER _dynamic_runtime INCLUDE REGEX "\\.rxbin$")
    list(REMOVE_DUPLICATES _dynamic_runtime)
    ll_compile(llcruncli "${CMAKE_CURRENT_SOURCE_DIR}/levelc/llcruncli.crexx"
      "${_dynamic_dir}" ${_dynamic_inputs})
    set(_c_command_inputs ${_base} ${_c_compiler_host} "${_io}")
    list(REMOVE_ITEM _c_command_inputs "${_classic_names}")
    foreach(_runtime_input IN LISTS _dynamic_runtime)
      if(NOT _runtime_input IN_LIST _c_command_inputs)
        list(APPEND _c_command_inputs "${_runtime_input}")
      endif()
    endforeach()
    list(APPEND _c_command_inputs "${_dynamic_dir}/llcruncli.rxbin")
    if(_mode STREQUAL "opt")
      # Only the packed public command links the interpreter. Existing bytecode
      # CLI/worker hosts keep their lean providers and private metadata root.
      ll_compile(llccommand "${CMAKE_CURRENT_SOURCE_DIR}/levelc/llccommand.crexx"
        "${_dir}" ${_c_command_inputs})
      add_custom_command(OUTPUT "${_dir}/llrxc-c-command-linked.rxbin"
        COMMAND $<TARGET_FILE:rxlink> -s -o "${_dir}/llrxc-c-command-linked"
          "${_dir}/llccommand.rxbin" ${_c_command_inputs}
        DEPENDS rxlink "${_dir}/llccommand.rxbin" ${_c_command_inputs} VERBATIM)
      add_custom_command(OUTPUT "${_dir}/llrxc-c-launcher.c"
        COMMAND $<TARGET_FILE:rxcpack> -o "${_dir}/llrxc-c-launcher" "${_dir}/llrxc-c-command-linked"
        DEPENDS rxcpack "${_dir}/llrxc-c-command-linked.rxbin" VERBATIM)
      add_executable(llrxc-c "${_dir}/llrxc-c-launcher.c")
      set_target_properties(llrxc-c PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${_products}")
      configure_linker_for_static_lib(llrxc-c fs)
      configure_linker_for_static_lib(llrxc-c cstream)
      configure_linker_for_static_lib(llrxc-c llcontract PROVIDER_ID rxcontract)
      target_link_libraries(llrxc-c rxvml rxpashim)
      install(TARGETS llrxc-c RUNTIME DESTINATION bin COMPONENT ResearchCompilers EXCLUDE_FROM_ALL)
      ll_compile(llrexx "${CMAKE_CURRENT_SOURCE_DIR}/levelc/llrexx.crexx"
        "${_dynamic_dir}" ${_dynamic_inputs} "${_dynamic_dir}/llcruncli.rxbin")
      set(_interpreter_control "INPUT ${_dynamic_dir}/llrexx.rxbin\nINPUT ${_dynamic_dir}/llcruncli.rxbin\n")
      foreach(_runtime_input IN LISTS _dynamic_runtime)
        string(APPEND _interpreter_control "INPUT ${_runtime_input}\n")
      endforeach()
      string(APPEND _interpreter_control "ROOT llrexx\nOUTPUT ${_products}/llrexx\n")
      file(GENERATE OUTPUT "${_dynamic_dir}/interpreter.ctl" CONTENT "${_interpreter_control}")
      add_custom_command(OUTPUT "${_products}/llrexx.rxbin"
        COMMAND $<TARGET_FILE:rxlink> -s -c "${_dynamic_dir}/interpreter.ctl"
        DEPENDS rxlink classlib rxunicode_core "${_dynamic_dir}/llrexx.rxbin"
          "${_dynamic_dir}/llcruncli.rxbin" ${_dynamic_runtime}
          "${_dynamic_dir}/interpreter.ctl" VERBATIM)
      add_custom_target(llcompiler_llrexx_opt DEPENDS "${_products}/llrexx.rxbin")
      add_custom_command(OUTPUT "${_dynamic_dir}/llrexx-launcher.c"
        COMMAND $<TARGET_FILE:rxcpack> -o "${_dynamic_dir}/llrexx-launcher" "${_products}/llrexx"
        DEPENDS rxcpack "${_products}/llrexx.rxbin" VERBATIM)
      add_executable(llrexx "${_dynamic_dir}/llrexx-launcher.c")
      set_target_properties(llrexx PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${_products}")
      configure_linker_for_static_lib(llrexx fs)
      configure_linker_for_static_lib(llrexx cstream)
      configure_linker_for_static_lib(llrexx llcontract PROVIDER_ID rxcontract)
      target_link_libraries(llrexx rxvml rxpashim)
      install(TARGETS llrexx RUNTIME DESTINATION bin COMPONENT ResearchCompilers EXCLUDE_FROM_ALL)
      install(FILES "${_products}/llrexx.rxbin" DESTINATION bin COMPONENT ResearchCompilers EXCLUDE_FROM_ALL)
      set(_dynamic_control "")
      foreach(_file IN LISTS _dynamic_inputs)
        if(_file MATCHES "rxplugin$")
          continue()
        endif()
        get_filename_component(_name "${_file}" NAME_WE)
        if(_name STREQUAL "library" OR _name STREQUAL "rxfnsc" OR _name STREQUAL "llctarget")
          # Existing shared application images remain external dependencies.
          # INPUT would resolve and embed their bodies, creating duplicate
          # providers when a caller links its ordinary Classic runtime.
          continue()
        endif()
        string(APPEND _dynamic_control "INPUT ${_file}\n")
        set(_selectors "${_name}")
        if(_name STREQUAL "rxfnsl-analysis")
          get_property(_selectors GLOBAL PROPERTY RXFNSL_ANALYSIS_SELECTORS)
        elseif(_name STREQUAL "rxfnsl-execute")
          get_property(_selectors GLOBAL PROPERTY RXFNSL_EXECUTE_SELECTORS)
        elseif(_name STREQUAL "rxfnsl")
          # Core members are selected transitively; this is an artifact stem.
          continue()
        endif()
        foreach(_selector IN LISTS _selectors)
          string(APPEND _dynamic_control "INCLUDE ${_selector}\n")
        endforeach()
      endforeach()
      string(APPEND _dynamic_control "PRESERVE INLINE\nOUTPUT ${_products}/llcdynamic\n")
      file(GENERATE OUTPUT "${_dynamic_dir}/dynamic.ctl" CONTENT "${_dynamic_control}")
      add_custom_command(OUTPUT "${_products}/llcdynamic.rxbin"
        COMMAND $<TARGET_FILE:rxlink> -c "${_dynamic_dir}/dynamic.ctl"
        DEPENDS rxlink ${_dynamic_inputs} "${_dynamic_dir}/dynamic.ctl" VERBATIM)
      add_custom_target(llcdynamic ALL DEPENDS "${_products}/llcdynamic.rxbin")
      install(FILES "${_products}/llcdynamic.rxbin" DESTINATION bin
        COMPONENT ResearchCompilers EXCLUDE_FROM_ALL)
    endif()
    add_custom_target(llcompiler_dynamic_${_mode}
      DEPENDS "${_dynamic_dir}/llcdynamic.rxbin")
    if(_mode STREQUAL "opt")
      ll_compile(dynamic-command-contracts
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/dynamic/command-contracts.crexx"
        "${_dynamic_dir}" ${_dynamic_inputs})
      add_custom_command(OUTPUT "${_dynamic_dir}/dynamic-command-contracts-linked.rxbin"
        COMMAND $<TARGET_FILE:rxlink> -s -o "${_dynamic_dir}/dynamic-command-contracts-linked"
          "${_dynamic_dir}/dynamic-command-contracts.rxbin" ${_dynamic_runtime}
        DEPENDS rxlink classlib rxunicode_core
          "${_dynamic_dir}/dynamic-command-contracts.rxbin" ${_dynamic_runtime} VERBATIM)
      add_custom_target(llcompiler_dynamic_command_opt
        DEPENDS "${_dynamic_dir}/dynamic-command-contracts-linked.rxbin")
    endif()
    foreach(_case source runtime coordinator binding-snapshot)
      set(_case_inputs ${_dynamic_inputs})
      set(_case_runtime ${_dynamic_runtime})
      set(_case_link_targets classlib rxunicode_core)
      if(_case STREQUAL "source")
        set(_case_inputs "${CMAKE_BINARY_DIR}/bin/library.rxbin" "${_classic_names}"
          "${_dynamic_dir}/llcsource.rxbin" "${CMAKE_BINARY_DIR}/bin/rxfs.rxplugin")
        set(_case_runtime "${CMAKE_BINARY_DIR}/bin/library.rxbin" "${_classic_names}"
          "${_dynamic_dir}/llcsource.rxbin")
        set(_case_link_targets)
      elseif(_case STREQUAL "runtime")
        set(_case_inputs "${CMAKE_BINARY_DIR}/bin/library.rxbin"
          "${CMAKE_BINARY_DIR}/bin/rxfnsc.rxbin" "${_dynamic_dir}/llcdynamicbudget.rxbin")
        set(_case_runtime ${_case_inputs} "${CMAKE_BINARY_DIR}/bin/classlib.rxbin"
          "${CMAKE_BINARY_DIR}/bin/rxunicode-core.rxbin")
      endif()
      ll_compile(dynamic-${_case}-contracts
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/dynamic/${_case}-contracts.crexx"
        "${_dynamic_dir}" ${_case_inputs})
      add_custom_command(OUTPUT "${_dynamic_dir}/dynamic-${_case}-contracts-linked.rxbin"
        COMMAND $<TARGET_FILE:rxlink> -s
          -o "${_dynamic_dir}/dynamic-${_case}-contracts-linked"
          "${_dynamic_dir}/dynamic-${_case}-contracts.rxbin" ${_case_runtime}
        DEPENDS rxlink ${_case_link_targets}
          "${_dynamic_dir}/dynamic-${_case}-contracts.rxbin" ${_case_runtime}
        VERBATIM)
      add_custom_target(llcompiler_dynamic_${_case}_${_mode}
        DEPENDS "${_dynamic_dir}/dynamic-${_case}-contracts-linked.rxbin")
      if(_mode STREQUAL "opt" AND TARGET rxbvm)
        set(_test "llcompiler_dynamic_${_case}_opt_rxbvm")
        add_test(NAME ${_test} COMMAND $<TARGET_FILE:rxbvm>
          "${_dynamic_dir}/dynamic-${_case}-contracts-linked.rxbin" -a "${_dynamic_dir}")
        set(_dynamic_pass "PASS: Classic dynamic")
        if(_case STREQUAL "binding-snapshot")
          set(_dynamic_pass "failures=0")
        endif()
        set_tests_properties(${_test} PROPERTIES
          PASS_REGULAR_EXPRESSION "${_dynamic_pass}" TIMEOUT 300
          LABELS "level-l;levelc;compiler;component")
        crexx_register_test_prep_targets(TESTS ${_test} TARGETS
          llcompiler_dynamic_${_case}_${_mode} rxbvm fs)
      endif()
    endforeach()

    set(_c_contract_inputs ${_base} ${_infrastructure} ${_shared}
      "${_front}/llcgrammar.rxbin"
      "${CMAKE_CURRENT_BINARY_DIR}/levelc/${_mode}/llcir.rxbin"
      "${CMAKE_CURRENT_BINARY_DIR}/levelc/${_mode}/llclanguage.rxbin"
      "${CMAKE_CURRENT_BINARY_DIR}/levelc/${_mode}/llcsymbols.rxbin"
      "${CMAKE_CURRENT_BINARY_DIR}/levelc/${_mode}/llcbifs.rxbin"
      "${CMAKE_CURRENT_BINARY_DIR}/levelc/${_mode}/llcbindings.rxbin"
      "${CMAKE_CURRENT_BINARY_DIR}/levelc/${_mode}/llcdiagnostics.rxbin"
      "${CMAKE_CURRENT_BINARY_DIR}/levelc/${_mode}/llcsrcmap.rxbin"
      "${CMAKE_CURRENT_BINARY_DIR}/levelc/${_mode}/llcdoprotocol.rxbin"
      "${CMAKE_CURRENT_BINARY_DIR}/levelc/${_mode}/llcclauseprotocol.rxbin"
      "${CMAKE_CURRENT_BINARY_DIR}/levelc/${_mode}/llcparseprogram.rxbin"
      "${CMAKE_CURRENT_BINARY_DIR}/levelc/${_mode}/llcprocedureprotocol.rxbin"
      "${CMAKE_CURRENT_BINARY_DIR}/levelc/${_mode}/llcsignalprotocol.rxbin"
      "${CMAKE_CURRENT_BINARY_DIR}/levelc/${_mode}/llccommandprotocol.rxbin"
      "${CMAKE_CURRENT_BINARY_DIR}/levelc/${_mode}/llctraceemit.rxbin"
      "${CMAKE_CURRENT_BINARY_DIR}/levelc/${_mode}/llccallemit.rxbin"
      "${CMAKE_CURRENT_BINARY_DIR}/levelc/${_mode}/llceffects.rxbin"
      ${_classic_names}
      "${CMAKE_CURRENT_BINARY_DIR}/levelc/${_mode}/llcrxas.rxbin")
    list(REMOVE_DUPLICATES _c_contract_inputs)
    set(_infrastructure_inputs ${_base} "${CMAKE_BINARY_DIR}/bin/rxfnsl-analysis.rxbin"
      "${_common}/llscalar.rxbin"
      "${CMAKE_CURRENT_BINARY_DIR}/levelc/${_mode}/llcir.rxbin"
      "${CMAKE_CURRENT_BINARY_DIR}/levelc/${_mode}/llceffects.rxbin")
    ll_compile(infrastructure-contracts "${CMAKE_CURRENT_SOURCE_DIR}/tests/infrastructure/contracts.crexx"
      "${_dir}" ${_infrastructure_inputs})
    add_custom_command(OUTPUT "${_dir}/infrastructure-contracts-linked.rxbin"
      COMMAND $<TARGET_FILE:rxlink> -s -o "${_dir}/infrastructure-contracts-linked"
        "${_dir}/infrastructure-contracts.rxbin" ${_infrastructure_inputs}
      DEPENDS rxlink "${_dir}/infrastructure-contracts.rxbin" ${_infrastructure_inputs} VERBATIM)
    add_custom_target(llcompiler_infrastructure_contracts_${_mode}
      DEPENDS "${_dir}/infrastructure-contracts-linked.rxbin")
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
    list(REMOVE_DUPLICATES _c_drop_inputs)
    ll_compile(drop-contracts "${CMAKE_CURRENT_SOURCE_DIR}/tests/drop/contracts.crexx"
      "${_dir}" ${_c_drop_inputs})
    add_custom_command(OUTPUT "${_dir}/drop-contracts-linked.rxbin"
      COMMAND $<TARGET_FILE:rxlink> -s -o "${_dir}/drop-contracts-linked"
        "${_dir}/drop-contracts.rxbin" ${_c_drop_inputs}
      DEPENDS rxlink "${_dir}/drop-contracts.rxbin" ${_c_drop_inputs} VERBATIM)
    add_custom_target(llcompiler_c_drop_contracts_${_mode}
      DEPENDS "${_dir}/drop-contracts-linked.rxbin")
    ll_compile(prepared-invalid-grammar "${CMAKE_CURRENT_SOURCE_DIR}/tests/prepared-invalid-grammar.crexx"
      "${_dir}" ${_base})
    ll_compile(prepared-language "${CMAKE_CURRENT_SOURCE_DIR}/tests/prepared-language.crexx"
      "${_dir}" ${_c_drop_inputs})
    add_custom_command(OUTPUT "${_dir}/prepared-language-linked.rxbin"
      COMMAND $<TARGET_FILE:rxlink> -s -o "${_dir}/prepared-language-linked"
        "${_dir}/prepared-language.rxbin" ${_c_drop_inputs}
      DEPENDS rxlink "${_dir}/prepared-language.rxbin" ${_c_drop_inputs} VERBATIM)
    set(_c_invalid_inputs ${_c_drop_inputs})
    list(REMOVE_ITEM _c_invalid_inputs "${_front}/llcgrammar.rxbin")
    list(APPEND _c_invalid_inputs "${_dir}/prepared-invalid-grammar.rxbin")
    add_custom_command(OUTPUT "${_dir}/prepared-language-invalid-linked.rxbin"
      COMMAND $<TARGET_FILE:rxlink> -s -o "${_dir}/prepared-language-invalid-linked"
        "${_dir}/prepared-language.rxbin" ${_c_invalid_inputs}
      DEPENDS rxlink "${_dir}/prepared-language.rxbin" ${_c_invalid_inputs} VERBATIM)
    add_custom_target(llcompiler_c_prepared_${_mode}
      DEPENDS "${_dir}/prepared-language-linked.rxbin"
        "${_dir}/prepared-language-invalid-linked.rxbin" ${_c_contract_runtime})
    set(_prepared_vm rxbvm)
    if(_mode STREQUAL "noopt" AND TARGET rxtvm)
      set(_prepared_vm rxtvm)
    endif()
    foreach(_prepared_case valid invalid)
      set(_prepared_image "${_dir}/prepared-language-linked.rxbin")
      set(_prepared_args)
      if(_prepared_case STREQUAL "invalid")
        set(_prepared_image "${_dir}/prepared-language-invalid-linked.rxbin")
        set(_prepared_args invalid)
      endif()
      set(_prepared_test "llcompiler_c_prepared_${_prepared_case}_${_mode}_${_prepared_vm}")
      add_test(NAME ${_prepared_test} COMMAND $<TARGET_FILE:${_prepared_vm}>
        "${_prepared_image}" -a "${_dir}/context-contract-runtime" ${_prepared_args})
      set_tests_properties(${_prepared_test} PROPERTIES
        PASS_REGULAR_EXPRESSION "C_PREPARED[|]checks=[0-9]+[|]failures=0"
        TIMEOUT 300 RUN_SERIAL TRUE RESOURCE_LOCK llcompiler_backend
        LABELS "compiler;level-l;optimizer-parity")
      crexx_register_test_prep_targets(TESTS ${_prepared_test}
        TARGETS llcompiler_c_prepared_${_mode} ${_prepared_vm})
    endforeach()
    # One poisoned first-use owner, separate from the absent-grammar module.
    if(_mode STREQUAL "opt" AND TARGET rxbvm)
      ll_compile(prepared-failing-grammar "${CMAKE_CURRENT_SOURCE_DIR}/tests/prepared-failing-grammar.crexx"
        "${_dir}" ${_base} "${CMAKE_CURRENT_BINARY_DIR}/levelc/${_mode}/llclanguage.rxbin")
      set(_c_failing_inputs ${_c_invalid_inputs})
      list(REMOVE_ITEM _c_failing_inputs "${_dir}/prepared-invalid-grammar.rxbin")
      list(APPEND _c_failing_inputs "${_dir}/prepared-failing-grammar.rxbin")
      add_custom_command(OUTPUT "${_dir}/prepared-language-failing-linked.rxbin"
        COMMAND $<TARGET_FILE:rxlink> -s -o "${_dir}/prepared-language-failing-linked"
          "${_dir}/prepared-language.rxbin" ${_c_failing_inputs}
        DEPENDS rxlink "${_dir}/prepared-language.rxbin" ${_c_failing_inputs} VERBATIM)
      add_custom_target(llcompiler_c_prepared_fault_opt
        DEPENDS "${_dir}/prepared-language-failing-linked.rxbin" ${_c_contract_runtime})
      set(_prepared_fault "llcompiler_c_prepared_fault_opt_rxbvm")
      add_test(NAME ${_prepared_fault} COMMAND $<TARGET_FILE:rxbvm>
        "${_dir}/prepared-language-failing-linked.rxbin" -a "${_dir}/context-contract-runtime" fault)
      set_tests_properties(${_prepared_fault} PROPERTIES
        PASS_REGULAR_EXPRESSION "C_PREPARED[|]checks=[0-9]+[|]failures=0"
        TIMEOUT 300 RUN_SERIAL TRUE RESOURCE_LOCK llcompiler_backend
        LABELS "compiler;level-l;optimizer-parity")
      crexx_register_test_prep_targets(TESTS ${_prepared_fault}
        TARGETS llcompiler_c_prepared_fault_opt rxbvm)
    endif()
    set(_prepared_shape "llcompiler_c_prepared_rxas_${_mode}")
    add_test(NAME ${_prepared_shape} COMMAND ${Python3_EXECUTABLE}
      "${CMAKE_CURRENT_SOURCE_DIR}/tests/check-prepared-rxas.py"
      "${CMAKE_CURRENT_BINARY_DIR}/levelc/${_mode}")
    set_tests_properties(${_prepared_shape} PROPERTIES
      PASS_REGULAR_EXPRESSION "C_PREPARED_RXAS[|]PASS" TIMEOUT 30
      LABELS "compiler;level-l;optimizer-parity")
    crexx_register_test_prep_targets(TESTS ${_prepared_shape}
      TARGETS llcompiler_c_prepared_${_mode})
    ll_compile(call-contracts "${CMAKE_CURRENT_SOURCE_DIR}/tests/call/contracts.crexx"
      "${_dir}" ${_c_drop_inputs})
    add_custom_command(OUTPUT "${_dir}/call-contracts-linked.rxbin"
      COMMAND $<TARGET_FILE:rxlink> -s -o "${_dir}/call-contracts-linked"
        "${_dir}/call-contracts.rxbin" ${_c_drop_inputs}
      DEPENDS rxlink "${_dir}/call-contracts.rxbin" ${_c_drop_inputs} VERBATIM)
    add_custom_target(llcompiler_call_contracts_${_mode}
      DEPENDS "${_dir}/call-contracts-linked.rxbin")
    set(_c_external_inputs ${_c_drop_inputs}
      "${CMAKE_CURRENT_BINARY_DIR}/levelc/${_mode}/llccompiler.rxbin")
    ll_compile(call-external-contracts "${CMAKE_CURRENT_SOURCE_DIR}/tests/call/external/contracts.crexx"
      "${_dir}" ${_c_external_inputs})
    add_custom_command(OUTPUT "${_dir}/call-external-contracts-linked.rxbin"
      COMMAND $<TARGET_FILE:rxlink> -s -o "${_dir}/call-external-contracts-linked"
        "${_dir}/call-external-contracts.rxbin" ${_c_external_inputs}
      DEPENDS rxlink "${_dir}/call-external-contracts.rxbin" ${_c_external_inputs} VERBATIM)
    add_custom_target(llcompiler_call_external_contracts_${_mode}
      DEPENDS "${_dir}/call-external-contracts-linked.rxbin")
    ll_compile(command-contracts "${CMAKE_CURRENT_SOURCE_DIR}/tests/command/contracts.crexx"
      "${_dir}" ${_c_external_inputs})
    add_custom_command(OUTPUT "${_dir}/command-contracts-linked.rxbin"
      COMMAND $<TARGET_FILE:rxlink> -s -o "${_dir}/command-contracts-linked"
        "${_dir}/command-contracts.rxbin" ${_c_external_inputs}
      DEPENDS rxlink "${_dir}/command-contracts.rxbin" ${_c_external_inputs} VERBATIM)
    add_custom_target(llcompiler_command_contracts_${_mode}
      DEPENDS "${_dir}/command-contracts-linked.rxbin")
    ll_compile(clause-contracts "${CMAKE_CURRENT_SOURCE_DIR}/tests/clause/contracts.crexx"
      "${_dir}" ${_c_external_inputs})
    add_custom_command(OUTPUT "${_dir}/clause-contracts-linked.rxbin"
      COMMAND $<TARGET_FILE:rxlink> -s -o "${_dir}/clause-contracts-linked"
        "${_dir}/clause-contracts.rxbin" ${_c_external_inputs}
      DEPENDS rxlink "${_dir}/clause-contracts.rxbin" ${_c_external_inputs} VERBATIM)
    add_custom_target(llcompiler_clause_contracts_${_mode}
      DEPENDS "${_dir}/clause-contracts-linked.rxbin")
    ll_compile(return-contracts "${CMAKE_CURRENT_SOURCE_DIR}/tests/return/contracts.crexx"
      "${_dir}" ${_c_drop_inputs})
    add_custom_command(OUTPUT "${_dir}/return-contracts-linked.rxbin"
      COMMAND $<TARGET_FILE:rxlink> -s -o "${_dir}/return-contracts-linked"
        "${_dir}/return-contracts.rxbin" ${_c_drop_inputs}
      DEPENDS rxlink "${_dir}/return-contracts.rxbin" ${_c_drop_inputs} VERBATIM)
    add_custom_target(llcompiler_return_contracts_${_mode}
      DEPENDS "${_dir}/return-contracts-linked.rxbin")
    ll_compile(exit-contracts "${CMAKE_CURRENT_SOURCE_DIR}/tests/exit/contracts.crexx"
      "${_dir}" ${_c_drop_inputs})
    add_custom_command(OUTPUT "${_dir}/exit-contracts-linked.rxbin"
      COMMAND $<TARGET_FILE:rxlink> -s -o "${_dir}/exit-contracts-linked"
        "${_dir}/exit-contracts.rxbin" ${_c_drop_inputs}
      DEPENDS rxlink "${_dir}/exit-contracts.rxbin" ${_c_drop_inputs} VERBATIM)
    add_custom_target(llcompiler_exit_contracts_${_mode}
      DEPENDS "${_dir}/exit-contracts-linked.rxbin")
    ll_compile(llc-queue-insertion-contracts "${CMAKE_CURRENT_SOURCE_DIR}/tests/queue-insertion/contracts.crexx"
      "${_dir}" ${_c_drop_inputs})
    add_custom_command(OUTPUT "${_dir}/llc-queue-insertion-contracts-linked.rxbin"
      COMMAND $<TARGET_FILE:rxlink> -s -o "${_dir}/llc-queue-insertion-contracts-linked"
        "${_dir}/llc-queue-insertion-contracts.rxbin" ${_c_drop_inputs}
      DEPENDS rxlink "${_dir}/llc-queue-insertion-contracts.rxbin" ${_c_drop_inputs} VERBATIM)
    add_custom_target(llcompiler_queue_insertion_contracts_${_mode}
      DEPENDS "${_dir}/llc-queue-insertion-contracts-linked.rxbin")
    ll_compile(address-contracts "${CMAKE_CURRENT_SOURCE_DIR}/tests/address/contracts.crexx"
      "${_dir}" ${_c_drop_inputs})
    add_custom_command(OUTPUT "${_dir}/address-contracts-linked.rxbin"
      COMMAND $<TARGET_FILE:rxlink> -s -o "${_dir}/address-contracts-linked"
        "${_dir}/address-contracts.rxbin" ${_c_drop_inputs}
      DEPENDS rxlink "${_dir}/address-contracts.rxbin" ${_c_drop_inputs} VERBATIM)
    add_custom_target(llcompiler_address_contracts_${_mode}
      DEPENDS "${_dir}/address-contracts-linked.rxbin")
    ll_compile(numeric-contracts "${CMAKE_CURRENT_SOURCE_DIR}/tests/numeric/contracts.crexx"
      "${_dir}" ${_c_drop_inputs})
    add_custom_command(OUTPUT "${_dir}/numeric-contracts-linked.rxbin"
      COMMAND $<TARGET_FILE:rxlink> -s -o "${_dir}/numeric-contracts-linked"
        "${_dir}/numeric-contracts.rxbin" ${_c_drop_inputs}
      DEPENDS rxlink "${_dir}/numeric-contracts.rxbin" ${_c_drop_inputs} VERBATIM)
    add_custom_target(llcompiler_numeric_contracts_${_mode}
      DEPENDS "${_dir}/numeric-contracts-linked.rxbin")
    ll_compile(trace-contracts "${CMAKE_CURRENT_SOURCE_DIR}/tests/trace/contracts.crexx"
      "${_dir}" ${_c_drop_inputs})
    add_custom_command(OUTPUT "${_dir}/trace-contracts-linked.rxbin"
      COMMAND $<TARGET_FILE:rxlink> -s -o "${_dir}/trace-contracts-linked"
        "${_dir}/trace-contracts.rxbin" ${_c_drop_inputs}
      DEPENDS rxlink "${_dir}/trace-contracts.rxbin" ${_c_drop_inputs} VERBATIM)
    add_custom_target(llcompiler_trace_contracts_${_mode}
      DEPENDS "${_dir}/trace-contracts-linked.rxbin")
    # One warm fixture compiler and focused C22 admission contracts. Native C
    # sources/oracles remain owned by compiler/tests; only harnesses adapt.
    foreach(_conditions_part fixtures contracts)
      set(_conditions_source "compile-fixtures")
      set(_conditions_name "conditions-fixtures")
      if(_conditions_part STREQUAL "contracts")
        set(_conditions_source "contracts")
        set(_conditions_name "conditions-observations-contracts")
      endif()
      set(_conditions_inputs ${_c_drop_inputs} "${_io}")
      if(_conditions_part STREQUAL "fixtures")
        list(APPEND _conditions_inputs "${CMAKE_CURRENT_BINARY_DIR}/levelc/${_mode}/llccompiler.rxbin")
      endif()
      ll_compile(${_conditions_name}
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/conditions-observations/${_conditions_source}.crexx"
        "${_dir}" ${_conditions_inputs})
      add_custom_command(OUTPUT "${_dir}/${_conditions_name}-linked.rxbin"
        COMMAND $<TARGET_FILE:rxlink> -s -o "${_dir}/${_conditions_name}-linked"
          "${_dir}/${_conditions_name}.rxbin" ${_conditions_inputs}
        DEPENDS rxlink "${_dir}/${_conditions_name}.rxbin" ${_conditions_inputs} VERBATIM)
    endforeach()
    add_custom_target(llcompiler_conditions_observations_${_mode}
      DEPENDS "${_dir}/conditions-fixtures-linked.rxbin"
        "${_dir}/conditions-observations-contracts-linked.rxbin")
    ll_compile(label-contracts "${CMAKE_CURRENT_SOURCE_DIR}/tests/labels/contracts.crexx"
      "${_dir}" ${_c_drop_inputs})
    add_custom_command(OUTPUT "${_dir}/label-contracts-linked.rxbin"
      COMMAND $<TARGET_FILE:rxlink> -s -o "${_dir}/label-contracts-linked"
        "${_dir}/label-contracts.rxbin" ${_c_drop_inputs}
      DEPENDS rxlink "${_dir}/label-contracts.rxbin" ${_c_drop_inputs} VERBATIM)
    add_custom_target(llcompiler_label_contracts_${_mode}
      DEPENDS "${_dir}/label-contracts-linked.rxbin")
    ll_compile(signal-contracts "${CMAKE_CURRENT_SOURCE_DIR}/tests/signal/contracts.crexx"
      "${_dir}" ${_c_drop_inputs})
    add_custom_command(OUTPUT "${_dir}/signal-contracts-linked.rxbin"
      COMMAND $<TARGET_FILE:rxlink> -s -o "${_dir}/signal-contracts-linked"
        "${_dir}/signal-contracts.rxbin" ${_c_drop_inputs}
      DEPENDS rxlink "${_dir}/signal-contracts.rxbin" ${_c_drop_inputs} VERBATIM)
    add_custom_target(llcompiler_signal_contracts_${_mode}
      DEPENDS "${_dir}/signal-contracts-linked.rxbin")
    ll_compile(conditions-contracts "${CMAKE_CURRENT_SOURCE_DIR}/tests/signal/conditions-contracts.crexx"
      "${_dir}" ${_c_drop_inputs})
    add_custom_command(OUTPUT "${_dir}/conditions-contracts-linked.rxbin"
      COMMAND $<TARGET_FILE:rxlink> -s -o "${_dir}/conditions-contracts-linked"
        "${_dir}/conditions-contracts.rxbin" ${_c_drop_inputs}
      DEPENDS rxlink "${_dir}/conditions-contracts.rxbin" ${_c_drop_inputs} VERBATIM)
    add_custom_target(llcompiler_signal_conditions_contracts_${_mode}
      DEPENDS "${_dir}/conditions-contracts-linked.rxbin")
    ll_compile(procedure-contracts "${CMAKE_CURRENT_SOURCE_DIR}/tests/procedure/contracts.crexx"
      "${_dir}" ${_c_drop_inputs})
    add_custom_command(OUTPUT "${_dir}/procedure-contracts-linked.rxbin"
      COMMAND $<TARGET_FILE:rxlink> -s -o "${_dir}/procedure-contracts-linked"
        "${_dir}/procedure-contracts.rxbin" ${_c_drop_inputs}
      DEPENDS rxlink "${_dir}/procedure-contracts.rxbin" ${_c_drop_inputs} VERBATIM)
    add_custom_target(llcompiler_procedure_contracts_${_mode}
      DEPENDS "${_dir}/procedure-contracts-linked.rxbin")
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
          TIMEOUT 900 RUN_SERIAL TRUE RESOURCE_LOCK llcompiler_backend
          LABELS "level-l;levelc;compiler-backend;optimizer-parity")
        list(APPEND _ll_private_nested_tests ${_contracts_test})
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
        # Private nested tools use bounded ordinary admission; retain the hang guard.
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
        list(APPEND _ll_private_nested_tests ${_test})
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
          # Bound the private grammar workload with the ordinary admission below.
          set_tests_properties(${_test} PROPERTIES TIMEOUT 900
            RUN_SERIAL TRUE RESOURCE_LOCK llcompiler_backend)
          list(APPEND _ll_private_nested_tests ${_test})
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
    list(APPEND _ll_private_nested_tests ${_test})
        crexx_register_test_prep_targets(TESTS ${_test} TARGETS
          llcompiler_llrxc-c_${_mode} llcompiler_target_${_mode}
          library classlib rxfnsc rxunicode_core fs cstream llcontract rxas rxlink ${_vm})
      endforeach()
    endforeach()
  endforeach()
  # ARG keeps exact syntax/runtime oracles with one combined shape/control pair.
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
        PASS_REGULAR_EXPRESSION "ARG[|]phase=all[|].*commands=25[|]failures=0"
        TIMEOUT 900 RUN_SERIAL TRUE RESOURCE_LOCK llcompiler_backend
        LABELS "library;level-l;levelc;compiler-frontends;optimizer-parity")
    list(APPEND _ll_private_nested_tests ${_test})
      crexx_register_test_prep_targets(TESTS ${_test} TARGETS
        llcompiler_llrxc-c_${_mode} llcompiler_target_${_mode}
        library classlib rxfnsc rxunicode_core fs cstream llcontract rxas rxlink ${_arg_vm})
    endforeach()
  endforeach()
  # Complementary lanes cover both host/IR modes and available VM engines.
  # Private nested tools use the bounded ordinary admission policy below.
  # The timeout is a generous hang guard, not a speed criterion.
  foreach(_mode opt noopt)
    set(_procedure_vm rxbvm)
    set(_procedure_ir on)
    if(_mode STREQUAL "noopt")
      set(_procedure_ir off)
      if(TARGET rxtvm)
        set(_procedure_vm rxtvm)
      endif()
    endif()
    set(_test "llcompiler_c_procedure_${_mode}_${_procedure_ir}_${_procedure_vm}")
    add_test(NAME ${_test} COMMAND ${Python3_EXECUTABLE}
      "${CMAKE_CURRENT_SOURCE_DIR}/tests/procedure/check-procedure.py"
      --build "${CMAKE_BINARY_DIR}"
      --work "${CMAKE_CURRENT_BINARY_DIR}/checks/${_test}"
      --mode ${_mode} --vm ${_procedure_vm} --ir ${_procedure_ir})
    set_tests_properties(${_test} PROPERTIES
      PASS_REGULAR_EXPRESSION "PROCEDURE[|]phase=all[|].*commands=[0-9]+[|]failures=0"
      TIMEOUT 3600 RUN_SERIAL TRUE RESOURCE_LOCK llcompiler_backend
      LABELS "library;level-l;levelc;compiler-frontends;compiler-backend;optimizer-parity")
    list(APPEND _ll_private_nested_tests ${_test})
    crexx_register_test_prep_targets(TESTS ${_test} TARGETS
      llcompiler_llrxc-c_${_mode} llcompiler_target_${_mode}
      llcompiler_procedure_contracts_${_mode}
      library classlib rxfnsc rxunicode_core fs cstream llcontract rxc rxas rxlink ${_procedure_vm})
  endforeach()
  # Complementary host/IR/assembler/VM lanes retain independent CALL oracles.
  # Bound independent nested tools below; this deadline protects against hangs.
  foreach(_mode opt noopt)
    set(_call_vm rxbvm)
    set(_call_ir on)
    if(_mode STREQUAL "noopt")
      set(_call_ir off)
      if(TARGET rxtvm)
        set(_call_vm rxtvm)
      endif()
    endif()
    set(_test "llcompiler_c_call_${_mode}_${_call_ir}_${_call_vm}")
    add_test(NAME ${_test} COMMAND ${Python3_EXECUTABLE}
      "${CMAKE_CURRENT_SOURCE_DIR}/tests/call/check-call.py"
      --build "${CMAKE_BINARY_DIR}"
      --work "${CMAKE_CURRENT_BINARY_DIR}/checks/${_test}"
      --mode ${_mode} --vm ${_call_vm} --ir ${_call_ir})
    set_tests_properties(${_test} PROPERTIES
      PASS_REGULAR_EXPRESSION "CALL_CHECK[|].*phase=all[|].*commands=[0-9]+[|]PASS"
      TIMEOUT 3600 RUN_SERIAL TRUE RESOURCE_LOCK llcompiler_backend
      LABELS "library;level-l;levelc;compiler-frontends;compiler-backend;optimizer-parity")
    list(APPEND _ll_private_nested_tests ${_test})
    crexx_register_test_prep_targets(TESTS ${_test} TARGETS
      llcompiler_llrxc-c_${_mode} llcompiler_target_${_mode}
      llcompiler_call_contracts_${_mode} llcdynamic
      library classlib rxfnsc rxunicode_core fs cstream llcontract rxas rxlink ${_call_vm})
    # Explicit owning B/G preparation stays outside the staged C request host.
    set(_external_test "llcompiler_c_call_external_${_mode}_${_call_ir}_${_call_vm}")
    add_test(NAME ${_external_test} COMMAND ${Python3_EXECUTABLE}
      "${CMAKE_CURRENT_SOURCE_DIR}/tests/call/external/check-external.py"
      --build "${CMAKE_BINARY_DIR}"
      --work "${CMAKE_CURRENT_BINARY_DIR}/checks/${_external_test}"
      --mode ${_mode} --vm ${_call_vm} --ir ${_call_ir})
    set_tests_properties(${_external_test} PROPERTIES
      PASS_REGULAR_EXPRESSION "EXTERNAL_CHECK[|].*phase=all[|].*commands=[0-9]+[|]PASS"
      TIMEOUT 3600 RUN_SERIAL TRUE RESOURCE_LOCK llcompiler_backend
      LABELS "library;level-l;levelc;compiler-frontends;compiler-backend;optimizer-parity")
    list(APPEND _ll_private_nested_tests ${_external_test})
    crexx_register_test_prep_targets(TESTS ${_external_test} TARGETS
      llcompiler_llrxc-c_${_mode} llcompiler_target_${_mode}
      llcompiler_call_external_contracts_${_mode} llcdynamic
      library classlib rxfnsc rxunicode_core fs cstream llcontract rxc rxas rxlink ${_call_vm})
  endforeach()
  # RETURN shares one source compilation across source, cleanup and B-host
  # controls in each complementary lane. Keep nested tools serialized.
  foreach(_mode opt noopt)
    set(_return_vm rxbvm)
    set(_return_ir on)
    if(_mode STREQUAL "noopt")
      set(_return_ir off)
      if(TARGET rxtvm)
        set(_return_vm rxtvm)
      endif()
    endif()
    set(_test "llcompiler_c_return_${_mode}_${_return_ir}_${_return_vm}")
    add_test(NAME ${_test} COMMAND ${Python3_EXECUTABLE}
      "${CMAKE_CURRENT_SOURCE_DIR}/tests/return/check-return.py"
      --build "${CMAKE_BINARY_DIR}"
      --work "${CMAKE_CURRENT_BINARY_DIR}/checks/${_test}"
      --mode ${_mode} --vm ${_return_vm} --ir ${_return_ir})
    set_tests_properties(${_test} PROPERTIES
      PASS_REGULAR_EXPRESSION "RETURN_CHECK[|].*phase=all[|].*commands=[0-9]+[|]PASS"
      TIMEOUT 1800 RUN_SERIAL TRUE RESOURCE_LOCK llcompiler_backend
      LABELS "library;level-l;levelc;compiler-frontends;compiler-backend;optimizer-parity")
    list(APPEND _ll_private_nested_tests ${_test})
    crexx_register_test_prep_targets(TESTS ${_test} TARGETS
      llcompiler_llrxc-c_${_mode} llcompiler_target_${_mode}
      llcompiler_return_contracts_${_mode}
      library classlib rxfnsc rxunicode_core fs cstream llcontract rxc rxas rxlink ${_return_vm})
  endforeach()
  # EXIT shares one source image across full-result/lifetime host controls.
  # Complementary lanes include independent raw admission with bounded admission.
  foreach(_mode opt noopt)
    set(_exit_vm rxbvm)
    set(_exit_ir on)
    if(_mode STREQUAL "noopt")
      set(_exit_ir off)
      if(TARGET rxtvm)
        set(_exit_vm rxtvm)
      endif()
    endif()
    set(_test "llcompiler_c_exit_${_mode}_${_exit_ir}_${_exit_vm}")
    add_test(NAME ${_test} COMMAND ${Python3_EXECUTABLE}
      "${CMAKE_CURRENT_SOURCE_DIR}/tests/exit/check-exit.py"
      --build "${CMAKE_BINARY_DIR}"
      --work "${CMAKE_CURRENT_BINARY_DIR}/checks/${_test}"
      --mode ${_mode} --vm ${_exit_vm} --ir ${_exit_ir})
    set_tests_properties(${_test} PROPERTIES
      PASS_REGULAR_EXPRESSION "EXIT_CHECK[|].*phase=all[|].*commands=[0-9]+[|]PASS"
      TIMEOUT 1800 RUN_SERIAL TRUE RESOURCE_LOCK llcompiler_backend
      LABELS "library;level-l;levelc;compiler-frontends;compiler-backend;optimizer-parity")
    list(APPEND _ll_private_nested_tests ${_test})
    crexx_register_test_prep_targets(TESTS ${_test} TARGETS
      llcompiler_llrxc-c_${_mode} llcompiler_target_${_mode}
      llcompiler_exit_contracts_${_mode}
      library classlib rxfnsc rxunicode_core fs cstream llcontract rxc rxas rxlink ${_exit_vm})
  endforeach()
  # Bare PULL still acquires input; reuse one source image across host/shape checks.
  foreach(_mode opt noopt)
    set(_pull_vm rxbvm)
    set(_pull_ir on)
    if(_mode STREQUAL "noopt")
      set(_pull_ir off)
      if(TARGET rxtvm)
        set(_pull_vm rxtvm)
      endif()
    endif()
    set(_test "llcompiler_c_pull_${_mode}_${_pull_ir}_${_pull_vm}")
    add_test(NAME ${_test} COMMAND ${Python3_EXECUTABLE}
      "${CMAKE_CURRENT_SOURCE_DIR}/tests/pull/check-pull.py"
      --build "${CMAKE_BINARY_DIR}"
      --work "${CMAKE_CURRENT_BINARY_DIR}/checks/${_test}"
      --mode ${_mode} --vm ${_pull_vm} --ir ${_pull_ir})
    set_tests_properties(${_test} PROPERTIES
      PASS_REGULAR_EXPRESSION "PULL_CHECK[|].*phase=all[|].*commands=[0-9]+[|]PASS"
      TIMEOUT 1800 RUN_SERIAL TRUE RESOURCE_LOCK llcompiler_backend
      LABELS "library;level-l;levelc;compiler-frontends;compiler-backend;optimizer-parity")
    list(APPEND _ll_private_nested_tests ${_test})
    crexx_register_test_prep_targets(TESTS ${_test} TARGETS
      llcompiler_llrxc-c_${_mode} llcompiler_target_${_mode}
      library classlib rxfnsc rxunicode_core fs cstream llcontract rxc rxas rxlink ${_pull_vm})
  endforeach()
  # Each insertion source image serves queue/EXIT/host checks; share one raw probe.
  foreach(_instruction push queue)
    string(TOUPPER "${_instruction}" _insertion_marker)
    foreach(_mode opt noopt)
      set(_insertion_vm rxbvm)
      set(_insertion_ir on)
      if(_mode STREQUAL "noopt")
        set(_insertion_ir off)
        if(TARGET rxtvm)
          set(_insertion_vm rxtvm)
        endif()
      endif()
      set(_test "llcompiler_c_${_instruction}_${_mode}_${_insertion_ir}_${_insertion_vm}")
      add_test(NAME ${_test} COMMAND ${Python3_EXECUTABLE}
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/${_instruction}/check-${_instruction}.py"
        --build "${CMAKE_BINARY_DIR}"
        --work "${CMAKE_CURRENT_BINARY_DIR}/checks/${_test}"
        --mode ${_mode} --vm ${_insertion_vm} --ir ${_insertion_ir})
      set_tests_properties(${_test} PROPERTIES
        PASS_REGULAR_EXPRESSION "${_insertion_marker}_CHECK[|].*phase=all[|].*commands=[0-9]+[|]PASS"
        TIMEOUT 1800 RUN_SERIAL TRUE RESOURCE_LOCK llcompiler_backend
        LABELS "library;level-l;levelc;compiler-frontends;compiler-backend;optimizer-parity")
    list(APPEND _ll_private_nested_tests ${_test})
      crexx_register_test_prep_targets(TESTS ${_test} TARGETS
        llcompiler_llrxc-c_${_mode} llcompiler_target_${_mode}
        llcompiler_queue_insertion_contracts_${_mode}
        library classlib rxfnsc rxunicode_core fs cstream llcontract rxc rxas rxlink ${_insertion_vm})
    endforeach()
  endforeach()
  # Explicit ADDRESS reuses one source image for state, bindings and cleanup.
  foreach(_mode opt noopt)
    set(_address_vm rxbvm)
    set(_address_ir on)
    if(_mode STREQUAL "noopt")
      set(_address_ir off)
      if(TARGET rxtvm)
        set(_address_vm rxtvm)
      endif()
    endif()
    set(_test "llcompiler_c_address_${_mode}_${_address_ir}_${_address_vm}")
    add_test(NAME ${_test} COMMAND ${Python3_EXECUTABLE}
      "${CMAKE_CURRENT_SOURCE_DIR}/tests/address/check-address.py"
      --build "${CMAKE_BINARY_DIR}"
      --work "${CMAKE_CURRENT_BINARY_DIR}/checks/${_test}"
      --mode ${_mode} --vm ${_address_vm} --ir ${_address_ir})
    set_tests_properties(${_test} PROPERTIES
      PASS_REGULAR_EXPRESSION "ADDRESS_CHECK[|].*phase=all[|].*commands=[0-9]+[|]PASS"
      TIMEOUT 1800 RUN_SERIAL TRUE RESOURCE_LOCK llcompiler_backend
      LABELS "library;level-l;levelc;compiler-frontends;compiler-backend;optimizer-parity")
    list(APPEND _ll_private_nested_tests ${_test})
    crexx_register_test_prep_targets(TESTS ${_test} TARGETS
      llcompiler_llrxc-c_${_mode} llcompiler_target_${_mode}
      llcompiler_address_contracts_${_mode}
      library classlib rxfnsc rxunicode_core fs cstream llcontract rxc rxas rxlink ${_address_vm})
  endforeach()
  # Required label arms share one source image for entry/host/error checks.
  foreach(_mode opt noopt)
    set(_label_vm rxbvm)
    set(_label_ir on)
    if(_mode STREQUAL "noopt")
      set(_label_ir off)
      if(TARGET rxtvm)
        set(_label_vm rxtvm)
      endif()
    endif()
    set(_test "llcompiler_c_labels_${_mode}_${_label_ir}_${_label_vm}")
    add_test(NAME ${_test} COMMAND ${Python3_EXECUTABLE}
      "${CMAKE_CURRENT_SOURCE_DIR}/tests/labels/check-labels.py"
      --build "${CMAKE_BINARY_DIR}"
      --work "${CMAKE_CURRENT_BINARY_DIR}/checks/${_test}"
      --mode ${_mode} --vm ${_label_vm} --ir ${_label_ir})
    set_tests_properties(${_test} PROPERTIES
      PASS_REGULAR_EXPRESSION "LABEL_CHECK[|].*phase=all[|].*commands=[0-9]+[|]PASS"
      TIMEOUT 1800 RUN_SERIAL TRUE RESOURCE_LOCK llcompiler_backend
      LABELS "library;level-l;levelc;compiler-frontends;compiler-backend;optimizer-parity")
    list(APPEND _ll_private_nested_tests ${_test})
    crexx_register_test_prep_targets(TESTS ${_test} TARGETS
      llcompiler_llrxc-c_${_mode} llcompiler_target_${_mode}
      llcompiler_label_contracts_${_mode}
      library classlib rxfnsc rxunicode_core fs cstream llcontract rxc rxas rxlink ${_label_vm})
  endforeach()
  # Implicit commands and owned warnings share the explicit command lifecycle.
  foreach(_mode opt noopt)
    set(_command_vm rxbvm)
    set(_command_ir on)
    if(_mode STREQUAL "noopt")
      set(_command_ir off)
      if(TARGET rxtvm)
        set(_command_vm rxtvm)
      endif()
    endif()
    set(_test "llcompiler_c_command_${_mode}_${_command_ir}_${_command_vm}")
    add_test(NAME ${_test} COMMAND ${Python3_EXECUTABLE}
      "${CMAKE_CURRENT_SOURCE_DIR}/tests/command/check-command.py"
      --build "${CMAKE_BINARY_DIR}"
      --work "${CMAKE_CURRENT_BINARY_DIR}/checks/${_test}"
      --mode ${_mode} --vm ${_command_vm} --ir ${_command_ir})
    set_tests_properties(${_test} PROPERTIES
      PASS_REGULAR_EXPRESSION "COMMAND_CHECK[|].*phase=all[|].*commands=[0-9]+[|]PASS"
      TIMEOUT 1800 RUN_SERIAL TRUE RESOURCE_LOCK llcompiler_backend
      LABELS "library;level-l;levelc;compiler-frontends;compiler-backend;optimizer-parity")
    list(APPEND _ll_private_nested_tests ${_test})
    crexx_register_test_prep_targets(TESTS ${_test} TARGETS
      llcompiler_llrxc-c_${_mode} llcompiler_target_${_mode}
      llcompiler_command_contracts_${_mode}
      library classlib rxfnsc rxunicode_core fs cstream llcontract rxc rxas rxlink ${_command_vm})
  endforeach()
  # Begin-only clock witnesses use one source/host execution per cell.
  foreach(_mode opt noopt)
    set(_clause_vm rxbvm)
    set(_clause_ir on)
    if(_mode STREQUAL "noopt")
      set(_clause_ir off)
      if(TARGET rxtvm)
        set(_clause_vm rxtvm)
      endif()
    endif()
    set(_test "llcompiler_c_clause_${_mode}_${_clause_ir}_${_clause_vm}")
    add_test(NAME ${_test} COMMAND ${Python3_EXECUTABLE}
      "${CMAKE_CURRENT_SOURCE_DIR}/tests/clause/check-clause.py"
      --build "${CMAKE_BINARY_DIR}"
      --work "${CMAKE_CURRENT_BINARY_DIR}/checks/${_test}"
      --mode ${_mode} --vm ${_clause_vm} --ir ${_clause_ir})
    set_tests_properties(${_test} PROPERTIES
      PASS_REGULAR_EXPRESSION "CLAUSE_CHECK[|].*phase=all[|].*commands=[0-9]+[|]PASS"
      TIMEOUT 1800 RUN_SERIAL TRUE RESOURCE_LOCK llcompiler_backend
      LABELS "library;level-l;levelc;compiler-frontends;compiler-backend;optimizer-parity")
    list(APPEND _ll_private_nested_tests ${_test})
    crexx_register_test_prep_targets(TESTS ${_test} TARGETS
      llcompiler_llrxc-c_${_mode} llcompiler_target_${_mode}
      llcompiler_clause_contracts_${_mode}
      library classlib rxfnsc rxunicode_core fs cstream llcontract rxc rxas rxlink ${_clause_vm})
  endforeach()
  # Complete practical TRACE uses one component lane. The runner can select
  # other modes for later formal QA; registration does not commission a matrix.
  set(_trace_test llcompiler_c_trace_opt_on_rxbvm)
  add_test(NAME ${_trace_test} COMMAND ${Python3_EXECUTABLE}
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/trace/check-trace.py"
    --build "${CMAKE_BINARY_DIR}"
    --work "${CMAKE_CURRENT_BINARY_DIR}/checks/${_trace_test}"
    --mode opt --vm rxbvm --ir on)
  set_tests_properties(${_trace_test} PROPERTIES
    PASS_REGULAR_EXPRESSION "TRACE_CHECK[|].*phase=all[|].*commands=[0-9]+[|]PASS"
    TIMEOUT 1800 RUN_SERIAL TRUE RESOURCE_LOCK llcompiler_backend
    LABELS "library;level-l;levelc;compiler-frontends;compiler-backend;trace")
  list(APPEND _ll_private_nested_tests ${_trace_test})
  crexx_register_test_prep_targets(TESTS ${_trace_test} TARGETS
    llcompiler_llrxc-c_opt llcompiler_target_opt llcompiler_trace_contracts_opt
    library classlib rxfnsc rxunicode_core fs cstream llcontract rxc rxas rxlink rxbvm)
  # One whole-SIGNAL runner owns retained static, condition and native controls.
  # Two complementary private cells use bounded ordinary admission; their
  # timeout is hang protection, not a performance acceptance threshold.
  foreach(_mode opt noopt)
    set(_signal_vm rxbvm)
    set(_signal_ir on)
    if(_mode STREQUAL "noopt")
      set(_signal_ir off)
      if(TARGET rxtvm)
        set(_signal_vm rxtvm)
      endif()
    endif()
    set(_test "llcompiler_c_signal_${_mode}_${_signal_ir}_${_signal_vm}")
    add_test(NAME ${_test} COMMAND ${Python3_EXECUTABLE}
      "${CMAKE_CURRENT_SOURCE_DIR}/tests/signal/check-conditions.py"
      --build "${CMAKE_BINARY_DIR}"
      --work "${CMAKE_CURRENT_BINARY_DIR}/checks/${_test}"
      --mode ${_mode} --vm ${_signal_vm} --ir ${_signal_ir})
    set_tests_properties(${_test} PROPERTIES
      PASS_REGULAR_EXPRESSION "SIGNAL_CONDITIONS_CHECK[|].*phase=all[|].*commands=[0-9]+[|]PASS"
      TIMEOUT 7200 RUN_SERIAL TRUE RESOURCE_LOCK llcompiler_backend
      LABELS "library;level-l;levelc;compiler-frontends;compiler-backend;optimizer-parity;signal")
    list(APPEND _ll_private_nested_tests ${_test})
    crexx_register_test_prep_targets(TESTS ${_test} TARGETS
      llcompiler_llrxc-c_${_mode} llcompiler_target_${_mode}
      llcompiler_signal_contracts_${_mode} llcompiler_signal_conditions_contracts_${_mode}
      run_tests_signal_origin
      library classlib rxfnsc rxunicode_core fs cstream llcontract rxc rxas rxlink ${_signal_vm})
  endforeach()
  # Conditions/observations is one complete development increment. Reuse one
  # prepared compiler, native fixtures and immutable expected outputs, on one VM.
  # Group 2 qualifies one prepared normal lane; full compiler QA stays later.
  set(_test "llcompiler_c_group2_opt_rxbvm")
  add_test(NAME ${_test} COMMAND ${Python3_EXECUTABLE}
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/group2/check-group2.py"
    --build "${CMAKE_BINARY_DIR}"
    --work "${CMAKE_CURRENT_BINARY_DIR}/checks/${_test}"
    --process-fixture $<TARGET_FILE:g2_process_fixture>
    --argv-fixture $<TARGET_FILE:address_argv_fixture>
    --spans-fixture $<TARGET_FILE:test_rxvml_spans>)
  set_tests_properties(${_test} PROPERTIES
    PASS_REGULAR_EXPRESSION "GROUP2_CHECK[|]PASS[|]cases=[0-9]+"
    TIMEOUT 3600 RUN_SERIAL TRUE RESOURCE_LOCK llcompiler_backend
    LABELS "library;level-l;levelc;compiler-backend")
  list(APPEND _ll_private_nested_tests ${_test})
  crexx_register_test_prep_targets(TESTS ${_test} TARGETS
    llcompiler_llrxc-c_opt llcompiler_target_opt llcompiler_call_external_contracts_opt llcdynamic
    library classlib rxfnsc rxunicode_core fs cstream llcontract _hash
    g2_process_fixture address_argv_fixture test_rxvml_spans rxc rxas rxlink rxbvm)
  # The static language/BIF crosswalk is deliberately one prepared component
  # lane. Formal compiler QA and other mode/platform coverage remain separate.
  set(_test "llcompiler_c_group3_opt_rxbvm")
  add_test(NAME ${_test} COMMAND ${Python3_EXECUTABLE}
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/group3/check-group3.py"
    --build "${CMAKE_BINARY_DIR}"
    --work "${CMAKE_CURRENT_BINARY_DIR}/checks/${_test}")
  set_tests_properties(${_test} PROPERTIES
    PASS_REGULAR_EXPRESSION "GROUP3_STATIC[|]checks=[0-9]+[|]PASS"
    TIMEOUT 7200 RUN_SERIAL TRUE RESOURCE_LOCK llcompiler_backend
    LABELS "library;level-l;levelc;compiler-backend")
  list(APPEND _ll_private_nested_tests ${_test})
  crexx_register_test_prep_targets(TESTS ${_test} TARGETS
    llcompiler_llrxc-c_opt llcompiler_target_opt
    library classlib rxfnsc rxunicode_core fs cstream llcontract rxc rxas rxlink rxbvm)
  set(_test "llcompiler_c_conditions_observations_opt_on_rxbvm")
  add_test(NAME ${_test} COMMAND ${Python3_EXECUTABLE}
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/conditions-observations/check-conditions.py"
    --build "${CMAKE_BINARY_DIR}"
    --work "${CMAKE_CURRENT_BINARY_DIR}/checks/${_test}")
  set_tests_properties(${_test} PROPERTIES
    PASS_REGULAR_EXPRESSION "CONDITIONS_OBSERVATIONS[|].*PASS"
    TIMEOUT 1800 RUN_SERIAL TRUE RESOURCE_LOCK llcompiler_backend
    LABELS "library;level-l;levelc;compiler-frontends;compiler-backend;conditions")
  list(APPEND _ll_private_nested_tests ${_test})
  crexx_register_test_prep_targets(TESTS ${_test} TARGETS
    llcompiler_conditions_observations_opt llcompiler_llrxc-c_opt llcompiler_target_opt
    library classlib rxfnsc rxunicode_core fs cstream llcontract rxc rxas rxlink rxbvm)
  # NUMERIC reuses one source image for settings, frame and operand-EXIT checks.
  # Complementary nested lanes are serialized with a generous hang guard.
  foreach(_mode opt noopt)
    set(_numeric_vm rxbvm)
    set(_numeric_ir on)
    if(_mode STREQUAL "noopt")
      set(_numeric_ir off)
      if(TARGET rxtvm)
        set(_numeric_vm rxtvm)
      endif()
    endif()
    set(_test "llcompiler_c_numeric_${_mode}_${_numeric_ir}_${_numeric_vm}")
    add_test(NAME ${_test} COMMAND ${Python3_EXECUTABLE}
      "${CMAKE_CURRENT_SOURCE_DIR}/tests/numeric/check-numeric.py"
      --build "${CMAKE_BINARY_DIR}"
      --work "${CMAKE_CURRENT_BINARY_DIR}/checks/${_test}"
      --mode ${_mode} --vm ${_numeric_vm} --ir ${_numeric_ir})
    set_tests_properties(${_test} PROPERTIES
      PASS_REGULAR_EXPRESSION "NUMERIC_CHECK[|].*phase=all[|].*commands=[0-9]+[|]PASS"
      TIMEOUT 1800 RUN_SERIAL TRUE RESOURCE_LOCK llcompiler_backend
      LABELS "library;level-l;levelc;compiler-frontends;compiler-backend;optimizer-parity")
    list(APPEND _ll_private_nested_tests ${_test})
    crexx_register_test_prep_targets(TESTS ${_test} TARGETS
      llcompiler_llrxc-c_${_mode} llcompiler_target_${_mode}
      llcompiler_numeric_contracts_${_mode}
      library classlib rxfnsc rxunicode_core fs cstream llcontract rxc rxas rxlink ${_numeric_vm})
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
  list(APPEND _ll_private_nested_tests llcompiler_c_drop)
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
  list(APPEND _ll_private_nested_tests llcompiler_c_assignment)
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
      fs cstream llcontract rxas rxlink rxbvm
    USES_TERMINAL VERBATIM)
  add_custom_target(llcompiler_c_calls_measure
    COMMAND ${CMAKE_COMMAND} -E time ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/tests/check-step07.py"
      --build "${CMAKE_BINARY_DIR}" --work "${CMAKE_CURRENT_BINARY_DIR}/c-calls-measure"
      --mode opt --vm rxbvm --c-only
    DEPENDS llcompiler_llrxc-c_opt llcompiler_target_opt library classlib rxfnsc rxunicode_core
      fs cstream llcontract rxas rxlink rxbvm
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
        list(APPEND _ll_private_nested_tests ${_calls})
        crexx_register_test_prep_targets(TESTS ${_calls}
          TARGETS llcompiler_llrxc-c_${_mode} llcompiler_target_${_mode}
            library classlib rxfnsc rxunicode_core fs cstream llcontract rxas rxlink ${_vm})
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
        list(APPEND _ll_private_nested_tests ${_test})
        crexx_register_test_prep_targets(TESTS ${_test}
          TARGETS llcompiler_llrxc-c_${_mode} llcompiler_target_${_mode}
            library classlib rxfnsc rxunicode_core fs cstream llcontract rxas rxlink ${_vm})
      endif()
    endforeach()
  endforeach()
endif()
if("c" IN_LIST _languages AND "g" IN_LIST _languages)
  include("${CMAKE_CURRENT_SOURCE_DIR}/tests/BackendTests.cmake")
endif()

if("c" IN_LIST _languages AND TARGET rxbvm)
  # One warm compiler and one VM cover the wider registered dynamic inventory.
  # Focused Debug preparation is costly; allow a generous full-inventory guard
  # and serialize it. Registration does not commission this full run or matrices.
  add_test(NAME llcompiler_c_dynamic_opt_rxbvm COMMAND ${Python3_EXECUTABLE}
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/dynamic/check-dynamic.py"
    --build "${CMAKE_BINARY_DIR}"
    --work "${CMAKE_CURRENT_BINARY_DIR}/checks/llcompiler_c_dynamic_opt_rxbvm")
  set_tests_properties(llcompiler_c_dynamic_opt_rxbvm PROPERTIES
    PASS_REGULAR_EXPRESSION "DYNAMIC_C[|]checks=[0-9]+[|]PASS"
    TIMEOUT 18000 RUN_SERIAL TRUE RESOURCE_LOCK llcompiler_backend
    LABELS "level-l;levelc;compiler;component")
  list(APPEND _ll_private_nested_tests llcompiler_c_dynamic_opt_rxbvm)
  crexx_register_test_prep_targets(TESTS llcompiler_c_dynamic_opt_rxbvm TARGETS
    llcompiler_llrxc-c_opt llcompiler_target_opt llcdynamic
    library classlib rxfnsc rxunicode_core fs cstream llcontract rxc rxas rxlink rxbvm)
  add_test(NAME llcompiler_c_interpreter_command_opt_rxbvm COMMAND ${Python3_EXECUTABLE}
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/dynamic/check-dynamic.py"
    --build "${CMAKE_BINARY_DIR}"
    --work "${CMAKE_CURRENT_BINARY_DIR}/checks/llcompiler_c_interpreter_command_opt_rxbvm"
    --phase entry)
  set_tests_properties(llcompiler_c_interpreter_command_opt_rxbvm PROPERTIES
    PASS_REGULAR_EXPRESSION "COMMAND_ENTRY[|]checks=[0-9]+[|]PASS"
    TIMEOUT 1200 LABELS "level-l;levelc;compiler;component;levelc-interaction")
  list(APPEND _ll_private_nested_tests llcompiler_c_interpreter_command_opt_rxbvm)
  crexx_register_test_prep_targets(TESTS llcompiler_c_interpreter_command_opt_rxbvm TARGETS
    llrexx llrxc-c llcompiler_llrxc-c_opt llcompiler_dynamic_command_opt rxbvm fs cstream llcontract)
  # Compact C-only interaction contrasts; the existing full default stays intact.
  # Prepared products are read-only and each invocation owns private scratch.
  foreach(_contrast compiler_noopt ir_off target_rxtvm assembler_noopt)
    set(_compact_mode opt)
    set(_compact_ir on)
    set(_compact_vm rxbvm)
    set(_compact_assembler opt)
    set(_compact_cases live-once child-signal-resumes-group source-call-expression-arguments)
    set(_compact_count 3)
    if(_contrast STREQUAL "compiler_noopt")
      set(_compact_mode noopt)
    elseif(_contrast STREQUAL "ir_off")
      set(_compact_ir off)
    elseif(_contrast STREQUAL "target_rxtvm")
      set(_compact_vm rxtvm)
    else()
      set(_compact_assembler noopt)
      set(_compact_cases live-once authored-trace)
      set(_compact_count 2)
    endif()
    if(TARGET ${_compact_vm})
      set(_compact_test "llcompiler_c_dynamic_${_contrast}")
      set(_compact_selectors)
      foreach(_case IN LISTS _compact_cases)
        list(APPEND _compact_selectors --only ${_case})
      endforeach()
      add_test(NAME ${_compact_test} COMMAND ${Python3_EXECUTABLE}
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/dynamic/check-dynamic.py"
        --build "${CMAKE_BINARY_DIR}"
        --work "${CMAKE_CURRENT_BINARY_DIR}/checks/${_compact_test}"
        --mode ${_compact_mode} --ir ${_compact_ir} --vm ${_compact_vm}
        --assembler ${_compact_assembler} ${_compact_selectors})
      set_tests_properties(${_compact_test} PROPERTIES
        PASS_REGULAR_EXPRESSION "DYNAMIC_C[|]checks=${_compact_count}[|]PASS"
        TIMEOUT 1800 LABELS "level-l;levelc;compiler;component;levelc-interaction")
      list(APPEND _ll_private_nested_tests ${_compact_test})
      set(_compact_engines rxbvm)
      if(NOT _compact_vm STREQUAL "rxbvm")
        list(APPEND _compact_engines ${_compact_vm})
      endif()
      crexx_register_test_prep_targets(TESTS ${_compact_test} TARGETS
        llcompiler_llrxc-c_${_compact_mode} llcompiler_target_${_compact_mode} llcdynamic
        library classlib rxfnsc rxunicode_core fs cstream llcontract rxc rxas rxlink
        ${_compact_engines})
    endif()
  endforeach()
  add_test(NAME llcompiler_c_infrastructure_contracts_opt_rxbvm
    COMMAND $<TARGET_FILE:rxbvm> "${CMAKE_CURRENT_BINARY_DIR}/backend/opt/infrastructure-contracts-linked.rxbin")
  set_tests_properties(llcompiler_c_infrastructure_contracts_opt_rxbvm PROPERTIES
    PASS_REGULAR_EXPRESSION "failures=0" TIMEOUT 300 LABELS "level-l;compiler;component")
  crexx_register_test_prep_targets(TESTS llcompiler_c_infrastructure_contracts_opt_rxbvm
    TARGETS llcompiler_infrastructure_contracts_opt rxbvm)
  add_test(NAME llcompiler_c_infrastructure_opt_rxbvm COMMAND ${Python3_EXECUTABLE}
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/infrastructure/check-infrastructure.py"
    --build "${CMAKE_BINARY_DIR}" --work "${CMAKE_CURRENT_BINARY_DIR}/infrastructure-check")
  set_tests_properties(llcompiler_c_infrastructure_opt_rxbvm PROPERTIES
    PASS_REGULAR_EXPRESSION "INFRASTRUCTURE_CONSUMERS[|].*[|]PASS"
    TIMEOUT 1800 RUN_SERIAL TRUE LABELS "level-l;compiler;component")
  list(APPEND _ll_private_nested_tests llcompiler_c_infrastructure_opt_rxbvm)
  crexx_register_test_prep_targets(TESTS llcompiler_c_infrastructure_opt_rxbvm
    TARGETS llcompiler_conditions_observations_opt llcompiler_target_opt rxbvm)
endif()
if("g" IN_LIST _languages AND TARGET rxbvm)
  add_test(NAME llcompiler_g_infrastructure_opt_rxbvm COMMAND ${Python3_EXECUTABLE}
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/check-backend.py"
    --build "${CMAKE_BINARY_DIR}" --work "${CMAKE_CURRENT_BINARY_DIR}/infrastructure-check"
    --mode opt --vm rxbvm --ir on --case calls-g --case scalars-g)
  set_tests_properties(llcompiler_g_infrastructure_opt_rxbvm PROPERTIES
    TIMEOUT 900 RUN_SERIAL TRUE LABELS "level-l;compiler;component")
  crexx_register_test_prep_targets(TESTS llcompiler_g_infrastructure_opt_rxbvm
    TARGETS llcompiler_llrxc-g_opt rxbvm)
endif()

if("c" IN_LIST _languages AND TARGET rxbvm)
  # STEP07 is a standalone execution feasibility component, not a compiler
  # rebuild or mode/VM matrix. The generated target consumes only its adapter.
  set(_execution_dir "${CMAKE_CURRENT_BINARY_DIR}/ir-execution")
  set(_execution_source "${CMAKE_CURRENT_SOURCE_DIR}/tests/ir-execution")
  set(_flags)
  set(_execution_inputs ${_base} "${CMAKE_BINARY_DIR}/bin/rxfnsl-analysis.rxbin"
    "${CMAKE_BINARY_DIR}/bin/rxfnsl-execute.rxbin" "${CMAKE_BINARY_DIR}/bin/rxfnsc.rxbin")
  ll_compile(classicbridge "${_execution_source}/classicbridge.crexx"
    "${_execution_dir}" ${_execution_inputs})
  ll_compile(runtime-contracts "${_execution_source}/runtime-contracts.crexx"
    "${_execution_dir}" ${_execution_inputs} "${_execution_dir}/classicbridge.rxbin")
  set(_execution_runtime ${_execution_inputs} "${_execution_dir}/classicbridge.rxbin"
    "${CMAKE_BINARY_DIR}/bin/classlib.rxbin" "${CMAKE_BINARY_DIR}/bin/rxunicode-core.rxbin")
  add_custom_command(OUTPUT "${_execution_dir}/runtime-contracts-linked.rxbin"
    COMMAND $<TARGET_FILE:rxlink> -s -o "${_execution_dir}/runtime-contracts-linked"
      "${_execution_dir}/runtime-contracts.rxbin" ${_execution_runtime}
    DEPENDS rxlink classlib rxunicode_core "${_execution_dir}/runtime-contracts.rxbin"
      ${_execution_runtime} VERBATIM)
  add_custom_target(llcompiler_ir_execution
    DEPENDS "${_execution_dir}/runtime-contracts-linked.rxbin")
  add_test(NAME llcompiler_ir_execution_opt_rxbvm COMMAND ${Python3_EXECUTABLE}
    "${_execution_source}/check-runtime.py" --build "${CMAKE_BINARY_DIR}")
  set_tests_properties(llcompiler_ir_execution_opt_rxbvm PROPERTIES
    PASS_REGULAR_EXPRESSION "IR_EXECUTION_RUNTIME[|]PASS" TIMEOUT 300 RUN_SERIAL TRUE
    LABELS "level-l;compiler;component")
  crexx_register_test_prep_targets(TESTS llcompiler_ir_execution_opt_rxbvm
    TARGETS llcompiler_ir_execution rxbvm)
endif()


# Explicit audited C cells only: private mutable work/read-only prepared inputs.
# floor(jobs/5)+1 admits at most four at j30; Windows/audit retain serial locks.
# The four direct prepared-language VM tests and all G owners stay unchanged.
if(BUILD_TESTING AND _ll_private_nested_tests)
  if(WIN32 OR CREXX_QA_AUDIT_NATIVE_SOURCES)
    set_tests_properties(${_ll_private_nested_tests} PROPERTIES
      RUN_SERIAL TRUE RESOURCE_LOCK llcompiler_backend)
  else()
    set_tests_properties(${_ll_private_nested_tests} PROPERTIES
      RUN_SERIAL FALSE PROCESSORS ${_ll_private_nested_processors} RESOURCE_LOCK "")
  endif()
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
  list(APPEND _selected llctarget llrexx)
endif()
crexx_register_test_prep_targets(TESTS llcompiler_product_inventory TARGETS ${_selected}
  rxfnsl rxfnsl_analysis rxfnsl_rxas rxfnsl_execute rxlang rxlanglib
  llrxastext llfileoutput llbinmeta)

# The generic test directory is configured before these selected host products.
# Check their actual member/import controls here without rebuilding in the test.
set(_package_closure_args)
foreach(_language IN LISTS _languages)
  list(APPEND _package_closure_args --language "${_language}")
endforeach()
add_test(NAME llcompiler_package_closures COMMAND ${Python3_EXECUTABLE}
  "${CMAKE_SOURCE_DIR}/langtools/tests/check-package-closures.py"
  --build "${CMAKE_BINARY_DIR}" --config "$<CONFIG>" ${_package_closure_args})
set_tests_properties(llcompiler_package_closures PROPERTIES
  PASS_REGULAR_EXPRESSION "LEVEL_L_PACKAGE_CLOSURES[|]PASS" TIMEOUT 60
  LABELS "level-l;compiler;component")
crexx_register_test_prep_targets(TESTS llcompiler_package_closures TARGETS ${_selected}
  rxfnsl rxfnsl_analysis rxfnsl_rxas rxfnsl_execute rxlang
  llrxastext llfileoutput llbinmeta)

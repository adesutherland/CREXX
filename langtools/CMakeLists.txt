cmake_minimum_required(VERSION 3.24)
if(CREXX_BUILD_RESEARCH_COMPILERS)
  # Selected compiler-host service only: no SDK/bootstrap/runtime dependency.
  include(${CMAKE_SOURCE_DIR}/rxpa/RXPluginFunction.cmake)
  add_dynamic_plugin_target(llcontract native/rxcontract.c PROVIDER_ID rxcontract)
  add_static_plugin_target(llcontract native/rxcontract.c PROVIDER_ID rxcontract)
  foreach(_ll_contract_target llcontract llcontract_static)
    target_include_directories(${_ll_contract_target} PRIVATE ${CMAKE_SOURCE_DIR}/binutils/include)
    target_link_libraries(${_ll_contract_target} PRIVATE rxbin)
  endforeach()
  set_target_properties(llcontract PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/providers
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/providers)
  add_rxpa_provider_package(llcontract PROVIDER_ID rxcontract LINK_TARGETS rxbin)
  install(FILES $<TARGET_FILE:llcontract> $<TARGET_FILE:llcontract_static>
    DESTINATION bin/providers COMPONENT ResearchCompilers EXCLUDE_FROM_ALL)
  install(FILES $<TARGET_FILE:llcontract_static> DESTINATION bin/providers
    RENAME rxcontract${CMAKE_STATIC_LIBRARY_SUFFIX} COMPONENT ResearchCompilers EXCLUDE_FROM_ALL)
  install(FILES ${CMAKE_BINARY_DIR}/bin/providers/rxcontract.native.json
    $<TARGET_LINKER_FILE:rxbin> DESTINATION bin/providers
    COMPONENT ResearchCompilers EXCLUDE_FROM_ALL)
  if(BUILD_TESTING)
    set(_ll_contract_test ${CMAKE_CURRENT_BINARY_DIR}/native-contract/projection)
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/native-contract)
    crexx_add_fixture_command(OUTPUT ${_ll_contract_test}.rxbin
      COMMAND $<TARGET_FILE:rxc> -x --no-exe-import
        -i "${CMAKE_BINARY_DIR}/bin;${CMAKE_BINARY_DIR}/bin/providers"
        -o ${_ll_contract_test} ${CMAKE_CURRENT_SOURCE_DIR}/tests/contract-projection.crexx
      COMMAND $<TARGET_FILE:rxas> ${_ll_contract_test}
      DEPENDS rxc rxas library llcontract ${CMAKE_CURRENT_SOURCE_DIR}/tests/contract-projection.crexx
      VERBATIM)
    add_custom_target(llcontract_test DEPENDS ${_ll_contract_test}.rxbin)
    # Reuse the maintained C-only object provider; no object consumer matrix.
    add_dependencies(llcontract_test _rxpa_objects)
    add_test(NAME llcontract_projection
      COMMAND $<TARGET_FILE:rxbvm> ${_ll_contract_test}.rxbin
        ${CMAKE_BINARY_DIR}/bin/library.rxbin $<TARGET_FILE:llcontract>
        -a $<TARGET_FILE:llcontract> $<TARGET_FILE:_rxpa_objects>)
    set_tests_properties(llcontract_projection PROPERTIES
      PASS_REGULAR_EXPRESSION "CONTRACT_PROJECT[|]checks=[0-9]+[|]failures=0"
      FAIL_REGULAR_EXPRESSION "FAIL[|]|PANIC:" LABELS "level-l;metadata;rxpa" TIMEOUT 60
      CREXX_PREP_TARGETS llcontract_test)
  endif()
endif()
set(_ll_flags)
if(CREXX_LEVEL_L_NOOPT)
  list(APPEND _ll_flags -n)
endif()
# The unchanged native toolchain and Level B library precede all three stages.
# No dependency on rxvme (which embeds Level G), compiler exits, or classlib.
if(CMAKE_CROSSCOMPILING)
  message(FATAL_ERROR "Level L bootstrap requires a native host-tools build. Cross-target bootstrap is not yet qualified; do not execute target tools during generation.")
endif()
if(TARGET rxtvm)
  set(_ll_vm rxtvm)
else()
  set(_ll_vm rxbvm)
endif()
set(_ll_root "${CMAKE_CURRENT_BINARY_DIR}/bootstrap")
set(_ll_spec "${CMAKE_CURRENT_SOURCE_DIR}/grammar/levell.levell")
set(_ll_front "${CMAKE_CURRENT_SOURCE_DIR}/rexx/llfront.crexx")
set(_ll_seed "${CMAKE_CURRENT_SOURCE_DIR}/bootstrap/seed-reader.inc")
set(_ll_prepare "${CMAKE_CURRENT_SOURCE_DIR}/bootstrap/prepare-front.cmake")
crexx_add_import_root(levell_base_imports
  DIRECTORY "${_ll_root}/imports"
  OUTPUT_VARIABLE _ll_imports
  FILES "${CMAKE_BINARY_DIR}/bin/library.rxbin" "${CMAKE_BINARY_DIR}/bin/rxfnsl.rxbin"
  DEPENDS library rxfnsl)
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/GeneratorStamp.cmake")
foreach(stage RANGE 0 2)
  set(dir "${_ll_root}/L${stage}")
  file(MAKE_DIRECTORY "${dir}/source")
  set(syntax)
  set(_ll_front_imports "${_ll_root}/imports")
  set(_ll_syntax_import_files)
  set(_ll_syntax_import_target)
  set(seed OFF)
  if(stage EQUAL 0)
    set(seed ON)
  else()
    math(EXPR previous "${stage} - 1")
    crexx_add_fixture_command(OUTPUT "${dir}/llsyntax.rxas"
      COMMAND ${CMAKE_COMMAND} -DLEVEL_L_VM=$<TARGET_FILE:${_ll_vm}>
        -DLEVEL_L_BYTECODE=${_ll_root}/L${previous}/rxlang-linked.rxbin
        -DSPEC=${_ll_spec} -DOUTPUT=${dir}/llsyntax.rxas -DFORMAT=rxas
        -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/generate.cmake"
      DEPENDS ${_ll_vm} "${_ll_root}/L${previous}/rxlang-linked.rxbin"
        "${_ll_spec}" "${CMAKE_CURRENT_SOURCE_DIR}/cmake/generate.cmake" VERBATIM)
    crexx_add_fixture_command(OUTPUT "${dir}/llsyntax.rxbin"
      COMMAND $<TARGET_FILE:rxas> ${_ll_flags} "${dir}/llsyntax"
      DEPENDS rxas "${dir}/llsyntax.rxas" VERBATIM)
    set(syntax "${dir}/llsyntax.rxbin")
    set(_ll_syntax_import_target levell_syntax_imports_${stage})
    crexx_add_import_root(${_ll_syntax_import_target}
      DIRECTORY "${dir}/syntax-imports"
      OUTPUT_VARIABLE _ll_syntax_import_files
      FILES "${syntax}"
      DEPENDS "${syntax}")
    string(APPEND _ll_front_imports ";${dir}/syntax-imports")
  endif()
  add_custom_command(OUTPUT "${dir}/source/llfront.crexx"
    COMMAND ${CMAKE_COMMAND} -DSOURCE=${_ll_front} -DSEED=${seed}
      -DSEED_SOURCE=${_ll_seed} -DOUTPUT=${dir}/source/llfront.crexx
      -P "${_ll_prepare}"
    DEPENDS "${_ll_front}" "${_ll_seed}" "${_ll_prepare}" VERBATIM)
  crexx_add_fixture_command(OUTPUT "${dir}/llfront.rxbin"
    COMMAND $<TARGET_FILE:rxc> ${_ll_flags} -x --no-exe-import
      -i "${_ll_front_imports}" -o "${dir}/llfront" "${dir}/source/llfront.crexx"
    COMMAND $<TARGET_FILE:rxas> ${_ll_flags} "${dir}/llfront"
    DEPENDS rxc rxas levell_base_imports ${_ll_imports} ${syntax}
      ${_ll_syntax_import_target} ${_ll_syntax_import_files} "${dir}/source/llfront.crexx"
    VERBATIM)
  # Never scan a directory containing an older linked driver for imports.
  # Its embedded metadata can shadow a changed frontend factory signature.
  crexx_add_import_root(levell_driver_imports_${stage}
    DIRECTORY "${dir}/driver-imports"
    OUTPUT_VARIABLE _ll_driver_import_files
    FILES "${dir}/llfront.rxbin" ${syntax}
    DEPENDS "${dir}/llfront.rxbin" ${syntax})
  crexx_add_fixture_command(OUTPUT "${dir}/llgenerate.rxbin"
    COMMAND $<TARGET_FILE:rxc> ${_ll_flags} -x --no-exe-import
      -i "${_ll_root}/imports;${dir}/driver-imports" -o "${dir}/llgenerate" "${_ll_prepared_generator}"
    COMMAND $<TARGET_FILE:rxas> ${_ll_flags} "${dir}/llgenerate"
    DEPENDS rxc rxas levell_generator_source levell_driver_imports_${stage} ${_ll_driver_import_files}
      ${_ll_imports} "${_ll_prepared_generator}" VERBATIM)
  crexx_add_import_root(levell_service_imports_${stage}
    DIRECTORY "${dir}/service-imports" OUTPUT_VARIABLE _ll_service_import_files
    FILES "${dir}/llgenerate.rxbin" DEPENDS "${dir}/llgenerate.rxbin")
  crexx_add_fixture_command(OUTPUT "${dir}/rxlang-linked.rxbin"
    BYPRODUCTS "${dir}/rxlang.rxas" "${dir}/rxlang.rxbin"
    COMMAND $<TARGET_FILE:rxc> ${_ll_flags} -x --no-exe-import
      -i "${_ll_root}/imports;${dir}/driver-imports;${dir}/service-imports" -o "${dir}/rxlang" "${CMAKE_CURRENT_SOURCE_DIR}/rxlang.crexx"
    COMMAND $<TARGET_FILE:rxas> ${_ll_flags} "${dir}/rxlang"
    COMMAND $<TARGET_FILE:rxlink> -s -o "${dir}/rxlang-linked" "${dir}/rxlang"
      "${dir}/llgenerate" "${dir}/llfront" ${syntax} "${CMAKE_BINARY_DIR}/bin/rxfnsl" "${CMAKE_BINARY_DIR}/bin/library"
    DEPENDS rxc rxas rxlink levell_service_imports_${stage} ${_ll_service_import_files} levell_driver_imports_${stage}
      ${_ll_driver_import_files} "${dir}/llfront.rxbin" ${syntax} ${_ll_imports}
      "${CMAKE_CURRENT_SOURCE_DIR}/rxlang.crexx" VERBATIM)
endforeach()
add_custom_command(OUTPUT "${_ll_root}/verified.stamp"
  COMMAND ${CMAKE_COMMAND} -E compare_files "${_ll_root}/L1/llsyntax.rxas" "${_ll_root}/L2/llsyntax.rxas"
  COMMAND ${CMAKE_COMMAND} -E touch "${_ll_root}/verified.stamp"
  DEPENDS "${_ll_root}/L1/llsyntax.rxas" "${_ll_root}/L2/llsyntax.rxas"
    "${_ll_root}/L2/rxlang-linked.rxbin" VERBATIM)
add_custom_target(rxlang_bootstrap DEPENDS "${_ll_root}/verified.stamp")
# The callable specification compiler is a tool library, separate from the SDK.
set(_ll_control "${_ll_root}/library.ctl")
file(GENERATE OUTPUT "${_ll_control}" CONTENT
  "INPUT ${_ll_root}/L2/llfront.rxbin\nINPUT ${_ll_root}/L2/llsyntax.rxbin\nINPUT ${_ll_root}/L2/llgenerate.rxbin\nINCLUDE llfront\nINCLUDE llsyntax\nINCLUDE llgenerate\nPRESERVE INLINE\nOUTPUT ${CMAKE_BINARY_DIR}/bin/rxlanglib\n")
crexx_add_fixture_command(OUTPUT "${CMAKE_BINARY_DIR}/bin/rxlanglib.rxbin"
  COMMAND $<TARGET_FILE:rxlink> -c "${_ll_control}"
  DEPENDS rxlink "${_ll_root}/verified.stamp" "${_ll_control}" VERBATIM)
add_custom_target(rxlanglib ALL DEPENDS "${CMAKE_BINARY_DIR}/bin/rxlanglib.rxbin")
# Keep the bootstrap's single owner before downstream Make consumers.
add_dependencies(rxlanglib rxlang_bootstrap)
if(NOT CREXX_LEVEL_L_STANDALONE)
add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/rxlang.c"
  COMMAND $<TARGET_FILE:rxcpack> -o "${CMAKE_CURRENT_BINARY_DIR}/rxlang" "${_ll_root}/L2/rxlang-linked"
  DEPENDS rxcpack "${_ll_root}/verified.stamp" "${_ll_root}/L2/rxlang-linked.rxbin" VERBATIM)
add_executable(rxlang "${CMAKE_CURRENT_BINARY_DIR}/rxlang.c")
target_link_libraries(rxlang rxvml rxpashim)
add_dependencies(rxlang rxlanglib)
endif()

if(UNIX AND NOT CREXX_LEVEL_L_STANDALONE)
  find_program(_ll_make NAMES gmake make)
  if(_ll_make)
    find_package(Python3 REQUIRED COMPONENTS Interpreter)
    add_custom_target(rxlang_bootstrap_order_measure
      COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/tests/check-bootstrap-build.py"
        --source "${CMAKE_SOURCE_DIR}" --work "${CMAKE_CURRENT_BINARY_DIR}/order-measure"
        --cmake "${CMAKE_COMMAND}" --make "${_ll_make}"
      VERBATIM)
    # Expanded stamp/order fixture: isolated Debug 32s. Source-only sanitizer
    # scheduling follows AGENTS.md. Tool stand-ins check ordering separately
    # from real-tool byte identity and execution.
    add_test(NAME rxlang_bootstrap_build_order
      COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/tests/check-bootstrap-build.py"
        --source "${CMAKE_SOURCE_DIR}" --work "${CMAKE_CURRENT_BINARY_DIR}/order-check"
        --cmake "${CMAKE_COMMAND}" --make "${_ll_make}")
    set_tests_properties(rxlang_bootstrap_build_order PROPERTIES
      PASS_REGULAR_EXPRESSION "LEVEL_L_BUILD_ORDER[|]PASS"
      RUN_SERIAL TRUE TIMEOUT 600 LABELS "level-l;cmake;optimizer-parity;comprehensive")
    crexx_schedule_directory_qa_tier_finalization()
  endif()
endif()

if(NOT CREXX_LEVEL_L_STANDALONE)
  add_subdirectory(common)
  add_subdirectory(examples)
  add_subdirectory(tests)
endif()

# The standalone graph produces bytecode without native product installation.
if(NOT CREXX_LEVEL_L_STANDALONE)
  crexx_install_targets(rxlang)
  crexx_install_bytecode(rxlanglib "${CMAKE_BINARY_DIR}/bin/rxlanglib.rxbin")
endif()

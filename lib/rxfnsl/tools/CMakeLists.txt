cmake_minimum_required(VERSION 3.24)
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
set(_ll_spec "${CMAKE_CURRENT_SOURCE_DIR}/../grammar/levell.levell")
set(_ll_front "${CMAKE_CURRENT_SOURCE_DIR}/../rexx/llfront.crexx")
set(_ll_seed "${CMAKE_CURRENT_SOURCE_DIR}/../bootstrap/seed-reader.inc")
set(_ll_prepare "${CMAKE_CURRENT_SOURCE_DIR}/../bootstrap/prepare-front.cmake")
crexx_add_import_root(levell_base_imports
  DIRECTORY "${_ll_root}/imports"
  OUTPUT_VARIABLE _ll_imports
  FILES "${CMAKE_BINARY_DIR}/bin/library.rxbin" "${CMAKE_BINARY_DIR}/bin/rxfnsl-base.rxbin"
  DEPENDS library rxfnsl_base)
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
    add_custom_command(OUTPUT "${dir}/llsyntax.rxas"
      COMMAND ${CMAKE_COMMAND} -DLEVEL_L_VM=$<TARGET_FILE:${_ll_vm}>
        -DLEVEL_L_BYTECODE=${_ll_root}/L${previous}/levell-linked.rxbin
        -DSPEC=${_ll_spec} -DOUTPUT=${dir}/llsyntax.rxas -DFORMAT=rxas
        -P "${CMAKE_CURRENT_SOURCE_DIR}/generate.cmake"
      DEPENDS ${_ll_vm} "${_ll_root}/L${previous}/levell-linked.rxbin"
        "${_ll_spec}" "${CMAKE_CURRENT_SOURCE_DIR}/generate.cmake" VERBATIM)
    add_custom_command(OUTPUT "${dir}/llsyntax.rxbin"
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
  add_custom_command(OUTPUT "${dir}/llfront.rxbin"
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
  add_custom_command(OUTPUT "${dir}/levell-linked.rxbin"
    BYPRODUCTS "${dir}/levell.rxas" "${dir}/levell.rxbin"
    COMMAND $<TARGET_FILE:rxc> ${_ll_flags} -x --no-exe-import
      -i "${_ll_root}/imports;${dir}/driver-imports" -o "${dir}/levell" "${CMAKE_CURRENT_SOURCE_DIR}/levell.crexx"
    COMMAND $<TARGET_FILE:rxas> ${_ll_flags} "${dir}/levell"
    COMMAND $<TARGET_FILE:rxlink> -s -o "${dir}/levell-linked" "${dir}/levell"
      "${dir}/llfront" ${syntax} "${CMAKE_BINARY_DIR}/bin/rxfnsl-base" "${CMAKE_BINARY_DIR}/bin/library"
    DEPENDS rxc rxas rxlink levell_driver_imports_${stage}
      ${_ll_driver_import_files} "${dir}/llfront.rxbin" ${syntax} ${_ll_imports}
      "${CMAKE_CURRENT_SOURCE_DIR}/levell.crexx" VERBATIM)
endforeach()
add_custom_command(OUTPUT "${_ll_root}/verified.stamp"
  COMMAND ${CMAKE_COMMAND} -E compare_files "${_ll_root}/L1/llsyntax.rxas" "${_ll_root}/L2/llsyntax.rxas"
  COMMAND ${CMAKE_COMMAND} -E touch "${_ll_root}/verified.stamp"
  DEPENDS "${_ll_root}/L1/llsyntax.rxas" "${_ll_root}/L2/llsyntax.rxas"
    "${_ll_root}/L2/levell-linked.rxbin" VERBATIM)
add_custom_target(levell_bootstrap DEPENDS "${_ll_root}/verified.stamp")
# Public library contains the generated frontend only. The seed is not shipped.
set(_ll_control "${_ll_root}/library.ctl")
set(_ll_control_text "")
foreach(module IN LISTS RXFNSL_BASE_BINS)
  get_filename_component(name "${module}" NAME_WE)
  string(APPEND _ll_control_text "INPUT ${module}\nINCLUDE ${name}\n")
endforeach()
string(APPEND _ll_control_text
  "INPUT ${_ll_root}/L2/llfront.rxbin\nINPUT ${_ll_root}/L2/llsyntax.rxbin\nINCLUDE llfront\nINCLUDE llsyntax\nPRESERVE INLINE\nOUTPUT ${CMAKE_BINARY_DIR}/bin/rxfnsl\n")
file(GENERATE OUTPUT "${_ll_control}" CONTENT "${_ll_control_text}")
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/bin/rxfnsl.rxbin"
  COMMAND $<TARGET_FILE:rxlink> -c "${_ll_control}"
  DEPENDS rxlink ${RXFNSL_BASE_BINS} "${_ll_root}/verified.stamp" "${_ll_control}"
  VERBATIM)
add_custom_target(rxfnsl ALL DEPENDS "${CMAKE_BINARY_DIR}/bin/rxfnsl.rxbin")
if(NOT CREXX_LEVEL_L_STANDALONE)
add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/levell.c"
  COMMAND $<TARGET_FILE:rxcpack> -o "${CMAKE_CURRENT_BINARY_DIR}/levell" "${_ll_root}/L2/levell-linked"
  DEPENDS rxcpack "${_ll_root}/verified.stamp" "${_ll_root}/L2/levell-linked.rxbin" VERBATIM)
add_executable(levell_cli "${CMAKE_CURRENT_BINARY_DIR}/levell.c")
set_target_properties(levell_cli PROPERTIES OUTPUT_NAME levell)
target_link_libraries(levell_cli rxvml rxpashim)
add_dependencies(levell_cli rxfnsl)
endif()

# The Unicode builder is a downstream consumer of the completed L2 tool.
set(_gennorm_spec "${CMAKE_CURRENT_SOURCE_DIR}/../grammar/gennorm2.levell")
set(_gennorm_rxas "${CMAKE_CURRENT_BINARY_DIR}/gennorm2_language.rxas")
add_custom_command(OUTPUT "${_gennorm_rxas}"
  COMMAND ${CMAKE_COMMAND} -DLEVEL_L_VM=$<TARGET_FILE:${_ll_vm}>
    -DLEVEL_L_BYTECODE=${_ll_root}/L2/levell-linked.rxbin
    -DSPEC=${_gennorm_spec} -DOUTPUT=${_gennorm_rxas} -DFORMAT=rxas
    -P "${CMAKE_CURRENT_SOURCE_DIR}/generate.cmake"
  DEPENDS ${_ll_vm} "${_ll_root}/verified.stamp" "${_ll_root}/L2/levell-linked.rxbin" "${_gennorm_spec}" "${CMAKE_CURRENT_SOURCE_DIR}/generate.cmake" VERBATIM)
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/bin/gennorm2_language.rxbin"
  COMMAND $<TARGET_FILE:rxas> -o "${CMAKE_BINARY_DIR}/bin/gennorm2_language" "${_gennorm_rxas}"
  DEPENDS rxas "${_gennorm_rxas}" VERBATIM)
add_custom_target(levell_unicode_lexer DEPENDS "${CMAKE_BINARY_DIR}/bin/gennorm2_language.rxbin")

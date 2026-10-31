cmake_minimum_required(VERSION 3.24)

crexx_add_import_root(llrxinline_imports
  DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/imports" OUTPUT_VARIABLE _imports
  FILES "${CMAKE_BINARY_DIR}/bin/library.rxbin"
    "${CMAKE_BINARY_DIR}/bin/rxfnsl.rxbin"
    "${CMAKE_BINARY_DIR}/bin/rxfnsl-rxas.rxbin"
    "${CMAKE_BINARY_DIR}/bin/rxfnsl-target.rxbin"
    "${CMAKE_BINARY_DIR}/bin/rxfnsl-target-io.rxbin"
    "${CMAKE_BINARY_DIR}/langtools/common/llfileoutput.rxbin"
    "${CMAKE_BINARY_DIR}/bin/rxfs.rxplugin"
  DEPENDS library rxfnsl rxfnsl_rxas rxfnsl_target rxfnsl_target_io llfileoutput fs)
crexx_add_fixture_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/llrxinlinestage.rxbin"
  COMMAND $<TARGET_FILE:rxc> -x --no-exe-import
    -i "${CMAKE_CURRENT_BINARY_DIR}/imports"
    -o "${CMAKE_CURRENT_BINARY_DIR}/llrxinlinestage"
    "${CMAKE_CURRENT_SOURCE_DIR}/llrxinlinestage.crexx"
  COMMAND $<TARGET_FILE:rxas> "${CMAKE_CURRENT_BINARY_DIR}/llrxinlinestage"
  DEPENDS rxc rxas llrxinline_imports ${_imports}
    "${CMAKE_CURRENT_SOURCE_DIR}/llrxinlinestage.crexx" VERBATIM)
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/objects")
crexx_add_fixture_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/objects/llrxinline.rxbin"
  COMMAND $<TARGET_FILE:rxc> -x --no-exe-import
    -i "${CMAKE_CURRENT_BINARY_DIR}/imports;${CMAKE_CURRENT_BINARY_DIR}"
    -o "${CMAKE_CURRENT_BINARY_DIR}/objects/llrxinline"
    "${CMAKE_CURRENT_SOURCE_DIR}/llrxinline.crexx"
  COMMAND $<TARGET_FILE:rxas> "${CMAKE_CURRENT_BINARY_DIR}/objects/llrxinline"
  DEPENDS rxc rxas llrxinline_imports ${_imports} "${CMAKE_CURRENT_BINARY_DIR}/llrxinlinestage.rxbin"
    "${CMAKE_CURRENT_SOURCE_DIR}/llrxinline.crexx" VERBATIM)
set(_service_control "${CMAKE_CURRENT_BINARY_DIR}/llrxinline.ctl")
file(GENERATE OUTPUT "${_service_control}" CONTENT
  "INPUT ${CMAKE_CURRENT_BINARY_DIR}/objects/llrxinline.rxbin\nINPUT ${CMAKE_CURRENT_BINARY_DIR}/llrxinlinestage.rxbin\nINCLUDE llrxinline\nINCLUDE llrxinlinestage\nPRESERVE INLINE\nOUTPUT ${CMAKE_CURRENT_BINARY_DIR}/llrxinline\n")
crexx_add_fixture_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/llrxinline.rxbin"
  COMMAND $<TARGET_FILE:rxlink> -c "${_service_control}"
  DEPENDS rxlink "${_service_control}" "${CMAKE_CURRENT_BINARY_DIR}/objects/llrxinline.rxbin"
    "${CMAKE_CURRENT_BINARY_DIR}/llrxinlinestage.rxbin" VERBATIM)
add_custom_target(llrxinline
  DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/llrxinline.rxbin")
set_property(TARGET llrxinline PROPERTY LLRXINLINE_BYTECODE
  "${CMAKE_CURRENT_BINARY_DIR}/llrxinline.rxbin")

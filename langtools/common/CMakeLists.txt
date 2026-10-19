foreach(_module llrxastext llfileoutput llbinmeta)
  set(_deps library rxfnsl)
  set(_files "${CMAKE_BINARY_DIR}/bin/library.rxbin" "${CMAKE_BINARY_DIR}/bin/rxfnsl.rxbin")
  if(_module STREQUAL "llfileoutput")
    list(APPEND _deps fs)
    list(APPEND _files "${CMAKE_BINARY_DIR}/bin/rxfs.rxplugin")
  endif()
  crexx_add_import_root(${_module}_imports DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${_module}-imports"
    OUTPUT_VARIABLE _imports FILES ${_files} DEPENDS ${_deps})
  add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${_module}.rxbin"
    COMMAND $<TARGET_FILE:rxc> -x --no-exe-import -i "${CMAKE_CURRENT_BINARY_DIR}/${_module}-imports"
      -o "${CMAKE_CURRENT_BINARY_DIR}/${_module}" "${CMAKE_CURRENT_SOURCE_DIR}/${_module}.crexx"
    COMMAND $<TARGET_FILE:rxas> "${CMAKE_CURRENT_BINARY_DIR}/${_module}"
    DEPENDS rxc rxas ${_module}_imports ${_imports} "${_module}.crexx" VERBATIM)
  add_custom_target(${_module} ALL DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/${_module}.rxbin")
  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${_module}.rxbin" DESTINATION bin)
endforeach()

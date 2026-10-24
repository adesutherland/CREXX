# A maintained teaching module, deliberately absent from the SDK package.
set(_tinyexpr "${CMAKE_CURRENT_BINARY_DIR}/tinyexpr")
set(_tinyexpr_import_root "${CMAKE_CURRENT_BINARY_DIR}/imports/tinyexpr")
crexx_add_import_root(rxlang_example_tinyexpr_imports
  DIRECTORY "${_tinyexpr_import_root}"
  OUTPUT_VARIABLE _tinyexpr_import_files
  FILES "${CMAKE_BINARY_DIR}/bin/library.rxbin"
  DEPENDS library)
add_custom_command(OUTPUT "${_tinyexpr}.rxbin"
  COMMAND $<TARGET_FILE:rxc> -x --no-exe-import
    -i "${_tinyexpr_import_root}" -o "${_tinyexpr}"
    "${CMAKE_CURRENT_SOURCE_DIR}/tinyexpr/tinyexpr.crexx"
  COMMAND $<TARGET_FILE:rxas> "${_tinyexpr}"
  DEPENDS rxc rxas rxlang_example_tinyexpr_imports ${_tinyexpr_import_files}
    "${CMAKE_CURRENT_SOURCE_DIR}/tinyexpr/tinyexpr.crexx" VERBATIM)
add_custom_target(rxlang_example_tinyexpr ALL DEPENDS "${_tinyexpr}.rxbin")

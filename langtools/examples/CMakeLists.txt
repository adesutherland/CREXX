# A maintained teaching module, deliberately absent from the SDK package.
set(_tinyexpr "${CMAKE_CURRENT_BINARY_DIR}/tinyexpr")
add_custom_command(OUTPUT "${_tinyexpr}.rxbin"
  COMMAND $<TARGET_FILE:rxc> -x --no-exe-import
    -i "${CMAKE_BINARY_DIR}/bin" -o "${_tinyexpr}"
    "${CMAKE_CURRENT_SOURCE_DIR}/tinyexpr/tinyexpr.crexx"
  COMMAND $<TARGET_FILE:rxas> "${_tinyexpr}"
  DEPENDS library rxc rxas "${CMAKE_BINARY_DIR}/bin/library.rxbin"
    "${CMAKE_CURRENT_SOURCE_DIR}/tinyexpr/tinyexpr.crexx" VERBATIM)
add_custom_target(rxlang_example_tinyexpr ALL DEPENDS "${_tinyexpr}.rxbin")

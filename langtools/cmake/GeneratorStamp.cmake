# This closure is entirely pre-L0 and shared by all three prepared services.
get_filename_component(_ll_stamp_root "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)
include("${CMAKE_CURRENT_LIST_DIR}/generator-inputs.cmake")
file(GLOB_RECURSE _ll_stamp_discovered CONFIGURE_DEPENDS RELATIVE "${_ll_stamp_root}"
  "${_ll_stamp_root}/langtools/rexx/*.crexx"
  "${_ll_stamp_root}/lib/rxfnsl/rexx/*.crexx"
  "${_ll_stamp_root}/lib/rxfnsl/unicode/*.crexx"
  "${_ll_stamp_root}/lib/rxfnsl/unicode/*.cmake")
foreach(_path IN LISTS _ll_stamp_discovered)
  if(NOT _path IN_LIST LL_GENERATOR_STAMP_SOURCES)
    message(FATAL_ERROR "Generator stamp closure omits maintained module: ${_path}")
  endif()
endforeach()
set(_ll_stamp_sources)
foreach(_path IN LISTS LL_GENERATOR_STAMP_SOURCES)
  if(NOT EXISTS "${_ll_stamp_root}/${_path}")
    message(FATAL_ERROR "Missing generator stamp source: ${_path}")
  endif()
  list(APPEND _ll_stamp_sources "${_ll_stamp_root}/${_path}")
endforeach()
set(_ll_prepared_generator "${_ll_root}/source/llgenerate.crexx")
file(MAKE_DIRECTORY "${_ll_root}/source")
set(_ll_stamp_flags "${_ll_root}/source/generator-flags.txt")
file(GENERATE OUTPUT "${_ll_stamp_flags}" CONTENT
  "rxc=${_ll_flags} -x --no-exe-import\nrxas=${_ll_flags}\nrxlink=-s\nconfig=${CMAKE_BUILD_TYPE}\nnative=${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_DEBUG} ${CMAKE_C_FLAGS_RELEASE} ${CMAKE_C_FLAGS_RELWITHDEBINFO} ${CMAKE_C_FLAGS_MINSIZEREL}\n")
add_custom_command(OUTPUT "${_ll_prepared_generator}"
  BYPRODUCTS "${_ll_root}/source/generator.identity"
  COMMAND ${CMAKE_COMMAND} "-DROOT=${_ll_stamp_root}"
    "-DLIBRARY=${CMAKE_BINARY_DIR}/bin/library.rxbin"
    "-DSDK=${CMAKE_BINARY_DIR}/bin/rxfnsl.rxbin"
    "-DCOMPILER=$<TARGET_FILE:rxc>" "-DASSEMBLER=$<TARGET_FILE:rxas>"
    "-DLINKER=$<TARGET_FILE:rxlink>" "-DVM=$<TARGET_FILE:${_ll_vm}>"
    "-DFLAGS=${_ll_stamp_flags}" "-DOUTPUT=${_ll_prepared_generator}"
    -P "${CMAKE_CURRENT_LIST_DIR}/prepare-generator.cmake"
  DEPENDS library rxfnsl rxc rxas rxlink ${_ll_vm} ${_ll_stamp_sources}
    "${CMAKE_BINARY_DIR}/bin/library.rxbin" "${CMAKE_BINARY_DIR}/bin/rxfnsl.rxbin"
    "${_ll_stamp_flags}" VERBATIM)
add_custom_target(levell_generator_source DEPENDS "${_ll_prepared_generator}")

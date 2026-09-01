cmake_minimum_required(VERSION 3.24)

set(RXFNSC_DIRECT_BIF_MODULES
  RexxClassicBifAbbrev
  RexxClassicBifAbs
  RexxClassicBifAddress
  RexxClassicBifB2x
  RexxClassicBifBitand
  RexxClassicBifBitor
  RexxClassicBifBitxor
  RexxClassicBifC2d
  RexxClassicBifC2x
  RexxClassicBifCenter
  RexxClassicBifChangestr
  RexxClassicBifCompare
  RexxClassicBifCopies
  RexxClassicBifCountstr
  RexxClassicBifD2x
  RexxClassicBifD2c
  RexxClassicBifDate
  RexxClassicBifDatatype
  RexxClassicBifDelstr
  RexxClassicBifDelword
  RexxClassicBifFormat
  RexxClassicBifInsert
  RexxClassicBifLastpos
  RexxClassicBifLeft
  RexxClassicBifLength
  RexxClassicBifMax
  RexxClassicBifMin
  RexxClassicBifNumeric
  RexxClassicBifOverlay
  RexxClassicBifPos
  RexxClassicBifRandom
  RexxClassicBifReverse
  RexxClassicBifRight
  RexxClassicBifSign
  RexxClassicBifSpace
  RexxClassicBifStrip
  RexxClassicBifSubstr
  RexxClassicBifSubword
  RexxClassicBifSymbol
  RexxClassicBifTime
  RexxClassicBifTrace
  RexxClassicBifTranslate
  RexxClassicBifTrunc
  RexxClassicBifValue
  RexxClassicBifVerify
  RexxClassicBifWord
  RexxClassicBifWordindex
  RexxClassicBifWordlength
  RexxClassicBifWordpos
  RexxClassicBifWords
  RexxClassicBifX2b
  RexxClassicBifX2c
  RexxClassicBifX2d
  RexxClassicBifXrange
)

set(RXFNSC_MODULES
  RexxValue
  RexxStem
  RexxClassicConfig
  RexxClassicDatatype
  RexxClassicCharacterScan
  RexxClassicCharacterOps
  RexxClassicEncoding
  RexxClassicState
  RexxVariablePool
  RexxClassicBifs
  RexxClassicBitFunctions
  ${RXFNSC_DIRECT_BIF_MODULES}
)

set(RXFNSC_BINS)
set(RXFNSC_MEMBER_ROOT "${CMAKE_CURRENT_BINARY_DIR}/members")
foreach(MOD IN LISTS RXFNSC_MODULES)
  list(APPEND RXFNSC_BINS
    "${RXFNSC_MEMBER_ROOT}/${MOD}/${MOD}.rxbin")
endforeach()

set(RXFNSC_MEMBER_BUILD_DEPS
  rxc
  rxas
  rxfnsc_imports
)

set(RXFNSC_IMPORT_ROOT "${CMAKE_CURRENT_BINARY_DIR}/imports")
set(RXFNSC_IMPORT_LIBRARY "${RXFNSC_IMPORT_ROOT}/library.rxbin")
set(RXFNSC_IMPORT_CLASSLIB "${RXFNSC_IMPORT_ROOT}/classlib.rxbin")
set(RXFNSC_IMPORT_EXITS "${RXFNSC_IMPORT_ROOT}/rxcexits.rxbin")
add_custom_command(
  OUTPUT
    "${RXFNSC_IMPORT_LIBRARY}"
    "${RXFNSC_IMPORT_CLASSLIB}"
    "${RXFNSC_IMPORT_EXITS}"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${RXFNSC_IMPORT_ROOT}"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_BINARY_DIR}/bin/library.rxbin"
    "${RXFNSC_IMPORT_LIBRARY}"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_BINARY_DIR}/bin/classlib.rxbin"
    "${RXFNSC_IMPORT_CLASSLIB}"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_BINARY_DIR}/bin/rxcexits.rxbin"
    "${RXFNSC_IMPORT_EXITS}"
  DEPENDS
    library
    classlib
    compiler_exit_bin
    "${CMAKE_BINARY_DIR}/bin/library.rxbin"
    "${CMAKE_BINARY_DIR}/bin/classlib.rxbin"
    "${CMAKE_BINARY_DIR}/bin/rxcexits.rxbin"
  COMMENT "Staging immutable rxfnsc compiler imports ..."
  VERBATIM
)
add_custom_target(rxfnsc_imports
  DEPENDS
    "${RXFNSC_IMPORT_LIBRARY}"
    "${RXFNSC_IMPORT_CLASSLIB}"
    "${RXFNSC_IMPORT_EXITS}"
)

set(RXFNSC_SELECTORS ${RXFNSC_MODULES})
set(RXFNSC_DEPS_RexxStem RexxValue)
set(RXFNSC_DEPS_RexxClassicState RexxValue)
set(RXFNSC_DEPS_RexxVariablePool RexxValue RexxStem RexxClassicState)
set(RXFNSC_DEPS_RexxClassicBifs RexxValue RexxStem RexxClassicConfig RexxClassicDatatype RexxClassicCharacterScan RexxClassicEncoding RexxClassicBitFunctions RexxClassicState RexxVariablePool)
set(RXFNSC_DEPS_RexxClassicConfig RexxValue)
set(RXFNSC_DEPS_RexxClassicDatatype RexxValue RexxClassicConfig)
set(RXFNSC_DEPS_RexxClassicEncoding RexxValue RexxClassicConfig)
set(RXFNSC_DEPS_RexxClassicCharacterOps RexxValue RexxClassicConfig RexxClassicCharacterScan)
set(RXFNSC_DEPS_RexxClassicBitFunctions RexxClassicEncoding)
foreach(MOD IN LISTS RXFNSC_DIRECT_BIF_MODULES)
  set(RXFNSC_DEPS_${MOD} RexxClassicBifs)
endforeach()
set(RXFNSC_DEPS_RexxClassicBifBitand RexxClassicBitFunctions)
set(RXFNSC_DEPS_RexxClassicBifBitor RexxClassicBitFunctions)
set(RXFNSC_DEPS_RexxClassicBifBitxor RexxClassicBitFunctions)
set(RXFNSC_DEPS_RexxClassicBifC2d RexxClassicBifs RexxClassicEncoding RexxClassicBifX2d)
set(RXFNSC_DEPS_RexxClassicBifC2x RexxClassicBifs RexxClassicEncoding)
set(RXFNSC_DEPS_RexxClassicBifD2c RexxClassicBifs RexxClassicEncoding RexxClassicBifD2x)
set(RXFNSC_DEPS_RexxClassicBifTranslate RexxClassicBifs RexxClassicEncoding)
set(RXFNSC_DEPS_RexxClassicBifDatatype RexxClassicBifs RexxClassicDatatype)
set(RXFNSC_DEPS_RexxClassicBifSymbol RexxClassicBifs RexxClassicDatatype)
set(RXFNSC_DEPS_RexxClassicBifX2c RexxClassicBifs RexxClassicEncoding)
set(RXFNSC_DEPS_RexxClassicBifXrange RexxClassicBifs RexxClassicEncoding)
foreach(MOD RexxClassicBifDelword RexxClassicBifSubword RexxClassicBifWord RexxClassicBifWordindex RexxClassicBifWordlength RexxClassicBifWordpos RexxClassicBifWords)
  set(RXFNSC_DEPS_${MOD} RexxClassicBifs RexxClassicCharacterScan)
endforeach()
foreach(MOD RexxClassicBifLastpos RexxClassicBifLeft RexxClassicBifLength RexxClassicBifPos RexxClassicBifReverse RexxClassicBifRight RexxClassicBifSubstr)
  set(RXFNSC_DEPS_${MOD} RexxClassicBifs RexxClassicCharacterOps)
endforeach()

foreach(MOD RexxClassicBifAbbrev RexxClassicBifCountstr)
  set(RXFNSC_DEPS_${MOD} RexxClassicBifs RexxClassicCharacterOps)
endforeach()

set(RXFNSC_DEPS_RexxClassicBifChangestr RexxClassicBifs RexxClassicCharacterOps RexxClassicCharacterScan)
set(RXFNSC_DEPS_RexxClassicBifCenter RexxClassicBifs RexxClassicCharacterOps RexxClassicCharacterScan)
set(RXFNSC_DEPS_RexxClassicBifDelstr RexxClassicBifs RexxClassicCharacterScan)
foreach(MOD RexxClassicBifInsert RexxClassicBifOverlay)
  set(RXFNSC_DEPS_${MOD} RexxClassicBifs RexxClassicCharacterOps RexxClassicCharacterScan)
endforeach()

foreach(MOD RexxClassicBifSpace RexxClassicBifStrip)
  set(RXFNSC_DEPS_${MOD} RexxClassicBifs RexxClassicCharacterScan)
endforeach()
list(APPEND RXFNSC_DEPS_RexxClassicBifSpace RexxClassicCharacterOps)

foreach(_binfile IN LISTS RXFNSC_BINS)
  get_filename_component(_basename "${_binfile}" NAME_WE)
  get_filename_component(_member_dir "${_binfile}" DIRECTORY)
  # Custom-command WORKING_DIRECTORY must exist before Make enters the recipe;
  # do not rely on Ninja's output-directory materialization.
  file(MAKE_DIRECTORY "${_member_dir}")
  set(_rxfnsc_source_deps)
  set(_rxfnsc_staged_sources)
  set(_rxfnsc_stage_source_commands)
  set(_rxfnsc_dep_closure ${RXFNSC_DEPS_${_basename}})
  set(_rxfnsc_dep_index 0)
  list(LENGTH _rxfnsc_dep_closure _rxfnsc_dep_count)
  while(_rxfnsc_dep_index LESS _rxfnsc_dep_count)
    list(GET _rxfnsc_dep_closure ${_rxfnsc_dep_index} _rxfnsc_dep)
    foreach(_rxfnsc_transitive_dep IN LISTS RXFNSC_DEPS_${_rxfnsc_dep})
      if(NOT _rxfnsc_transitive_dep IN_LIST _rxfnsc_dep_closure)
        list(APPEND _rxfnsc_dep_closure "${_rxfnsc_transitive_dep}")
      endif()
    endforeach()
    math(EXPR _rxfnsc_dep_index "${_rxfnsc_dep_index} + 1")
    list(LENGTH _rxfnsc_dep_closure _rxfnsc_dep_count)
  endwhile()
  string(JOIN ";" _rxfnsc_dep_signature_input ${_rxfnsc_dep_closure})
  string(SHA256 _rxfnsc_dep_signature
    "${_basename};${_rxfnsc_dep_signature_input}")
  string(SUBSTRING "${_rxfnsc_dep_signature}" 0 12 _rxfnsc_dep_signature_short)
  set(_member_source_root
    "${_member_dir}/sources-${_rxfnsc_dep_signature_short}")
  set(_member_source "${_member_source_root}/${_basename}.crexx")
  foreach(_dep IN LISTS _rxfnsc_dep_closure)
    set(_rxfnsc_dep_source "${CMAKE_CURRENT_SOURCE_DIR}/${_dep}.crexx")
    set(_rxfnsc_staged_source "${_member_source_root}/${_dep}.crexx")
    list(APPEND _rxfnsc_source_deps "${_rxfnsc_dep_source}")
    list(APPEND _rxfnsc_staged_sources "${_rxfnsc_staged_source}")
    list(APPEND _rxfnsc_stage_source_commands
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${_rxfnsc_dep_source}"
        "${_rxfnsc_staged_source}")
  endforeach()
  add_custom_command(
    OUTPUT "${_binfile}"
    BYPRODUCTS
      "${_member_source}"
      "${_member_dir}/${_basename}.rxas"
      ${_rxfnsc_staged_sources}
    COMMAND ${CMAKE_COMMAND} -E make_directory "${_member_source_root}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "${CMAKE_CURRENT_SOURCE_DIR}/${_basename}.crexx"
      "${_member_source}"
    ${_rxfnsc_stage_source_commands}
    COMMAND $<TARGET_FILE:rxc>
    --no-exe-import
    -i "${RXFNSC_IMPORT_ROOT}"
    -o "${_basename}"
    "${_member_source}"
    COMMAND $<TARGET_FILE:rxas> "${_basename}"
    DEPENDS
      "${CMAKE_CURRENT_SOURCE_DIR}/${_basename}.crexx"
      rxfnsc_imports
      "${RXFNSC_IMPORT_LIBRARY}"
      "${RXFNSC_IMPORT_CLASSLIB}"
      "${RXFNSC_IMPORT_EXITS}"
      ${_rxfnsc_source_deps}
      ${RXFNSC_MEMBER_BUILD_DEPS}
    WORKING_DIRECTORY "${_member_dir}"
    COMMENT "cREXX compile and assemble rxfnsc runtime ${_basename} ..."
    VERBATIM
  )
endforeach()

set(_rxfnsc_control_file ${CMAKE_CURRENT_BINARY_DIR}/rxfnsc_link_$<CONFIG>.ctl)
set(_rxfnsc_link_root "${CMAKE_CURRENT_BINARY_DIR}/linked")
set(_rxfnsc_link_image "${_rxfnsc_link_root}/rxfnsc.rxbin")
set(_rxfnsc_publish_image "${CMAKE_BINARY_DIR}/bin/rxfnsc.rxbin")
set(_rxfnsc_publish_temp "${CMAKE_CURRENT_BINARY_DIR}/publish/rxfnsc.rxbin.tmp")
string(CONCAT _rxfnsc_control_text
  "* Auto-generated control file for the consolidated rxfnsc runtime library\n")
foreach(_rxfnsc_bin IN LISTS RXFNSC_BINS)
  string(APPEND _rxfnsc_control_text "INPUT ${_rxfnsc_bin}\n")
endforeach()
foreach(_selector IN LISTS RXFNSC_SELECTORS)
  string(APPEND _rxfnsc_control_text "INCLUDE ${_selector}\n")
endforeach()
string(APPEND _rxfnsc_control_text
  "PRESERVE INLINE\n"
  "$<$<NOT:$<CONFIG:Debug>>:STRIP SOURCE\n>"
  "OUTPUT ${_rxfnsc_link_root}/rxfnsc\n")

file(GENERATE OUTPUT ${_rxfnsc_control_file} CONTENT "${_rxfnsc_control_text}")

add_custom_command(
  OUTPUT "${_rxfnsc_link_image}"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${_rxfnsc_link_root}"
  COMMAND $<TARGET_FILE:rxlink> -c ${_rxfnsc_control_file}
  DEPENDS rxlink ${RXFNSC_BINS} ${_rxfnsc_control_file}
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
  COMMENT "cREXX Linked rxfnsc Runtime Library Image ..."
  VERBATIM
)

add_custom_command(
  OUTPUT "${_rxfnsc_publish_image}"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/publish"
  COMMAND ${CMAKE_COMMAND} -E copy
    "${_rxfnsc_link_image}"
    "${_rxfnsc_publish_temp}"
  COMMAND ${CMAKE_COMMAND} -E rename
    "${_rxfnsc_publish_temp}"
    "${_rxfnsc_publish_image}"
  DEPENDS "${_rxfnsc_link_image}"
  COMMENT "Publishing consolidated rxfnsc runtime library image ..."
  VERBATIM
)

add_custom_target(rxfnsc ALL
  DEPENDS "${_rxfnsc_publish_image}"
)

add_dependencies(rxfnsc library classlib)

add_subdirectory(tests_functional)

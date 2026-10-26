cmake_minimum_required(VERSION 3.24)

set(_ll_flags)
if(CREXX_LEVEL_L_NOOPT)
  list(APPEND _ll_flags -n)
endif()

set(RXFNSL_REXX_MODULES
  lllimits llprepared lludata llproperties llir llirservices
  llcore llcheck llcap llcfg lllr llmunch llproject llstage llcharset
  llregular llcomments llsource llpeg llforest llearley llpratt llregional lllanguage llrecover lltree llast llannotation llflow
)

set(RXFNSL_REXX_BINS)
set(RXFNSL_REXX_TARGETS)
foreach(_module IN LISTS RXFNSL_REXX_MODULES)
  list(APPEND RXFNSL_REXX_BINS "${CMAKE_CURRENT_BINARY_DIR}/${_module}.rxbin")
  # Independent Make import roots must wait for one canonical producer.
  add_custom_target(rxfnsl_${_module}
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/${_module}.rxbin")
  list(APPEND RXFNSL_REXX_TARGETS rxfnsl_${_module})
endforeach()

set(_rxfnsl_import_root "${CMAKE_CURRENT_BINARY_DIR}/imports")
crexx_add_import_root(rxfnsl_imports
  DIRECTORY "${_rxfnsl_import_root}"
  OUTPUT_VARIABLE _rxfnsl_import_files
  FILES "${CMAKE_BINARY_DIR}/bin/library.rxbin"
  DEPENDS library
  COMMENT "Staging deterministic Level L imports ...")

include("${CMAKE_CURRENT_SOURCE_DIR}/../unicode/Properties.cmake")

set(RXFNSL_DEPS_llirservices llir)
set(RXFNSL_DEPS_llcore llprepared)
set(RXFNSL_DEPS_llproperties lludata llcharset)
set(RXFNSL_DEPS_llcheck llcore lllimits)
set(RXFNSL_DEPS_llcap llcore llcheck)
set(RXFNSL_DEPS_llcfg llcore llcheck lllimits llprepared)
set(RXFNSL_DEPS_lllr lllimits llcore llcheck llcap llcfg llprepared)
set(RXFNSL_DEPS_llmunch lllimits llcore llprepared)
set(RXFNSL_DEPS_llproject lllimits llcore llprepared)
set(RXFNSL_DEPS_llstage lllimits llcore llprepared)
set(RXFNSL_DEPS_llcharset llcore)
set(RXFNSL_DEPS_llregular llcore llmunch lllimits)
set(RXFNSL_DEPS_llpeg lllimits llcore llcheck llcap)
set(RXFNSL_DEPS_llearley lllimits llcore llcfg llcheck llcap llforest)
set(RXFNSL_DEPS_llpratt lllimits llcore llcheck llcap)
set(RXFNSL_DEPS_llregional lllimits llcore llcfg llpratt lllr)
set(RXFNSL_DEPS_lllanguage lllimits llcore lllr llpeg llearley llforest llpratt llregional llproject llmunch llstage llsource llcomments llast llprepared)
set(RXFNSL_DEPS_llrecover lllimits llcore lllanguage llmunch llsource llast)
set(RXFNSL_DEPS_llcomments llmunch lllimits llprepared)
set(RXFNSL_DEPS_llsource lllimits llcomments)
set(RXFNSL_DEPS_lltree llcore lllimits)
set(RXFNSL_DEPS_llast llcore lltree lllimits llprepared)
set(RXFNSL_DEPS_llannotation llast)
set(RXFNSL_DEPS_llflow llast llsource)
foreach(_binfile IN LISTS RXFNSL_REXX_BINS)
  get_filename_component(_basename "${_binfile}" NAME_WE)

  set(_module_source "${CMAKE_CURRENT_SOURCE_DIR}/${_basename}.crexx")
  if(_basename STREQUAL "lludata")
    set(_module_source "${_ll_data_source}")
  endif()
  set(_source_root "${CMAKE_CURRENT_BINARY_DIR}/source/${_basename}")
  # Imported inline bodies and attribute types can refer to a dependency's
  # own imports. Stage that declared closure, never the active output folder.
  set(_module_dep_names ${RXFNSL_DEPS_${_basename}})
  set(_pending_deps ${_module_dep_names})
  while(_pending_deps)
    list(POP_FRONT _pending_deps _dep)
    foreach(_transitive IN LISTS RXFNSL_DEPS_${_dep})
      if(NOT _transitive IN_LIST _module_dep_names)
        list(APPEND _module_dep_names "${_transitive}")
        list(APPEND _pending_deps "${_transitive}")
      endif()
    endforeach()
  endwhile()
  set(_module_deps)
  set(_module_dep_targets)
  foreach(_dep IN LISTS _module_dep_names)
    list(APPEND _module_deps "${CMAKE_CURRENT_BINARY_DIR}/${_dep}.rxbin")
    list(APPEND _module_dep_targets rxfnsl_${_dep})
  endforeach()
  set(_module_import_root "${CMAKE_CURRENT_BINARY_DIR}/imports/${_basename}")
  crexx_add_import_root(rxfnsl_${_basename}_imports
    DIRECTORY "${_module_import_root}"
    OUTPUT_VARIABLE _module_import_files
    FILES "${CMAKE_BINARY_DIR}/bin/library.rxbin" ${_module_deps}
    DEPENDS library ${_module_dep_targets}
    COMMENT "Staging deterministic Level L imports for ${_basename} ...")
  crexx_add_fixture_command(
    OUTPUT "${_binfile}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${_source_root}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${_module_source}" "${_source_root}/${_basename}.crexx"
    COMMAND $<TARGET_FILE:rxc>
            ${_ll_flags} -x
            --no-exe-import
            -i "${_module_import_root}"
            -o "${_basename}"
            "${_source_root}/${_basename}.crexx"
    COMMAND $<TARGET_FILE:rxas> ${_ll_flags} "${_basename}"
    DEPENDS
      "${_module_source}"
      rxfnsl_${_basename}_imports
      ${_module_import_files}
      rxc
      rxas
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    COMMENT "cREXX compile and assemble ${_basename} ..."
    VERBATIM
  )
endforeach()

set(_rxfnsl_control_file "${CMAKE_CURRENT_BINARY_DIR}/rxfnsl_link_$<CONFIG>.ctl")
string(CONCAT _rxfnsl_control_text
  "* Auto-generated control file for the consolidated cREXX Level L library\n")
foreach(_module_bin IN LISTS RXFNSL_REXX_BINS)
  string(APPEND _rxfnsl_control_text "INPUT ${_module_bin}\n")
endforeach()
foreach(_selector IN LISTS RXFNSL_REXX_MODULES)
  string(APPEND _rxfnsl_control_text "INCLUDE ${_selector}\n")
endforeach()
string(APPEND _rxfnsl_control_text
  "PRESERVE INLINE\n"
  "$<$<NOT:$<CONFIG:Debug>>:STRIP SOURCE\n>"
  "OUTPUT ${CMAKE_BINARY_DIR}/bin/rxfnsl\n")

file(GENERATE OUTPUT "${_rxfnsl_control_file}" CONTENT "${_rxfnsl_control_text}")

crexx_add_fixture_command(
  OUTPUT "${CMAKE_BINARY_DIR}/bin/rxfnsl.rxbin"
  COMMAND $<TARGET_FILE:rxlink> -c "${_rxfnsl_control_file}"
  DEPENDS rxlink ${RXFNSL_REXX_BINS} "${_rxfnsl_control_file}"
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
  COMMENT "cREXX Linked Level L Library Image ..."
  VERBATIM
)

add_custom_target(rxfnsl ALL
  DEPENDS "${CMAKE_BINARY_DIR}/bin/rxfnsl.rxbin"
)

add_dependencies(rxfnsl library ${RXFNSL_REXX_TARGETS})
set(RXFNSL_BASE_BINS "${RXFNSL_REXX_BINS}" PARENT_SCOPE)

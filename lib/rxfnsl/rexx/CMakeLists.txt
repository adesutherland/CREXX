cmake_minimum_required(VERSION 3.24)

set(_ll_flags)
if(CREXX_LEVEL_L_NOOPT)
  list(APPEND _ll_flags -n)
endif()

set(RXFNSL_REXX_MODULES
  lllimits lludata llproperties llir llirservices
  llcore llcheck llcap llcfg lllr llmunch llproject llstage llcharset
  llregular llcomments llsource llpeg llforest llearley llpratt llregional lllanguage llrecover lltree llast llannotation llflow
)

set(RXFNSL_REXX_BINS)
foreach(_module IN LISTS RXFNSL_REXX_MODULES)
  list(APPEND RXFNSL_REXX_BINS "${CMAKE_CURRENT_BINARY_DIR}/${_module}.rxbin")
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
set(RXFNSL_DEPS_llproperties lludata llcharset)
set(RXFNSL_DEPS_llcheck llcore lllimits)
set(RXFNSL_DEPS_llcap llcore llcheck)
set(RXFNSL_DEPS_llcfg llcore llcheck lllimits)
set(RXFNSL_DEPS_lllr lllimits llcore llcheck llcap llcfg)
set(RXFNSL_DEPS_llmunch lllimits llcore)
set(RXFNSL_DEPS_llproject lllimits llcore)
set(RXFNSL_DEPS_llstage lllimits llcore)
set(RXFNSL_DEPS_llcharset llcore)
set(RXFNSL_DEPS_llregular llcore llmunch lllimits)
set(RXFNSL_DEPS_llpeg lllimits llcore llcheck llcap)
set(RXFNSL_DEPS_llearley lllimits llcore llcfg llcheck llcap llforest)
set(RXFNSL_DEPS_llpratt lllimits llcore llcheck llcap)
set(RXFNSL_DEPS_llregional lllimits llcore llcfg llpratt lllr)
set(RXFNSL_DEPS_lllanguage lllimits llcore lllr llpeg llearley llforest llpratt llregional llproject llmunch llstage llsource llcomments llast)
set(RXFNSL_DEPS_llrecover lllimits llcore lllanguage llmunch llsource llast)
set(RXFNSL_DEPS_llcomments llmunch lllimits)
set(RXFNSL_DEPS_llsource lllimits llcomments)
set(RXFNSL_DEPS_lltree llcore lllimits)
set(RXFNSL_DEPS_llast llcore lltree lllimits)
set(RXFNSL_DEPS_llannotation llast)
set(RXFNSL_DEPS_llflow llast llsource)
foreach(_binfile IN LISTS RXFNSL_REXX_BINS)
  get_filename_component(_basename "${_binfile}" NAME_WE)

  set(_module_source "${CMAKE_CURRENT_SOURCE_DIR}/${_basename}.crexx")
  if(_basename STREQUAL "lludata")
    set(_module_source "${_ll_data_source}")
  endif()
  set(_source_root "${CMAKE_CURRENT_BINARY_DIR}/source/${_basename}")
  set(_module_deps)
  foreach(_dep IN LISTS RXFNSL_DEPS_${_basename})
    list(APPEND _module_deps "${CMAKE_CURRENT_BINARY_DIR}/${_dep}.rxbin")
  endforeach()
  crexx_add_fixture_command(
    OUTPUT "${_binfile}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${_source_root}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${_module_source}" "${_source_root}/${_basename}.crexx"
    COMMAND $<TARGET_FILE:rxc>
            ${_ll_flags} -x
            --no-exe-import
            -i "${_rxfnsl_import_root};${CMAKE_CURRENT_BINARY_DIR}"
            -o "${_basename}"
            "${_source_root}/${_basename}.crexx"
    COMMAND $<TARGET_FILE:rxas> ${_ll_flags} "${_basename}"
    DEPENDS
      "${_module_source}"
      rxfnsl_imports
      ${_module_deps}
      ${_rxfnsl_import_files}
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

add_dependencies(rxfnsl library)
set(RXFNSL_BASE_BINS "${RXFNSL_REXX_BINS}" PARENT_SCOPE)

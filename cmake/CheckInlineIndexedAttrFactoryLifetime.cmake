if(NOT DEFINED RXC OR NOT EXISTS "${RXC}")
  message(FATAL_ERROR "rxc executable was not provided")
endif()
if(NOT DEFINED SOURCE OR NOT EXISTS "${SOURCE}")
  message(FATAL_ERROR "indexed-attribute lifetime source was not provided")
endif()
if(NOT DEFINED INCLUDE_DIR OR NOT IS_DIRECTORY "${INCLUDE_DIR}")
  message(FATAL_ERROR "compiler include directory was not provided")
endif()
if(NOT DEFINED WORK)
  message(FATAL_ERROR "compiler inspection work directory was not provided")
endif()

file(REMOVE_RECURSE "${WORK}")
file(MAKE_DIRECTORY "${WORK}")
set(output_base "${WORK}/inline_indexed_attr_factory_lifetime")

execute_process(
  COMMAND "${RXC}" -i "${INCLUDE_DIR}" -o "${output_base}" "${SOURCE}"
  OUTPUT_VARIABLE compiler_output
  ERROR_VARIABLE compiler_error
  RESULT_VARIABLE compiler_result)
if(NOT compiler_result EQUAL 0)
  message(FATAL_ERROR
    "optimized indexed-attribute lifetime compile failed:\n${compiler_output}${compiler_error}")
endif()

file(READ "${output_base}.rxas" rxas)
if(rxas MATCHES
   "call[0-9]*[^\n]*inline_indexed_attr_factory_lifetime\\.lifetimepool\\.add\\(\\)")
  set(profitability_shape "bounded ordinary-call")
else()
  set(profitability_shape "inlined")
endif()

string(FIND "${rxas}" "items_[item_id] = .LifetimeItem()" marker_start)
if(marker_start LESS 0)
  message(FATAL_ERROR "could not locate indexed factory assignment in optimized RXAS")
endif()
string(SUBSTRING "${rxas}" ${marker_start} -1 assignment_tail)
string(REGEX MATCH
  "minlinkattr1[ \t]+(r[0-9]+),(r[0-9]+),r[0-9]+,0"
  link_match "${assignment_tail}")
if(NOT link_match)
  message(FATAL_ERROR "indexed factory assignment lacks its minlinkattr1 owner pair")
endif()
set(destination "${CMAKE_MATCH_1}")
set(owner "${CMAKE_MATCH_2}")
string(FIND "${assignment_tail}" "${link_match}" link_start)
string(FIND "${assignment_tail}" "unlinkn ${destination},${owner}" unlink_start)
if(unlink_start LESS_EQUAL link_start)
  message(FATAL_ERROR
    "indexed factory assignment lost balanced unlinkn ${destination},${owner}")
endif()
math(EXPR protected_length "${unlink_start} - ${link_start}")
string(SUBSTRING "${assignment_tail}" ${link_start} ${protected_length} protected_window)
if(protected_window MATCHES
   "[\r\n][ \t]+[a-z][a-z0-9]*[ \t]+${owner}(,|[\r\n])")
  message(FATAL_ERROR
    "indexed aggregate owner ${owner} is overwritten before unlinkn:\n${protected_window}")
endif()

message(STATUS
  "${profitability_shape} indexed aggregate owner ${owner} remains reserved through factory evaluation")

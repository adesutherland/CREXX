if(NOT DEFINED RXAS OR NOT EXISTS "${RXAS}")
  message(FATAL_ERROR "rxunicode RXAS file is missing: ${RXAS}")
endif()
if(NOT DEFINED SOURCE OR NOT EXISTS "${SOURCE}")
  message(FATAL_ERROR "rxunicode case-fold template is missing: ${SOURCE}")
endif()

file(READ "${RXAS}" _rxas)
file(READ "${SOURCE}" _source)
string(REGEX MATCHALL
  "\\.const §rxc\\.const\\.[0-9]+\\.unicode_casefold_data binary 0x"
  _constant_definitions "${_rxas}")
list(LENGTH _constant_definitions _constant_count)
if(NOT _constant_count EQUAL 1)
  message(FATAL_ERROR
    "rxunicode must emit exactly one named case-fold binary constant; found ${_constant_count}")
endif()

foreach(_required IN ITEMS
    ".meta \"_rxunicode.casefold\""
    "strchar"
    "appendchar"
    "bgetu32"
    "bgetu8")
  string(FIND "${_rxas}" "${_required}" _position)
  if(_position EQUAL -1)
    message(FATAL_ERROR "rxunicode RXAS is missing required shape: ${_required}")
  endif()
endforeach()

string(FIND "${_source}"
  "signal failure \"rxunicode case-fold data is corrupt or unsupported\""
  _signal_position)
if(_signal_position EQUAL -1)
  message(FATAL_ERROR "rxunicode case folding must use the Level B SIGNAL surface")
endif()
foreach(_forbidden_source IN ITEMS "assembler signal" "call raise")
  string(FIND "${_source}" "${_forbidden_source}" _position)
  if(NOT _position EQUAL -1)
    message(FATAL_ERROR
      "rxunicode case-fold source bypasses the Level B SIGNAL surface: ${_forbidden_source}")
  endif()
endforeach()

foreach(_forbidden IN ITEMS
    "._image"
    " stobin "
    " bintos "
    " bcopy "
    " bresize ")
  string(FIND "${_rxas}" "${_forbidden}" _position)
  if(NOT _position EQUAL -1)
    message(FATAL_ERROR "rxunicode RXAS contains forbidden copy/table shape: ${_forbidden}")
  endif()
endforeach()

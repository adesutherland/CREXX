if(NOT DEFINED RXAS OR NOT EXISTS "${RXAS}")
  message(FATAL_ERROR "rxunicode RXAS file is missing: ${RXAS}")
endif()

file(READ "${RXAS}" _rxas)
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

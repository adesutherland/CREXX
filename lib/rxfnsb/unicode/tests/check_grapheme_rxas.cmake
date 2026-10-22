if(NOT DEFINED RXAS OR NOT EXISTS "${RXAS}")
  message(FATAL_ERROR "rxunicode grapheme RXAS file is missing: ${RXAS}")
endif()
if(NOT DEFINED SOURCE OR NOT EXISTS "${SOURCE}")
  message(FATAL_ERROR "rxunicode grapheme template is missing: ${SOURCE}")
endif()

file(READ "${RXAS}" _rxas)
file(READ "${SOURCE}" _source)
string(REGEX MATCHALL
  "\\.const §rxc\\.const\\.[0-9]+\\.unicode_grapheme_data binary 0x"
  _constant_definitions "${_rxas}")
list(LENGTH _constant_definitions _constant_count)
if(NOT _constant_count EQUAL 1)
  message(FATAL_ERROR
    "rxunicode must emit exactly one named grapheme binary constant; found ${_constant_count}")
endif()

foreach(_required IN ITEMS
    ".meta \"_rxunicode._graphemescan.§factory\""
    ".meta \"_rxunicode.grapheme_count\""
    ".meta \"_rxunicode.grapheme_boundaries\""
    ".meta \"_rxunicode.grapheme_substr\""
    ".meta \"_rxunicode.grapheme_substr_indexed\""
    "strchar"
    "bgetu8"
    "substring"
    "append")
  string(FIND "${_rxas}" "${_required}" _position)
  if(_position EQUAL -1)
    message(FATAL_ERROR "rxunicode grapheme RXAS is missing required shape: ${_required}")
  endif()
endforeach()

foreach(_forbidden IN ITEMS
    " stobin "
    " bintos ")
  string(FIND "${_rxas}" "${_forbidden}" _position)
  if(NOT _position EQUAL -1)
    message(FATAL_ERROR
      "rxunicode grapheme RXAS contains forbidden conversion shape: ${_forbidden}")
  endif()
endforeach()

string(REGEX MATCH "bcopy[^\n]*unicode_grapheme_data" _constant_copy "${_rxas}")
if(NOT "${_constant_copy}" STREQUAL "")
  message(FATAL_ERROR "rxunicode grapheme prepared constant is copied at runtime")
endif()

foreach(_required_source IN ITEMS
    "scan = ._graphemescan(text, 1, 0, 0, 0)"
    "scan = ._graphemescan(text, 3, start, requested_length, length_supplied)"
    "offsets = grapheme_boundaries(text)"
    "signal failure \"rxunicode grapheme data is corrupt or unsupported\""
    "signal out_of_range \"rxunicode grapheme position is outside the source\""
    "signal invalid_arguments \"GRAPHEMESUBSTR start must be positive\"")
  string(FIND "${_source}" "${_required_source}" _position)
  if(_position EQUAL -1)
    message(FATAL_ERROR
      "rxunicode grapheme source is missing the selected streaming/indexed shape: ${_required_source}")
  endif()
endforeach()

foreach(_forbidden_source IN ITEMS
    "re2c"
    "UTF32"
    "lexer_"
    "_image = .binary"
    "_properties = .binary"
    "assembler signal"
    "call raise")
  string(FIND "${_source}" "${_forbidden_source}" _position)
  if(NOT _position EQUAL -1)
    message(FATAL_ERROR
      "rxunicode grapheme runtime contains rejected decoder/table ownership: ${_forbidden_source}")
  endif()
endforeach()

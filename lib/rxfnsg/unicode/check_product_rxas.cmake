if(NOT DEFINED RXAS OR NOT EXISTS "${RXAS}")
  message(FATAL_ERROR "rxunicode ${KIND} RXAS file is missing: ${RXAS}")
endif()
if(NOT DEFINED KIND)
  message(FATAL_ERROR "rxunicode RXAS shape kind is missing")
endif()

file(READ "${RXAS}" _rxas)

if(KIND STREQUAL "normalization")
  set(_constant_name "unicode_normalization_data")
  set(_required
    ".meta \"_rxunicode.unicode_normalize\""
    ".meta \"_rxunicode.unicode_is_normalized\""
    "strchar" "appendchar" "bgetu8" "bgetu32")
elseif(KIND STREQUAL "case_mapping")
  set(_constant_name "unicode_case_mapping_data")
  set(_required
    ".meta \"_rxunicode.unicode_case_convert\""
    ".meta \"_rxunicode._unicode_following_cased\""
    "strchar" "appendchar" "bgetu8" "bgetu32")
elseif(KIND STREQUAL "codec")
  set(_constant_name "unicode_encoding_data")
  set(_required
    ".meta \"_rxunicode.unicode_encode\""
    ".meta \"_rxunicode.unicode_decode\""
    ".meta \"_rxunicode.unicode_is_decodable\""
    "strchar" "appendchar" "bgetu8" "bgetu16" "bgetu32")
else()
  message(FATAL_ERROR "unknown rxunicode RXAS shape kind: ${KIND}")
endif()

string(REGEX MATCHALL
  "\\.const §rxc\\.const\\.[0-9]+\\.${_constant_name} binary 0x"
  _constant_definitions "${_rxas}")
list(LENGTH _constant_definitions _constant_count)
if(NOT _constant_count EQUAL 1)
  message(FATAL_ERROR
    "rxunicode ${KIND} must emit one named prepared constant; found ${_constant_count}")
endif()

foreach(_needle IN LISTS _required)
  string(FIND "${_rxas}" "${_needle}" _position)
  if(_position EQUAL -1)
    message(FATAL_ERROR
      "rxunicode ${KIND} RXAS is missing required shape: ${_needle}")
  endif()
endforeach()

string(REGEX MATCH
  "(bcopy|bappend|move|copy)[^\n]*${_constant_name}"
  _constant_copy "${_rxas}")
if(NOT "${_constant_copy}" STREQUAL "")
  message(FATAL_ERROR
    "rxunicode ${KIND} prepared constant is copied at runtime")
endif()

if(KIND STREQUAL "normalization" OR KIND STREQUAL "case_mapping")
  foreach(_forbidden IN ITEMS " stobin " " bintos ")
    string(FIND "${_rxas}" "${_forbidden}" _position)
    if(NOT _position EQUAL -1)
      message(FATAL_ERROR
        "rxunicode ${KIND} must remain on the VM codepoint path: ${_forbidden}")
    endif()
  endforeach()
endif()

if(KIND STREQUAL "normalization")
  foreach(_forbidden_symbol IN ITEMS
      "unicodednormalizer.normalize_binary\""
      "unicodednormalizer.is_normalized_binary\""
      "unicodednormalizer.lexer_begin\""
      "unicodednormalizer.lexer_raw\""
      "unicodednormalizer.lexer_mark\""
      "unicodednormalizer.lexer_mapping\""
      "unicodednormalizer.lexer_hangul\""
      "unicodednormalizer.lexer_finish\""
      "unicodednormalizer._decoded_codepoint\""
      "unicodednormalizer._decoded_length\""
      "unicodednormalizer._output\""
      "unicodednormalizer._raw_start\""
      "unicodednormalizer._raw_length\""
      "unicodednormalizer._begin\""
      "unicodednormalizer._consume\""
      "unicodednormalizer._decode\""
      "unicodednormalizer._process_scalar\""
      "unicodednormalizer._queue_raw\""
      "unicodednormalizer._flush_segment\""
      "unicodednormalizer._append_codepoint\""
      "unicodednormalizer._ensure_output\""
      "unicodednormalizer._finish\""
      "unicodednormalizer._quick_check\"")
    string(FIND "${_rxas}" "${_forbidden_symbol}" _position)
    if(NOT _position EQUAL -1)
      message(FATAL_ERROR
        "rxunicode normalization product retained rejected binary-path symbol: ${_forbidden_symbol}")
    endif()
  endforeach()
endif()

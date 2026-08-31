foreach(_input IN ITEMS
    SOURCE DOC USER_DOC NORMALIZATION_TEST CASE_MAPPING_TEST CASEFOLD_TEST
    GRAPHEME_TEST CODEC_TEST)
  if(NOT DEFINED ${_input} OR NOT EXISTS "${${_input}}")
    message(FATAL_ERROR "rxunicode contract input is missing: ${_input}")
  endif()
  file(READ "${${_input}}" "_${_input}")
endforeach()

function(require_surface _procedure _return_type _test_variable)
  string(FIND "${_SOURCE}" "${_procedure}: procedure = ${_return_type}"
    _source_position)
  string(FIND "${_DOC}" "${_procedure}(" _doc_position)
  string(FIND "${_USER_DOC}" "${_procedure}(" _user_doc_position)
  string(FIND "${${_test_variable}}" "${_procedure}(" _test_position)
  if(_source_position EQUAL -1 OR _doc_position EQUAL -1 OR
     _user_doc_position EQUAL -1 OR
     _test_position EQUAL -1)
    message(FATAL_ERROR
      "rxunicode procedure is not present in source, both docs, and tests: ${_procedure}")
  endif()
endfunction()

require_surface(version .string _CODEC_TEST)

foreach(_procedure IN ITEMS toNFD toNFC toNFKD toNFKC)
  require_surface(${_procedure} .string _NORMALIZATION_TEST)
endforeach()
foreach(_procedure IN ITEMS isNFD isNFC isNFKD isNFKC)
  require_surface(${_procedure} .boolean _NORMALIZATION_TEST)
endforeach()

foreach(_procedure IN ITEMS toUppercase toLowercase)
  require_surface(${_procedure} .string _CASE_MAPPING_TEST)
endforeach()

foreach(_procedure IN ITEMS
    toCasefold toSimpleCasefold toTurkicCasefold toTurkicSimpleCasefold)
  require_surface(${_procedure} .string _CASEFOLD_TEST)
endforeach()

require_surface(encode .binary _CODEC_TEST)
require_surface(decode .string _CODEC_TEST)
require_surface(isDecodable .boolean _CODEC_TEST)
require_surface(isEncodingSupported .boolean _CODEC_TEST)

require_surface(graphemeCount .int _GRAPHEME_TEST)
require_surface(graphemeSubstr .string _GRAPHEME_TEST)
require_surface(graphemePos .int _GRAPHEME_TEST)
require_surface(graphemeReverse .string _GRAPHEME_TEST)

foreach(_method IN ITEMS
    text count at substr pos reverse codepointStart codepointLength iterator
    version profile)
  string(FIND "${_SOURCE}" "${_method}: method" _source_position)
  string(FIND "${_DOC}" "`${_method}" _doc_position)
  string(FIND "${_USER_DOC}" "`${_method}" _user_doc_position)
  string(FIND "${_GRAPHEME_TEST}" ".${_method}(" _test_position)
  if(_source_position EQUAL -1 OR _doc_position EQUAL -1 OR
     _user_doc_position EQUAL -1 OR
     _test_position EQUAL -1)
    message(FATAL_ERROR
      "rxunicode grapheme snapshot contract is incomplete: ${_method}")
  endif()
endforeach()

foreach(_method IN ITEMS hasNext next index reset codepointStart codepointLength)
  string(FIND "${_SOURCE}" "${_method}: method" _source_position)
  string(FIND "${_DOC}" "`${_method}" _doc_position)
  string(FIND "${_USER_DOC}" "${_method}(" _user_doc_position)
  string(FIND "${_GRAPHEME_TEST}" ".${_method}(" _test_position)
  if(_source_position EQUAL -1 OR _doc_position EQUAL -1 OR
     _user_doc_position EQUAL -1 OR _test_position EQUAL -1)
    message(FATAL_ERROR
      "rxunicode grapheme iterator contract is incomplete: ${_method}")
  endif()
endforeach()

set(_namespace
  "namespace rxunicode expose version toNFD toNFC toNFKD toNFKC isNFD isNFC isNFKD isNFKC toUppercase toLowercase toCasefold toSimpleCasefold toTurkicCasefold toTurkicSimpleCasefold encode decode isDecodable isEncodingSupported graphemeCount graphemeSubstr graphemePos graphemeReverse graphemes graphemeiterator")
string(FIND "${_SOURCE}" "${_namespace}" _namespace_position)
if(_namespace_position EQUAL -1)
  message(FATAL_ERROR "rxunicode exported namespace surface has drifted")
endif()

string(FIND "${_SOURCE}" "casefolder: class" _casefolder_position)
if(NOT _casefolder_position EQUAL -1)
  message(FATAL_ERROR
    "rxunicode must not wrap stateless case folding in a reusable object")
endif()

string(FIND "${_SOURCE}" "assembler " _assembler_position)
if(NOT _assembler_position EQUAL -1)
  message(FATAL_ERROR "the public Level G rxunicode facade must remain assembler-free")
endif()

foreach(_doc_boundary IN ITEMS
    "## Version and normalization"
    "## Default case mapping and folding"
    "## Typed codecs"
    "## Default extended grapheme clusters")
  string(FIND "${_DOC}" "${_doc_boundary}" _doc_position)
  if(_doc_position EQUAL -1)
    message(FATAL_ERROR "rxunicode source-adjacent documentation is incomplete: ${_doc_boundary}")
  endif()
endforeach()

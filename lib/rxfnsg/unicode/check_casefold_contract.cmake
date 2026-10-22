foreach(_input IN ITEMS
    SOURCE API_SOURCE DOC USER_DOC NORMALIZATION_TEST CASE_MAPPING_TEST CASEFOLD_TEST
    GRAPHEME_TEST CODEC_TEST)
  if(NOT DEFINED ${_input} OR NOT EXISTS "${${_input}}")
    message(FATAL_ERROR "rxunicode contract input is missing: ${_input}")
  endif()
  file(READ "${${_input}}" "_${_input}")
endforeach()

function(require_surface _procedure _return_type _test_variable)
  string(FIND "${_API_SOURCE}" "${_procedure}: procedure = ${_return_type}"
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
  string(REGEX MATCHALL "(^|\n)${_procedure}: procedure" _definitions "${_API_SOURCE}")
  list(LENGTH _definitions _definition_count)
  string(REGEX MATCH "(^|\n)${_procedure}: procedure" _g_definition "${_SOURCE}")
  if(NOT _definition_count EQUAL 1 OR NOT "${_g_definition}" STREQUAL "")
    message(FATAL_ERROR "rxunicode public procedure must have one B body and no G body: ${_procedure}")
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

# Retain the exact argument/default/reference contract while changing its owner.
foreach(_signature IN ITEMS
    "toNFD|.string|expose text = .string"
    "toNFC|.string|expose text = .string"
    "toNFKD|.string|expose text = .string"
    "toNFKC|.string|expose text = .string"
    "isNFD|.boolean|expose text = .string"
    "isNFC|.boolean|expose text = .string"
    "isNFKD|.boolean|expose text = .string"
    "isNFKC|.boolean|expose text = .string"
    "toUppercase|.string|text = .string"
    "toLowercase|.string|text = .string"
    "toCasefold|.string|text = .string"
    "toSimpleCasefold|.string|text = .string"
    "toTurkicCasefold|.string|text = .string"
    "toTurkicSimpleCasefold|.string|text = .string"
    "encode|.binary|text = .string, encoding = \"UTF-8\", replacement = \"\"x as .binary"
    "decode|.string|data = .binary, encoding = \"UTF-8\", replacement = \"\""
    "isDecodable|.boolean|data = .binary, encoding = \"UTF-8\""
    "isEncodingSupported|.boolean|encoding = .string"
    "graphemeCount|.int|text = .string"
    "graphemeSubstr|.string|text = .string, start = .int, length = 0, pad = \" \""
    "graphemePos|.int|needle = .string, haystack = .string, start = 1"
    "graphemeReverse|.string|text = .string")
  string(REPLACE "|" ";" _signature_fields "${_signature}")
  list(GET _signature_fields 0 _procedure)
  list(GET _signature_fields 1 _return_type)
  list(GET _signature_fields 2 _arguments)
  string(FIND "${_API_SOURCE}"
    "${_procedure}: procedure = ${_return_type}\n  arg ${_arguments}\n"
    _signature_position)
  if(_signature_position EQUAL -1)
    message(FATAL_ERROR "rxunicode public signature/default/reference contract has drifted: ${_procedure}")
  endif()
endforeach()
string(FIND "${_API_SOURCE}" "version: procedure = .string\n  return \"17.0.0\"\n"
  _version_position)
if(_version_position EQUAL -1)
  message(FATAL_ERROR "rxunicode version signature or pinned version has drifted")
endif()

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

set(_api_namespace
  "namespace rxunicode expose version toNFD toNFC toNFKD toNFKC isNFD isNFC isNFKD isNFKC toUppercase toLowercase toCasefold toSimpleCasefold toTurkicCasefold toTurkicSimpleCasefold encode decode isDecodable isEncodingSupported graphemeCount graphemeSubstr graphemePos graphemeReverse")
string(FIND "${_API_SOURCE}" "${_api_namespace}\n" _api_namespace_position)
string(FIND "${_SOURCE}" "namespace rxunicode expose graphemes graphemeiterator\n"
  _g_namespace_position)
if(_api_namespace_position EQUAL -1 OR _g_namespace_position EQUAL -1)
  message(FATAL_ERROR "rxunicode B/G exported namespace union has drifted")
endif()
string(REGEX MATCHALL "(^|\n)[A-Za-z][A-Za-z0-9_]*: procedure" _api_procedures "${_API_SOURCE}")
list(LENGTH _api_procedures _api_procedure_count)
string(REGEX MATCH "(^|\n)[A-Za-z][A-Za-z0-9_]*: procedure" _g_procedure "${_SOURCE}")
string(FIND "${_API_SOURCE}" ": class" _api_class_position)
string(FIND "${_API_SOURCE}" "options levelb\n" _api_level_position)
string(FIND "${_SOURCE}" "options levelg\n" _g_level_position)
if(NOT _api_procedure_count EQUAL 23 OR NOT "${_g_procedure}" STREQUAL "" OR
   NOT _api_class_position EQUAL -1 OR _api_level_position EQUAL -1 OR
   _g_level_position EQUAL -1)
  message(FATAL_ERROR "rxunicode ownership must remain 23 B procedures and G-only classes")
endif()

string(FIND "${_SOURCE}" "casefolder: class" _casefolder_position)
if(NOT _casefolder_position EQUAL -1)
  message(FATAL_ERROR
    "rxunicode must not wrap stateless case folding in a reusable object")
endif()

string(FIND "${_SOURCE}" "assembler " _assembler_position)
string(FIND "${_API_SOURCE}" "assembler " _api_assembler_position)
if(NOT _assembler_position EQUAL -1 OR NOT _api_assembler_position EQUAL -1)
  message(FATAL_ERROR "the public B/G rxunicode facades must remain assembler-free")
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

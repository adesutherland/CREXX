foreach(_input IN ITEMS SOURCE DOC TEST GRAPHEME_TEST)
  if(NOT DEFINED ${_input} OR NOT EXISTS "${${_input}}")
    message(FATAL_ERROR "rxunicode contract input is missing: ${_input}")
  endif()
endforeach()

file(READ "${SOURCE}" _source)
file(READ "${DOC}" _doc)
file(READ "${TEST}" _test)
file(READ "${GRAPHEME_TEST}" _grapheme_test)

foreach(_procedure IN ITEMS
    toCasefold
    toSimpleCasefold
    toTurkicCasefold
    toTurkicSimpleCasefold)
  string(FIND "${_source}" "${_procedure}: procedure = .string" _source_position)
  string(FIND "${_doc}" "`${_procedure}(text)`" _doc_position)
  string(FIND "${_test}" "${_procedure}(" _test_position)
  if(_source_position EQUAL -1 OR _doc_position EQUAL -1 OR _test_position EQUAL -1)
    message(FATAL_ERROR
      "rxunicode procedure contract is not present in source, docs, and tests: ${_procedure}")
  endif()
endforeach()

foreach(_procedure IN ITEMS
    graphemeCount
    graphemeSubstr
    graphemePos
    graphemeReverse)
  string(FIND "${_source}" "${_procedure}: procedure =" _source_position)
  string(FIND "${_doc}" "`${_procedure}(" _doc_position)
  string(FIND "${_grapheme_test}" "${_procedure}(" _test_position)
  if(_source_position EQUAL -1 OR _doc_position EQUAL -1 OR
     _test_position EQUAL -1)
    message(FATAL_ERROR
      "rxunicode grapheme procedure is not present in source, docs, and tests: ${_procedure}")
  endif()
endforeach()

foreach(_factory IN ITEMS full simple turkic turkicSimple)
  string(FIND "${_source}" "${_factory}: factory" _source_position)
  string(FIND "${_doc}" ".casefolder.${_factory}()" _doc_position)
  string(FIND "${_test}" ".casefolder.${_factory}()" _test_position)
  if(_source_position EQUAL -1 OR _doc_position EQUAL -1 OR _test_position EQUAL -1)
    message(FATAL_ERROR
      "rxunicode casefolder contract is not present in source, docs, and tests: ${_factory}")
  endif()
endforeach()

string(FIND "${_source}" "namespace rxunicode expose toCasefold toSimpleCasefold toTurkicCasefold toTurkicSimpleCasefold graphemeCount graphemeSubstr graphemePos graphemeReverse casefolder graphemes graphemeiterator" _namespace_position)
if(_namespace_position EQUAL -1)
  message(FATAL_ERROR "rxunicode exported namespace surface has drifted")
endif()

string(FIND "${_source}" "assembler " _assembler_position)
if(NOT _assembler_position EQUAL -1)
  message(FATAL_ERROR "the public Level G rxunicode facade must remain assembler-free")
endif()

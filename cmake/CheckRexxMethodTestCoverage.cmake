if(NOT DEFINED CLASS_SOURCE OR NOT EXISTS "${CLASS_SOURCE}")
  message(FATAL_ERROR "CLASS_SOURCE does not name an existing Rexx class source")
endif()
if(NOT DEFINED TEST_SOURCE OR NOT EXISTS "${TEST_SOURCE}")
  message(FATAL_ERROR "TEST_SOURCE does not name an existing Rexx class test")
endif()

file(STRINGS "${CLASS_SOURCE}" _method_lines
  REGEX "^  [A-Za-z][A-Za-z0-9]*: method")
file(READ "${TEST_SOURCE}" _test_source)
string(TOLOWER "${_test_source}" _test_source_lower)

set(_missing_methods)
set(_method_count 0)
foreach(_line IN LISTS _method_lines)
  string(REGEX REPLACE
    "^  ([A-Za-z][A-Za-z0-9]*): method.*$" "\\1" _method "${_line}")
  string(TOLOWER "${_method}" _method_lower)
  math(EXPR _method_count "${_method_count} + 1")
  string(REGEX MATCH
    "\\.${_method_lower}[ \t\r\n]*\\(" _invocation "${_test_source_lower}")
  if(NOT _invocation)
    list(APPEND _missing_methods "${_method}")
  endif()
endforeach()

string(REGEX MATCH "\\.rexx[ \t\r\n]*\\(" _factory_invocation "${_test_source_lower}")
if(NOT _factory_invocation)
  list(APPEND _missing_methods "factory")
endif()

if(_missing_methods)
  list(JOIN _missing_methods ", " _missing_text)
  message(FATAL_ERROR
    "Rexx class public methods without a test invocation: ${_missing_text}")
endif()

message(STATUS
  "Rexx class coverage guard found factory plus ${_method_count} tested public methods")

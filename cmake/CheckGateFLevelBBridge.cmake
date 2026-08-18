if(NOT DEFINED SOURCE_FILE OR NOT EXISTS "${SOURCE_FILE}")
  message(FATAL_ERROR "Gate F Level B source was not provided")
endif()

file(READ "${SOURCE_FILE}" source)
string(TOLOWER "${source}" lower_source)

foreach(op IN ITEMS chanopen chanstart chanwait chancancel chanclose)
  if(NOT lower_source MATCHES "assembler[ \t]+${op}([ \t]|$)")
    message(FATAL_ERROR "Gate F Level B bridge does not use ${op}")
  endif()
endforeach()

foreach(forbidden IN ITEMS rxpa "@semantic-callable" procedurename)
  string(FIND "${lower_source}" "${forbidden}" found)
  if(NOT found EQUAL -1)
    message(FATAL_ERROR
      "Gate F Level B bridge contains forbidden dispatch seam '${forbidden}'")
  endif()
endforeach()

foreach(field IN ITEMS callableId factoryArguments imageDigest kind signatureDigest)
  string(FIND "${source}" "\"${field}\"" found)
  if(found EQUAL -1)
    message(FATAL_ERROR
      "Gate F sealed task target does not encode '${field}'")
  endif()
endforeach()

message(STATUS "Gate F Level B bridge uses the RXAS channel seam and sealed target fields")

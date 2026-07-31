foreach(required IN ITEMS ACTUAL EXPECTED)
    if(NOT DEFINED ${required} OR "${${required}}" STREQUAL "")
        message(FATAL_ERROR "compare_text_files.cmake requires ${required}")
    endif()
endforeach()

file(READ "${ACTUAL}" actual_text)
file(READ "${EXPECTED}" expected_text)
string(REPLACE "\r\n" "\n" actual_text "${actual_text}")
string(REPLACE "\r" "\n" actual_text "${actual_text}")
string(REPLACE "\r\n" "\n" expected_text "${expected_text}")
string(REPLACE "\r" "\n" expected_text "${expected_text}")

string(SHA256 actual_hash "${actual_text}")
string(SHA256 expected_hash "${expected_text}")
if(NOT actual_hash STREQUAL expected_hash)
    message(FATAL_ERROR
            "normalized text differs:\n"
            "  actual: ${ACTUAL}\n"
            "  expected: ${EXPECTED}")
endif()

# test_01_init
add_test(NAME test_rxas_01_init 
         COMMAND parser_tester -q 
         -p "$<TARGET_FILE:rxas>" 
         -s "${CMAKE_CURRENT_SOURCE_DIR}/test_01_init.rxas" 
         -g "${CMAKE_CURRENT_SOURCE_DIR}/test_01_init.golden" 
         "${CMAKE_CURRENT_SOURCE_DIR}/test_01_init.txt")

# test_02_edits
add_test(NAME test_rxas_02_edits 
         COMMAND parser_tester -q 
         -p "$<TARGET_FILE:rxas>" 
         -s "${CMAKE_CURRENT_SOURCE_DIR}/test_01_init.rxas" 
         -g "${CMAKE_CURRENT_SOURCE_DIR}/test_02_edits.golden" 
         "${CMAKE_CURRENT_SOURCE_DIR}/test_02_edits.txt")

# test_03_token_types
add_test(NAME test_rxas_03_token_types 
         COMMAND parser_tester -q 
         -p "$<TARGET_FILE:rxas>" 
         -s "${CMAKE_CURRENT_SOURCE_DIR}/test_03_token_types.rxas" 
         -g "${CMAKE_CURRENT_SOURCE_DIR}/test_03_token_types.golden" 
         "${CMAKE_CURRENT_SOURCE_DIR}/test_03_token_types.txt")

# test_04_errors
add_test(NAME test_rxas_04_errors 
         COMMAND parser_tester -q 
         -p "$<TARGET_FILE:rxas>" 
         -s "${CMAKE_CURRENT_SOURCE_DIR}/test_04_errors.rxas" 
         -g "${CMAKE_CURRENT_SOURCE_DIR}/test_04_errors.golden" 
         "${CMAKE_CURRENT_SOURCE_DIR}/test_04_errors.txt")

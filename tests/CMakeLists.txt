cmake_minimum_required(VERSION 3.00)

project(tests C)

set(CMAKE_C_STANDARD 90)

enable_testing()
find_program(CTEST ctest)

add_custom_command(
        COMMAND ${CMAKE_BINARY_DIR}/assembler/rxas -l ${CMAKE_CURRENT_SOURCE_DIR} ascommon
        COMMAND mv ${CMAKE_CURRENT_SOURCE_DIR}/ascommon.rxbin ${CMAKE_CURRENT_BINARY_DIR}/ascommon.rxbin
        COMMAND ${CTEST} -R ascommon --output-on-failure
        DEPENDS ascommon.rxas rxas rxvm rxbvm
        OUTPUT ascommon.rxbin
        COMMENT "cREXX-Assemble ascommon.rxas ..."
)

add_custom_command(
        COMMAND ${CMAKE_BINARY_DIR}/assembler/rxas -l ${CMAKE_CURRENT_SOURCE_DIR} asutf
        COMMAND mv ${CMAKE_CURRENT_SOURCE_DIR}/asutf.rxbin ${CMAKE_CURRENT_BINARY_DIR}/asutf.rxbin
        COMMAND ${CTEST} -R asutf --output-on-failure
        DEPENDS asutf.rxas rxas rxvm rxbvm
        OUTPUT asutf.rxbin
        COMMENT "cREXX-Assemble asutf.rxas ..."
)


add_custom_target(tests ALL
        DEPENDS ascommon.rxbin asutf.rxbin)

add_test(NAME ascommon COMMAND rxvm ascommon WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
add_test(NAME ascommon-bytecode COMMAND rxbvm ascommon WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
add_test(NAME asutf COMMAND rxvm asutf WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
add_test(NAME asutf-bytecode COMMAND rxbvm asutf WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

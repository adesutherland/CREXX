cmake_minimum_required(VERSION 3.00)

project(tests C)

set(CMAKE_C_STANDARD 90)

enable_testing()
find_program(CTEST ctest)

add_custom_command(
        COMMAND ${CMAKE_BINARY_DIR}/assembler/rxas -l ${CMAKE_CURRENT_SOURCE_DIR} as-tests
        COMMAND mv ${CMAKE_CURRENT_SOURCE_DIR}/as-tests.rxbin ${CMAKE_CURRENT_BINARY_DIR}/as-tests.rxbin
        COMMAND ${CTEST} -R as-tests --output-on-failure
        DEPENDS as-tests.rxas rxas rxvm rxbvm
        OUTPUT as-tests.rxbin
        COMMENT "cREXX-Assemble as-tests.rxas ..."
)

add_custom_target(as-tests ALL
        DEPENDS as-tests.rxbin)

add_test(NAME as-tests COMMAND rxvm as-tests WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
add_test(NAME as-tests-bytecode COMMAND rxbvm as-tests WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

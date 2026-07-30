cmake_minimum_required(VERSION 3.24)

project(CREXXContract C)

set(CMAKE_C_STANDARD 99)

add_executable(crexx-contract
        contractmain.c
        contract_model.c
        contract_json.c
        contract_compat.c
        contract_model.h)
target_include_directories(crexx-contract PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/binutils/include
        ${CMAKE_SOURCE_DIR}/platform)
target_link_libraries(crexx-contract PRIVATE rxbin platform)
if(MSVC)
    target_compile_options(crexx-contract PRIVATE /W4 /WX)
else()
    target_compile_options(crexx-contract PRIVATE -Wall -Wextra -Werror)
endif()

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

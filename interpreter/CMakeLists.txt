cmake_minimum_required(VERSION 3.00)

project(Interpreter C)

set(CMAKE_C_STANDARD 90)

# Threaded Version
add_executable(rxvm rxvmmain.c rxvmintp.h rxvmintp.c ../avl_tree/rxastree.h
        ../utf8/utf.h)
add_dependencies(rxvm machine)
target_link_libraries(rxvm machine avl_tree platform)
target_include_directories(rxvm PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
        ../assembler ../machine ../platform ../avl_tree ../utf8)

# Bytecode Version
add_executable(rxbvm rxvmmain.c rxvmintp.h rxvmintp.c ../avl_tree/rxastree.h
        ../utf8/utf.h)
add_dependencies(rxbvm machine)
target_link_libraries(rxbvm machine avl_tree platform)
target_include_directories(rxbvm PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
        ../assembler ../machine ../platform ../avl_tree ../utf8)
target_compile_definitions(rxbvm PRIVATE NTHREADED=1)

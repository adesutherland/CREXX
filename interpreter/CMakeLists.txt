cmake_minimum_required(VERSION 3.9)

project(Interpreter C)

set(CMAKE_C_STANDARD 99) # enables inline keyword
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG") # Can do more I am sure but this is about as good as it get in practice

##########################################
# Bare Interpreter
# Threaded Version
add_executable(rxvm rxvmmain.c rxvmintp.h rxvmintp.c rxvmload.c ../avl_tree/rxastree.h
        ../utf8/utf.h ../machine/rxbin.h)
set_property(TARGET rxvm PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
set_property(TARGET rxvm PROPERTY INTERPROCEDURAL_OPTIMIZATION_DEBUG FALSE)
set_property(TARGET rxvm PROPERTY INTERPROCEDURAL_OPTIMIZATION_MINSIZEREL TRUE)
add_dependencies(rxvm machine avl_tree platform)
target_link_libraries(rxvm machine avl_tree platform m)
target_include_directories(rxvm PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
        ../assembler ../machine ../platform ../avl_tree ../utf8)

# Bytecode Version
add_executable(rxbvm rxvmmain.c rxvmintp.h rxvmintp.c rxvmload.c ../avl_tree/rxastree.h
        ../utf8/utf.h ../machine/rxbin.h)
add_dependencies(rxbvm machine avl_tree platform)
target_link_libraries(rxbvm machine avl_tree platform m)
target_include_directories(rxbvm PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
        ../assembler ../machine ../platform ../avl_tree ../utf8)
target_compile_definitions(rxbvm PRIVATE NTHREADED=1)

##########################################
# Interpreter linked to CREXX Stdlib

# Packed Library
add_custom_command(
        OUTPUT library.c
        COMMAND ${CMAKE_BINARY_DIR}/cpacker/rxcpack -o library ${CMAKE_BINARY_DIR}/lib/rxfns/library
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        DEPENDS library rxcpack
)

# Threaded Version
add_executable(rxvme rxvmmain.c rxvmintp.h rxvmintp.c rxvmload.c ../avl_tree/rxastree.h
        ../utf8/utf.h ../machine/rxbin.h ${CMAKE_CURRENT_BINARY_DIR}/library.c)
target_compile_definitions(rxvme PRIVATE LINK_CREXX_LIB=1)
set_property(TARGET rxvme PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
set_property(TARGET rxvme PROPERTY INTERPROCEDURAL_OPTIMIZATION_DEBUG FALSE)
set_property(TARGET rxvme PROPERTY INTERPROCEDURAL_OPTIMIZATION_MINSIZEREL TRUE)
add_dependencies(rxvme machine avl_tree platform library)
target_link_libraries(rxvme machine avl_tree platform m)
target_include_directories(rxvme PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
        ../assembler ../machine ../platform ../avl_tree ../utf8)

add_executable(rxbvme rxvmmain.c rxvmintp.h rxvmintp.c rxvmload.c ../avl_tree/rxastree.h
        ../utf8/utf.h ../machine/rxbin.h ${CMAKE_CURRENT_BINARY_DIR}/library.c)
add_dependencies(rxbvme machine avl_tree platform library)
target_link_libraries(rxbvme machine avl_tree platform m)
target_include_directories(rxbvme PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
        ../assembler ../machine ../platform ../avl_tree ../utf8)
target_compile_definitions(rxbvme PRIVATE NTHREADED=1 LINK_CREXX_LIB=1)

##########################################
# C Library Interpreter -  to be linked to make a standalone CREXX exe

# Threaded Version
add_library(rxvml STATIC rxvmlib.c rxvmintp.h rxvmintp.c rxvmload.c ../avl_tree/rxastree.h
        ../utf8/utf.h ../machine/rxbin.h)
set_property(TARGET rxvml PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
set_property(TARGET rxvml PROPERTY INTERPROCEDURAL_OPTIMIZATION_DEBUG FALSE)
set_property(TARGET rxvml PROPERTY INTERPROCEDURAL_OPTIMIZATION_MINSIZEREL TRUE)
add_dependencies(rxvml machine avl_tree platform)
target_include_directories(rxvml PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
        ../assembler ../machine ../platform ../avl_tree ../utf8)

# Bytecode Version
add_library(rxbvml STATIC rxvmlib.c rxvmintp.h rxvmintp.c rxvmload.c ../avl_tree/rxastree.h
        ../utf8/utf.h ../machine/rxbin.h)
add_dependencies(rxbvml machine avl_tree platform)
target_include_directories(rxbvml PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
        ../assembler ../machine ../platform ../avl_tree ../utf8)
target_compile_definitions(rxbvml PRIVATE NTHREADED=1)

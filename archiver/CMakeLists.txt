cmake_minimum_required(VERSION 3.00)

project(Archiver C)

set(CMAKE_C_STANDARD 90)

add_executable(rxar
        rxarmain.c  rxarmain.h
        rxaradd.c   rxaradd.h
        rxarlist.c  rxarlist.h
        rxardel.c   rxardel.h
        rxarutil.c  rxarutil.h
        rxlntest.c  rxlntest.h queue.c queue.h)

target_link_libraries(rxar avl_tree common platform)

target_include_directories(rxar PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ../avl_tree ../assembler ../platform)

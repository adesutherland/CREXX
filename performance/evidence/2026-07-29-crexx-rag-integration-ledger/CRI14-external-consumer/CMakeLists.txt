cmake_minimum_required(VERSION 3.24)
project(CRI14ExternalGraphConsumer C)

find_package(CREXX CONFIG REQUIRED)

if(NOT TARGET CREXX::RXBIN)
    message(FATAL_ERROR
            "CREXX package has no installed semantic-graph development target CREXX::RXBIN")
endif()

add_executable(cri14_external_graph_consumer main.c)
target_link_libraries(cri14_external_graph_consumer PRIVATE CREXX::RXBIN)

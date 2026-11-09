cmake_minimum_required(VERSION 3.24)
project(crexx_rag_query_wasm C)
if(NOT EMSCRIPTEN)
    message(FATAL_ERROR "Configure with the Emscripten toolchain")
endif()

get_filename_component(CREXX_SOURCE "${CMAKE_CURRENT_LIST_DIR}/../../../.." ABSOLUTE)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(CREXX_BUILD_COMPILER OFF CACHE BOOL "" FORCE)
set(CREXX_VM_CLASSIC_PROVIDERS OFF CACHE BOOL "" FORCE)
set(CREXX_VM_CLASSIC_FULL_PROVIDERS OFF CACHE BOOL "" FORCE)
add_subdirectory("${CREXX_SOURCE}/ports/single-threaded" crexx-vm EXCLUDE_FROM_ALL)

# A separate, opt-in VM image keeps the ordinary UI Worker free of database,
# vector and filesystem providers. The RAG product supplies query RXBINs.
add_executable(crexx_rag_query_vm query_vm.c
        "${CREXX_SOURCE}/lib/plugins/sqlite/vendor/sqlite3.c"
        "${CREXX_SOURCE}/lib/plugins/sqlite/rxsqlite.c"
        "${CREXX_SOURCE}/lib/plugins/vector/rxvector.c"
        "${CREXX_SOURCE}/lib/plugins/vector/rxvector_index.c"
        "${CREXX_SOURCE}/lib/plugins/hash/rx_hash.c"
        "${CREXX_SOURCE}/lib/plugins/fs/rxfs.c"
        "${CREXX_SOURCE}/lib/plugins/platform/rxplatform.c")
target_include_directories(crexx_rag_query_vm PRIVATE
        "${CREXX_SOURCE}/lib/plugins/sqlite/vendor"
        "${CREXX_SOURCE}/lib/plugins/vector"
        "${CREXX_SOURCE}/lib/plugins/fs"
        "${CREXX_SOURCE}/rxpa"
        "${CREXX_SOURCE}/binutils/include"
        "${CREXX_SOURCE}/interpreter/rxvml")
target_compile_definitions(crexx_rag_query_vm PRIVATE CREXX_VM_SINGLE_THREADED=1)
set_source_files_properties("${CREXX_SOURCE}/lib/plugins/sqlite/rxsqlite.c"
        PROPERTIES COMPILE_DEFINITIONS "PLUGIN_ID=rxsqlite")
set_source_files_properties("${CREXX_SOURCE}/lib/plugins/vector/rxvector.c"
        PROPERTIES COMPILE_DEFINITIONS "PLUGIN_ID=rxvector")
set_source_files_properties("${CREXX_SOURCE}/lib/plugins/hash/rx_hash.c"
        PROPERTIES COMPILE_DEFINITIONS "PLUGIN_ID=rx_hash")
set_source_files_properties("${CREXX_SOURCE}/lib/plugins/fs/rxfs.c"
        PROPERTIES COMPILE_DEFINITIONS "PLUGIN_ID=rxfs")
set_source_files_properties("${CREXX_SOURCE}/lib/plugins/platform/rxplatform.c"
        PROPERTIES COMPILE_DEFINITIONS "PLUGIN_ID=rxplatform")
set_source_files_properties("${CREXX_SOURCE}/lib/plugins/sqlite/vendor/sqlite3.c"
        PROPERTIES COMPILE_DEFINITIONS
        "SQLITE_THREADSAFE=1;SQLITE_ENABLE_FTS5=1;SQLITE_DQS=0;SQLITE_OMIT_LOAD_EXTENSION=1;SQLITE_ENABLE_API_ARMOR=1;SQLITE_SECURE_DELETE=1"
        COMPILE_OPTIONS "-w")
target_link_libraries(crexx_rag_query_vm PRIVATE rxvm_single_embed m)
set_target_properties(crexx_rag_query_vm PROPERTIES SUFFIX ".mjs")
target_link_options(crexx_rag_query_vm PRIVATE --no-entry
        -sALLOW_MEMORY_GROWTH=1 -sINITIAL_MEMORY=134217728
        -sMAXIMUM_MEMORY=1073741824 -sSTACK_SIZE=1048576
        -sMODULARIZE=1 -sEXPORT_ES6=1 -sENVIRONMENT=worker,node
        "-sEXPORTED_FUNCTIONS=['_rag_vm_create','_rag_vm_destroy','_rag_vm_load','_rag_vm_run','_rag_vm_program_rc','_rag_vm_error','_malloc','_free']"
        "-sEXPORTED_RUNTIME_METHODS=['FS','ccall','HEAPU8']")

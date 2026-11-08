cmake_minimum_required(VERSION 3.24)
project(crexx_ui_web_vm C)
if(NOT EMSCRIPTEN)
    message(FATAL_ERROR "Configure with the Emscripten toolchain")
endif()
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(CREXX_BUILD_COMPILER OFF CACHE BOOL "" FORCE)
set(CREXX_VM_CLASSIC_PROVIDERS OFF CACHE BOOL "" FORCE)
set(CREXX_VM_CLASSIC_FULL_PROVIDERS OFF CACHE BOOL "" FORCE)
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../single-threaded" crexx-vm EXCLUDE_FROM_ALL)
add_executable(crexx_ui_vm ui_vm.c streams.c)
target_link_libraries(crexx_ui_vm PRIVATE rxvm_single_embed)
set_target_properties(crexx_ui_vm PROPERTIES SUFFIX ".mjs")
target_link_options(crexx_ui_vm PRIVATE --no-entry
        -sASYNCIFY=1 -sASYNCIFY_STACK_SIZE=1048576 -sSTACK_SIZE=1048576
        -sALLOW_MEMORY_GROWTH=1 -sINITIAL_MEMORY=67108864
        -sMODULARIZE=1 -sEXPORT_ES6=1 -sENVIRONMENT=worker,node
        "-sEXPORTED_FUNCTIONS=['_ui_vm_create','_ui_vm_destroy','_ui_vm_load','_ui_vm_run','_ui_vm_program_rc','_ui_vm_error','_malloc','_free']"
        "-sEXPORTED_RUNTIME_METHODS=['ccall','HEAPU8','HEAPU32']"
        -Wl,--wrap=__wasi_fd_read,--wrap=__wasi_fd_write)

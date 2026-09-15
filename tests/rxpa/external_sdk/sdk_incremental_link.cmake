# Exercise the installed declaration/static/relative helpers after an archive
# changes without changing its consumer. A target-order dependency alone leaves
# the old executable in place on Unix linkers.
file(WRITE "${CMAKE_BINARY_DIR}/incremental-library.c"
    "int rxincremental_init_;\nint incremental_value(void) { return 41; }\n")
file(WRITE "${CMAKE_BINARY_DIR}/incremental-main.c"
    "#include <stdlib.h>\nint incremental_value(void);\nint main(int argc, char **argv) { return argc != 2 || incremental_value() != atoi(argv[1]); }\n")
add_library(incremental_decl STATIC "${CMAKE_BINARY_DIR}/incremental-library.c")
add_library(incremental_static STATIC "${CMAKE_BINARY_DIR}/incremental-library.c")
foreach(kind IN ITEMS decl static relative)
    add_executable(incremental_${kind}_consumer
        "${CMAKE_BINARY_DIR}/incremental-main.c")
endforeach()
configure_linker_for_decl_lib(incremental_decl_consumer incremental)
configure_linker_for_static_lib(incremental_static_consumer incremental)
configure_linker_for_static_lib_rel(incremental_relative_consumer unused incremental)
add_dependencies(incremental_decl_consumer incremental_decl)
add_dependencies(incremental_static_consumer incremental_static)
add_dependencies(incremental_relative_consumer incremental_static)
add_custom_target(rxpa_incremental_consumers DEPENDS
    incremental_decl_consumer incremental_static_consumer incremental_relative_consumer)
file(GENERATE OUTPUT "${CMAKE_BINARY_DIR}/incremental-paths.cmake" CONTENT
    "set(incremental_consumers \"$<TARGET_FILE:incremental_decl_consumer>\" \"$<TARGET_FILE:incremental_static_consumer>\" \"$<TARGET_FILE:incremental_relative_consumer>\")\n")

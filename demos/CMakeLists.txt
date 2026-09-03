cmake_minimum_required(VERSION 3.24)

add_subdirectory(llm)
add_subdirectory(native/sqlite)

add_custom_target(crexx-demos)
add_dependencies(crexx-demos
        llm_generate_demos_bin
        llm_address_demos_bin
        sqlite_address_demo_bin)

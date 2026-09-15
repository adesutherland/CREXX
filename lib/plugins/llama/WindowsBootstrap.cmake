# MIT. Executable-adjacent Windows imports, without duplicating GPU payloads.
include("${INPUT}")
include("${CMAKE_CURRENT_LIST_DIR}/../../../rxpa/RuntimePackagePaths.cmake" NO_POLICY_SCOPE)
get_filename_component(bin "${output}" DIRECTORY)
file(GET_RUNTIME_DEPENDENCIES LIBRARIES ${bootstrap_roots}
    DIRECTORIES "${output}" ${runtime_search_directories}
    RESOLVED_DEPENDENCIES_VAR bootstrap_dependencies
    UNRESOLVED_DEPENDENCIES_VAR missing
    PRE_EXCLUDE_REGEXES "api-ms-.*" "ext-ms-.*"
    POST_EXCLUDE_REGEXES "${crexx_windows_system_dll_regex}")
if(missing)
    message(FATAL_ERROR "Unresolved inference bootstrap dependencies: ${missing}")
endif()
set(files ${bootstrap_roots} ${bootstrap_dependencies} ${bootstrap_runtime})
list(REMOVE_DUPLICATES files)
set(names "")
foreach(path IN LISTS files)
    get_filename_component(name "${path}" NAME)
    file(COPY_FILE "${path}" "${bin}/${name}" ONLY_IF_DIFFERENT)
    list(APPEND names "${name}")
endforeach()
list(REMOVE_DUPLICATES names)
list(SORT names)
string(JOIN "\n" names ${names})
# QA inventory stays outside the installed bin directory. The complete public
# native/runtime manifests continue to describe the provider package.
get_filename_component(inventory_directory "${inventory}" DIRECTORY)
file(MAKE_DIRECTORY "${inventory_directory}")
file(WRITE "${inventory}" "${names}\n")

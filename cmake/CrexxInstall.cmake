include_guard(GLOBAL)

# Product owners register only their published targets. Test targets never enter
# this set merely because they share the build output directory.
function(crexx_install_targets)
    install(TARGETS ${ARGN}
        RUNTIME DESTINATION bin LIBRARY DESTINATION bin ARCHIVE DESTINATION bin)
    set_property(GLOBAL APPEND PROPERTY CREXX_INSTALLED_PRODUCT_TARGETS ${ARGN})
endfunction()

function(crexx_install_bytecode target path)
    install(FILES "${path}" DESTINATION bin)
    set_property(GLOBAL APPEND PROPERTY CREXX_INSTALLED_PRODUCT_TARGETS "${target}")
endfunction()

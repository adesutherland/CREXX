# This inventory is intentionally prepared through stage-product, never through
# the individual producers: an ALL-only target does not satisfy CI's selection.
foreach(_name IN ITEMS llcompilers.rxbin llctarget.rxbin
        "llrxc-c${EXECUTABLE_SUFFIX}" "llrxc-g${EXECUTABLE_SUFFIX}")
    set(_file "${BIN_DIRECTORY}/${_name}")
    if(NOT EXISTS "${_file}" OR IS_DIRECTORY "${_file}")
        message(FATAL_ERROR "Level L product is missing: ${_name}")
    endif()
    file(SIZE "${_file}" _bytes)
    if(_bytes EQUAL 0)
        message(FATAL_ERROR "Level L product is empty: ${_name}")
    endif()
endforeach()
message(STATUS "Level L compiler product inventory: PASS")

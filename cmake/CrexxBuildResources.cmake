# Named Ninja resource pools for build actions whose pressure is not represented
# by the global --parallel value.  Other generators retain their normal
# scheduling; the profile remains visible so the same configuration can be
# carried into a Ninja or future Level B scheduler.

set(CREXX_BUILD_RESOURCE_PROFILE "auto" CACHE STRING
        "Build resource profile: auto, developer-fast, portable, or memory-constrained")
set_property(CACHE CREXX_BUILD_RESOURCE_PROFILE PROPERTY STRINGS
        auto developer-fast portable memory-constrained)
set(CREXX_VM_COMPILE_POOL_DEPTH "" CACHE STRING
        "Override the selected profile's concurrent VM core compile limit")
set(CREXX_NATIVE_LINK_POOL_DEPTH "" CACHE STRING
        "Override the selected profile's concurrent native link limit")

string(TOLOWER "${CREXX_BUILD_RESOURCE_PROFILE}" _crexx_resource_profile)
if(_crexx_resource_profile STREQUAL "auto")
    if(APPLE AND CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "^(arm64|aarch64)$")
        set(_crexx_resource_profile "developer-fast")
    else()
        set(_crexx_resource_profile "portable")
    endif()
endif()

if(_crexx_resource_profile STREQUAL "developer-fast")
    set(_crexx_default_vm_compile_depth 4)
    set(_crexx_default_native_link_depth 6)
elseif(_crexx_resource_profile STREQUAL "portable")
    set(_crexx_default_vm_compile_depth 2)
    set(_crexx_default_native_link_depth 2)
elseif(_crexx_resource_profile STREQUAL "memory-constrained")
    set(_crexx_default_vm_compile_depth 1)
    set(_crexx_default_native_link_depth 1)
else()
    message(FATAL_ERROR
            "CREXX_BUILD_RESOURCE_PROFILE must be auto, developer-fast, portable, or memory-constrained")
endif()

if(CREXX_VM_COMPILE_POOL_DEPTH STREQUAL "")
    set(_crexx_vm_compile_depth ${_crexx_default_vm_compile_depth})
else()
    set(_crexx_vm_compile_depth ${CREXX_VM_COMPILE_POOL_DEPTH})
endif()
if(CREXX_NATIVE_LINK_POOL_DEPTH STREQUAL "")
    set(_crexx_native_link_depth ${_crexx_default_native_link_depth})
else()
    set(_crexx_native_link_depth ${CREXX_NATIVE_LINK_POOL_DEPTH})
endif()

foreach(_crexx_depth IN ITEMS
        _crexx_vm_compile_depth
        _crexx_native_link_depth)
    if(NOT "${${_crexx_depth}}" MATCHES "^[1-9][0-9]*$")
        message(FATAL_ERROR
                "${_crexx_depth} must resolve to a positive integer, got '${${_crexx_depth}}'")
    endif()
endforeach()

set(CREXX_RESOLVED_BUILD_RESOURCE_PROFILE "${_crexx_resource_profile}"
        CACHE INTERNAL "Resolved cREXX build resource profile" FORCE)
set(CREXX_RESOLVED_VM_COMPILE_POOL_DEPTH "${_crexx_vm_compile_depth}"
        CACHE INTERNAL "Resolved VM compile pool depth" FORCE)
set(CREXX_RESOLVED_NATIVE_LINK_POOL_DEPTH "${_crexx_native_link_depth}"
        CACHE INTERNAL "Resolved native link pool depth" FORCE)

set(CREXX_VM_COMPILE_JOB_POOL "")
set(CREXX_NATIVE_LINK_JOB_POOL "")
set(CREXX_BUILD_RESOURCE_POOLS_ACTIVE OFF)
if(CMAKE_GENERATOR MATCHES "^Ninja")
    set(CREXX_VM_COMPILE_JOB_POOL crexx_vm_compile)
    set(CREXX_NATIVE_LINK_JOB_POOL crexx_native_link)
    set_property(GLOBAL APPEND PROPERTY JOB_POOLS
            "${CREXX_VM_COMPILE_JOB_POOL}=${_crexx_vm_compile_depth}"
            "${CREXX_NATIVE_LINK_JOB_POOL}=${_crexx_native_link_depth}")
    set(CMAKE_JOB_POOL_LINK "${CREXX_NATIVE_LINK_JOB_POOL}")
    set(CREXX_BUILD_RESOURCE_POOLS_ACTIVE ON)
    set(_crexx_resource_backend "Ninja pools")
else()
    set(_crexx_resource_backend "generator default")
endif()

function(crexx_apply_vm_compile_resource_pool target_name)
    if(CREXX_BUILD_RESOURCE_POOLS_ACTIVE)
        if(NOT TARGET ${target_name})
            message(FATAL_ERROR
                    "Cannot apply the VM compile resource pool to missing target ${target_name}")
        endif()
        set_property(TARGET ${target_name} PROPERTY
                JOB_POOL_COMPILE "${CREXX_VM_COMPILE_JOB_POOL}")
    endif()
endfunction()

message(STATUS
        "cREXX build resources: profile=${_crexx_resource_profile}, "
        "vm-compile=${_crexx_vm_compile_depth}, native-link=${_crexx_native_link_depth}, "
        "backend=${_crexx_resource_backend}")

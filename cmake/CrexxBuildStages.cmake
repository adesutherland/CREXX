include_guard(GLOBAL)

# Human-facing stage targets name useful product selections without owning files
# or restating the underlying action graph.  CMake/Ninja still schedules each
# real target from its direct dependencies, so these aggregates add no stage
# barrier or shared publication point.
function(crexx_add_build_stage STAGE_NAME)
    set(options)
    set(oneValueArgs DESCRIPTION)
    set(multiValueArgs REQUIRED_TARGETS OPTIONAL_TARGETS)
    cmake_parse_arguments(CREXX_STAGE
            "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(TARGET "${STAGE_NAME}")
        message(FATAL_ERROR "Build stage target already exists: ${STAGE_NAME}")
    endif()

    add_custom_target("${STAGE_NAME}")
    set_property(TARGET "${STAGE_NAME}" PROPERTY FOLDER "Build stages")
    if(CREXX_STAGE_DESCRIPTION)
        set_property(TARGET "${STAGE_NAME}" PROPERTY
                CREXX_STAGE_DESCRIPTION "${CREXX_STAGE_DESCRIPTION}")
    endif()

    foreach(_crexx_stage_dependency IN LISTS CREXX_STAGE_REQUIRED_TARGETS)
        if(NOT TARGET "${_crexx_stage_dependency}")
            message(FATAL_ERROR
                    "Build stage ${STAGE_NAME} requires missing target: ${_crexx_stage_dependency}")
        endif()
        add_dependencies("${STAGE_NAME}" "${_crexx_stage_dependency}")
    endforeach()

    foreach(_crexx_stage_dependency IN LISTS CREXX_STAGE_OPTIONAL_TARGETS)
        if(TARGET "${_crexx_stage_dependency}")
            add_dependencies("${STAGE_NAME}" "${_crexx_stage_dependency}")
        endif()
    endforeach()
endfunction()

crexx_add_build_stage(stage-c0-native
        DESCRIPTION "C0 native foundations"
        REQUIRED_TARGETS platform avl_tree rxbin rxpa)

crexx_add_build_stage(stage-c1-toolchain
        DESCRIPTION "C1 core C toolchain"
        REQUIRED_TARGETS rxc rxas rxlink rxdas rxbvm rxvm rxseq rxcpack crexx-contract
        OPTIONAL_TARGETS rxtvm)

crexx_add_build_stage(stage-b0-bootstrap
        DESCRIPTION "B0 Level B bootstrap library"
        REQUIRED_TARGETS library)

crexx_add_build_stage(stage-x-exits
        DESCRIPTION "X certified exits"
        REQUIRED_TARGETS compiler_exit_bin)

crexx_add_build_stage(stage-b1-substrate
        DESCRIPTION "B1 Level B class and native substrate"
        REQUIRED_TARGETS classlib classlib_native veclib _hash float fs stats vector)

crexx_add_build_stage(stage-c-rexx-tools
        DESCRIPTION "C core REXX-based tools and Level C library"
        REQUIRED_TARGETS rxfnsc rexxscript rxpp rxdb
        OPTIONAL_TARGETS rxpp_sh)

crexx_add_build_stage(stage-g-library
        DESCRIPTION "G Level G library"
        REQUIRED_TARGETS rxfnsg)

crexx_add_build_stage(stage-l-libraries
        DESCRIPTION "L Level L libraries"
        REQUIRED_TARGETS rxfnsl)

crexx_add_build_stage(stage-product
        DESCRIPTION "Assembled cREXX product"
        REQUIRED_TARGETS
            stage-c-rexx-tools
            stage-g-library
            stage-l-libraries
            crexx
            rexxscript_cli
            rxvme
            rxbvme
            rxvml
            rxbvml
            crexxsaa
            crexxsaa_tool)

crexx_add_build_stage(stage-optional
        DESCRIPTION "Optional examples and demonstrations"
        REQUIRED_TARGETS crexx-examples crexx-demos)

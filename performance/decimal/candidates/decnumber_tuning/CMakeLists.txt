cmake_minimum_required(VERSION 3.24)

set(_d2_mc_root
    ${CMAKE_SOURCE_DIR}/interpreter/rxvmplugin/rxvmplugins/mc_decimal)
set(_d2_decnumber_root ${_d2_mc_root}/decnumber)
set(_d2_targets)

foreach(_d2_dpun IN ITEMS 3 4 8 9)
    foreach(_d2_digits IN ITEMS 18 34 64)
        foreach(_d2_buffer IN ITEMS 20 36 64 128)
            set(_d2_id
                d2_p${_d2_dpun}_n${_d2_digits}_b${_d2_buffer})
            set(_d2_library decnumber_${_d2_id})
            set(_d2_plugin mc_decimal_${_d2_id}_dynamic)
            set(_d2_definitions
                DECDPUN=${_d2_dpun}
                DECNUMDIGITS=${_d2_digits}
                DECBUFFER=${_d2_buffer})

            add_library(${_d2_library} STATIC
                ${_d2_decnumber_root}/decNumber.c
                ${_d2_decnumber_root}/decContext.c)
            target_include_directories(${_d2_library}
                PUBLIC ${_d2_decnumber_root})
            target_compile_definitions(${_d2_library}
                PRIVATE ${_d2_definitions})
            set_target_properties(${_d2_library} PROPERTIES
                POSITION_INDEPENDENT_CODE ON)

            add_dynamic_rxvmplugin_target(${_d2_plugin}
                ${_d2_mc_root}/mc_decimal.c)
            target_include_directories(${_d2_plugin}
                PRIVATE ${_d2_decnumber_root})
            target_compile_definitions(${_d2_plugin}
                PRIVATE ${_d2_definitions})
            target_link_libraries(${_d2_plugin} ${_d2_library})
            set_target_properties(${_d2_plugin} PROPERTIES
                OUTPUT_NAME mc_decimal_${_d2_id})
            list(APPEND _d2_targets ${_d2_plugin})

            if(TARGET mc_decimal_full_tests)
                add_test(
                    NAME ${_d2_id}_contract
                    COMMAND $<TARGET_FILE:mc_decimal_full_tests>
                            rxvm_mc_decimal_${_d2_id})
                set_tests_properties(${_d2_id}_contract PROPERTIES
                    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
                    LABELS "candidate;decimal;decimal01;d2;qualification")
            endif()
        endforeach()
    endforeach()
endforeach()

add_custom_target(decnumber_tuning_candidates DEPENDS ${_d2_targets})

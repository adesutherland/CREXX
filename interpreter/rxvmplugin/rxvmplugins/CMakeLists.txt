cmake_minimum_required(VERSION 3.24)

add_subdirectory(mc_decimal)
add_subdirectory(db_decimal)

option(CREXX_BUILD_MPD_DECIMAL_CANDIDATE
       "Build the non-default DECIMAL-01 libmpdec candidate plugin" OFF)
if(CREXX_BUILD_MPD_DECIMAL_CANDIDATE)
    add_subdirectory(
            ${CMAKE_SOURCE_DIR}/performance/decimal/candidates/mpd_decimal
            ${CMAKE_BINARY_DIR}/performance/decimal/candidates/mpd_decimal)
endif()

option(CREXX_BUILD_DECNUMBER_TUNING_CANDIDATES
       "Build the non-default DECIMAL-01 decNumber tuning grid" OFF)
if(CREXX_BUILD_DECNUMBER_TUNING_CANDIDATES)
    add_subdirectory(
            ${CMAKE_SOURCE_DIR}/performance/decimal/candidates/decnumber_tuning
            ${CMAKE_BINARY_DIR}/performance/decimal/candidates/decnumber_tuning)
endif()

option(CREXX_BUILD_DECQUAD_DECIMAL_CANDIDATE
       "Build the non-default fixed-34 DECIMAL-01 decQuad candidate" OFF)
if(CREXX_BUILD_DECQUAD_DECIMAL_CANDIDATE)
    add_subdirectory(
            ${CMAKE_SOURCE_DIR}/performance/decimal/candidates/decquad_decimal
            ${CMAKE_BINARY_DIR}/performance/decimal/candidates/decquad_decimal)
endif()

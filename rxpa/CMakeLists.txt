# Minimum CMake version requirement
cmake_minimum_required(VERSION 3.24)

# Project name declaration (optional for subdirectories)
project(CREXX_PLUGIN_ARCHITECTURE)

# Build library for the RX Plugin Architecture
add_library(rxpa STATIC rxpa.c rxpa.h crexxpa.h)

# rxpa is linked into shared objects such as libcrexxsaa on ELF platforms.
set_target_properties(rxpa PROPERTIES POSITION_INDEPENDENT_CODE ON)

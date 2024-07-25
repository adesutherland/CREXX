# Minimum CMake version requirement
cmake_minimum_required(VERSION 3.5)

# Project name declaration (optional for subdirectories)
project(CREXX_PLUGIN_ARCHITECTURE)

# Build library for the RX Plugin Architecture
add_library(rxpa STATIC rxpa.c rxpa.h crexxpa.h)

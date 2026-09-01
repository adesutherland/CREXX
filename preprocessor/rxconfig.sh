#!/bin/bash

# -------------------------------------------------------------
# Configuration File loaded
# -------------------------------------------------------------

echo "Configuration File loaded"

# Set precompiler name and plugin
preCompiler="rxpp"
plugin="precomp"
conf="L"

# Set paths
home="$HOME/CLionProjects/CREXX/250601"
build="$home/cmake-build-debug"
pluglib="$build/bin"
sourcelib="$home/preprocessor"
lib="$build/bin/library"
rxc="$build/bin"
rxas="$build/bin"
rxvm="$build/bin"
rxpre="$build/bin/$preCompiler"

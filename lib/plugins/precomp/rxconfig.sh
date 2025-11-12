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
pluglib="$build/lib/plugins/$plugin"
sourcelib="$home/lib/plugins/$plugin"
lib="$build/lib/rxfnsb/library"
rxc="$build/compiler"
rxas="$build/assembler"
rxvm="$build/interpreter"
rxpre="$pluglib/$preCompiler"

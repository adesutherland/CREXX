#!/bin/bash

# Paths to tools (relative to project root)
RXC="../../../cmake-build-debug/compiler/rxc"
RXAS="../../../cmake-build-debug/assembler/rxas"
RXVM="../../../cmake-build-debug/interpreter/rxvm"

# Ensure we are in the correct directory
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd "$DIR"

# Function to compile and assemble a file
compile_file() {
    local file=$1
    local name="${file%.*}"
    
    echo "--------------------------------------------------"
    echo "Compiling $file..."
    $RXC -o "$name" "$file" > "$name.rxc.out" 2>&1
    local res=$?
    if [ $res -ne 0 ]; then
        echo "ERROR: Compilation failed for $file"
        cat "$name.rxc.out"
        return 1
    fi
    
    echo "Assembling $name.rxas..."
    $RXAS -o "$name.rxbin" "$name.rxas" > "$name.rxas.out" 2>&1
    res=$?
    if [ $res -ne 0 ]; then
        echo "ERROR: Assembly failed for $name.rxas"
        cat "$name.rxas.out"
        return 1
    fi
    echo "Success: $name.rxbin created."
    return 0
}

echo "=== Step 2: Interface Syntax Verification ==="

# Compile all files
compile_file nonclass2_lib.rexx
compile_file nonclass2_main.rexx
compile_file class2_lib.rexx
compile_file class2_main.rexx

echo ""
echo "=== Execution Phase ==="

# 1. Non-class Test
if [ -f "nonclass2_main.rxbin" ] && [ -f "nonclass2_lib.rxbin" ]; then
    echo "Running nonclass2 test..."
    $RXVM nonclass2_main.rxbin nonclass2_lib.rxbin
    echo "Exit code: $?"
else
    echo "Skipping nonclass2 execution due to missing binaries."
fi

echo ""

# 2. Class Test
if [ -f "class2_main.rxbin" ] && [ -f "class2_lib.rxbin" ]; then
    echo "Running class2 test..."
    $RXVM class2_main.rxbin class2_lib.rxbin
    echo "Exit code: $?"
else
    echo "Skipping class2 execution due to missing binaries."
fi

#!/bin/bash

# Paths to tools (relative to project root)
RXC="../../../cmake-build-debug/compiler/rxc"
RXAS="../../../cmake-build-debug/assembler/rxas"
RXVM="../../../cmake-build-debug/interpreter/rxvm"

# Ensure we are in the correct directory
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd "$DIR"

# Function to run a test
run_test() {
    local file=$1
    local name="${file%.*}"
    
    echo "=================================================="
    echo "Testing $file..."
    
    # 1. Compile
    echo "Step 1: Compiling $file to $name.rxas..."
    $RXC -o "$name" "$file" > "$name.rxc.out" 2>&1
    local res=$?
    if [ $res -ne 0 ]; then
        echo "ERROR: Compilation failed with exit code $res"
        cat "$name.rxc.out"
        return 1
    fi
    echo "Compilation successful."
    
    # 2. Assemble
    echo "Step 2: Assembling $name.rxas to $name.rxbin..."
    $RXAS -o "$name.rxbin" "$name.rxas" > "$name.rxas.out" 2>&1
    res=$?
    if [ $res -ne 0 ]; then
        echo "ERROR: Assembly failed with exit code $res"
        cat "$name.rxas.out"
        return 1
    fi
    echo "Assembly successful."
    
    # 3. Run
    echo "Step 3: Running $name.rxbin with rxvm..."
    $RXVM "$name.rxbin"
    res=$?
    if [ $res -ne 0 ]; then
        echo "ERROR: Execution failed with exit code $res"
        return 1
    fi
    echo "Execution successful."
    return 0
}

run_test nonclass1.rexx
run_test class1.rexx

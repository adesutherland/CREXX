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

echo "=== Step 3: Source-Based Class Import Verification ==="

# Compile library first
compile_file class3_lib.rexx

# Compile main (which imports class3_lib)
compile_file class3_main.rexx

# Show generated RXAS and ensure stubs are present
echo ""
echo "--- class3_main.rxas (tail) ---"
tail -n 30 class3_main.rxas || true

echo ""
echo "--- Checking for imported class stubs ---"
grep -n "§class3_lib\.counter\." class3_main.rxas || true

echo ""
echo "=== Execution Phase ==="

if [ -f "class3_main.rxbin" ] && [ -f "class3_lib.rxbin" ]; then
    echo "Running class3 test..."
    $RXVM class3_main.rxbin class3_lib.rxbin
    echo "Exit code: $?"
else
    echo "Skipping class3 execution due to missing binaries."
fi

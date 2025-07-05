#!/bin/bash

# -------------------------------------------------------------
# Call configuration file for pre-compile, compile, assembly and run
# -------------------------------------------------------------

if [ "$conf" != "L" ]; then
    source ./rxconfig.sh
fi

# Change to pluglib directory
pushd "$pluglib" > /dev/null

# Build the command
cmd="$rxc/rxc -i $build/lib/rxfnsb:$pluglib -o $member $sourcelib/$member"

echo "----------------------------------------------------"
echo "[$(date +%T)] Compile: $cmd"
echo "----------------------------------------------------"

# Execute the command
eval "$cmd"
ret=$?

echo "[$(date +%T)] Compile completed with Return $ret"
echo "===================================================="

# Return to previous directory
popd > /dev/null

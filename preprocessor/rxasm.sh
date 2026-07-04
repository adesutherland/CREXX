#!/bin/bash

# -------------------------------------------------------------
# Call configuration file for pre-compile, compile, assembly and run
# -------------------------------------------------------------

if [ "$conf" != "L" ]; then
    source ./rxconfig.sh
fi

# Change to pluglib directory safely
if [ -d "$pluglib" ]; then
    pushd "$pluglib" > /dev/null
else
    echo "Directory '$pluglib' does not exist. Exiting."
    exit 1
fi

# Build the assembly command
cmd="$rxas/rxas -l $pluglib -o $member $member"

# Logging the command
echo "----------------------------------------------------"
echo "[$(date +%T)] ASM and Link: $cmd"
echo "----------------------------------------------------"

# Execute the command
eval "$cmd"
ret=$?

# Log result
echo "[$(date +%T)] ASM and Link completed with Return $ret"
echo "===================================================="

# Return to previous directory
popd > /dev/null

# Optionally return the exit status
exit $ret

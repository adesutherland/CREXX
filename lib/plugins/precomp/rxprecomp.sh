#!/bin/bash

# -------------------------------------------------------------
# Call configuration file for pre-compile
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

# Build the PreCompile command
cmd="$rxvm/rxvm $rxpre rx_$plugin $lib -a -i \"$sourcelib/$inrexx\" -o \"$sourcelib/$genrexx\" -m \"$sourcelib/$maclib\""

# Log the command
echo "----------------------------------------------------"
echo "[$(date +%T)] PreCompile: $cmd"
echo "----------------------------------------------------"

# Execute the command
eval $cmd
ret=$?

# Log result
echo "[$(date +%T)] PreCompile completed with Return $ret"
echo "===================================================="

# Return to the previous directory
popd > /dev/null

# Return the exit status
exit $ret

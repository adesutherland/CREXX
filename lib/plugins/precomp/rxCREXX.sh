#!/bin/bash
# -------------------------------------------------------------
# Compile a CREXX Script
# -------------------------------------------------------------

# Input parameters
flags="$1"
inrexx="$2"
genrexx="$3"
maclib="$4"

# Extract filename without extension
member=$(basename "$genrexx" | cut -d. -f1)

# Source the rxflags.sh script to set compilation flags
source ./rxflags.sh

# -------------------------------------------------------------
# Pre Compile CREXX Script
# -------------------------------------------------------------
if [ "$precomp" = "P" ]; then
    source ./rxprecomp.sh
fi

# -------------------------------------------------------------
# Compile CREXX Script
# -------------------------------------------------------------
if [ "$compile" = "C" ]; then
    source ./rxcompile.sh
fi

# -------------------------------------------------------------
# Assembly and Link the compiled CREXX Script
# -------------------------------------------------------------
if [ "$asm" = "A" ]; then
    source ./rxasm.sh
fi

# -------------------------------------------------------------
# Execute the CREXX script
# -------------------------------------------------------------
if [ "$run" = "R" ]; then
    source ./rxrun.sh
fi

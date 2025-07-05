#!/bin/bash

# ----------------------------------------
# Parse up to 5 characters from $flags
# ----------------------------------------

# Initialize control flags
PRECOMP=""
COMPILE=""
ASM=""
RUN=""

# Loop over first 5 characters of flags
for (( i=0; i<5 && i<${#flags}; i++ )); do
    char="${flags:$i:1}"
    case "$char" in
        P) PRECOMP="P" ;;
        C) COMPILE="C" ;;
        A) ASM="A" ;;
        R) RUN="R" ;;
    esac
done

conf="L"

# Debug output (uncomment to see results)
# echo "RUN=$RUN"
# echo "PRECOMP=$PRECOMP"
# echo "COMPILE=$COMPILE"
# echo "ASM=$ASM"

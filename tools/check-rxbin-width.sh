#!/bin/sh
# Focused RXBIN reader/writer check. A native RXAS generates the input; only the
# portable binary library and its regression are compiled for the selected ABI.
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
if [ "$#" -ne 2 ]; then
    echo "usage: $0 native-rxas output-directory (CC/CFLAGS select target ABI)" >&2
    exit 2
fi
rxas=$1
mkdir -p "$2"
work=$(CDPATH= cd -- "$2" && pwd)
"$rxas" -o "$work/tests_compact_format" "$root/interpreter/tests/tests_compact_format.rxas"
# CFLAGS deliberately undergoes word splitting to accept compiler switches.
${CC:-cc} ${CFLAGS:-} -std=c99 -I "$root/binutils/include" -I "$root/platform" \
    -I "$root/assembler" "$root/interpreter/tests/test_compact_format.c" \
    "$root/binutils/rxbin.c" "$root/binutils/rxbin007.c" \
    "$root/binutils/rxsignature.c" "$root/binutils/rxsha256.c" \
    "$root/binutils/rxgraph.c" -o "$work/compact-format"
(cd "$work" && ./compact-format)
echo "PASS RXBIN cross-width reader/writer (${CC:-cc} ${CFLAGS:-native})"

#!/bin/sh
echo "Performance Test"

echo "rxc perf"
../../cmake-build-debug/compiler/rxc -v
../../cmake-build-debug/compiler/rxc perf

echo "rxas perf"
../../cmake-build-release/assembler/rxas perf

echo "rxdas perf"
../../cmake-build-release/disassembler/rxdas perf

echo "rxvm perf - release"
../../cmake-build-release/interpreter/rxvm perf

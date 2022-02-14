#!/bin/sh
echo "Performance Test 1 "

echo "rxas perf1"
../../cmake-build-release/assembler/rxas perf1

echo "rxdas perf1"
../../cmake-build-release/disassembler/rxdas perf1

echo "rxvm perf1 - debug"
../../cmake-build-debug/interpreter/rxvm perf1

echo "rxvm perf1 - release"
../../cmake-build-release/interpreter/rxvm perf1

echo "rxvm perf1 - bytecode mode - debug"
../../cmake-build-debug/interpreter/rxbvm perf1

echo "rxvm perf1 - bytecode mode - release"
../../cmake-build-release/interpreter/rxbvm perf1

#!/bin/sh
echo "Test Scripts"

echo "rxas main"
../cmake-build-debug/assembler/rxas main

echo "rxas func1"
../cmake-build-debug/assembler/rxas func1

echo "rxas func2"
../cmake-build-debug/assembler/rxas func2

echo "rxdas main"
../cmake-build-debug/disassembler/rxdas main

echo "rxdas func1"
../cmake-build-debug/disassembler/rxdas func1

echo "rxdas func2"
../cmake-build-debug/disassembler/rxdas func2

echo "rxvm main func1 func2"
../cmake-build-debug/interpreter/rxvm main func1 func2

#!/bin/sh
echo "Running $1"

echo "rxc $1"
../cmake-build-debug/compiler/rxc $1

echo "rxas $1"
../cmake-build-debug/assembler/rxas $1

#echo "rxdas $1"
#../cmake-build-debug/disassembler/rxdas $1

echo "rxvm $1"
../cmake-build-debug/interpreter/rxvm $1

#!/bin/sh
echo "Running $1"

echo "rxc -d $1"
../cmake-build-debug/compiler/rxc -d $1

echo "rxas -d $1"
../cmake-build-debug/assembler/rxas -d $1

#echo "rxdas -d $1"
#../cmake-build-debug/disassembler/rxdas -d $1

echo "rxvm -d $1"
../cmake-build-debug/interpreter/rxvm -d $1

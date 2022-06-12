#!/bin/sh
echo "UTF Test 1"

echo "rxc utf1"
../cmake-build-debug/compiler/rxc utf1

echo "rxas utf1"
../cmake-build-debug/assembler/rxas utf1

echo "rxdas utf1"
../cmake-build-debug/disassembler/rxdas utf1

echo "rxvm utf1"
../cmake-build-debug/interpreter/rxvm utf1

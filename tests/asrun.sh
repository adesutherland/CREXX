#!/bin/sh
echo "Running $1"

echo "cleanup"
rm $1".rxbin"

echo "rxas $1"
../cmake-build-debug/assembler/rxas $1

echo "rxvm $1"
../cmake-build-debug/interpreter/rxvm $1

#!/bin/sh
echo "Running $1"

echo "cleanup"
rm $1".rxas"
rm $1".rxbin"

echo "rxc $1"
../cmake-build-debug-mingw/compiler/rxc $1

echo "rxas $1"
../cmake-build-debug-mingw/assembler/rxas $1

echo "rxvm $1"
../cmake-build-debug-mingw/interpreter/rxvm $1

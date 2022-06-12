#!/bin/sh
echo "cleanup"
rm $1".rxas"
rm $1".rxbin"

echo "rxc $1"
../cmake-build-debug-mingw/compiler/rxc $1

echo "rxas $1"
../cmake-build-debug-mingw/assembler/rxas $1

echo "rxdas $1"
../cmake-build-debug-mingw/disassembler/rxdas $1

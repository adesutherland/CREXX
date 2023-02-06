#!/bin/sh
echo "Debugging $1"

echo "cleanup"
rm $1".rxas"
rm $1".rxbin"

echo "rxc $1"
../cmake-build-debug/compiler/rxc -i ../cmake-build-debug/lib/rxfns $1

echo "rxas $1"
../cmake-build-debug/assembler/rxas $1

echo "rxdb $1"
../cmake-build-debug/debugger/rxdb $1

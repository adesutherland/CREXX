#!/bin/sh
echo "Running $1"

echo "cleanup"
rm $1".rxas"
rm $1".rxbin"

echo "rxc $1"
../cmake-build-debug/compiler/rxc -i ../cmake-build-debug/lib/rxfnsb $1

echo "rxas $1"
../cmake-build-debug/assembler/rxas $1

#echo "rxdas $1"
#../cmake-build-debug/disassembler/rxdas $1

echo "rxvme $1"
../cmake-build-debug/interpreter/rxvme $1

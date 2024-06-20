#!/bin/sh
echo "Running $1 $2 $3 $4"

echo "cleanup"
rm $1".rxas"
rm $1".rxbin"

echo "rxc $1"
../cmake-build-release/compiler/rxc -i ../cmake-build-release/lib/rxfns $1

echo "rxas $1"
../cmake-build-release/assembler/rxas $1

echo "rxdas $1"
../cmake-build-release/disassembler/rxdas $1

echo "rxvm $1"
../cmake-build-release/interpreter/rxvm $1 ../cmake-build-release/lib/rxfns/library -a $2 $3 $4
echo rc=$?
#../cmake-build-release/interpreter/rxvm $1
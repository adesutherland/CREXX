#!/bin/sh
echo "Running $1"

echo "cleanup"
rm $1".rxas"
rm $1".rxbin"
rm astgraph0.dot astgraph0.png
rm astgraph1.dot astgraph1.png
rm astgraph2.dot astgraph2.png
rm astgraph3.dot astgraph3.png

echo "rxc -d $1"
../cmake-build-debug/compiler/rxc -i ../cmake-build-debug/lib/rxfnsb -d $1

echo "rxas -d $1"
../cmake-build-debug/assembler/rxas -d $1

echo "rxdas -p $1"
../cmake-build-debug/disassembler/rxdas -p $1

echo "rxvm -d $1"
../cmake-build-debug/interpreter/rxvm -d $1 ../cmake-build-debug/lib/rxfnsb/library

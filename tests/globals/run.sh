#!/bin/sh
echo "Running test"

echo "cleanup"
rm test".rxas"
rm test".rxbin"

echo "rxc test"
../../cmake-build-debug/compiler/rxc -i ../../cmake-build-debug/lib/rxfnsb test

echo "rxc test2"
../../cmake-build-debug/compiler/rxc -i ../../cmake-build-debug/lib/rxfnsb test2

echo "rxc test3"
../../cmake-build-debug/compiler/rxc -i ../../cmake-build-debug/lib/rxfnsb test3

echo "rxas test"
../../cmake-build-debug/assembler/rxas test

echo "rxas test2"
../../cmake-build-debug/assembler/rxas test2

echo "rxas test3"
../../cmake-build-debug/assembler/rxas test3

echo "rxdas test"
../../cmake-build-debug/disassembler/rxdas test

echo "rxdas test2"
../../cmake-build-debug/disassembler/rxdas test2

echo "rxdas test3"
../../cmake-build-debug/disassembler/rxdas test3

echo "rxvm test"
../../cmake-build-debug/interpreter/rxvm test test2 test3 ../../cmake-build-debug/lib/rxfnsb/library
echo Return Code is $?

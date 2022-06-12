#!/bin/sh
# cREXX Assembler Regression Tests

echo "cREXX Assembler Regression Tests"

echo "rxas ascommon"
../cmake-build-debug/assembler/rxas ascommon

echo "rxas asutf"
../cmake-build-debug/assembler/rxas asutf

echo "rxas asebcdic"
../cmake-build-debug/assembler/rxas asebcdic

echo "rxdas ascommon"
../cmake-build-debug/disassembler/rxdas ascommon

echo "rxdas asutf"
../cmake-build-debug/disassembler/rxdas asutf

echo "rxdas asebcdic"
../cmake-build-debug/disassembler/rxdas asebcdic

echo "rxvm ascommon"
../cmake-build-debug/interpreter/rxvm ascommon

echo "rxvm asutf"
../cmake-build-debug/interpreter/rxvm asutf

echo "rxvm asebcdic"
../cmake-build-debug/interpreter/rxvm asebcdic

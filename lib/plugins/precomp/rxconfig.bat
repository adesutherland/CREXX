@echo off
:: Set precompiler name and used plugin
echo Configuration File loaded 
set preCompiler=rxpp
set plugin=precomp
set conf=L

:: Set paths
set home=C:/Users/PeterJ/CLionProjects/CREXX/250606
set build=%home%/cmake-build-debug
set pluglib=%build%/lib/plugins/%plugin%
set sourcelib=%home%/lib/plugins/%plugin%
set lib=%build%/lib/rxfnsb/library
set rxc=%build%/compiler
set rxas=%build%/assembler
set rxvm=%build%/interpreter
set rxpre=%pluglib%/%preCompiler%

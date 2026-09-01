@echo off
:: Set precompiler name and used plugin
echo Configuration File loaded 
set preCompiler=rxpp
set plugin=precomp
set conf=L

:: Set paths
set home=C:/Users/PeterJ/CLionProjects/CREXX/250606
set build=%home%/cmake-build-debug
set pluglib=%build%/bin
set sourcelib=%home%/preprocessor
set lib=%build%/bin/library
set rxc=%build%/bin
set rxas=%build%/bin
set rxvm=%build%/bin
set rxpre=%build%/bin/%preCompiler%

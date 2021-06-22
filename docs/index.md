# CREXX

_Release Documentation - 16 Jan 2021_

## REXX Language Implementation Architecture

Project to develop a modern ground up implementation of a REXX
interpreter and compiler, and experiments with language improvements.
To implement REXX using the best language tools available
today, including the LLVM Compiler Infrastructure. These tools will
allow a REXX compiler to be produced supporting multiple backends
(including 64 bit architectures).

One aspect of the project is to revisit the REXX language - what can be
improved? And most importantly how can it be improved while keeping the
essence of REXX:

> to make programming easier than before

CREXX will be targeted to run on VM/370 (a nod to REXX's heritage)
and it will also run on Linux, Windows, OSX, and z/Architecture.

## Wiki Based Documentation

This represents the latest thoughts, aims, architecture, designs and details; some of this may have been built, some things may have been built to older designs and some may just be a future wish. This is the place where we are developing the cREXX Architecture.

Key Links:

- [Wiki Home](https://github.com/adesutherland/CREXX/wiki "CREXX Wiki Home")
- [Issues](https://github.com/adesutherland/CREXX/issues "CREXX Issues")
- [Discussions](https://github.com/adesutherland/CREXX/discussions "CREXX Discussions")
- [Project Kanban](https://github.com/adesutherland/CREXX/projects/1 "CREXX Kanban")
- And of course the [Github Home](https://github.com/adesutherland/CREXX "CREXX Github Home")

## Release Based Documentation

This is the "As Built" documentation, specific to its release; the current [develop branch](https://adesutherland.github.io/CREXX/) version is availabke as a website.

The documentation is stored in the code repository/branch under the [/doc](https://github.com/adesutherland/CREXX/tree/develop/docs) directory as markdown files.

# Current Component User Documentation

cREXX-Phase-0 v0.1.6-f0022

## Running a REXX program

Assuming test.rexx is the source program

    rxc test
    rxas test.rxas
    rxvm test.rxbin

## Compiler

Handles REXX Level B subset 
- Assignments / Expressions
- SAY
- IF/THEN/ELSE
- DO/TO/BY
- ADDRESS 

Type 

    rxc -h 

for command format / options

## Assembler

See [REXX Assembler Specification](assembler).

Type

    rxas -h 

for command format / options

Type 

    rxas -i 

for list of assembler instructions

## Disassembler

Type

    rxdas -h 

for command format / options

## VM/Interpreter

Type

    rxvm -h 

for command format / options

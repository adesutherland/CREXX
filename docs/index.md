# CREXX

_Release Documentation - crexx-dev-260110 - Jan 2026_

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

This is the "As Built" documentation, specific to its release; the current [develop branch](https://adesutherland.github.io/CREXX/) version is available as a website.

The documentation is stored in the code repository/branch under the [/doc](https://github.com/adesutherland/CREXX/tree/develop/docs) directory as markdown files.

# Current Component User Documentation

crexx-dev-260110

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

### Recent Compiler Changes (crexx-dev-260110)

- Fixpoint validation loop hardened for idempotency. All walkers inside the loop are validated to be idempotent; under `-d3` the compiler stress‑tests by multi‑invoking walkers and forcing extra iterations.
- Built‑in AST/Symbol validator integrated under debug flags:
  - `-d2`: structural validator runs between passes.
  - `-d3`: validator + stress testing (multiple walker invocations and extra loop iterations).
- Explicit Scope Hierarchy introduced (`SCOPE_UNIVERSE`, `SCOPE_NAMESPACE`, `SCOPE_CLASS`, `SCOPE_PROCEDURE`, `SCOPE_LOCAL`) with invariant checks to prevent wrongly linked symbols.
- Specialized symbol resolution APIs adopted (Local → Attribute → Global) to make name binding predictable and resilient across passes.
- EXPOSE/hoisting made robust and idempotent; globals exposed from procedures are consistently promoted to the namespace scope.
- Import duplication fixed to preserve original scope types when copying AST from imported files (procedures remain procedures, etc.).
- Level B AST restructuring clarified: initial post‑parse fixer re‑nests procedures and normalizes nodes; documentation comments added in the code to avoid confusion with the raw Lemon parser shape.

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

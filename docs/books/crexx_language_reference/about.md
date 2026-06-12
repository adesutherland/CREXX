# About This Book

This language reference documents the implemented cRexx language surface for
the Release 1 beta line.

The main release language is Level B: a typed Rexx-family systems language
compiled by `rxc`, assembled by `rxas`, and executed by the `rxvm` runtime.
Level B is not a complete Classic Rexx compatibility mode, and this reference
does not describe planned future levels as current behaviour.

## Audience

This reference is for programmers who need precise syntax and behaviour:

- application and library authors writing Level B source
- contributors changing compiler, library, or VM behaviour
- users inspecting generated RXAS or bytecode
- maintainers checking whether documentation matches the implementation

New users should start with the README, the documentation map, and the
programming guide. This book is the detailed reference.

## Document Structure

This document is in three parts:

Objectives
: Current design goals and the release scope.

Overview
: The language model, modules, calls, options, and program structure.

Reference
: Syntax and behaviour for types, literals, variables, operators, classes,
  namespaces, numeric settings, statements, and libraries.

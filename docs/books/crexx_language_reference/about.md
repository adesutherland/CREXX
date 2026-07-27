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
: The Objectives part explains what cRexx is intended to achieve and defines the
scope of the current release. It describes the principles that guide the
language design, the intended uses of the language, and the balance between
compatibility with traditional Rexx and the introduction of new facilities. It
also identifies which language levels and features are included in this
release, which are still under development, and which are deliberately outside
its scope.

Overview
: The Overview introduces the language as a whole before its individual features
are described in detail. It explains the cRexx language model, the organization
of source programs into modules, and the mechanisms used to call routines,
functions, and methods. It also describes compiler and language options, the
structure of a program, and the relationships between declarations, executable
code, imported modules, and external libraries. This part provides the
conceptual framework needed to understand the more formal reference material.

Reference
: The Reference gives the detailed definition of the language. It specifies the
syntax and behaviour of types, literal values, variables, expressions, and
operators. It describes classes and objects, namespaces and name resolution,
numeric settings and arithmetic, and all executable and declarative statements.
It also documents the standard libraries and their interfaces. Where relevant,
the reference states the applicable language level, default behaviour,
restrictions, error conditions, and interactions with other language features.



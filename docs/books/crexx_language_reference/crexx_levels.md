# Language Levels

\crexx{} uses language levels to name related \rexx{}-family language surfaces
without pretending they are all implemented by the same compiler. The letters
are a map, not a compatibility promise.

For the Release 1 beta line, Level B is the supported \crexx{} language documented
by this reference. Other levels are either historical context, project
direction, or editor/tooling targets unless their documentation explicitly says
that a compiler feature is implemented and tested.

## Summary

- use Level B for current \crexx{} programs
- treat Level C as Classic \rexx{} compatibility work with real DSLSH
  syntax highlighting / parser-mode progress, but not yet a release compiler
  language
- treat Levels E and N as planned DSLSH syntax-highlighting targets for
  Object \rexx{} and NetRexx source, not as languages that \crexx{} intends to compile
  or run
- treat Levels D, G, and L as \crexx{} direction for future language work

## Level B: Current \crexx{}

Level B is the foundation language for the current compiler, standard library,
tooling, and examples. It is deliberately more explicit than Classic \rexx{}:

- source files normally begin with `options levelb`
- values have known types such as `.int`, `.boolean`, `.float`, `.decimal`,
  `.string`, `.binary`, `.object`, and `.void`
- arrays are declared from typed values, for example `.string[]`
- reusable code is grouped through `namespace`, `import`, and `expose`
- procedures, methods, factories, and interfaces have checked signatures
- modules compile to `.rxbin` bytecode and can be linked into deployable images

Level B is used to implement much of the standard library. That is an
intentional part of the architecture: the same language available to users is
also used to build the platform.

## Level Catalogue

The following names are used by the project, but their status is deliberately
different:

| Level | Meaning | Current project status |
|-------|---------|------------------------|
| Level A | Early compact \rexx{} proof-of-concept foundation. | Historical only. Not a release target. |
| Level B | Current typed \crexx{} foundation language. | Implemented and documented as the Release 1 beta user language. |
| Level C | Classic REXX compatibility. | Not a release compiler language yet. A dedicated DSLSH syntax-highlighting / parser-mode front end now exists as the first concrete compatibility slice. |
| Level D | A cREXX-compatible extension direction above Classic REXX. | Direction only. Not a release language yet. |
| Level E | Object REXX / ooRexx relationship point. | Planned only as a DSLSH syntax-highlighting target. cREXX does not plan to compile or run ooRexx as Level E. |
| Level G | General-purpose modern cREXX direction built on Level B. | Directional, with some real library work such as `rxfnsg`; not the baseline user language for this release. |
| Level L | Language-engineering cREXX direction for parser, grammar, AST, and symbol-table work. | Direction only. Not a release language yet.|
| Level N | NetRexx relationship point: Rexx-family syntax with Java/JVM integration. | Planned only as a DSLSH syntax-highlighting target. cREXX does not plan to compile or run NetRexx as Level N.|

## DSLSH And Compatibility

DSLSH syntax-highlighting support is allowed to be wider than the compiler. It
can help users edit and diagnose \rexx{}-family source without claiming that \crexx{}
can execute that source.

Level C is the first example of that split. The project now has Level C
parser-mode and syntax-highlighting work for Classic \rexx{} source, while normal
`rxc` compilation of `options levelc` remains gated as unsupported. This lets
the project build useful editor support and standard-diagnostic experience
before committing to a full Classic \rexx{} lowering/runtime path.

Levels E and N should be understood in that same tooling sense. They reserve
clear names for Object \rexx{} and NetRexx editor support, but they are not \crexx{}
runtime or source-compatibility commitments.

## Defaults

The compiler can be given a default level with `rxc --level levelb`. The source
file wins if it contains an explicit `options` instruction.

The `crexx` driver gives headerless top-level scripts a practical default by
compiling them as Level B and importing `rxfnsb`. This convenience does not
change the recommendation for reusable source: write the `options` and
`import` lines explicitly.

## Compatibility

Level B borrows from \rexx{}, but it is not a drop-in Classic \rexx{} interpreter.
Programs that rely on Classic \rexx{}'s untyped variable model, late binding, or
default built-in function visibility usually need small, explicit Level B
changes.

The compatibility goals for future levels are important project direction, but
this Release 1 beta documentation should not describe future compatibility as
current behaviour.

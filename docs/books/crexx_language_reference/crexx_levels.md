# Language Levels

cRexx uses language levels to name related Rexx-family language surfaces
without pretending they are all implemented by the same compiler. The letters
are a map, not a compatibility promise.

For the Release 1 beta line, Level B is the supported cRexx language documented
by this reference. Other levels are either historical context, project
direction, or editor/tooling targets unless their documentation explicitly says
that a compiler feature is implemented and tested.

## Summary

- Level B will be used for current cRexx programs
- Level C represents Classic Rexx compatibility work with real DSLSH
  syntax highlighting / parser-mode progress, but not yet a release compiler
  language
- Levels E and N are planned DSLSH syntax-highlighting targets for
  Object Rexx and NetRexx source, not as languages that cRexx intends to compile
  or run
- Level G has an implemented initial task/parallel surface and libraries;
  Levels D and L remain directions for future language work

## Level B: Current cRexx

Level B is the foundation language for the current compiler, standard library,
tooling, and examples. It is deliberately more explicit than Classic Rexx:

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
| Level A | Early compact Rexx proof-of-concept foundation. | Historical only. Not a release target. |
| Level B | Current typed cRexx foundation language. | Implemented and documented as the Release 1 beta user language. |
| Level C | Classic Rexx compatibility. | Not a release compiler language yet. A dedicated DSLSH syntax-highlighting / parser-mode front end now exists as the first concrete compatibility slice. |
| Level D | A cRexx-compatible extension direction above Classic Rexx. | Direction only. Not a release language yet. |
| Level E | Object Rexx / ooRexx relationship point. | Planned only as a DSLSH syntax-highlighting target. cRexx does not plan to compile or run ooRexx as Level E. |
| Level G | General-purpose modern cRexx built on Level B. | Task declarations, ordinary-call task expressions, `DO PARALLEL`, concurrency classes, concurrent HTTP, mathematics, and explicit Unicode text services are implemented behind `OPTIONS LEVELG`; Level G is not the stable baseline language for this release. |
| Level L | Language-engineering cRexx direction for parser, grammar, AST, and symbol-table work. | Directional, with an initial `rxfnsl` generated-output proving demo; not a release language yet.|
| Level N | NetRexx relationship point: Rexx-family syntax with Java/JVM integration. | Planned only as a DSLSH syntax-highlighting target. cRexx does not plan to compile or run NetRexx as Level N.|

## DSLSH And Compatibility

DSLSH syntax-highlighting support is allowed to be wider than the compiler. It
can help users edit and diagnose Rexx-family source without claiming that cRexx
can execute that source.

Level C is the first example of that split. The project has Level C parser-mode
and syntax-highlighting work for Classic Rexx source, and normal `rxc`
compilation now lowers a proven subset of Classic Rexx into the canonical
Level B runtime path. Classic constructs outside that implemented slice still
fail closed with a Level C unsupported-shape diagnostic. This lets the project
build useful editor support, standard-diagnostic experience, and incremental
runtime coverage without claiming full Classic Rexx compatibility before the
remaining lowering / runtime work is complete.

Level C owns the Classic Rexx byte-text compatibility decision. Direct Level C
BIF calls carry a `RexxClassicConfig` through `RexxBifCallContext`: `BYTE` is
the default compatibility profile and `UTF8` is the opt-in text profile. This
library/runtime configuration does not require a compiler or lowering change.
Per-value `RexxValue` flags describe text/binary validity but do not select the
profile. See [Unicode](unicode.md) for the exact contracts.

Level G already owns the initial structured-concurrency surface above
Level B. Its syntax is enabled only by `OPTIONS LEVELG` and lowers through the
public Level B concurrency classes. See
[Concurrent programming](../crexx_programming_guide/concurrency.md)
for checked examples and [Tasks and parallel execution](concurrency.md)
for the formal source rules.

Level G also owns explicit Unicode services above the Level B codepoint
contract. `rxunicode` provides Unicode 17.0.0 normalization and predicates,
full default case mapping, case folding, default extended grapheme clusters,
and strict-by-default typed codecs between `.string` and `.binary`. The
implementation uses private Level B executors and generated immutable data;
normalization certificates are VM-carried in a protected language-owned
register-flag sub-band and are invalidated by content mutation. Importing the
module does not change ordinary equality, codepoint indexing, or text I/O. See
[Unicode](unicode.md) and [Unicode text
services](../crexx_library_reference/unicode.md).

Level L currently uses the Level B-derived compiler pipeline plus
`options levell` for library-shaped experiments. Its first concrete library,
`rxfnsl`, is a generated-output proof rather than a generator: it shows what a
future lexer/parser generator might emit using binary constants, packed token
records, and direct RXAS binary-memory operations. That target shape should be
proved by examples before deciding whether to port a generator such as re2c or
adapt a generator backend to emit cRexx/RXAS directly.

Levels E and N should be understood in that same tooling sense. They reserve
clear names for Object Rexx and NetRexx editor support, but they are not cRexx
runtime or source-compatibility commitments.

## Defaults

The compiler can be given a default level with `rxc --level levelb`. The source
file wins if it contains an explicit `options` instruction.

The `crexx` driver gives headerless top-level scripts a practical default by
compiling them as Level B and importing `rxfnsb`. This convenience does not
change the recommendation for reusable source: write the `options` and
`import` lines explicitly.

## Compatibility

Level B borrows from Rexx, but it is not a drop-in Classic Rexx interpreter.
Programs that rely on Classic Rexx's untyped variable model, late binding, or
default built-in function visibility usually need small, explicit Level B
changes.

The compatibility goals for future levels are important project direction, but
this Release 1 beta documentation should not describe future compatibility as
current behaviour.

## Extensibility

A small language is easy to learn and easy to use. When the language is too small, and there is no large, well organised runtime library, a lot of function needs to be provided by the user. Classic Rexx provides extensibility through addressing environments and function packages. These environments need to be addressed each in their own way, the function packages need to be registered and checked; the searching and linkage conventions differ per platform and operating system.

cRexx adds an easily extensible module system which integrates extended runtime functionality which behaves in the same manner over library extensions written in Rexx or native functions written in C and in other languages. These work in tandem with the compiler exit facility.

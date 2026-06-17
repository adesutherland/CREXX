# Language Levels

cRexx uses language levels to name related Rexx-family language surfaces
without pretending they are all implemented by the same compiler. The letters
are a map, not a compatibility promise.

For the Release 1 beta line, Level B is the supported cRexx language documented
by this reference. Other levels are either historical context, project
direction, or editor/tooling targets unless their documentation explicitly says
that a compiler feature is implemented and tested.

## Summary

- use Level B for current cRexx programs
- treat Level C as Classic Rexx compatibility work with real DSLSH
  syntax highlighting / parser-mode progress, but not yet a release compiler
  language
- treat Levels E and N as planned DSLSH syntax-highlighting targets for
  Object Rexx and NetRexx source, not as languages that cRexx intends to compile
  or run
- treat Levels D, G, and L as cRexx direction for future language work

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
| Level G | General-purpose modern cRexx direction built on Level B. | Directional, with some real library work such as `rxfnsg`; not the baseline user language for this release. |
| Level L | Language-engineering cRexx direction for parser, grammar, AST, and symbol-table work. | Direction only. Not a release language yet.|
| Level N | NetRexx relationship point: Rexx-family syntax with Java/JVM integration. | Planned only as a DSLSH syntax-highlighting target. cRexx does not plan to compile or run NetRexx as Level N.|

## DSLSH And Compatibility

DSLSH syntax-highlighting support is allowed to be wider than the compiler. It
can help users edit and diagnose Rexx-family source without claiming that cRexx
can execute that source.

Level C is the first example of that split. The project now has Level C
parser-mode and syntax-highlighting work for Classic Rexx source, while normal
`rxc` compilation of `options levelc` remains gated as unsupported. This lets
the project build useful editor support and standard-diagnostic experience
before committing to a full Classic Rexx lowering/runtime path.

Level C also owns the Classic Rexx byte-text compatibility decision. Level B and
Level G `.string` values are valid UTF-8 text, while `.binary` carries arbitrary
bytes. A future Level C lowering path may choose a compatibility option such as
`bytetext`, but that choice should be isolated to Level C and accompanied by an
audit of Classic BIFs so byte-oriented legacy programs and UTF text programs do
not silently share incompatible semantics.

Level G owns richer Unicode services above the Level B codepoint contract. The
VM reserves private status bits for normalization-form cache knowledge, but NFC,
NFD, NFKC, and NFKD bits should only be assigned meaning when Level G APIs set
and consume them. `utf8proc` is the preferred first implementation candidate for
the Unicode plugin, subject to vendoring/build work and carrying its MIT expat
and Unicode data license notices. Initial coverage should target normalization,
case folding, Unicode property checks, and grapheme/word/sentence segmentation.
There is also room for a Level B cRexx proof of concept of UTF helper libraries
while the Level G design settles.

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

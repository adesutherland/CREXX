# Language Levels

\crexx{} uses language levels to separate the implemented Level B systems
language from planned or experimental REXX-family layers.

For Release 1 beta 1, Level B is the supported language level documented by
this reference. Other levels are project direction unless their documentation
explicitly states that a feature is implemented and tested.

## Level B

Level B is the foundation language for the current compiler, standard library,
tooling, and examples. It is deliberately more explicit than Classic REXX:

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

## Current Level Status

The following names are used by the project, but their status is different:

| Level | Release 1 beta 1 status |
| --- | --- |
| Level A | Historical proof of concept. Not a release target. |
| Level B | Implemented foundation language and the focus of this documentation. |
| Level C | Classic REXX compatibility direction. Not implemented as a release language. |
| Level D | Direction for Classic-compatible extensions. Not a release language. |
| Level E | OO REXX relationship point. Not delivered by this project. |
| Level G | Modern general-purpose direction built on Level B. Some library work exists, but the level is not the baseline user language for this release. |
| Level L | Language-engineering direction. Not a release language. |

## Defaults

The compiler can be given a default level with `rxc --level levelb`. The source
file wins if it contains an explicit `options` instruction.

The `crexx` driver gives headerless top-level scripts a practical default by
compiling them as Level B and importing `rxfnsb`. This convenience does not
change the recommendation for reusable source: write the `options` and
`import` lines explicitly.

## Compatibility

Level B borrows from REXX, but it is not a drop-in Classic REXX interpreter.
Programs that rely on Classic REXX's untyped variable model, late binding, or
default built-in function visibility usually need small, explicit Level B
changes.

The compatibility goal for future levels is important project direction, but
this Release 1 beta documentation should not describe future compatibility as
current behaviour.

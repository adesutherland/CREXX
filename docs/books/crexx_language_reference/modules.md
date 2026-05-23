# Modules

Each source file compiles to a module. New \crexx{} source normally uses `.crexx`
or `.crx`; `.rexx` remains the compatibility/classic extension. The compiler
writes RXAS assembly (`.rxas`), and the assembler writes RXBIN bytecode
(`.rxbin`).

A module can contain:

- procedures and functions
- global values
- classes and interfaces
- factories, methods, and class/interface metadata
- import and namespace information used by downstream compilation and runtime
  linking

## Namespace

The namespace is the public identity of reusable code. The source filename is a
convenient discovery hint, but the namespace is what callers import and what
qualified references name.

```rexx
options levelb
namespace rxfnsb expose abs

abs: procedure = .string
  arg number = .string
  ...
```

Only exposed symbols are intended to be used from outside the namespace.

## Imports

`import name` makes another namespace visible to the current source file:

```rexx
options levelb
import rxfnsb

say date("w")
```

The compiler searches source roots for source imports and binary roots for
`.rxbin`, optional `.rxas`, and `.rxplugin` imports. `rxc -s` adds source roots;
`rxc -i` adds binary roots. Repeated `-s` and `-i` options are accumulated.
The source file's directory is a source root only, not an implicit binary root;
use `-i .` or `-i build-dir` when a nearby `.rxbin` should satisfy compile-time
imports.

Source roots search `.crexx`, `.crx`, and `.rexx`. If the initial source file
uses another extension, such as `.the`, that extension is searched as source
for this compile too. Default levels are Level G for `.crexx`, `.crx`, and
arbitrary initial extensions, and Level C for `.rexx`, unless the source has an
explicit `options level...` clause.

Qualified references use the imported namespace:

```rexx
say rxfnsb..date("w")
```

`namespace::symbol` remains accepted as a compatibility spelling, but new docs
and examples should prefer `namespace..symbol`.

## Libraries and Linking

Multiple `.rxbin` modules can be loaded together by the VM. For deployable
images, use `rxlink`. It combines selected modules into one linked image with a
shared constant pool while preserving module boundaries and runtime metadata.

`rxlink` can also strip source/file metadata for smaller deployable artifacts.
Runtime contract metadata for imports, interfaces, implementations, members,
and factories is preserved by the current linker.

Simple bytecode concatenation can still be useful for development or archive
experiments, but `rxlink` is the release path for compact deployable images.

## Classes and Interfaces in Modules

Classes and interfaces are implemented in the current Level B toolchain. Their
contracts are recorded in RXBIN metadata so downstream imports can reconstruct
headers without reparsing source bodies.

The current metadata model carries:

- interface declarations
- class implementation relationships
- callable members
- factories and method contracts

See [Classes and Interfaces](classes_and_interfaces.md#classes-and-interfaces)
for the source model.

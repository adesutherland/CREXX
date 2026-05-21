# Architecture Overview

\crexx{} is a compiler toolchain and runtime rather than a single monolithic
interpreter. The main flow is:

```bash
.crexx source -> rxc -> .rxas assembly -> rxas -> .rxbin bytecode -> rxvm
```

`rxlink` can combine bytecode modules into a shared-pool linked image, and
`rxcpack` can serialize a linked image as C data for native packaging.

## Compiler

`rxc` reads Level B source, tokenizes and parses it, builds the compiler tree,
validates types and scopes, applies supported optimizations, and emits RXAS
assembly.

The parser is generated from Lemon grammar files and the scanners are generated
with re2c. The compiler has two important internal views:

- a source tree used for authored source structure, diagnostics, parser mode,
  highlighting, and editor-facing metadata
- a mutable compiler tree used for imports, exits, validation, optimization,
  and emission

Compiler exits are used for features such as `PARSE`, `ADDRESS`, and `TRACE`.
They are part of the current architecture, not a compatibility afterthought.

## Assembler

`rxas` turns RXAS text into RXBIN bytecode. RXBIN is a record-stream format
containing module records, a constant pool, instructions, and metadata. The
Release 1 beta bytecode stream uses format version `003`.

RXAS is useful both as compiler output and as a low-level language for runtime
support. Inline assembler in Level B source is available for code that needs
direct VM instructions, but most application code should stay at Level B.

## Linker

`rxlink` combines one or more RXBIN inputs into a linked image with one shared
constant pool. It preserves module boundaries while resolving imports and
interface-provider relationships that are visible in the selected link set.

The linker can strip source/file metadata for deployment while keeping the
runtime metadata needed for modules, imports, classes, interfaces, methods, and
factories.

## Virtual Machine

`rxvm` executes RXBIN bytecode. The VM is register based: instructions operate
on explicit registers rather than on an operand stack. Values carry runtime
type information for integer, float, decimal, string, binary, and object data.

The VM loader reads RXBIN records, links modules, exposes imported symbols, and
builds the registries used for class/interface method and factory dispatch.

## Libraries and Plugins

The standard library is mostly ordinary \crexx{} code compiled to RXBIN, with
native support where the runtime needs platform access. The plugin architecture
lets native code expose callable functions without Level B source needing a
different call form.

Compile-time import discovery and runtime loading are separate:

- `rxc -s` adds source roots
- `rxc -i` adds binary import roots
- the VM and `crexx -l` control runtime libraries and native packaging inputs

That split keeps source development, binary reuse, and deployment packaging
from accidentally depending on the same path setting.

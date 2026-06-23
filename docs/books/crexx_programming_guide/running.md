# Running cRexx on Linux, macOS, and Windows

cRexx programs are compiled before they run. The compiler emits *rxas*
assembly, the assembler emits *rxbin* bytecode, and a VM executable runs this
bytecode.

For most users, the `crexx` driver is the simplest entry point.

## A First Program

Create `hello.crexx`:

```rexx
options levelb
import rxfnsb

say "hello cREXX world!"
say "today is" date("w")
return 0
```

Run it with:

```bash
crexx hello.crexx
```

The driver compiles, assembles, and executes the program. It keeps
intermediate files by default so you can inspect the generated `.rxas` and
`.rxbin` while learning or debugging.

## Headerless Scripts

For small top-level scripts, `crexx` accepts a convenient headerless form:

```rexx
say "hello cREXX world!"
say "today is" date("w")
```

Headerless top-level scripts are compiled as Level B with `rxfnsb` imported.
Reusable libraries, tests, and examples should still write the explicit
`options levelb` and `import` lines. That keeps source behaviour independent
from driver convenience.

## Running the Individual Tools

The equivalent explicit tool sequence is:

```bash
rxc hello.crexx
rxas hello.rxas
rxvme hello.rxbin
```

`rxvme` includes the standard library image used by common Level B programs.
The base `rxvm` can also run bytecode, but programs that call library functions
need the relevant library image or linked modules available.

You can pass program arguments after `-a`:

```bash
rxvme hello.rxbin -a first second third
```

The VM also accepts multiple bytecode files when a program is split across
modules.

## Imports and Libraries

Level B imports are explicit:

```rexx
options levelb
import rxfnsb
import rxjson
```

At compile time, `rxc -s` adds source import roots and `rxc -i` adds binary
import roots. With the `crexx` driver, `-s[path]` and `-i[path]` affect the
compiler phase only. Runtime/native loading still uses the runtime library path
controlled by `-l`.

The source file's directory is not an implicit binary import root. If a local
`.rxbin` is meant to provide compile-time signatures or class/interface
metadata, pass that directory with `-i`; this keeps stale generated `.rxbin`
files from unexpectedly winning over source edits.

## Linked Images

For deployable bytecode, link the needed modules:

```bash
rxlink -o hello_linked.rxbin hello.rxbin library.rxbin
rxvme hello_linked.rxbin
```

The exact library image names depend on the build and packaging layout. The key
point is that `rxlink` produces a compact shared-pool image that the VM can
load as a normal RXBIN file.

## Native Executables

The `crexx` driver can package a native executable:

```bash
crexx hello.crexx -native
```

Native packaging links the bytecode image, serializes it with `rxcpack`, and
uses the platform C toolchain to build an executable. Optional plugins and
platform libraries depend on build configuration, so test packaged binaries on
the target platform before release.

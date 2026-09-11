# Intralanguage calls

This chapter discusses calls from one cRexx procedure to another,
including the built-in function package. Search order is an
intrinsically related concept: how is the called component found. There
are some differences with Classic Rexx and ooRexx, which can call (and interpret)
external procedures in source. In this respect, cRexx behaves like
\nr{}, where a called component needs to be compiled, executable
code; for \nr{} a `.class` file and in cRexx an `.rxbin`
file. Level B introduces a package (module) system where a program can
be part of a package and be imported into calling code.

## At compile time

At compile time, a program uses the `CALL` statement, or the
function notation with parentheses (also called round brackets). Going
forward, and moving into object oriented notations for other Rexx
variants, the latter is going to gain importance, while the
`CALL` statement will be fixed in its current functionality. For
that reason, most examples will be in the function notation.

The compiler needs the callee's *signature*[^1], which it can obtain from
source or compiled metadata. Reading an imported source contract does not
automatically produce or load that module's executable bytecode. At runtime,
any call that remains in the compiled caller needs the library to be loaded.

[^1]: with signature we mean the combination of parameter types and return type.

This implies that there are inherent dependencies to be followed
while building an application system that consists of multiple
modules; this is not different than in other compiled
languages. Building utilities like Make or Ninja can provide these
services, and these can be orchestrated by meta-build tools like
CMake. The cRexx toolchain itself is built using CMake and from its
build specification in CMake most of these patterns can be gleaned.

The `import` statement\label{intraImport} tells the compiler we want to
import functions from a certain package.


<!-- \section{At runtime} -->

## Imported interfaces and classes

Level B extends the same import model to classes and interfaces. An imported
namespace may expose:

- procedures
- interfaces
- classes

An interface call is normally written against the contract name:

```rexx
import garage

current = .vehicle("mini")
say current.describe()
```

When two imported namespaces expose the same contract name, qualify the
reference with `namespace..`:

```rexx
import qifa
import qifb

left  = .qifa..vehicle("one")
right = .qifb..vehicle.from_name("two")
```

The token to the left of `..` must be an imported namespace. Qualification is a
disambiguation mechanism; it does not create a second way to reach symbols that
have not been imported. `::` remains accepted as a compatibility alias.

## Casts and type inspection

Object contracts can be checked explicitly:

```rexx
generic = .car("roadster") as .object
vehicle = generic as .garage..vehicle
current = vehicle as .garage..car

say typeof(current)
say current is .garage..vehicle
say current is .garage..car
```

`as` performs a checked object cast, `is` performs a boolean type test, and
`typeof` returns the canonical runtime type name.

## Source and binary imports

At compile time, `rxc` distinguishes between:

- source roots, used for CREXX source files
- binary roots, used for `.rxbin`, optional `.rxas`, and plugins

The source file being compiled contributes its own directory as the primary
source root. Additional source roots may be passed with `-s`. Binary roots are
passed with `-i`. The primary source directory is not automatically a binary
root, so a sibling `.rxbin` is visible only when that directory is also passed
with `-i`.

Source-root discovery includes `.crexx`, `.crx`, and `.rexx`. If the initial
source file has another extension, for example `.the`, that extension is added
to source-root discovery for that compile. The default language level is Level
G for `.crexx`, `.crx`, and arbitrary initial extensions, and Level C for
`.rexx`; explicit `options level...` always wins.

`.rxpp` remains the extension for existing RXPP preprocessor workflows. A
future idempotent preprocessor path may reduce the need for a separate
preprocessor extension, but that is not part of the source import rule.

For ordinary project work this means:

- keep project source modules in source roots
- keep packaged binary libraries in binary roots
- use namespace qualification only when imported namespaces collide

This split is deliberate: it keeps active source preferred during development
and stops stale generated `.rxbin` files beside source files from silently
satisfying imports.

## Supplying the library at runtime

Consider these two files. `libmod.crexx` exposes a function that calls a
private helper:

```rexx
options levelb
namespace libmod expose A

A: procedure = .string
  return B("fixed")

B: procedure = .string
  arg y = .string
  return "hello from B with " || y
```

`usemod.crexx` imports that namespace:

```rexx
options levelb
import libmod
say libmod..A()
exit 0
```

Running `crexx usemod.crexx` can compile successfully by reading
`libmod.crexx`, yet fail with `FUNCTION_NOT_FOUND` for `libmod.a` because the
library has not been supplied to the VM. A very small function might appear to
work because optimization inlines it into the caller. That does not establish
that the library was loaded, and adding a helper or disabling optimization can
expose the missing dependency.

Compile the library and supply its exact runtime path:

```sh
rxc libmod.crexx
rxas libmod.rxas
crexx -l ./libmod.rxbin usemod.crexx
```

Alternatively, build an explicit linked program from both source files:

```sh
crexx --program combined usemod.crexx libmod.crexx
rxvm combined.rxbin
```

Both routes print `hello from B with fixed`. The `./` in the first command
selects the application-local library path; a bare `-l` library name is
resolved relative to the tool installation.

Packaged binary imports provide another route. When the compiler selects a
`.rxbin` library through a binary root such as `-i .`, it records the exact
package stem for runtime autoload. The VM can load that package from its module
roots when an unresolved callable needs it. This does not search for or compile
source files, and compiler `-s`/`-i` roots do not themselves add VM search roots.

# Intralanguage calls

This chapter discusses calls from one \crexx{} procedure to another,
including the built-in function package. Search order is an
intrinsically related concept: how is the called component found. There
are some differences with Classic Rexx and ooRexx, which can call (and interpret)
external procedures in source. In this respect, \crexx{} behaves like
\nr{}, where a called component needs to be compiled, executable
code; for \nr{} a \code{.class} file and in \crexx{} an \code{.rxbin}
file. Level B introduces a package (module) system where a program can
be part of a package and be imported into calling code.
\section{At compile time}

At compile time, a program uses the \code{CALL} statement, or the
function notation with parentheses (also called round brackets). Going
forward, and moving into object oriented notations for other \rexx{}
variants, the latter is going to gain importance, while the
\code{CALL} statement will be fixed in its current functionality. For
that reason, most examples will be in the function notation.

The compiler needs to verify if it is possible to call the called
code: it must be present in executable form, and it needs to have the right
\emph{signature}\footnote{with signature we mean the combination of
  parameter types and return type}. The compiler will not automatically compile a callee
of which the source can be located but the executable form is missing;
existing systems based on interpreters will happily interrupt their
work and tokenize another source file when called; the \crexx{}
\code{rxc} compiler will not.

This implies that there are inherent dependencies to be followed
while building an application system that consists of multiple
modules; this is not different than in other compiled
languages. Building utilities like Make or Ninja can provide these
services, and these can be orchestrated by meta-build tools like
CMake. The \crexx{} toolchain itself is built using CMake and from its
build specification in CMake most of these patterns can be gleaned.

The \textbf{import} statement\label{intraImport} tells the compiler we want to
import functions from a certain package.


<!-- \section{At runtime} -->

\section{Imported interfaces and classes}

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

left = .qifa..vehicle("one")
right = .qifb..vehicle.from_name("two")
```

The token to the left of `..` must be an imported namespace. Qualification is a
disambiguation mechanism; it does not create a second way to reach symbols that
have not been imported. `::` remains accepted as a compatibility alias.

\section{Casts and type inspection}

Object contracts can now be checked explicitly:

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

\section{Source and binary imports}

At compile time, \code{rxc} now distinguishes between:

- source roots, used for `.rexx`
- binary roots, used for `.rxbin`, optional `.rxas`, and plugins

The source file being compiled contributes its own directory as the primary
source root. Additional source roots may be passed with `-s`. Binary roots are
passed with `-i`. The primary source directory is not automatically a binary
root, so a sibling `.rxbin` is visible only when that directory is also passed
with `-i`.

Only `.rexx` files are discovered from source roots. Files with extensions such
as `.crexx` or `.arexx` may be compiled directly, but they are not scanned as
source import candidates.

For ordinary project work this means:

- keep project source modules in source roots
- keep packaged binary libraries in binary roots
- use namespace qualification only when imported namespaces collide

This split is deliberate: it keeps active source preferred during development
and stops stale generated `.rxbin` files beside source files from silently
satisfying imports.

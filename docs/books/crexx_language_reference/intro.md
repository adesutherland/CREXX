# Introduction

cRexx is a Rexx-family language and toolchain that compiles source
programs to an open bytecode format, then executes that bytecode on the
cRexx virtual machine or packages it into a native executable.

The Release 1 beta line is centred on Level B. Level B is the implemented systems
language used by the project itself: it is statically typed, module based, and
close enough to Rexx to keep the language readable while giving the compiler
and VM explicit type, module, and contract information.

The public documentation should describe what the current toolchain can do.
Ideas for future levels remain part of the project direction, but they are not
release facts. In this book, unless a section says otherwise, syntax and
behaviour refer to the current Level B implementation.

## What The Release 1 Beta Line Contains

The implemented Level B surface includes:

- source modules compiled from `.crexx` to `.rxas` assembly and then to
  `.rxbin` bytecode
- namespaces, imports, exposed procedures, global values, and class/interface
  metadata
- static scalar types, arrays, and object values
- procedures, functions, methods, factories, and varargs
- structured control flow including `if`, `do`, `select`, `leave`, `iterate`,
  and `return`
- `parse`, `address`, `signal`, and `trace` through the current compiler-exit
  and VM runtime support
- Level B classes and interfaces, including default/final interface methods,
  factories, checked casts, type tests, and concrete type introspection
- a standard library built largely in cRexx itself, with native support
  where direct VM or platform access is required

The main tools are:

- `rxc`, the compiler from `.rexx` source to `.rxas` assembly
- `rxas`, the assembler from `.rxas` to `.rxbin`
- `rxlink`, the linker for combining modules into a shared-pool linked image
- `rxvm` and related interpreters for running `rxbin` bytecode
- `crexx`, the convenience driver that compiles, links or packages, and runs
  common programs

## Design Position

cRexx keeps the Rexx emphasis on readability and directness, but Level B is
not Classic Rexx. It is the typed foundation used to build libraries, tools,
and later language layers. That is why Level B source normally starts with:

```rexx
options levelb
```

and why reusable libraries usually declare a namespace:

```rexx
namespace rxfnsb expose abs
```

Headerless top-level scripts run through the `crexx` driver are treated as
Level B scripts with `rxfnsb` imported automatically. Library and compiler work
should still use explicit `options`, `namespace`, and `import` declarations so
the source remains clear to both readers and tools.

## Reading This Reference

The reference is intentionally factual rather than aspirational:

- Syntax sections describe accepted source forms.
- Tool sections describe the current command-line tools and generated files.
- VM sections describe the current bytecode and runtime model at the level
  useful to RXAS authors and embedding work.
- Experimental areas are labelled as experimental when the implementation is
  intentionally small or still being shaped.

<!-- If a page conflicts with compiler tests, source, or the agent context files in -->
<!-- `docs/ai-context/`, treat that as a documentation bug. -->

# Programs and Libraries

Level B code is organized as modules that publish procedures, functions,
classes, interfaces, methods, factories, and global values. Call resolution is
checked by the compiler using the declarations available in the current source
file and its imports.

## Procedures and Functions

A callable procedure is declared with a label and `procedure`:

```rexx
twice: procedure = .int
  arg value = .int
  return value * 2
```

Call it by name:

```rexx
say twice(21)
```

The return type after `=` is part of the callable contract. Use `.void` when no
value is returned.

Arguments are declared with `arg` lines. Optional arguments, varargs, and
caller-owned state are described in the argument and system-symbol chapters.

## Imports

Level B does not assume Classic REXX built-in functions are always visible.
Import the library that defines the function:

```rexx
options levelb
import rxfnsb

say date("w")
```

This explicit import model lets the compiler check signatures and lets programs
choose which library namespace they use.

## Local Shadowing and Literal Calls

A local procedure shadows an imported function with the same call name. The
compiler warns when this happens:

```rexx
options levelb
import rxfnsb

translate: procedure = .string
  arg text = .string
  return "[" || text || "]"
```

Calls to `translate()` in that file resolve to the local procedure.

If code intentionally needs the imported external function while a local symbol
shadows it, a literal function call can bypass the local unexposed procedure:

```rexx
say "TRANSLATE"("abc", "ABC", "abc")
```

The compiler emits a warning for this compatibility path. Prefer a clearer
namespace or local procedure name in new code when possible.

## Methods and Factories

Methods are called through object values:

```rexx
shape = .box()
say shape.summary()
```

Factories are declared on classes and interfaces. The default factory uses `*`;
named factories use their declared member name. Interface factory calls select
an implementing class through the current factory-provider rules.

```rexx
asset = .asset("log.txt")
asset2 = .asset.from_size(8)
```

See [Classes and Interfaces](classes_and_interfaces.md#classes-and-interfaces)
for factory selection, `match`, casts, type tests, and default methods.

## External Libraries and Plugins

An imported callable may be implemented in \crexx{}, in RXAS, or in native code
through the plugin architecture. The source-level call form is the same:

```rexx
import rxfnsb
import rxsocket

sock = socketcreate()
```

Runtime loading depends on how the program is run or linked. During
compilation, `rxc -s` controls source import roots and `rxc -i` controls binary
import roots. At runtime, provide the needed RXBIN modules, linked image, or
plugins through the VM or `crexx` driver library path.

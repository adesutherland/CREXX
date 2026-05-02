# Variables {#variables}

Level B variables are typed. The compiler can infer many local variable types,
but once a variable has a type, later assignments must be compatible with that
type.

## Keywords

Keywords, instruction names, and operators cannot be used as variable names.

## Declaration by Assignment

A variable is often declared by its first assignment:

```rexx
count = 0
price = 1.25
name = "Ada"
ready = .boolean(1)
```

The inferred types are based on the assigned expression. Use constructors when
the intended type needs to be explicit:

```rexx
count = .int(0)
name = .string("Ada")
ratio = .float(1.25)
money = .decimal("1.25")
payload = .binary()
```

The canonical integer type name is `.int`.

## Arrays

Array variables are declared from a typed array value:

```rexx
args = .string[]
scores = .int[10]
grid = .int[10, 10]
```

Array arguments use the same notation in procedure signatures:

```rexx
main: procedure = .int
  arg args = .string[]
```

## Object Variables

Class and interface values are object-shaped. Factories use dotted class or
interface names:

```rexx
asset = .asset("log.txt")
box = .box()
```

When a class implements an interface, a class instance can be assigned to that
interface contract. Use `expr is .type`, `expr as .type`, and `typeof(expr)`
when code needs runtime type checks or concrete type information.

## Globals and Expose

Top-level values in a namespace are global to that module. Exposed globals can
be imported by other modules. Procedures have their own local scope unless
state is deliberately exposed through the current Level B expose mechanisms.

Prefer explicit arguments and return values for ordinary application code. Use
global exposed state for library constants, runtime integration points, and
cases where shared module state is genuinely the simplest contract.

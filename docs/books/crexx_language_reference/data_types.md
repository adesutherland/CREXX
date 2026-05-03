# Data Types

Level B is statically typed. Every expression has a type known to the compiler,
and assignments, arguments, returns, method calls, and factory calls are
checked against those types.

## Built-In Types

The built-in Level B value types are:

| Type | Purpose |
| --- | --- |
| `.void` | No usable value. Used for procedures that do not return a value. |
| `.boolean` | Boolean truth value. |
| `.int` | Integer value. |
| `.float` | Binary floating-point value unless the source uses decimal float options. |
| `.decimal` | Decimal numeric value. |
| `.string` | Character string value. |
| `.binary` | Binary byte sequence. |
| `.object` | Object value. Interfaces and classes are object-shaped contracts. |

The canonical integer spelling in source is `.int`.

## Constructors and Type Literals

Type names can be used as constructors:

```rexx
count = .int(0)
name = .string("Ada")
ok = .boolean(1)
payload = .binary()
```

Class and interface names also use the dotted form. A factory call creates an
object through the selected class or interface provider:

```rexx
item = .cacheentry("abc")
asset = .asset("log.txt")
```

Namespace-qualified contracts use a double dot:

```rexx
client = .net..httpclient("example.com", 443, 1)
```

The left side of `namespace..symbol` must name an imported namespace.
`namespace::symbol` remains accepted as a compatibility alias.

## Arrays

Arrays are declared from a base type:

```rexx
words = .string[]
numbers = .int[10]
grid = .int[10, 10]
window = .int[0 to 10]
grow = .int[-2 to *]
```

An array value carries its element type and dimensions. Procedure signatures can
accept arrays in the same way:

```rexx
main: procedure = .int
  arg args = .string[]
```

## Numeric Values

Level B supports integer, float, and decimal arithmetic. The file-level
`options` instruction selects the parser's arithmetic standard, and a
procedure-level `numeric` instruction can set numeric context such as digits,
form, fuzz, case, and standard.

The compiler performs type validation before bytecode emission. Numeric
conversions are explicit where precision or representation could otherwise be
surprising:

```rexx
i = .int(42)
f = .float(i)
d = .decimal("42.50")
```

## Strings and Binary Values

`.string` values are character data. `.binary` values are byte data. Keep the
two distinct when working with sockets, files, encodings, or native payloads:
string operations are text operations, while binary operations preserve bytes.

## Object Values

Classes and interfaces are object contracts. A concrete class instance can be
assigned to an interface it implements:

```rexx
shape = .box()
```

Level B supports:

- `expr is .type` for boolean type tests
- `expr as .type` for checked casts
- `typeof(expr)` for concrete type introspection

Objects can also carry native payloads when exposed through the plugin API, but
ordinary Level B code should interact with objects through factories, methods,
and interfaces.

## Type Inference

The compiler infers a variable type from the first binding in many common
cases:

```rexx
total = 0        /* .int */
label = "ready" /* .string */
```

Use explicit constructors or declarations when the intended type is not obvious
from the initializer, when a signature is being published, or when a value must
be an object or array contract.

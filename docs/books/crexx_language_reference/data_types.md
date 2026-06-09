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

## References

Reference values are explicit weak aliases to storage. A reference does not keep
its target alive; if the target storage is destroyed, later use raises
`REFERENCE_INVALID`. Use `reference` as a type modifier anywhere a Level B type
is accepted:

```rexx
count_ref = reference .int
items_ref = reference .string[]

read_count: procedure = .int
  arg r = reference .int
```

Use `reference target` to create a reference to aliasable storage, and
`dereference ref` when an explicit snapshot copy is required:

```rexx
count = 1
count_ref = reference count
copy = dereference count_ref
```

Reference values are not assignment-compatible with their target type. Passing a
`.T` where `reference .T` is expected is an error, and passing `reference .T`
where `.T` is expected is also an error; spell either `reference target` or
`dereference ref` at the boundary.

Nested reference containers, reference casts, reference type tests, and
implicit member/index access through a reference are not part of the current
Level B source surface. These are reserved for possible Level G convenience
features. Level B code should keep reference boundaries explicit with
`reference`, `dereference`, and `refvalid`.

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

The checked cast form can also be used for scalar conversions:

```rexx
f = 1 as .float
i = "42" as .int
d = "42.50" as .decimal
s = 42 as .string
ok = "1" as .boolean
```

Scalar casts use the same conversion rules as the corresponding constructor or
promotion opcode. A cast is still type checked; for example, `.binary` values
only cast back to `.string` when the cast is explicit and the bytes are valid
UTF-8.

## Strings and Binary Values

`.string` values are character data. `.binary` values are byte data. Keep the
two distinct when working with sockets, files, encodings, or native payloads:
string operations are text operations, while binary operations preserve bytes.

In UTF builds, `.string` source values are valid UTF-8 text. Converting a
string to `.binary` stores the exact UTF-8 bytes currently held by the string;
the conversion does not normalize, transcode, or reinterpret the text:

```rexx
payload = "alpha" as .binary
```

Converting `.binary` to `.string` validates the byte sequence as UTF-8:

```rexx
payload = "ceb1"x as .binary
text = payload as .string     /* "α" */
```

An invalid binary-to-string conversion raises `UNICODE_ERROR` at runtime. If the
invalid bytes are visible as a constant literal in the cast, the compiler rejects
the program with `CANNOT_CAST_BINARY`.

Invalid UTF-8 byte sequences are only valid in an explicit binary context:

```rexx
payload = .binary
payload = 'ffff'x

other = 'ffff'x as .binary
```

A first untyped assignment such as `payload = 'ffff'x` is treated as a text
assignment and is rejected when the decoded bytes are not valid UTF-8. That rule
keeps accidental invalid text out of string operations; use `.binary` when the
program is handling bytes.

Binary concatenation is byte concatenation. If either operand of `||` is
`.binary`, the expression result is `.binary`; string operands in that binary
expression are converted to their exact UTF-8 bytes. Blank concatenation remains
a text operation and should not be used for binary payload assembly.

```rexx
prefix = "ff"x as .binary
packet = prefix || "OK"      /* bytes ff 4f 4b */
```

The `rxfnsb` library provides byte-oriented helpers for common binary work:
`binlength`, `binbyte`, `binsetbyte`, `binsubstr`, `binconcat`, `binoverlay`,
`bininsert`, `bindelstr`, `binpos`, `bincompare`, `bin2x`, and `x2bin`.

The same boundary applies outside source literals. Native RXVML string setters,
CREXXSAA ADDRESS variable setters, RXPA native return/argument trees,
command-line arguments passed through RXVML, ADDRESS callback text, text file
reads, socket text reads, and explicit binary-to-string casts validate UTF-8 in
normal Level B builds. Invalid bytes should be read or carried as `.binary`
first, then decoded to `.string` only when the program has a valid encoding.

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

Level B does not define implicit object-to-string promotion. Statements such as
`say value`, string concatenation, and string comparison operate on values whose
types are already string-compatible under the Level B type rules; they do not
automatically call a `toString()` method on arbitrary objects. A future Level G
object-promotion capability is still undesigned. The likely direction is an
explicit contract, such as a supported interface for string rendering, rather
than a convention based only on a method name.

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

# Level B Named Operators

Status: approved initial implementation

Level B supports a small fixed set of named operators using a contiguous
angle-bracket token:

```rexx
flags = flags <or> RV_FLAG_INT
if flags <has> RV_FLAG_TEXT then ...
flags = flags <clear> RV_FLAG_BINARY
```

The scanner recognizes `<id>` as one token. Embedded blanks or comments are not
allowed:

```rexx
a <or> b      /* named operator */
a < or > b    /* ordinary tokens, not a named operator */
```

The syntax is Level B system-programmer surface. It exists so Level C runtime
classes, which are implemented in Level B, can express low-level integer and
flag operations without overloading Classic Rexx logical operators. Level C
Classic syntax remains unchanged.

## Initial Fixed Operators

The first implementation is deliberately fixed and compiler-known:

| Form | Result | Lowering |
|------|--------|----------|
| `a <and> b` | `.int` | RXAS `iand` |
| `a <or> b` | `.int` | RXAS `ior` |
| `a <xor> b` | `.int` | RXAS `ixor` |
| `<not> a` | `.int` | RXAS `inot` |
| `a <idiv> b` | numeric | Integer division, independent of `OPTIONS NUMERIC_*` token rules |
| `a <mod> b` | numeric | Remainder, independent of `OPTIONS NUMERIC_*` token rules |
| `a <rem> b` | numeric | Alias for `<mod>` |
| `a <shl> b` | `.int` | RXAS `ishl` |
| `a <shr> b` | `.int` | RXAS `ishr` |
| `a <has> b` | `.boolean` | `(a <and> b) <> 0` |
| `a <set> b` | `.int` | `a <or> b` |
| `a <clear> b` | `.int` | `a <and> <not> b` |
| `a <refsame> b` | `.boolean` | RXAS `refsame`; true only for two references retaining the same storage-identity cell |

Operator names are case-insensitive. Level G additionally reserves `<eq>` as
the canonical object-equivalence spelling and lowers it to the opted-in
`.ObjectEquatable.equivalent()` contract. General user-defined named operators
and operator-method extension remain deferred. If added, they should use this
same token shape, but with fixed compiler precedence classes rather than
arbitrary per-operator precedence.

## Precedence

Fixed named operators bind with the closest existing operator family:

| Precedence | Operators |
|------------|-----------|
| Prefix | `<not>` |
| Multiply/divide | `<idiv>` `<mod>` `<rem>` |
| After add/subtract | `<shl>` `<shr>` |
| After shift | `<and>` `<has>` `<clear>` |
| After named AND | `<xor>` |
| After named XOR, before concatenation/comparison | `<or>` `<set>` |
| Comparison | `<refsame>` and Level G `<eq>` |

All binary named operators are left-associative within their own family. Unknown
`<id>` forms are parsed in the lowest named-operator family so the compiler can
diagnose `UNKNOWN_NAMED_OPERATOR`.

This keeps the flag idiom readable:

```rexx
if flags <has> RV_FLAG_STRING then ...
```

and lets arithmetic operands remain natural:

```rexx
mask = 1 <shl> bit + 1
```

which is parsed as:

```rexx
mask = 1 <shl> (bit + 1)
```

`<refsame>` is deliberately narrower than object equality. Both operands must
be compatible reference types. It compares the retained storage-identity cell,
does not dereference either operand, and remains true for two copied references
after their common target expires. Cleared or empty references have no identity
and compare false. Ordinary `=` and `==` remain invalid for references.

Mixed bit/flag expressions follow the same family ordering:

```rexx
flags = flags <set> 1 <shl> bit
```

which is parsed as:

```rexx
flags = flags <set> (1 <shl> bit)
```

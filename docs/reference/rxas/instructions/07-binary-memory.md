# Binary Memory

Binary-memory instructions treat a `.binary` register or `BINARY_CONST` as
byte-addressable memory. They are intended for structured binary data, compact
indexes, lookup tables, parsers, and other code where copying into ordinary
Rexx variables would dominate runtime cost.

This chapter documents the Release 1 RXAS binary surface. Offsets and lengths
are zero-based byte counts unless an instruction explicitly says otherwise.
Fixed-width fields use canonical little-endian storage; they do not depend on
host struct layout, alignment, or native endian order.

## Shared Conventions

Binary registers are mutable. Binary constants are read-only constant-pool
values and may be written inline as `0x...` or named with `.const`:

```rxas
.const table binary 0x0011223344556677
.const key string "index"
```

An alias is accepted anywhere the corresponding inline binary or string
constant is accepted by that instruction form. Using a binary constant form
does not materialize the constant in a register.

Common operand names:

- `rBin`: mutable binary register.
- `rDst`: destination register.
- `rSrc`: source register or binary constant, depending on the form.
- `rOffset`: integer register containing a byte offset.
- `rLen`: integer register containing a byte length.
- `rValue`: integer or float register containing a field value.
- `rCmp`: integer register used by compare instructions.
- `bConst`: inline binary literal or binary constant alias.
- `sConst`: inline string literal or string constant alias.

Strict binary-memory instructions raise `OUT_OF_RANGE` for negative offsets,
negative lengths, ranges outside the logical binary length, or integer values
that cannot be represented in the requested field width. Allocation failure
raises `FAILURE`. UTF-8 text-field reads and conversions raise `UNICODE_ERROR`
when the source bytes are not valid UTF-8.

`bslice` takes an explicit start and length. Release 1 compiler lowering should
prefer direct offset instructions such as `bcopy`, `bget*`, `bset*`, `bgets`,
`bsets`, `bcmpb`, and `bcmps` when materializing a slice is unnecessary.

## `bappend`

Append the binary payload of one register to the end of another register.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x00bb` | `bappend rDst,rRight` | Append all bytes from `rRight` to `rDst`. |

### Operands And Semantics

`rDst` and `rRight` are binary registers. `rDst` is resized to hold its original
bytes followed by `rRight`. `rRight` is unchanged.

### Signals

Allocation failure raises `FAILURE`.

### Example

<!-- rxas-example name="binary-bappend" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r0,0x01
    load r1,0x0203
    bappend r0,r1
    ret
```

### Related

`bconcat`, `bcopy`, `bupdate`.

## `bcheckrange`

Check that a byte range fits inside a binary register without changing any
operand.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0257` | `bcheckrange rBin,rOffset,rLen` | Assert that `rOffset..rOffset+rLen` is inside `rBin`. |

### Operands And Semantics

`rOffset` and `rLen` are integer registers. The end offset is exclusive.

### Signals

Raises `OUT_OF_RANGE` for a negative offset, negative length, arithmetic
overflow while computing the end offset, or a range past the logical binary
length.

### Example

<!-- rxas-example name="binary-bcheckrange" test="run" -->
```rxas
.globals=0

main() .locals=3
    load r0,0x001122
    load r1,1
    load r2,2
    bcheckrange r0,r1,r2
    ret
```

### Related

`bcopy`, `bget*`, `bset*`, `bgets`, `bsets`.

## `bclear`

Clear a binary register by setting its logical byte length to zero.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0247` | `bclear rBin` | Set `rBin` to an empty binary value. |

### Operands And Semantics

The register remains a binary value with a zero logical length.

### Signals

This instruction does not signal.

### Example

<!-- rxas-example name="binary-bclear" test="run" -->
```rxas
.globals=0

main() .locals=1
    load r0,0x0011
    bclear r0
    ret
```

### Related

`bresize`, `blen`.

## `bcmpb`

Compare bytes from binary memory with another binary value without copying the
source slice into a temporary register.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x026f` | `bcmpb rCmp,rBin,rNeedle` | Compare a slice of `rBin` with binary register `rNeedle`. |
| `0x0270` | `bcmpb rCmp,bConst,rNeedle` | Compare a slice of binary constant `bConst` with binary register `rNeedle`. |
| `0x0271` | `bcmpb rCmp,rBin,bConst` | Compare a slice of `rBin` with binary constant `bConst`. |
| `0x0272` | `bcmpb rCmp,bConst,bConst` | Compare a slice of one binary constant with another binary constant. |

### Operands And Semantics

On entry, `rCmp` contains the byte offset into the source. The compare length is
the full logical byte length of the needle. On return, `rCmp` is overwritten
with `-1`, `0`, or `1` for unsigned-byte lexicographic ordering.

### Signals

Raises `OUT_OF_RANGE` when the source offset is negative or the needle-length
source range does not fit. If the caller still needs the offset after the
compare, copy it to a scratch register first.

### Example

<!-- rxas-example name="binary-bcmpb" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r0,1
    load r1,0x001122
    bcmpb r0,r1,0x1122
    ret
```

### Related

`bcmps`, `bgets`, `bcopy`.

## `bineq`

Compare two complete logical binary values for equality. Unlike `bcmpb`, this
compares both lengths and all bytes and does not interpret an integer offset.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x027d` | `bineq rResult,rLeft,rRight` | Set `rResult` to one when both complete binary values are equal. |
| `0x027e` | `bineq rResult,rLeft,bConst` | Compare a binary register with a binary constant. |

### Operands And Semantics

`rLeft` and `rRight` are binary registers. `bConst` is an inline binary literal
or named binary constant. `rResult` receives integer Boolean `0` or `1`; only
that payload changes and both inputs are unchanged.

### Signals

These comparisons do not resize, copy, or modify either input and do not raise
`OUT_OF_RANGE`.

### Example

<!-- rxas-example name="binary-bineq" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,0x0011
    bineq r0,r1,0x0011
    ret
```

### Related

`bcmpb`, `jumpb`, `bcopy`.

## `binne`

Compare two complete logical binary values for inequality.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x027f` | `binne rResult,rLeft,rRight` | Test register lengths and bytes. |
| `0x0280` | `binne rResult,rLeft,bConst` | Compare register with binary constant. |

### Operands And Semantics

`rResult` receives integer Boolean `1` when lengths or any bytes differ,
otherwise `0`. Only its integer payload changes; inputs are unchanged.

### Signals

This instruction does not signal or raise `OUT_OF_RANGE`.

### Example

<!-- rxas-example name="binary-binne" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,0x0011
    binne r0,r1,0x0022
    ret
```

### Related

`bineq`, `bcmpb`, `jumpb`.

## `bcmps`

Compare a zero-terminated UTF-8 field in binary memory with a string without
copying the source field into a temporary string register.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0273` | `bcmps rCmp,rBin,rString` | Compare a binary-register text field with string register `rString`. |
| `0x0274` | `bcmps rCmp,rBin,sConst` | Compare a binary-register text field with string constant `sConst`. |
| `0x0275` | `bcmps rCmp,bConst,rString` | Compare a binary-constant text field with string register `rString`. |
| `0x0276` | `bcmps rCmp,bConst,sConst` | Compare a binary-constant text field with string constant `sConst`. |

### Operands And Semantics

On entry, `rCmp` contains the byte offset of a zero-terminated UTF-8 field in
the binary source. On return, `rCmp` is overwritten with `-1`, `0`, or `1` using
normal string comparison ordering.

### Signals

Raises `OUT_OF_RANGE` if the offset is negative, the offset is beyond the
source, or no NUL terminator is found. Raises `UNICODE_ERROR` if the field bytes
before the terminator are not valid UTF-8.

### Example

<!-- rxas-example name="binary-bcmps" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r0,0
    load r1,0x616200
    bcmps r0,r1,"ab"
    ret
```

### Related

`bcmpb`, `bgets`, `bsets`.

## `bconcat`

Concatenate two binary registers into a destination register.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x00ba` | `bconcat rDst,rLeft,rRight` | Store `rLeft || rRight` as a binary value in `rDst`. |

### Operands And Semantics

All operands are registers. The source operands are read as binary values.

### Signals

Allocation failure raises `FAILURE`.

### Example

<!-- rxas-example name="binary-bconcat" test="run" -->
```rxas
.globals=0

main() .locals=3
    load r1,0x01
    load r2,0x02
    bconcat r0,r1,r2
    ret
```

### Related

`bappend`, `bcopy`.

## `bcopy`

Copy binary data either as a whole-register copy or as a target-sized slice from
a register or constant source.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0245` | `bcopy rDst,rSrc` | Copy the whole binary payload of register `rSrc` into `rDst`. |
| `0x0259` | `bcopy rDst,rSrc,rOffset` | Copy `blen(rDst)` bytes from binary register `rSrc` at `rOffset` into `rDst`. |
| `0x025a` | `bcopy rDst,bConst,rOffset` | Copy `blen(rDst)` bytes from binary constant `bConst` at `rOffset` into `rDst`. |

### Operands And Semantics

The three-operand forms use the current logical length of `rDst` as the copy
length. The destination must therefore be sized before the instruction runs.

### Signals

Three-operand forms raise `OUT_OF_RANGE` when `rOffset` is negative or the
target-sized slice does not fit in the source. Allocation failure raises
`FAILURE`.

### Example

<!-- rxas-example name="binary-bcopy-one" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,0x001122
    bcopy r0,r1
    ret
```

### Related

`bresize`, `blen`, `bmove`, `bmemmove`.

## `bfill`

Fill every byte in a binary register with one byte value.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0248` | `bfill rBin,rByte` | Fill the current logical byte range of `rBin` with `rByte`. |

### Operands And Semantics

`rByte` is an integer register and must contain a value in `0..255`.

### Signals

Raises `OUT_OF_RANGE` when the byte value is outside `0..255`.

### Example

<!-- rxas-example name="binary-bfill" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r0,0x000000
    load r1,255
    bfill r0,r1
    ret
```

### Related

`bresize`, `bclear`, `setbyte`.

## `bgetf32`

Read an IEEE binary32 field and widen it into a VM float register.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0264` | `bgetf32 rOut,rBin,rOffset` | Read 4 little-endian bytes from binary register `rBin`. |
| `0x0265` | `bgetf32 rOut,bConst,rOffset` | Read 4 little-endian bytes from binary constant `bConst`. |

### Operands And Semantics

`rOffset.int` is a zero-based byte offset. Only `rOut.float` changes; the
source, offset, and other destination state are unchanged.

### Signals

Raises `OUT_OF_RANGE` if the 4-byte field does not fit.

### Example

<!-- rxas-example name="binary-bgetf32" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,0x0000803f
    load r0,0
    bgetf32 r0,r1,r0
    ret
```

### Related

`bsetf32`, `bgetf64`.

## `bgetf64`

Read an IEEE binary64 field into a VM float register.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x024f` | `bgetf64 rOut,rBin,rOffset` | Read 8 little-endian bytes from binary register `rBin`. |
| `0x0261` | `bgetf64 rOut,bConst,rOffset` | Read 8 little-endian bytes from binary constant `bConst`. |

### Operands And Semantics

`rOffset.int` is a zero-based byte offset. Only `rOut.float` changes; sources
are unchanged.

### Signals

Raises `OUT_OF_RANGE` if the 8-byte field does not fit.

### Example

<!-- rxas-example name="binary-bgetf64" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,0x000000000000f03f
    load r0,0
    bgetf64 r0,r1,r0
    ret
```

### Related

`bsetf64`, `bgetf32`.

## `bgeti16`

Read a signed 16-bit little-endian integer field.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x024c` | `bgeti16 rOut,rBin,rOffset` | Read signed 16-bit field from binary register `rBin`. |
| `0x025e` | `bgeti16 rOut,bConst,rOffset` | Read signed 16-bit field from binary constant `bConst`. |

### Operands And Semantics

`rOffset.int` is zero-based. The field is sign-extended into `rOut.int`; sources
and other destination state are unchanged.

### Signals

Raises `OUT_OF_RANGE` if the 2-byte field does not fit.

### Example

<!-- rxas-example name="binary-bgeti16" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,0xfeff
    load r0,0
    bgeti16 r0,r1,r0
    ret
```

### Related

`bseti16`, `bgetu16`.

## `bgeti32`

Read a signed 32-bit little-endian integer field.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x024e` | `bgeti32 rOut,rBin,rOffset` | Read signed 32-bit field from binary register `rBin`. |
| `0x0260` | `bgeti32 rOut,bConst,rOffset` | Read signed 32-bit field from binary constant `bConst`. |

### Operands And Semantics

`rOffset.int` is zero-based. The field is sign-extended into `rOut.int`; sources
are unchanged.

### Signals

Raises `OUT_OF_RANGE` if the 4-byte field does not fit.

### Example

<!-- rxas-example name="binary-bgeti32" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,0xfeffffff
    load r0,0
    bgeti32 r0,r1,r0
    ret
```

### Related

`bseti32`, `bgetu32`.

## `bgeti64`

Read a signed 64-bit little-endian integer field. This is the Release 1 binary
storage form for `.int`.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0262` | `bgeti64 rOut,rBin,rOffset` | Read signed 64-bit field from binary register `rBin`. |
| `0x0263` | `bgeti64 rOut,bConst,rOffset` | Read signed 64-bit field from binary constant `bConst`. |

### Operands And Semantics

`rOffset.int` is zero-based. Only `rOut.int` changes; sources remain unchanged.
The active VM integer type must represent the decoded value.

### Signals

Raises `OUT_OF_RANGE` if the 8-byte field does not fit.

### Example

<!-- rxas-example name="binary-bgeti64" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,0xfeffffffffffffff
    load r0,0
    bgeti64 r0,r1,r0
    ret
```

### Related

`bseti64`.

## `bgeti8`

Read a signed 8-bit integer field and sign-extend it into an integer register.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x024a` | `bgeti8 rOut,rBin,rOffset` | Read signed byte from binary register `rBin`. |
| `0x025c` | `bgeti8 rOut,bConst,rOffset` | Read signed byte from binary constant `bConst`. |

### Operands And Semantics

`rOffset.int` is zero-based. The byte is sign-extended into `rOut.int`; sources
remain unchanged.

### Signals

Raises `OUT_OF_RANGE` if the byte offset is outside the source.

### Example

<!-- rxas-example name="binary-bgeti8" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,0xfe
    load r0,0
    bgeti8 r0,r1,r0
    ret
```

### Related

`bseti8`, `bgetu8`.

## `bgets`

Read a zero-terminated UTF-8 field from binary memory into a string register.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0268` | `bgets rString,rBin,rOffset` | Read a NUL-terminated UTF-8 field from binary register `rBin`. |
| `0x0269` | `bgets rString,bConst,rOffset` | Read a NUL-terminated UTF-8 field from binary constant `bConst`. |

### Operands And Semantics

`rOffset` is a byte offset. The source bytes before the first zero byte are
validated as UTF-8 and copied into `rString`. The NUL terminator is not part of
the destination string.

### Signals

Raises `OUT_OF_RANGE` if the offset is negative, the offset is beyond the
source, or no NUL terminator is found. Raises `UNICODE_ERROR` for invalid UTF-8
before the terminator.

### Example

<!-- rxas-example name="binary-bgets" test="run" -->
```rxas
.globals=0

main() .locals=3
    load r1,0x686900
    load r2,0
    bgets r0,r1,r2
    ret
```

### Related

`bsets`, `bcmps`, `bintos`.

## `bgetu16`

Read an unsigned 16-bit little-endian integer field.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x024b` | `bgetu16 rOut,rBin,rOffset` | Read unsigned 16-bit field from binary register `rBin`. |
| `0x025d` | `bgetu16 rOut,bConst,rOffset` | Read unsigned 16-bit field from binary constant `bConst`. |

### Operands And Semantics

`rOffset.int` is zero-based. The zero-extended value replaces only `rOut.int`;
sources remain unchanged.

### Signals

Raises `OUT_OF_RANGE` if the 2-byte field does not fit or if the value cannot
be represented in the active VM integer type.

### Example

<!-- rxas-example name="binary-bgetu16" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,0xffff
    load r0,0
    bgetu16 r0,r1,r0
    ret
```

### Related

`bsetu16`, `bgeti16`.

## `bgetu32`

Read an unsigned 32-bit little-endian integer field.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x024d` | `bgetu32 rOut,rBin,rOffset` | Read unsigned 32-bit field from binary register `rBin`. |
| `0x025f` | `bgetu32 rOut,bConst,rOffset` | Read unsigned 32-bit field from binary constant `bConst`. |

### Operands And Semantics

`rOffset.int` is zero-based. The zero-extended value replaces only `rOut.int`;
sources remain unchanged.

### Signals

Raises `OUT_OF_RANGE` if the 4-byte field does not fit or if the value cannot
be represented in the active VM integer type.

### Example

<!-- rxas-example name="binary-bgetu32" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,0xffffffff
    load r0,0
    bgetu32 r0,r1,r0
    ret
```

### Related

`bsetu32`, `bgeti32`.

## `bgetu8`

Read an unsigned byte field.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0249` | `bgetu8 rOut,rBin,rOffset` | Read one unsigned byte from binary register `rBin`. |
| `0x025b` | `bgetu8 rOut,bConst,rOffset` | Read one unsigned byte from binary constant `bConst`. |

### Operands And Semantics

`rOffset.int` is zero-based. The byte replaces only `rOut.int`; sources remain
unchanged.

### Signals

Raises `OUT_OF_RANGE` if the byte offset is outside the source.

### Example

<!-- rxas-example name="binary-bgetu8" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,0xff
    load r0,0
    bgetu8 r0,r1,r0
    ret
```

### Related

`bsetu8`, `bgeti8`, `getbyte`.

## `bintos`

Convert the current binary bytes in a register to a string value.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x00c1` | `bintos rReg` | Validate `rReg` binary bytes as UTF-8 and copy them to the string slot. |

### Operands And Semantics

`bintos` is a whole-register conversion. It does not require or consume a NUL
terminator.

### Signals

Raises `UNICODE_ERROR` when the binary bytes are not valid UTF-8.

### Example

<!-- rxas-example name="binary-bintos" test="run" -->
```rxas
.globals=0

main() .locals=1
    load r0,0x6869
    bintos r0
    ret
```

### Related

`stobin`, `bgets`.

## `blen`

Return the logical byte length of a binary register or binary constant.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x00b8` | `blen rOut,rBin` | Store the logical byte length of binary register `rBin` in `rOut`. |
| `0x0258` | `blen rOut,bConst` | Store the logical byte length of binary constant `bConst` in `rOut`. |

### Operands And Semantics

Only `rOut.int` changes. The source binary remains unchanged.

### Signals

This instruction does not signal.

### Example

<!-- rxas-example name="binary-blen" test="run" -->
```rxas
.globals=0

main() .locals=1
    blen r0,0x001122
    ret
```

### Related

`bresize`, `bcopy`.

## `bmemmove`

Move bytes within one binary register. Overlapping source and destination
ranges are safe.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x026e` | `bmemmove rBin,rDstOffset,rLen` | Copy `rLen` bytes within `rBin`. |

### Operands And Semantics

The source byte offset is read from the integer slot of `rBin`. The destination
byte offset is read from `rDstOffset`. `rLen` is the byte count.

### Signals

Raises `OUT_OF_RANGE` when either range is negative or outside `rBin`.

### Example

<!-- rxas-example name="binary-bmemmove" test="run" -->
```rxas
.globals=0

main() .locals=3
    load r0,0x00112233
    load r0,0
    load r1,1
    load r2,3
    bmemmove r0,r1,r2
    ret
```

### Related

`bmove`, `bcopy`.

## `bmove`

Copy bytes between two different binary registers using independent source and
destination offsets.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x026d` | `bmove rDst,rSrc,rLen` | Copy `rLen` bytes from `rSrc` to `rDst`. |

### Operands And Semantics

The destination byte offset is read from the integer slot of `rDst`. The source
byte offset is read from the integer slot of `rSrc`. `rLen` is the byte count.
`rDst` and `rSrc` must be different registers.

### Signals

Raises `OUT_OF_RANGE` when either range is negative or outside its binary
register. Raises `INVALID_ARGUMENTS` when `rDst` and `rSrc` are the same
register.

### Example

<!-- rxas-example name="binary-bmove" test="run" -->
```rxas
.globals=0

main() .locals=3
    load r0,0x000000
    load r1,0x112233
    load r0,0
    load r1,1
    load r2,2
    bmove r0,r1,r2
    ret
```

### Related

`bmemmove`, `bcopy`.

## `bresize`

Resize a binary register.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0246` | `bresize rBin,rLen` | Set the logical byte length of `rBin` to `rLen`. |

### Operands And Semantics

Existing bytes are preserved up to the new length. Growth is zero-filled.
`bresize` sets the logical length observed by `blen`; the VM may keep a larger
private physical allocation and grow that allocation in blocks so repeated
append/resize patterns do not reallocate on every logical growth.

### Signals

Raises `OUT_OF_RANGE` for a negative length. Allocation failure raises
`FAILURE`.

### Related

`bclear`, `bfill`, `blen`.

### Example

<!-- rxas-example name="binary-bresize" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r0,0x01
    load r1,4
    bresize r0,r1
    ret
```

## `bsetf32`

Write an IEEE binary32 field from a VM float register.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0267` | `bsetf32 rBin,rOffset,rValue` | Write 4 little-endian bytes to `rBin` at `rOffset`. |

### Operands And Semantics

`rOffset.int` is zero-based and `rValue.float` supplies the value. Four bytes
change; source registers are unchanged.

### Signals

Raises `OUT_OF_RANGE` if the 4-byte field does not fit.

### Example

<!-- rxas-example name="binary-bsetf32" test="run" -->
```rxas
.globals=0

main() .locals=3
    load r0,0x00000000
    load r1,0
    load r2,1.5
    bsetf32 r0,r1,r2
    ret
```

### Related

`bgetf32`, `bsetf64`.

## `bsetf64`

Write an IEEE binary64 field from a VM float register.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0256` | `bsetf64 rBin,rOffset,rValue` | Write 8 little-endian bytes to `rBin` at `rOffset`. |

### Operands And Semantics

`rOffset.int` is zero-based and `rValue.float` supplies the value. Eight bytes
change; sources are unchanged.

### Signals

Raises `OUT_OF_RANGE` if the 8-byte field does not fit.

### Related

`bgetf64`, `bsetf32`.

### Example

<!-- rxas-example name="binary-bsetf64" test="run" -->
```rxas
.globals=0

main() .locals=3
    load r0,0x0000000000000000
    load r1,0
    load r2,1.5
    bsetf64 r0,r1,r2
    ret
```

## `bseti16`

Write a signed 16-bit little-endian integer field.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0253` | `bseti16 rBin,rOffset,rValue` | Write signed 16-bit field to `rBin`. |

### Operands And Semantics

The zero-based offset and signed value come from integer payloads. Two bytes
change; source registers are unchanged.

### Signals

Raises `OUT_OF_RANGE` if the field does not fit or the value is outside the
signed 16-bit range.

### Related

`bgeti16`, `bsetu16`.

### Example

<!-- rxas-example name="binary-bseti16" test="run" -->
```rxas
.globals=0

main() .locals=3
    load r0,0x0000
    load r1,0
    load r2,-2
    bseti16 r0,r1,r2
    ret
```

## `bseti32`

Write a signed 32-bit little-endian integer field.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0255` | `bseti32 rBin,rOffset,rValue` | Write signed 32-bit field to `rBin`. |

### Operands And Semantics

The zero-based offset and signed value come from integer payloads. Four bytes
change; sources are unchanged.

### Signals

Raises `OUT_OF_RANGE` if the field does not fit or the value is outside the
signed 32-bit range.

### Related

`bgeti32`, `bsetu32`.

### Example

<!-- rxas-example name="binary-bseti32" test="run" -->
```rxas
.globals=0

main() .locals=3
    load r0,0x00000000
    load r1,0
    load r2,-2
    bseti32 r0,r1,r2
    ret
```

## `bseti64`

Write a signed 64-bit little-endian integer field. This is the Release 1 binary
storage form for `.int`.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0266` | `bseti64 rBin,rOffset,rValue` | Write signed 64-bit field to `rBin`. |

### Operands And Semantics

The zero-based offset and signed value come from integer payloads. Eight bytes
change; sources are unchanged.

### Signals

Raises `OUT_OF_RANGE` if the field does not fit.

### Example

<!-- rxas-example name="binary-bseti64" test="run" -->
```rxas
.globals=0

main() .locals=3
    load r0,0x0000000000000000
    load r1,0
    load r2,-2
    bseti64 r0,r1,r2
    ret
```

### Related

`bgeti64`.

## `bseti8`

Write a signed 8-bit integer field.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0251` | `bseti8 rBin,rOffset,rValue` | Write one signed byte to `rBin`. |

### Operands And Semantics

The zero-based offset and signed value come from integer payloads. One byte
changes; sources are unchanged.

### Signals

Raises `OUT_OF_RANGE` if the byte offset is outside the destination or the
value is outside the signed 8-bit range.

### Related

`bgeti8`, `bsetu8`.

### Example

<!-- rxas-example name="binary-bseti8" test="run" -->
```rxas
.globals=0

main() .locals=3
    load r0,0x00
    load r1,0
    load r2,-2
    bseti8 r0,r1,r2
    ret
```

## `bsets`

Write a string into binary memory as UTF-8 bytes followed by a zero terminator.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x026a` | `bsets rBin,rOffset,rString` | Write a string register plus NUL to `rBin`. |
| `0x026b` | `bsets rBin,rOffset,sConst` | Write a string constant plus NUL to `rBin`. |

### Operands And Semantics

The write length is the string byte length plus one terminator byte. The target
binary register is not resized by `bsets`; the complete write must fit.

### Signals

Raises `OUT_OF_RANGE` if the offset is negative or the string plus terminator
does not fit in the destination.

### Example

<!-- rxas-example name="binary-bsets" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r0,0x000000
    load r1,0
    bsets r0,r1,"hi"
    ret
```

### Related

`bgets`, `bcmps`, `stobin`.

## `bsetu16`

Write an unsigned 16-bit little-endian integer field.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0252` | `bsetu16 rBin,rOffset,rValue` | Write unsigned 16-bit field to `rBin`. |

### Operands And Semantics

The zero-based offset and unsigned value come from integer payloads. Two bytes
change; sources are unchanged.

### Signals

Raises `OUT_OF_RANGE` if the field does not fit or the value is outside
`0..65535`.

### Related

`bgetu16`, `bseti16`.

### Example

<!-- rxas-example name="binary-bsetu16" test="run" -->
```rxas
.globals=0
main() .locals=3
    load r0,0x0000
    load r1,0
    load r2,65535
    bsetu16 r0,r1,r2
    ret
```

## `bsetu32`

Write an unsigned 32-bit little-endian integer field.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0254` | `bsetu32 rBin,rOffset,rValue` | Write unsigned 32-bit field to `rBin`. |

### Operands And Semantics

The zero-based offset and unsigned value come from integer payloads. Four bytes
change; sources are unchanged.

### Signals

Raises `OUT_OF_RANGE` if the field does not fit or the value is outside the
unsigned 32-bit range.

### Related

`bgetu32`, `bseti32`.

### Example

<!-- rxas-example name="binary-bsetu32" test="run" -->
```rxas
.globals=0
main() .locals=3
    load r0,0x00000000
    load r1,0
    load r2,4294967295
    bsetu32 r0,r1,r2
    ret
```

## `bsetu8`

Write an unsigned byte field.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0250` | `bsetu8 rBin,rOffset,rValue` | Write one unsigned byte to `rBin`. |

### Operands And Semantics

The zero-based offset and byte value come from integer payloads. One byte
changes; sources are unchanged.

### Signals

Raises `OUT_OF_RANGE` if the byte offset is outside the destination or the
value is outside `0..255`.

### Related

`bgetu8`, `setbyte`, `bfill`.

### Example

<!-- rxas-example name="binary-bsetu8" test="run" -->
```rxas
.globals=0
main() .locals=3
    load r0,0x00
    load r1,0
    load r2,255
    bsetu8 r0,r1,r2
    ret
```

## `bslice`

Copy an explicit byte range from one binary register into another register.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x00be` | `bslice rDst,rSrc,rStart,rLen` | Copy bytes `[rStart,rStart+rLen)`. |

### Operands And Semantics

`rStart` and `rLen` are integer registers. A negative start clamps to zero; a
start or length beyond the source clips to its logical byte length. The source
is unchanged. Strict fixed-size field extraction should use target-sized
`bcopy` or typed binary reads.

### Related

`bcopy`, `bmove`, `bmemmove`.

### Signals

Negative lengths raise `OUT_OF_RANGE`; copying truncates at end of source.
Allocation failure raises `FAILURE`.

### Example

<!-- rxas-example name="binary-bslice" test="run" -->
```rxas
.globals=0
main() .locals=4
    load r1,0x001122
    load r0,0x
    load r2,1
    load r3,2
    bslice r0,r1,r2,r3
    ret
```

## `bupdate`

Overlay the whole binary payload of one register into another binary register.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x00bf` | `bupdate rDst,rOffset,rSrc` | Copy all bytes from `rSrc` into `rDst` at `rOffset`. |

### Operands And Semantics

`rOffset.int` is zero-based. The whole source payload overwrites the matching
destination range; logical lengths and source bytes are unchanged.

### Signals

Raises `OUT_OF_RANGE` when the offset is negative or the overlay would extend
past the logical length of `rDst`.

### Related

`bcopy`, `bmove`, `bmemmove`.

### Example

<!-- rxas-example name="binary-bupdate" test="run" -->
```rxas
.globals=0
main() .locals=3
    load r0,0x000000
    load r1,1
    load r2,0x1122
    bupdate r0,r1,r2
    ret
```

## `getbyte`

Read one byte from a binary register using the legacy tolerant byte-read
semantics.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x00a7` | `getbyte rOut,rBin,rOffset` | Store byte value or `-1` if the offset is outside `rBin`. |

### Operands And Semantics

`getbyte` has no binary-constant form. Use `bgetu8` for strict register and
constant byte reads.

### Related

`setbyte`, `bgetu8`.

### Signals

This tolerant form does not signal for an invalid offset; it stores `-1`.

### Example

<!-- rxas-example name="binary-getbyte" test="run" -->
```rxas
.globals=0
main() .locals=3
    load r1,0x11
    load r2,0
    getbyte r0,r1,r2
    ret
```

## `setbyte`

Write one byte to a binary register.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x00b9` | `setbyte rBin,rOffset,rByte` | Store byte `rByte` at byte offset `rOffset`. |

### Operands And Semantics

Both register operands supply integer payloads. One destination byte changes;
logical length and source registers are unchanged.

### Signals

Raises `OUT_OF_RANGE` if the offset is outside the destination or the byte
value is outside `0..255`.

### Related

`getbyte`, `bsetu8`.

### Example

<!-- rxas-example name="binary-setbyte" test="run" -->
```rxas
.globals=0
main() .locals=3
    load r0,0x00
    load r1,0
    load r2,255
    setbyte r0,r1,r2
    ret
```

## `sget`

Extract a codepoint-counted string slice from a string constant, starting at a
byte offset.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x026c` | `sget rString,sConst,rOffset` | Copy codepoints from string constant `sConst` into `rString`. |

### Operands And Semantics

`rOffset` is a byte offset into the UTF-8 string constant and must be on a
codepoint boundary. The existing codepoint length of `rString` is the requested
copy count. The destination is resized as needed and receives a safety NUL
outside the logical string value.

### Signals

Raises `UNICODE_ERROR` when `rOffset` is not a valid UTF-8 codepoint boundary.
Raises `OUT_OF_RANGE` when the requested codepoint count cannot be satisfied.

### Example

<!-- rxas-example name="binary-sget" test="run" -->
```rxas
.globals=0
main() .locals=2
    load r0,"xx"
    load r1,0
    sget r0,"hi",r1
    ret
```

### Related

`bgets`, `bsets`.

## `stobin`

Convert the current string bytes in a register to a binary value.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x00c0` | `stobin rReg` | Copy the register string bytes into its binary slot. |

### Operands And Semantics

`stobin` is a whole-register conversion. It copies the logical string bytes
exactly and does not append a NUL terminator.

### Related

`bintos`, `bsets`.

### Signals

Allocation failure raises `FAILURE`; the string is assumed already valid UTF-8.

### Example

<!-- rxas-example name="binary-stobin" test="run" -->
```rxas
.globals=0
main() .locals=1
    load r0,"hi"
    stobin r0
    ret
```

## Examples

The examples in this section are complete RXAS programs. They use plain `rxas`
fences for syntax highlighters and preceding `rxas-example` comments for future
doc-pipeline test extraction.

### `binary-bcopy`

Target-sized copying uses the destination binary length as the copy length, so
the destination is resized before the three-operand `bcopy`.

<!-- rxas-example name="binary-bcopy" test="run" -->
```rxas
/* Binary target-sized copy example */
.const table binary 0xaabbccdd
main() .locals=8
    load r1,0x0011223344556677
    load r2,3
    bresize r3,r2
    load r4,2
    bcopy r3,r1,r4
    load r4,0
    bgetu8 r5,r3,r4
    itos r5
    say r5
    load r4,2
    bgetu8 r6,r3,r4
    itos r6
    say r6

    load r4,1
    bcopy r3,table,r4
    load r4,0
    bgetu8 r5,r3,r4
    itos r5
    say r5

    blen r7,0x0102030405
    itos r7
    say r7
    ret
```

<!-- rxas-output for="binary-bcopy" -->
```text
34
68
187
5
```

### `binary-fixed-width`

Fixed-width reads and writes use explicit storage widths. Constants can be read
directly without first loading the whole binary into a register.

<!-- rxas-example name="binary-fixed-width" test="run" -->
```rxas
/* Binary fixed-width field example */
main() .locals=8
    load r1,0x
    load r2,16
    bresize r1,r2

    load r2,0
    load r3,-42
    bseti64 r1,r2,r3
    bgeti64 r4,r1,r2
    itos r4
    say r4

    load r2,8
    load r5,1.5
    bsetf32 r1,r2,r5
    bgetf32 r6,r1,r2
    ftos r6
    say r6

    load r2,0
    bgetu32 r7,0x78563412,r2
    itos r7
    say r7
    ret
```

<!-- rxas-output for="binary-fixed-width" -->
```text
-42
1.5
305419896
```

### `binary-text-fields`

Binary text fields are stored as UTF-8 bytes followed by a zero terminator.
`sget` is different: it extracts codepoints from a string constant, starting at
a byte offset.

<!-- rxas-example name="binary-text-fields" test="run" -->
```rxas
/* Binary text-field and string constant extraction example */
main() .locals=5
    load r1,0x
    load r2,16
    bresize r1,r2

    load r2,0
    bsets r1,r2,"index"
    bgets r3,r1,r2
    say r3

    load r4,"xxxxx"
    load r2,6
    sget r4,"hello world",r2
    say r4
    ret
```

<!-- rxas-output for="binary-text-fields" -->
```text
index
world
```

### `binary-move`

`bmove` copies between different registers using offsets stored in the integer
slots of the source and destination registers. `bmemmove` handles overlapping
copies inside one binary register.

<!-- rxas-example name="binary-move" test="run" -->
```rxas
/* Binary memory move example */
main() .locals=6
    load r1,0x001122334455
    load r2,0xaabbccdd
    load r1,1
    load r2,2
    load r3,2
    bmove r1,r2,r3
    load r5,1
    getbyte r4,r1,r5
    itos r4
    say r4

    load r1,0
    load r2,2
    load r3,4
    bmemmove r1,r2,r3
    load r5,2
    getbyte r4,r1,r5
    itos r4
    say r4
    ret
```

<!-- rxas-output for="binary-move" -->
```text
204
0
```

### `binary-compare`

Zero-copy compares use `rCmp` as both the input source offset and the output
compare result.

<!-- rxas-example name="binary-compare" test="run" -->
```rxas
/* Binary zero-copy compare example */
main() .locals=4
    load r1,0x6162630061626400
    load r2,0x616263

    load r3,0
    bcmpb r3,r1,r2
    itos r3
    say r3

    load r3,4
    bcmpb r3,r1,0x616263
    itos r3
    say r3

    load r3,0
    bcmps r3,r1,"abc"
    itos r3
    say r3

    load r3,4
    bcmps r3,r1,"abe"
    itos r3
    say r3
    ret
```

<!-- rxas-output for="binary-compare" -->
```text
0
1
0
-1
```

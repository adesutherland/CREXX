# Binary Memory Instructions

This section is the Release 1 reference for RXAS instructions that treat a
`.binary` value or `BINARY_CONST` as byte-addressable memory. All offsets and
lengths are zero-based byte counts unless explicitly stated otherwise. Strict
binary-memory instructions raise `OUT_OF_RANGE` for negative offsets, negative
lengths, ranges outside the logical binary length, or integer values that cannot
be represented in the target field width.

The instruction database source and focused binary-memory operand diagrams are
refreshed from the current `rxops.h`-backed assembler instruction dump. Until
the full VM specification instruction chapter is regenerated through the legacy
documentation pipeline, this section is the authoritative binary-memory
instruction reference.

## Constants and Aliases

Binary constants are written with byte-paired hexadecimal syntax, for example
`0x001122ff`. They are stored as `BINARY_CONST` constant-pool records with an
explicit byte length. The assembler also stores a safety zero byte after the
logical payload, but that byte is not part of the binary value.

Release 1 also accepts source-level aliases for large constants:

```rxas
.const table binary 0x0011223344556677
.const key string "index"
```

A constant alias is an assembler name for a constant-pool value. It is accepted
anywhere the corresponding inline binary or string literal is accepted, and it
does not create a register or runtime copy.

## `load`

`load rDst,0x...` materializes a whole binary constant into the binary slot of
`rDst`. It is useful for small values and tests, but large lookup tables should
be read directly through the constant-source forms below.

## `blen`

`blen rOut,rBin` stores the logical byte length of a binary register in `rOut`.
`blen rOut,bConst` does the same for a binary constant without materializing the
constant in a register.

## `bresize`, `bclear`, and `bfill`

`bresize rBin,rLen` changes the logical binary length. Existing bytes are
preserved up to the new length and growth is zero-filled. Negative lengths raise
`OUT_OF_RANGE`.

`bclear rBin` sets the logical binary length to zero.

`bfill rBin,rByte` fills the current logical byte range with `rByte`. The byte
value must be in `0..255`.

## `bcopy`

`bcopy rDst,rSrc` copies the whole binary payload from one register to another.

`bcopy rDst,rSrc,rOffset` copies exactly `blen(rDst)` bytes from `rSrc` starting
at `rOffset` into `rDst`. `rSrc` may be a binary register or a binary constant.
The destination must already have the required length.

\lstinputlisting[language=rxas]{bcopy.rxas}

<!--splice--rxas bcopy -->
<!--splice--rxvme bcopy -->

## `getbyte` and `setbyte`

`getbyte rOut,rBin,rOffset` is the legacy tolerant byte read. It stores the byte
at `rOffset`, or `-1` when the offset is outside the logical binary length. It
has no binary-constant form; use `bgetu8` for strict constant byte reads.

`setbyte rBin,rOffset,rByte` writes one byte into mutable binary memory. The
offset must be in range and the byte value must be in `0..255`.

## Fixed-Width Reads

`bgetu8`, `bgeti8`, `bgetu16`, `bgeti16`, `bgetu32`, `bgeti32`, `bgeti64`,
`bgetf32`, and `bgetf64` read fixed-width fields from a binary register or
binary constant. Multi-byte values use little-endian storage. Signed integer
forms sign-extend into the destination integer register. Unsigned forms raise
`OUT_OF_RANGE` if the result cannot be represented by the active VM integer
type.

`.int` maps to signed 64-bit storage through `bgeti64`. `.f32` is IEEE binary32
storage and widens to the VM float register. `.float` maps to the current VM
float representation, which is binary64 for Release 1.

## Fixed-Width Writes

`bsetu8`, `bseti8`, `bsetu16`, `bseti16`, `bsetu32`, `bseti32`, `bseti64`,
`bsetf32`, and `bsetf64` write fixed-width fields into mutable binary memory.
They use the same little-endian storage widths as the matching read
instructions.

\lstinputlisting[language=rxas]{fixedwidth.rxas}

<!--splice--rxas fixedwidth >null -->
<!--splice--rxvme fixedwidth -->

## `bcheckrange`

`bcheckrange rBin,rOffset,rLen` checks that the byte range
`rOffset..rOffset+rLen` is inside the logical binary length. It does not mutate
any operand. It is an explicit assertion; target-sized copy, typed reads, typed
writes, text field instructions, and compare instructions still perform their
own range checks.

## `bconcat`, `bappend`, and `bupdate`

`bconcat rDst,rLeft,rRight` concatenates two binary registers into `rDst`.

`bappend rDst,rSrc` appends the binary payload of `rSrc` to `rDst`.

`bupdate rDst,rOffset,rSrc` overlays the whole binary payload of `rSrc` into
`rDst` at `rOffset`. The overlay must fit in the existing destination length.

These instructions operate on raw bytes and do not validate UTF-8.

## `setbinpos`, `getbinpos`, and `bslice`

These legacy cursor instructions remain available for cursor-style byte
processing. New Release 1 source lowering should prefer target-sized `bcopy`
for binary field extraction.

`setbinpos rBin,rOffset` sets the binary cursor after clamping the offset to
`0..blen(rBin)`.

`getbinpos rOut,rBin` reads the current binary cursor.

`bslice rDst,rSrc,rLen` copies up to `rLen` bytes from `rSrc` starting at its
cursor into `rDst`. It truncates at end of buffer; generated strict reads should
use `bcheckrange` or target-sized `bcopy` instead.

## `stobin` and `bintos`

`stobin rReg` copies the register's current string bytes exactly into its binary
slot. It does not append a NUL terminator.

`bintos rReg` validates the register's current binary bytes as UTF-8 and copies
them into its string slot. Invalid UTF-8 raises `UNICODE_ERROR`.

Whole-register conversion is separate from the zero-terminated text-field
instructions below.

## `bgets` and `bsets`

`bgets rString,rSrc,rOffset` reads a zero-terminated UTF-8 field from binary
memory. `rSrc` may be a binary register or binary constant. The instruction
scans from `rOffset` to the first zero byte, validates the bytes before the
terminator as UTF-8, copies those bytes to `rString`, and excludes the
terminator from the string value. Missing terminator raises `OUT_OF_RANGE`;
invalid UTF-8 raises `UNICODE_ERROR`.

`bsets rBin,rOffset,rString` writes the string's UTF-8 bytes followed by one
zero byte into mutable binary memory. `bsets rBin,rOffset,"literal"` writes a
string constant. The complete write, including the terminator, must fit in the
existing binary length.

## `sget`

`sget rString,sConst,rOffset` extracts from a `STRING_CONST`, not from binary
memory. `rOffset` is a byte offset into the UTF-8 string constant and must be a
codepoint boundary. The current codepoint length of `rString` is the requested
copy length. The VM scans that many codepoints, copies the exact bytes, sets the
destination string byte length and codepoint count, and writes the safety NUL
outside the logical value.

\lstinputlisting[language=rxas]{textfields.rxas}

<!--splice--rxas textfields >null -->
<!--splice--rxvme textfields -->

## `bmove` and `bmemmove`

`bmove rDst,rSrc,rLen` copies `rLen` bytes between different binary registers.
The destination byte offset is read from `rDst`'s integer slot; the source byte
offset is read from `rSrc`'s integer slot. The registers must be different.

`bmemmove rBin,rDstOffset,rLen` copies `rLen` bytes within one binary register.
The source byte offset is read from `rBin`'s integer slot and the destination
offset is the explicit `rDstOffset` operand. Overlapping ranges are safe and
behave like C `memmove`.

```rxas <!--binarymove.rxas-->

```
\lstinputlisting[language=rxas]{move.rxas}

<!--splice--rxas move -->
<!--splice--rxvme move -->

## `bcmpb` and `bcmps`

`bcmpb rCmp,rSrc,rNeedle` compares bytes from binary memory without copying.
On entry, `rCmp` contains the byte offset into `rSrc`. `rSrc` may be a binary
register or binary constant. `rNeedle` may be a binary register or binary
constant. The compare length is the whole length of `rNeedle`. On success,
`rCmp` is overwritten with `-1`, `0`, or `1` for unsigned-byte lexicographic
ordering.

`bcmps rCmp,rSrc,rString` compares a zero-terminated UTF-8 field in binary
memory with a string register or string constant. On entry, `rCmp` contains the
byte offset into `rSrc`; on success it is overwritten with `-1`, `0`, or `1`.
The source field is validated exactly as `bgets` would validate it, but no
temporary string is allocated.

If the caller needs the offset after a compare, it must copy the offset into a
scratch compare register first.

\lstinputlisting[language=rxas]{compare.rxas}

<!--splice--rxas compare -->
<!--splice--rxvme compare -->

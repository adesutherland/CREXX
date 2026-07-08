# Binary RXAS Surface Near Draft

Status: temporary joint working file.

This file is for agreeing the Release 1 RXAS surface for binary memory,
binary constants, and the small text convention needed when UTF-8 text is
stored inside binary memory. It is intentionally narrower than the Level B/G
source syntax notes. Unless stated otherwise, all binary offsets and lengths
are byte counts, zero-based, and strict forms raise `OUT_OF_RANGE` rather than
wrapping or clamping.

## Core Decisions

1. Binary registers and binary constants are byte-addressable memory spaces.
2. The same RXAS mnemonic may accept register or constant operands where the
   operation is the same. The assembler can select the concrete opcode/format
   from operand kinds.
3. Binary constants are read-only, but should have the same read and compare
   surface as binary registers where no mutable state is required.
4. Variable-length binary copy uses the destination binary length as the copy
   count. The program/compiler resizes the destination first, then copies from
   a byte offset in the source.
5. Variable-length compare uses a whole needle operand to define the compare
   length. There is no general "span to span" binary compare in Release 1.
6. Compare instructions use an in/out integer register: on entry it contains
   the byte offset into the binary source; on success it is overwritten with
   `-1`, `0`, or `1`.
7. Text inside binary memory has one Release 1 convention: zero-terminated
   UTF-8. String reads and string compares from binary memory scan from a byte
   offset to the first zero byte, validate UTF-8, and signal if the field is
   malformed.
8. The compiler may emit zero-copy compare instructions directly. RXAS
   keyhole optimisation can also shorten copy-and-compare patterns later, but
   that optimisation must prove the temporary and offset/result registers are
   safe to rewrite.
9. Large constants need RXAS source-level names/aliases so they do not have to
   be written inline at every instruction use.
10. Register-to-register binary memory move/copy can use the binary registers'
    integer slots as implicit offsets, leaving the explicit operands for the
    binary registers and length.
11. Same-register binary memory move needs a distinct instruction because one
    register cannot carry both source and destination offsets in its integer
    slot.

## Working Terms

- `rBin`: a register whose binary slot is used as mutable binary memory.
- `rSrc`: a binary source operand, either `rBin` or a `BINARY_CONST`.
- `bConst`: an RXAS binary constant operand, written as `0x...` and stored as a
  `BINARY_CONST`.
- `rString`: a register whose string slot is used as a UTF-8 Rexx string.
- `sConst`: an RXAS string constant operand, stored as a `STRING_CONST`.
- `rOffset`: integer register containing a byte offset.
- `rCmp`: integer register used as an in/out compare operand. On entry it holds
  the source byte offset; on success it holds the compare result.
- `rLen`: integer register containing a byte length for sizing operations.
- `rValue`: integer or float register containing the scalar value to write.
- `rDst.int_value` / `rSrc.int_value`: proposed implicit byte offsets for
  register-to-register binary memory move/copy. The VM value already has
  separate integer and binary storage fields, but the exact setter/emission
  shape must preserve binary state deliberately.
- `rDstOffset`: integer register containing a destination byte offset for
  same-buffer memory move.

Status labels:

- Existing: implemented RXAS/VM surface.
- Release 1: agreed Release 1 surface, not necessarily implemented yet.
- Candidate: plausible later direct instruction, not yet agreed for Release 1.
- Deferred: not required for the immediate Release 1 binary-memory surface.
- N/A: not applicable, usually because constants are read-only.

## Constant And String Length Facts

RXBIN constants use `string_constant` for `STRING_CONST`, `BINARY_CONST`, and
`DECIMAL_CONST`. That record has an explicit `string_len` field. The assembler
also writes a trailing zero byte after the payload for safety and C helper
convenience, but that byte is not the logical length and must not be treated as
the definition of a binary or string field.

Consequences:

- Binary constants can contain zero bytes inside their logical payload.
- String constants are UTF-8 validated by RXAS and have a safety NUL after the
  explicit byte length.
- Runtime `.string` values carry an explicit byte length. In UTF builds they
  also carry a codepoint count.
- If a value is intended to be a string constant, it should be represented as a
  `STRING_CONST`, not as a binary constant containing UTF-8 bytes.
- If a value is intended to be raw bytes, it should be represented as a
  `BINARY_CONST` and compared with binary compare operations.

## Floating-Point Storage Widths

Binary memory must use explicit storage widths, not host C type names. The
current VM float register and `FLOAT_CONST` storage use C `double`, and current
`bgetf64`/`bsetf64` require `sizeof(double) == 8` at runtime. That is a useful
guard, but the binary memory format should be defined independently:

- `.f32` means IEEE binary32, 4 bytes, little-endian.
- `.f64` means IEEE binary64, 8 bytes, little-endian.
- `.float` in cREXX source maps to the current runtime float value, which is
  presently `double`; therefore `.float` should alias `.f64` unless the core
  runtime float contract changes.
- `.f32` is in Release 1 scope. Reads widen from binary32 to the runtime float
  register and writes narrow from the runtime float register to binary32.
- Binary32 has no UTF-like "invalid byte sequence" state: all 32-bit payloads
  are interpreted as IEEE binary32 encodings, including infinities and NaNs.
  Signalling is about unsupported platform conversion or floating-point
  exceptions during conversion, not about arbitrary bytes being invalid.
- `long double` must not be used as the implicit binary memory format. The
  existing decimal plugin notes already show `long double` varies by OS,
  compiler, and architecture.

Release 1 should add platform/build coverage that records `sizeof(float)`,
`sizeof(double)`, and `sizeof(long double)` on macOS, Linux, Windows, x86_64,
and ARM64. If any supported platform lacks 64-bit IEEE `double`, `.f64`
support must either be disabled with a clear `FAILURE` or implemented with a
portable conversion path.

## Binary/String Conversion Contract

Whole-register conversion remains length-preserving:

- `.string` to `.binary` copies exactly `string_length` bytes into the binary
  payload. It must not append a NUL terminator to the binary result.
- `.binary` to `.string` validates exactly `binary_length` bytes as UTF-8, sets
  the destination `string_length` to that byte count, copies exactly those
  bytes, and should ensure a safety NUL exists immediately after the logical
  string for C helper compatibility.

Text-in-binary operations are separate from whole-register conversion. When a
string is written into binary memory as a field, Release 1 writes the string's
UTF-8 bytes followed by one zero terminator. When a string is read from binary
memory, Release 1 scans from a byte offset to the first zero terminator and
validates the bytes before the terminator as UTF-8.

This gives binary memory one clear text convention:

- The terminator is stored in the binary memory but is not part of the Rexx
  string value.
- Missing terminator before the end of the binary source raises
  `OUT_OF_RANGE`.
- Invalid UTF-8 before the terminator raises `UNICODE_ERROR`.
- UTF-16, UTF-32, and other encodings should be copied as `.binary` and
  decoded or converted in Rexx/library land.

## Extraction And Copy Model

Fixed-size scalar extraction is straightforward because the storage width is
known:

| Requested value | Length source | Likely lowering |
| --- | --- | --- |
| `.u8` / `.i8` | 1 byte | `bgetu8` / `bgeti8` |
| `.u16` / `.i16` | 2 bytes | `bgetu16` / `bgeti16` |
| `.u32` / `.i32` | 4 bytes | `bgetu32` / `bgeti32` |
| `.int` / `.i64` | Release 1 signed 64-bit | `bgeti64`; cleanup to 64-bit `.int` is in scope now |
| `.f32` | 4 bytes for IEEE binary32 | `bgetf32`; widens to runtime float register |
| `.f64` / `.float` | 8 bytes for IEEE binary64 | `bgetf64`; `.float` aliases `.f64` while runtime float is `double` |

Release 1 should clean up `.int` as a signed 64-bit contract now. Related
32-bit-host issues are deferred to 32-bit validation; they should not block the
Release 1 64-bit `.int` language contract on supported primary hosts.

Variable-length binary copy does not need a slice instruction. The destination
length defines the number of bytes to copy:

```rxas
bresize rDst,rLen
bcopy   rDst,rSrc,rOffset
```

`bcopy rDst,rSrc,rOffset` copies exactly `blen(rDst)` bytes from `rSrc` at
byte offset `rOffset` into `rDst`. If the source range does not fit, it raises
`OUT_OF_RANGE`. `rSrc` may be a binary register or a binary constant. This is
not the in-place memory-move operation; generated extraction code should use a
separate destination register. Same-register use is only coherent for trivial
cases such as offset zero and should not be used to express overlapping moves.

This target-sized rule also covers arbitrary variable-length binary fields:
the program must know the field length from the data format, a prefix/header,
an index, or a prior probe, resize the destination to that length, then copy.
The resize is work the program already needs, so the performance cost is
limited and predictable.

## Text Fields In Binary Memory

Release 1 does not need codepoint-counted string extraction from binary memory.
Text fields in binary memory are zero-terminated UTF-8.

```rxas
bgets rString,rSrc,rOffset
bsets rBin,rOffset,rString
```

`bgets rString,rSrc,rOffset` scans `rSrc` from byte offset `rOffset` to the
first zero byte, validates the bytes before the terminator as UTF-8, copies the
bytes into `rString`, sets the string byte length and codepoint count, and
writes the safety NUL outside the Rexx string value. `rSrc` may be a binary
register or binary constant.

`bsets rBin,rOffset,rString` writes the string's UTF-8 bytes followed by one
zero byte into mutable binary memory. The whole write must fit in the existing
binary length or it raises `OUT_OF_RANGE`. A string constant operand variant is
also coherent:

```rxas
bsets rBin,rOffset,"literal"
```

This keeps binary memory binary. A Rexx string buffer is not treated as another
mutable binary memory space; it is only a source or destination for explicit
text field operations.

## Binary Memory Move

Target-sized `bcopy` handles extraction into a destination binary register. It
does not cover moving bytes between two offsets in existing binary memory.

For different source and destination registers, the preferred three-operand
shape is to store the offsets in the binary registers' integer slots:

```rxas
; rDst.int_value is destination byte offset
; rSrc.int_value is source byte offset
bmove rDst,rSrc,rLen
```

`bmove rDst,rSrc,rLen` copies `rLen` bytes from `rSrc` at `rSrc.int_value` to
`rDst` at `rDst.int_value`. `rDst` and `rSrc` must be different registers. Both
source and destination ranges are strict; invalid offsets, negative lengths, or
ranges beyond the logical binary lengths raise `OUT_OF_RANGE`.

Same-register `memmove` needs a separate instruction because the same value
cannot hold two implicit integer offsets. The coherent shape is to make the
destination offset explicit, matching the existing mutation family
`setbyte rBin,rOffset,rByte`, `bset* rBin,rOffset,rValue`,
`bupdate rDst,rOffset,rSrc`, and `bsets rBin,rOffset,rString`:

```rxas
; rBin.int_value is source byte offset
; rDstOffset is destination byte offset
bmemmove rBin,rDstOffset,rLen
```

`bmemmove rBin,rDstOffset,rLen` copies `rLen` bytes within `rBin` from
`rBin.int_value` to `rDstOffset`. Overlapping ranges are safe and behave like C
`memmove`. Source and destination ranges are strict; invalid offsets, negative
lengths, or ranges beyond the logical binary length raise `OUT_OF_RANGE`.

This design intentionally applies only to binary registers. Constants cannot
carry an integer offset, so constant-source extraction continues to use the
target-sized `bcopy rDst,bConst,rOffset` form, and constant overlay can use a
separate constant-source `bupdate` form if needed.

The compiler sets the implicit offset with normal integer assignment/load into
the binary register's integer slot. There is no separate binary-offset setter
in Release 1. That assignment path must preserve the binary payload; it may
clear transient span/cursor state as needed.

## Compare Model

The Release 1 compare instructions are zero-copy by construction. They read
from binary memory at an input byte offset and overwrite that same register
with a three-way compare result.

```rxas
load  rCmp,rOffset
bcmpb rCmp,rSrc,rNeedle

load  rCmp,rOffset
bcmps rCmp,rSrc,rString
```

`bcmpb rCmp,rSrc,rNeedle` compares bytes from `rSrc` starting at the byte offset
initially held in `rCmp` against the whole binary needle. The compare length is
`blen(rNeedle)`. The needle may be a binary register or binary constant. The
source may be a binary register or binary constant. The source range must fit
or the instruction raises `OUT_OF_RANGE`.

There is intentionally no general binary-memory-span to binary-memory-span
compare in Release 1. One operand is the memory source plus offset; the other is
a whole binary value whose length defines the compare. This is enough for
fixed-length keys, constant keys, and variable-length keys where the caller has
already established or checked the relevant length convention.

`bcmps rCmp,rSrc,rString` compares a zero-terminated UTF-8 field in `rSrc`
against the whole string needle. The source field starts at the byte offset
initially held in `rCmp`. The needle may be a string register or string
constant. The source may be a binary register or binary constant.

String compare from binary memory must preserve string-read semantics even
though it does not materialize a temporary string:

- It scans to the first zero terminator.
- Missing terminator raises `OUT_OF_RANGE`.
- Invalid UTF-8 before the terminator raises `UNICODE_ERROR`.
- It compares the field content, excluding the terminator, with the whole
  string needle using the same ordering as materialized string comparison.

If the caller needs the offset value after the compare, the compiler must copy
the offset into a scratch `rCmp` first. After a successful compare, the input
offset is gone.

## Common Mnemonics With Operand Variants

Prefer one mnemonic where the operation is the same and operand kinds differ.
Examples:

```rxas
blen   rOut,rBin
blen   rOut,0x...

bcopy  rDst,rBin,rOffset
bcopy  rDst,0x...,rOffset

bgetu16 rOut,rBin,rOffset
bgetu16 rOut,0x...,rOffset

bcmpb  rCmp,rBin,rNeedle
bcmpb  rCmp,0x...,rNeedle
bcmpb  rCmp,rBin,0x...

bcmps  rCmp,rBin,rString
bcmps  rCmp,rBin,"literal"
bcmps  rCmp,0x...,rString
```

This avoids a separate family of constant-specific names such as `bcget`,
`bcslice`, or `bccmp` unless assembler/runtime constraints force them.

## Constant Aliases

RXAS should support source-level names for constants so large binary or string
payloads can be declared once and reused by name:

```rxas
.const table binary 0x001122...
.const key string "index"

bcopy rDst,table,rOffset
bcmpb rCmp,page,table
bcmps rCmp,page,key
```

A constant alias is an assembler name for a constant-pool value, not a register
and not a runtime copy. It should be accepted anywhere the corresponding inline
literal would be accepted. The assembler should continue to deduplicate the
underlying constant values, and `rxdas` may choose aliases for long constants
to keep generated RXAS readable.

## Read Surface

| Operation | Binary register source | Binary constant source | Release 1 position |
| --- | --- | --- | --- |
| Copy whole binary to register | Existing: `bcopy rDst,rSrc` | Existing: `load rDst,bConst` | Keep both. |
| Byte length | Existing: `blen rOut,rBin` | Release 1: `blen rOut,bConst` | Required to avoid materializing large constants just to ask their size. |
| Target-sized copy from offset | Release 1: `bcopy rDst,rBin,rOffset` | Release 1: `bcopy rDst,bConst,rOffset` | Required. Copy count is `blen(rDst)`. |
| Read one byte, tolerant | Existing: `getbyte rOut,rBin,rOffset` | Deferred: no constant form | Keep as legacy/register-only compatibility. Constant byte reads use strict `bgetu8`. |
| Read strict unsigned byte | Existing: `bgetu8 rOut,rBin,rOffset` | Release 1: `bgetu8 rOut,bConst,rOffset` | The single byte-read surface across registers and constants. |
| Read strict signed byte | Existing: `bgeti8 rOut,rBin,rOffset` | Release 1: `bgeti8 rOut,bConst,rOffset` | Full fixed-width constant read set. |
| Read strict unsigned 16-bit | Existing: `bgetu16 rOut,rBin,rOffset` | Release 1: `bgetu16 rOut,bConst,rOffset` | Full fixed-width constant read set. |
| Read strict signed 16-bit | Existing: `bgeti16 rOut,rBin,rOffset` | Release 1: `bgeti16 rOut,bConst,rOffset` | Full fixed-width constant read set. |
| Read strict unsigned 32-bit | Existing: `bgetu32 rOut,rBin,rOffset` | Release 1: `bgetu32 rOut,bConst,rOffset` | Full fixed-width constant read set. |
| Read strict signed 32-bit | Existing: `bgeti32 rOut,rBin,rOffset` | Release 1: `bgeti32 rOut,bConst,rOffset` | Full fixed-width constant read set. |
| Read strict signed 64-bit | Release 1: `bgeti64 rOut,rBin,rOffset` | Release 1: `bgeti64 rOut,bConst,rOffset` | Required by the Release 1 signed 64-bit `.int` cleanup. |
| Read binary32 float | Release 1: `bgetf32 rOut,rBin,rOffset` | Release 1: `bgetf32 rOut,bConst,rOffset` | Explicit-width binary32 support. |
| Read binary64 float | Existing: `bgetf64 rOut,rBin,rOffset` | Release 1: `bgetf64 rOut,bConst,rOffset` | Full fixed-width constant read set. |
| Read zero-terminated UTF-8 string | Release 1: `bgets rString,rBin,rOffset` | Release 1: `bgets rString,bConst,rOffset` | Required for text fields in binary memory. |

## Write And Mutation Surface

| Operation | Mutable binary register | Binary constant destination | Binary/string constant as source |
| --- | --- | --- | --- |
| Resize | Existing: `bresize rBin,rLen` | N/A | N/A |
| Clear length | Existing: `bclear rBin` | N/A | N/A |
| Fill current length | Existing: `bfill rBin,rByte` | N/A | N/A |
| Set one byte | Existing: `setbyte rBin,rOffset,rByte` | N/A | N/A |
| Set typed integer/float field | Existing: `bsetu8`, `bseti8`, `bsetu16`, `bseti16`, `bsetu32`, `bseti32`, `bsetf64`; Release 1: `bseti64`, `bsetf32` | N/A | N/A |
| Overlay binary source at offset | Existing: `bupdate rDst,rOffset,rSrc` | N/A | Candidate: `bupdate rDst,rOffset,bConst` |
| Append binary source | Existing: `bappend rDst,rSrc` | N/A | Candidate: `bappend rDst,bConst` |
| Concatenate two binaries | Existing: `bconcat rDst,rLeft,rRight` | N/A | Candidate direct constant operand variants if measured useful |
| Write zero-terminated UTF-8 string | Release 1: `bsets rBin,rOffset,rString` | N/A | Release 1: `bsets rBin,rOffset,sConst` |
| Register-to-register memory move | Release 1: `bmove rDst,rSrc,rLen` using integer slots as offsets; different registers only | N/A | N/A |
| Same-register memory move | Release 1: `bmemmove rBin,rDstOffset,rLen`, source offset in `rBin.int_value` | N/A | N/A |

Register-to-register memory move covers moving between two binary values.
Same-register memory move is separate because it needs two offsets for one
binary value. The explicit offset is the destination offset because binary
mutation instructions consistently put the destination and destination offset
first.

## Compare Surface

| Operation | Binary register source | Binary constant source | Needle operand | Release 1 position |
| --- | --- | --- | --- | --- |
| Compare binary bytes | Release 1: `bcmpb rCmp,rBin,rNeedle` | Release 1: `bcmpb rCmp,bConst,rNeedle` | Binary register or `BINARY_CONST`; length is needle length | Replace current selected-span semantics before release. `rCmp` is input offset then output result. |
| Compare zero-terminated UTF-8 field to string | Release 1: `bcmps rCmp,rBin,rString` | Release 1: `bcmps rCmp,bConst,rString` | String register or `STRING_CONST`; source field length comes from NUL scan | Replace current selected-span semantics before release. Must validate UTF-8 and preserve string-read signal behavior. |
| Compare byte to zero | Existing lowering: `bgetu8` plus immediate compare/branch | Candidate direct form later | Integer immediate zero | Deferred until measurement. |
| Compare typed field to immediate | Existing lowering: `bget*` plus integer/float compare | Candidate direct form later | Integer/float immediate | Deferred until measurement. |
| Direct compare-and-branch | Existing lowering: compare result then branch | Candidate direct form later | Binary or string needle | Deferred until profiles justify it. |

## Compiler Emission And Keyhole Targets

The source surface should not depend on keyhole optimisation for correctness or
baseline performance. For zero-copy binary/string compares, the compiler should
be allowed to emit the direct compare forms immediately:

```rxas
load  rCmp,rOffset
bcmpb rCmp,rSrc,rNeedle
beq   found,rCmp,0
```

```rxas
load  rCmp,rOffset
bcmps rCmp,rSrc,"literal"
beq   found,rCmp,0
```

This is simpler and safer than requiring RXAS to recognise every
copy-and-compare pattern. The keyhole optimiser can still shorten canonical
fallbacks such as:

```rxas
bresize rTmp,rLen
bcopy   rTmp,rSrc,rOffset
; ordinary whole-binary or whole-string compare using rTmp
```

Such a rule must prove:

- the temporary copy is used only by the compare;
- the temporary is dead after the compare;
- the offset register is not needed after the compare, or the rule creates a
  scratch `rCmp`;
- the replacement preserves `OUT_OF_RANGE` and `UNICODE_ERROR` signalling;
- the replacement preserves binary-vs-string semantics.

This makes RXAS optimisation an opportunity rather than a semantic
requirement.

## Adjacent String Constant Extraction

String constants are not binary memory, but there is still a useful adjacent
operation for large text constants indexed by binary tables. This is in Release
1 scope and follows the same target-sized shape:

```rxas
sget rString,sConst,rOffset
```

`rOffset` is a byte offset into the logical UTF-8 `STRING_CONST` payload. The
copy count is the current target string codepoint length, so the caller must
size `rString` to the requested number of codepoints before the extract. The VM
checks that `rOffset` is a UTF-8 codepoint boundary, scans that many codepoints,
derives the byte count, copies the bytes, sets `string_length` and
`string_chars`, and writes the safety NUL outside the logical value.

This is not required for binary memory itself. It is for the separate use case
where a binary index table stores byte offsets into a large UTF-8 string
constant.

## Documentation Scope And Pipeline

The Release 1 binary-memory instructions are nuanced enough that the
documentation should be written before, or alongside, implementation. The
current docs are useful but incomplete:

- `docs/ai-context/RXAS_ASSEMBLER.md` and `docs/ai-context/RXVM_INTERPRETER.md`
  describe implementation architecture and the current old cursor/span model.
  They are not a per-instruction reference.
- The VM specification instruction chapter is generated by
  `docs/books/crexx_vm_spec/instruction_doc.rexx` from
  `docs/instructions/instructionbase.sqb`.
- That generator emits one section per mnemonic, includes an operand diagram
  from `docs/books/crexx_vm_spec/svg/<mnemonic>.gv`, pulls optional operation
  prose from `docs/books/crexx_vm_spec/operation/<mnemonic>.operation`, and
  includes/runs examples named
  `docs/books/crexx_vm_spec/examples/<mnemonic><operands>.rxas`.
- The example pipeline assembles and runs examples through LaTeX `\splice`, so
  failures should break the book build. It is not, by itself, an expected-output
  assertion framework. Every example-worthy behavior also needs normal focused
  tests with expected output or expected signal handling.
- The current instruction database is stale for binary memory. It contains
  `getbyte` but not the newer `blen`, `bcopy`, typed `bget*`/`bset*`,
  cursor/slice, span compare, or resize/fill instructions that exist in
  `binutils/include/rxops.h`.

Documentation gate for every new or changed binary instruction:

1. Add or update the instruction database source and regenerated
   `instructionbase.sqb` entry with mnemonic, operands, category, and concise
   opcode description.
2. Add the mnemonic to a dedicated category, preferably `Binary Memory`, rather
   than leaving these under generic string instructions.
3. Add `operation/<mnemonic>.operation` prose covering:
   operand forms, constant vs register behavior, offset and length source,
   result side effects, payload/cursor/span invalidation, exact signals, endian
   format, UTF-8/NUL rules, and zero-copy guarantees where relevant.
4. Add at least one runnable `.rxas` example per public operand shape that needs
   reader-visible clarification. For simple fixed-width families, one or two
   representative examples can document the shared rule, with the full matrix
   covered by tests.
5. Add normal CTest/interpreter/compiler tests for success and negative cases.
   Negative behavior should not rely on PDF examples alone.
6. Regenerate the instruction SVGs and `instruction_chapter.tex`, then build the
   VM specification PDF as a documentation integration check.
7. Synchronise the implementer docs:
   `docs/ai-context/RXAS_ASSEMBLER.md`,
   `docs/ai-context/RXVM_INTERPRETER.md`,
   `docs/ai-context/CREXX_ARCHITECTURE.md`, and the Release 1 plan.

Minimum example/test coverage for the new binary-memory subsection:

- `bcopy` target-sized extraction from a binary register and from a binary
  constant; source out of range.
- `blen` on a register and a large binary constant, without materialising the
  constant.
- Strict fixed-width reads from register and constant sources, including
  endian-visible bytes and out-of-range reads.
- Fixed-width writes, including overflow/bad byte and out-of-range writes.
- `bgeti64`/`bseti64` signed round trip and sign extension boundaries.
- `bgetf32`/`bsetf32` and `bgetf64`/`bsetf64` round trips, with build/platform
  guards documented.
- `bgets`/`bsets` valid zero-terminated UTF-8, missing terminator, invalid
  UTF-8, and string constant write source.
- `sget` byte-offset plus codepoint-count extraction from a string constant,
  including non-ASCII UTF-8 boundary checking.
- `bcmpb` equality, less/greater, constant source, constant needle, and
  out-of-range source range.
- `bcmps` equality, less/greater, string constant needle, binary constant
  source, missing terminator, and invalid UTF-8.
- `bmove` different-register move with independent implicit offsets.
- `bmemmove` same-register overlapping moves in both directions.
- `.const name binary ...` and `.const name string ...` alias use, duplicate or
  undefined alias errors, and disassembler/readability behavior if `rxdas`
  emits aliases.

## Impacted RXAS Instruction Schedule

This schedule is for implementation planning. "Process" means update opcode
tables, assembler operand recognition, VM execution, disassembly, optimiser
metadata, tests, and generated docs as applicable.

| Instruction or form | Current state | Release 1 action |
| --- | --- | --- |
| `load rDst,0x...` | Existing whole binary constant load, direct `BINARY_CONST` support | Keep and document as the whole-constant materialisation form, not as the large-constant lookup path. |
| `.const name binary ...` / `.const name string ...` | Not currently documented/implemented for this surface | Add as RXAS source aliases for constants; no runtime copy. |
| `blen rOut,rBin` | Existing register length | Keep and add `blen rOut,bConst`. |
| `bresize rBin,rLen` | Existing | Keep; document strict negative-length signal and zero-fill growth. |
| `bclear rBin` | Existing | Keep; document length/cursor/span invalidation. |
| `bfill rBin,rByte` | Existing | Keep; document byte range `0..255` and no resizing. |
| `bcopy rDst,rSrc` | Existing whole binary register payload copy | Keep as whole-register binary payload copy. |
| `bcopy rDst,rSrc,rOffset` | New shape | Add target-sized copy from register or `BINARY_CONST`; copy count is `blen(rDst)`. |
| `getbyte rOut,rBin,rOffset` | Existing tolerant byte read returning `-1` out of range | Keep register-only for compatibility; do not add constant form. Document strict reads as preferred for binary constants. |
| `setbyte rBin,rOffset,rByte` | Existing strict byte write | Keep. |
| `bgetu8` / `bgeti8` / `bgetu16` / `bgeti16` / `bgetu32` / `bgeti32` / `bgetf64` from `rBin` | Existing strict fixed-width reads | Keep, document little-endian storage and strict range/value behavior. |
| Same `bget*` from `bConst` | New constant-source forms | Add the full fixed-width constant read set. |
| `bgeti64 rOut,rSrc,rOffset` | New | Add for signed 64-bit `.int` cleanup, with register and constant source forms. |
| `bgetf32 rOut,rSrc,rOffset` | New | Add for explicit IEEE binary32 storage, with register and constant source forms. |
| `bsetu8` / `bseti8` / `bsetu16` / `bseti16` / `bsetu32` / `bseti32` / `bsetf64` | Existing strict fixed-width writes | Keep. |
| `bseti64 rBin,rOffset,rValue` | New | Add for signed 64-bit `.int` cleanup. |
| `bsetf32 rBin,rOffset,rFloat` | New | Add for explicit IEEE binary32 storage. |
| `bgets rString,rSrc,rOffset` | New | Add zero-terminated UTF-8 field read from binary register or `BINARY_CONST`. |
| `bsets rBin,rOffset,rString` / `bsets rBin,rOffset,sConst` | New | Add zero-terminated UTF-8 field write into mutable binary memory. |
| `sget rString,sConst,rOffset` | New adjacent string-constant operation | Add codepoint-counted extraction starting at byte offset into `STRING_CONST`. |
| `bupdate rDst,rOffset,rSrc` | Existing register-source overlay | Keep register form; add `BINARY_CONST` source only if implementation cost is small or compiler needs it. |
| `bappend rDst,rSrc` | Existing register-source append | Keep register form; constant source remains candidate, not required for lookup use cases. |
| `bconcat rDst,rLeft,rRight` | Existing register-source concat | Keep register form; constant operand variants are deferred until measured useful. |
| `stobin rReg` | Existing whole-register string-to-binary conversion | Keep; document that it copies exact string bytes and does not add a NUL. |
| `bintos rReg` | Existing whole-register binary-to-string conversion | Keep; document exact byte validation and safety NUL outside logical string length. |
| `bmove rDst,rSrc,rLen` | New | Add different-register binary memory move; offsets come from the integer slots of `rDst` and `rSrc`. |
| `bmemmove rBin,rDstOffset,rLen` | New | Add same-register overlapping move; source offset comes from `rBin.int_value`, destination offset is explicit. |
| `bcmpb rCmp,rSrc,rNeedle` | Existing mnemonic with old selected-span semantics | Replace semantics before Release 1. New form uses `rCmp` as input source offset and output `-1/0/1`, with register/constant source and register/constant binary needle. |
| `bcmps rCmp,rSrc,rString` | Existing mnemonic with old selected-span semantics | Replace semantics before Release 1. New form compares zero-terminated UTF-8 field to string register or `STRING_CONST`, preserving UTF-8 and missing-terminator signals. |
| `setbinspan rBin,rOffset,rLen` | Existing old selected-span setup | Remove from the Release 1 public binary-memory surface, or leave only as rejected/deprecated compatibility if opcode compatibility requires it. Compiler emission and tests should stop using it. |
| `setbinpos rBin,rOffset` / `getbinpos rOut,rBin` / `bslice rDst,rSrc,rLen` | Existing cursor/slice surface | Remove from compiler-generated binary-memory extraction and the new Release 1 surface. If retained as legacy RXAS, document as non-preferred and keep out of Level B/G lowering. |
| `bcheckrange rBin,rOffset,rLen` | Existing strict preflight range check | Review during implementation. The new target-sized operations perform their own checks, so this should either be kept as a general explicit assertion with docs/tests or removed from the public Release 1 binary-memory surface. |

Current implementation points known to need processing:

- `binutils/include/rxops.h` has binary byte-buffer opcodes around
  `LOAD_REG_BINARY` through `BINTOS_REG` and typed/span opcodes around
  `BCOPY_REG_REG` through `BCMPS_REG_REG_REG`; both blocks need a coherent
  Release 1 update.
- `compiler/rxcp_emit_expr.c` currently emits the old
  `bcheckrange`/`getbinpos`/`setbinpos`/`bslice` sequence for binary extraction;
  that lowering must be replaced by target-sized `bcopy`, `bgets`, or `sget`
  as appropriate.
- `compiler/rxcp_util.c` currently maps binary storage types only through
  `u8`, `i8`, `u16`, `i16`, `u32`, `i32`, `f64`, and `float`; it needs
  `i64`/`.int` and `f32`.
- `interpreter/tests/tests_binary_memory.rxas` uses `bcheckrange`,
  `setbinspan`, and old `bcmpb`/`bcmps` semantics; rewrite it around the new
  compare model and add negative signal cases.
- `interpreter/tests/tests_binary.rxas` uses `setbinpos`, `getbinpos`, and
  `bslice`; either rewrite or isolate these as legacy-only tests if the opcodes
  are retained.
- Compiler golden checks currently search for `bcheckrange`, `getbinpos`,
  `bslice`, and `bintos`; update them when the lowering changes.
- The docs instruction database, instruction SVGs, VM spec
  `instruction_chapter.tex`, reference cards, and architecture notes are stale
  for the binary-memory surface and must be regenerated or edited as part of
  the implementation slice.

## Current Implemented Binary RXAS Inventory

Current register-based binary operations:

```rxas
load       rDst,0x...
bcopy      rDst,rSrc
blen       rOut,rBin
bresize    rBin,rLen
bclear     rBin
bfill      rBin,rByte
getbyte    rOut,rBin,rOffset
setbyte    rBin,rOffset,rByte
bgetu8     rOut,rBin,rOffset
bgeti8     rOut,rBin,rOffset
bgetu16    rOut,rBin,rOffset
bgeti16    rOut,rBin,rOffset
bgetu32    rOut,rBin,rOffset
bgeti32    rOut,rBin,rOffset
bgetf64    rOut,rBin,rOffset
bsetu8     rBin,rOffset,rValue
bseti8     rBin,rOffset,rValue
bsetu16    rBin,rOffset,rValue
bseti16    rBin,rOffset,rValue
bsetu32    rBin,rOffset,rValue
bseti32    rBin,rOffset,rValue
bsetf64    rBin,rOffset,rFloat
bcheckrange rBin,rOffset,rLen
setbinspan rBin,rOffset,rLen
bcmpb      rOut,rBin,rOtherBin
bcmps      rOut,rBin,rString
bconcat    rDst,rLeft,rRight
bappend    rDst,rRight
setbinpos  rBin,rOffset
getbinpos  rOut,rBin
bslice     rDst,rSrc,rLen
bupdate    rDst,rOffset,rSrc
stobin     rReg
bintos     rReg
```

Direct binary-constant support currently consists only of:

```rxas
load       rDst,0x...
```

Everything else involving a binary constant currently requires materializing
the constant into a register first, which is not acceptable for large lookup
constants.

## Settled Release 1 Decisions

1. Use `bcopy` for target-sized copy from register or constant sources.
2. Replace the current selected-span `bcmpb`/`bcmps` semantics with the new
   in/out offset-result compare design before Release 1.
3. Use `bgets`/`bsets` for zero-terminated UTF-8 text fields in binary memory.
4. Include adjacent string-constant extraction in Release 1.
5. Use `bmove rDst,rSrc,rLen` for different-register binary memory move and
   `bmemmove rBin,rDstOffset,rLen` for same-register memory move. Do not add a
   separate binary-offset setter; use normal integer assignment/load into the
   value's integer slot.
6. Provide the full fixed-width constant read surface, not only byte and
   u32/i32. This means the current fixed-width set plus the agreed
   `i64`/`.int` and `.f32` additions; `.u64` is not implied unless separately
   added to the Release 1 binary type set.
7. Do not add a constant form of tolerant `getbyte`; use strict `bgetu8` across
   registers and constants.
8. Do the signed 64-bit `.int` cleanup now, including `bgeti64`/`bseti64`.
   32-bit-host consequences are handled during later 32-bit validation.
9. Include `.f32`/binary32 support in Release 1. Binary32 storage is an IEEE
   encoding; all 32-bit payloads are readable encodings, with signalling only
   for unsupported platform conversion or floating-point conversion failures.
10. Use `.const name binary ...` and `.const name string ...` for RXAS constant
    aliases unless implementation discovers a grammar conflict.

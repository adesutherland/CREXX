# Level B/G Binary Memory Working Design

Status: Rexx surface design and gap tracker, not an approved language
specification. The Release 1 RXAS binary-memory instruction surface is now
baselined in `docs/reference/rxas/instructions/07-binary-memory.md`.

This note captures the proposed Release 1 direction for treating `.binary`
values and `.binary` constants as typed memory spaces for high-performance Rexx
data structures. It also records less-confirmed Release 2 possibilities.

The target workloads are lookup-heavy algorithms: indexes, balanced trees,
tries, lexer/parser tables, string interning, search tables, packed records,
and future mapped-memory structures.

The core direction is that searchable layout and indexing should be solved with
Rexx-level data structures backed by binary memory, not by adding searchable
indexes to register attributes. Register attributes should remain fixed
physical slots and typed VM views. Binary memory should become the fast
substrate for Rexx-owned lookup policy.

## Release Framing

Release 1 should deliver the small, coherent foundation:

- one shared Level B/G source model;
- one RXAS instruction family for strict typed binary memory access;
- the same Rexx syntax for reading `.binary` variables and `.binary` constants;
- no special constant-pass ABI;
- function/helper fallbacks for clarity and tests, but syntax or compiler
  intrinsics for hot paths;
- enough proof fixtures to build packed lookup structures in Rexx.

Release 2 is the design space for:

- richer Level B source convenience;
- richer Level G syntax;
- optional read-only constant-pass/view types;
- binary struct declarations;
- mmap/shared-memory views;
- any RXAS operand model changes that are too large for Release 1.

## Current Baseline

The current tree already has useful building blocks:

- `.binary` is a distinct Level B type for arbitrary bytes.
- RXAS supports byte-paired binary literals such as `0x0011ff`.
- Level B source hex/binary literals can become `.binary` values when explicitly
  targeted as binary data.
- VM values carry a binary payload, byte length, byte cursor, and growable
  buffer capacity.
- RXAS has a Release 1 binary-memory instruction family covering strict
  fixed-width reads/writes, binary constants, target-sized copies,
  zero-terminated UTF-8 text fields, string-constant extraction, binary memory
  moves, and zero-copy compares. The human reference is
  `docs/reference/rxas/instructions/07-binary-memory.md`.
- Binary constants use the normal constant-pool path.
- `.string` remains valid UTF-8 text in normal UTF builds; `.binary` remains
  arbitrary bytes.

The current `rxfnsb` `bin*` helper family exists, but it has not become a
settled public design surface for larger Rexx code. Release 1 may keep,
rename, supersede, or narrow those helpers while defining the binary-memory
surface. The key point is to avoid letting existing one-based BIF-style helper
shape block a zero-based memory model.

## Release 1 Level B/G Joint Source Model

Release 1 should use the same source model for Level B and Level G. The
language levels may differ later in how much syntax sugar they allow, but the
Release 1 binary-memory semantics should be shared.

### Principles

1. `.binary` is the memory value.
2. Offsets are zero-based byte offsets.
3. Lengths are byte counts unless explicitly stated otherwise.
4. Fixed-width layout types use dotted storage names. The current source and
   RXAS surface covers `.u8`, `.i8`, `.u16`, `.i16`, `.u32`, `.i32`, `.i64`,
   `.int`, `.f32`, `.f64`, and `.float`. `.u64` remains a later candidate.
5. `.binary` constants and `.binary` variables use the same read syntax.
6. Byte positions are explicit: `<at..type>(x) buffer` means byte offset `x`
   in `buffer`. The unit of the offset expression is always bytes.
7. Typed access uses `<at..type>(offset) buffer`, with
   `<at..type>(offset, length) buffer` when a variable-size span needs an
   explicit byte length.
8. `<sizeof..type>` is the Release 1 size operator for fixed storage types.
9. Constants are read-only. Write, resize, reserve, and source-level cursor
   syntax must reject constants.
10. Passing a constant to an ordinary procedure, method, or helper in Release 1
   materializes a register in the normal way. There is no special constant-pass
   type in Release 1.
11. Constants should be scopeable across procedures so layout constants and
   binary lookup tables can be named once and used directly by hot code.
12. Helper functions are acceptable fallbacks, but hot lookup loops should be
   compiler-lowered syntax or recognized intrinsics to avoid call overhead and
   avoid unnecessary binary/string copies.
13. Binary-memory operations must preserve the `.string` UTF-8 contract:
    arbitrary bytes become `.string` only through explicit checked conversion.
14. The model should use offsets rather than native pointers so a future mmap
    or shared-memory implementation remains possible.
15. Compare syntax must be zero-copy. It must
    compare the source memory span directly against the other operand and must
    not materialize a temporary substring, binary slice, or string value.
16. Typed memory access does not scale offset expressions. Layout constants such
    as `NODE_BALANCE` are byte offsets. The type controls interpretation and
    field width, not the unit of the offset.

### Constants And Scope

From the Rexx programmer's perspective, reading a `.binary` constant should feel
the same as reading a `.binary` variable:

```rexx
check_magic: procedure = .int
  constant MAGIC = "4352584944583031"x as .binary
  constant MAGIC_PREFIX = "43525849"x as .binary

  if <at..u32>(0) MAGIC = <at..u32>(0) MAGIC_PREFIX then return 1
  return 0
```

That same syntax should work for variables:

```rexx
check_page: procedure = .int
  arg page = .binary
  constant MAGIC = "4352584944583031"x as .binary

  if <at..u32>(0) page = <at..u32>(0) MAGIC then return 1
  return 0
```

The implementation may differ. In Release 1, ordinary calls materialize
constants into registers:

```rexx
if bmemcmp(page, 0, MAGIC) = 0 then say "magic"
```

That fallback call may copy or at least load a register for `MAGIC`. Release 1
should accept that simplification. Avoiding it requires a special read-only
constant parameter/view type, which belongs in Release 2 if it is needed at all.

To mitigate the cost, Release 1 should encourage scoped constants:

```rexx
namespace packedindex expose find_node insert_node NODE_LEFT NODE_RIGHT NODE_SIZE EMPTY_NODE

packedindex_layout: procedure expose NODE_LEFT NODE_RIGHT NODE_SIZE EMPTY_NODE
  constant NODE_LEFT = 0
  constant NODE_RIGHT = 4
  constant NODE_SIZE = 32
  constant EMPTY_NODE = "00000000000000000000000000000000"x as .binary
  return

find_node: procedure = .int
  arg arena = .binary, key = .string
  /* NODE_* constants are available directly in this procedure. */

insert_node: procedure = .int
  arg expose arena = .binary, key = .string
  /* Same constants are available here too; arena is by reference because it is mutated. */
```

Release 1 requirement: binary layout constants should be declared in an explicit
procedure scope and be available across procedures when their names are part of
the namespace surface. The existing `namespace ... expose ...` shape should be
repeated for constants: constants in the expose list are visible to importers
and to procedures in that namespace in the same way as other exposed symbols.
Cross-module constant export/import can follow the existing constant metadata
path if that path is settled enough; otherwise it should remain an explicit
follow-up.

Library-style modules should keep executable setup inside explicit procedures.
Top-level executable statements before a `procedure` synthesize an implicit
`main()` for compatibility, which is useful for scripts but is the wrong signal
for packed binary libraries. Do not present top-level `constant` declarations as
the packed-layout library idiom. File-body `constant` declarations are a
Release 1 design defect to remove; use an explicit procedure scope and the
namespace expose list for shared layout constants.

### Constants And Mutability

The same read syntax does not mean the same mutability:

```rexx
version = <at..u32>(8) HEADER          /* OK for constant or variable */
<at..u32>(8) HEADER = 2                /* error: constant is read-only */
```

For mutable storage:

```rexx
page = .binary
page = bmemnew(4096)
<at..u32>(8) page = 2
```

Release 1 should make read-only failures clear at compile time when the target
is visibly a constant. If the target is hidden behind a future read-only view or
mapped page, runtime failure can be considered later.

### Function Fallbacks

Function-style helpers are still useful:

```rexx
version = bmemu32(page, 8)
call bmemputu32 page, 8, version + 1
```

They are good for:

- prototypes;
- tests;
- non-hot code;
- places where source syntax is not settled yet;
- Level C or compatibility wrappers if needed later.

They are not enough for the main performance goal. A helper call may allocate a
call frame, copy arguments, materialize constants, and return through ordinary
Rexx value paths. The Release 1 performance path needs syntax or compiler-known
intrinsics that lower directly to RXAS.

## Release 1 Inner Storage Types

Release 1 should focus on the storage types that real lookup structures need.
The preferred spelling is dotted, matching the rest of the Level B type surface:

- `.u8` and `.i8` for bytes, flags, tiny signed state, and node color/balance.
- `.u16` and `.i16` for compact lengths, table states, and small indexes.
- `.u32` and `.i32` for offsets, lengths, indexes, hashes, and record links.
- `.i64` for signed 64-bit stored integers, with `.int` accepted as the
  standard cREXX convenience spelling for the same Release 1 storage width.
- `.f32` for dense binary32 float storage.
- `.f64` for fixed binary64 storage, with `.float` accepted as the standard
  cREXX convenience spelling.
- `.u64` remains a future candidate for large offsets, 64-bit hashes, external
  ids, dense data, or binary interchange.

Standard cREXX types should be accepted as conveniences only where the
operation has a clear storage contract:

- `.float` as the normal cREXX float view, expected to map to binary64 storage
  unless a future numeric option says otherwise.
- `.int` as the normal cREXX integer view, mapped to signed 64-bit storage for
  Release 1 binary memory. Use `.i32` or `.u32` when a 4-byte persisted field is
  intended.
- `.string` for UTF-8 text spans.
- `.decimal` for decimal text spans.
- `.binary` for byte spans.

The working bias is that fixed persisted layouts should use explicit storage
names such as `.u32`, `.i32`, `.i64`, `.f32`, and `.f64`; ergonomic standard
aliases are allowed only where persistence and cross-platform byte width are
unambiguous.

## Release 1 Defined Source Syntax

Release 1 uses explicit byte offsets plus a compiler-recognized parameterized
intrinsic operator:

```rexx
<at..type>(offset) memory
<at..type>(offset, length) memory
```

The `at` head names the intrinsic family. `..type` is the Release 1 compact
single-specialization form, chosen to keep the syntax compatible with possible
future generic callable syntax such as `search.[.int, .string](tree, key)`.
The storage type is the same concept as `.u32` or `.string`; in this compact
operator form the leading dot is represented by the `..` specialization
separator, so `<at..u32>(...)` means the `.u32` storage view.
The runtime arguments inside `(...)` are ordinary expressions parsed by the
normal expression grammar.

The `<...>` intrinsic head is intentionally not an expression language. In
Release 1 it is compact outside type-parameter lists: no whitespace is allowed
between `<`, the intrinsic name/path, and `>`. Whitespace is accepted only
inside the future generic type-list form, for example
`<at.[ .u32, .string ]>`. That form is parsed so the lexer/parser shape is
validated, but it reports a Release 1 unsupported diagnostic rather than
lowering.

`<sizeof..u32>` is the compact intrinsic-head spelling for the `.u32` byte
width.

### Byte Access

Byte access is ordinary typed access with `u8` or `i8` and an explicit byte
offset:

```rexx
byte = <at..u8>(6) buffer
<at..u8>(6) buffer = 255
```

The byte result is an `.int` in the range `0..255`. Writes signal
`OUT_OF_RANGE` if the offset is outside the current binary length or the value
is not a byte.

Bare `buffer[x]` is not binary byte access in Release 1. That spelling remains
normal array-style indexing, which matters for arrays of `.binary` values:
`buffers[3]` should naturally mean the third binary value, not byte 3 of one
binary buffer.

### Precedence

The parser treats `<at..type>(args) memory` as one primary expression. Offset
and length arithmetic lives inside the normal argument list:

```rexx
b = <at..u8>(1 + 1) data
```

This reads byte offset `2`. Arithmetic after the memory operand applies to the
loaded value:

```rexx
b = <at..u8>(1) data + 1
```

This keeps layout expressions such as `node + NODE_BALANCE` natural inside the
argument list while avoiding the old high-precedence paired-token ambiguity.

### Fixed-Width Typed Access

Fixed-width fields are read by specializing `at` with the storage type and
passing a byte offset:

```rexx
i = <at..i8>(6) buffer
n = <at..u32>(pos) buffer
f = (<at..i8>(6) buffer) as .float
balance = <at..i8>(node + NODE_BALANCE) page
```

The `as .float` in the example is ordinary cREXX cast syntax applied after the
`.i8` load. `at` selects the binary memory object; the first argument supplies
a zero-based byte offset; `as` remains a cast.

Writes use the same left-hand shape:

```rexx
<at..u32>(pos) buffer = next_offset
<at..i8>(node + NODE_BALANCE) page = balance
```

Fixed-width types know their byte width. No length is written for `.u8`, `.i8`,
`.u16`, `.i16`, `.u32`, `.i32`, `.i64`, `.int`, `.f32`, `.f64`, or the
`.float` convenience alias. `.u64` remains a future candidate.

### Variable-Size Spans

Variable-size memory views pass a second length argument. `.binary` and
`.decimal` use byte counts. `.string` uses a UTF-8 codepoint count from a
zero-based byte offset:

```rexx
b = <at..binary>(6, 10) buffer
s = <at..string>(6, 10) buffer
d = <at..decimal>(pos, decimal_len) buffer
```

When the expression is materialized as a normal Rexx value, `.string` validates
UTF-8 and `.decimal` parses decimal text. `.binary` copies the selected bytes.

Variable-size writes use the same conceptual shape, but are not currently
lowered. The compiler rejects them with a specific diagnostic:

```rexx
<at..binary>(pos, key_len) buffer = key_bytes
<at..string>(pos, key_len) buffer = key_text
<at..decimal>(pos, amount_len) buffer = amount
```

Later slices can add the write lowering and decide whether any exact-length
write may omit the length argument. Persistent variable-size fields should
spell the length when a span length matters.

### Size Operator

`<sizeof..type>` is a compile-time operator for fixed storage types:

```rexx
layout_size: procedure = .int
  constant NODE_BALANCE = 12
  constant NODE_SIZE = 3 * <sizeof..u32> + <sizeof..i8>
  return NODE_SIZE
```

The operator returns bytes. It does not imply typed-element indexing; offsets are
always byte offsets. `NODE_BALANCE` and similar layout constants should be byte
offsets.

### Zero-Copy Compare Intrinsics

Comparisons that must avoid copies should be explicit intrinsic function calls.
Do not overload ordinary `=` on `<at>` spans for this. If a program writes
`<at..binary>(pos, len) buffer = key`, the left side is an ordinary materialized
Rexx value before `=` compares it.

The no-copy compare contract is:

```rexx
if <compare..binary>(buffer, pos, key_bytes) = 0 then say "found"
if <compare..binary>(buffer, pos, key_len, key_bytes) = 0 then say "found"
if <compare..binary>(HEADER_CONST, 4, key_bytes) = 0 then say "found"
if <compare..string>(buffer, 6, "index") = 0 then say "found"
if <compare..u32>(buffer, NODE_HASH, wanted_hash) = 0 then say "found"
```

`<compare..binary>` compares bytes from binary memory with a binary needle. The
source memory and needle may each be a register-backed variable or a binary
constant. The compare length may come from the needle or from an explicit length
argument when the program is comparing a fixed prefix/range. `<compare..string>`
compares a NUL-terminated UTF-8 field in binary memory with a string variable or
string constant. Fixed-width forms such as `<compare..u32>` use the storage type
to determine the number of bytes and convert the ordinary Rexx value to the
field representation for comparison.

The source intrinsic is not constrained by RXAS's three-operand instruction
shape. The compiler can lower a clear four-argument source form by preparing the
compare register, selecting `bcmpb`/`bcmps`, and emitting any required setup
instructions. The parenthesized arguments are ordinary expressions outside the
intrinsic head, so the lexer/parser only has to recognize `<compare..type>` as a
head and then parse a normal argument list. Shorter aliases such as `<bcmp>`,
`<scmp>`, or flatter spellings such as `<compare_binary>` remain naming options,
but the coherent family is `<compare..type>(...)`.

Use ordinary `=` when the code really wants normal Rexx value comparison. Use a
compare intrinsic when the binary memory location itself is part of the
operation.

### Ordinary Functions

Ordinary functions remain useful for buffer management and mutation:

```rexx
call binresize(page, size)
call binmemmove(page, dst_pos, src_pos, len)
call bincopy(dst, dst_pos, src, src_pos, len)
```

These functions may still be direct-lowered by the compiler/inliner when the
call target and argument shape are known. They should not be used for direct
reads from binary constants or for zero-copy compares; use the intrinsic forms
for those.

### Deferred Source Syntax

Source-level cursor syntax is deferred to Release 2. Release 1 should stay
offset/span based. Earlier bracket-cursor forms such as `buffer[+]` and
statement-level cursors are removed from the Release 1 source design.

## Release 1 Data-Structure Guidance

Binary memory is not a replacement for all Rexx variables, arrays, or objects.
Users need guidance on when to stay with normal values and when to pack data.

Use normal variables/registers when:

- the value is live scalar state in a loop;
- the value is frequently changed and immediately used for arithmetic;
- the value needs normal Rexx numeric context, especially decimal arithmetic;
- the value participates in method dispatch or interface contracts;
- clarity matters more than dense layout.

Use binary memory when:

- there are many records of the same shape;
- offsets and lengths are enough to represent links;
- the data is searched more often than it is materialized;
- the layout may be serialized, cached, or mapped later;
- object/attribute indirection is a measured cost;
- compare operations can work directly against stored bytes;
- tables are generated at compile time as constants.

Use fixed-width integer fields for:

- offsets;
- lengths;
- indexes;
- flags;
- cached hashes;
- tree links;
- enum-like parser states.

Use string/decimal byte spans for:

- keys that are mostly compared, not modified;
- canonical decimal text used for sorting or persistence;
- text that should be materialized only at API boundaries.

Avoid packing when:

- decimal arithmetic happens in the hot path;
- string normalization, case folding, or Unicode segmentation is required;
- field updates constantly change variable-length payload sizes;
- debugging clarity is more important than memory density.

Decimal-specific guidance:

- Store decimals as canonical decimal text when persistence or comparison is
  the goal.
- Keep decimals as `.decimal` variables when calculating.
- Do not assume byte-order comparison of decimal text is numerically correct
  unless the stored representation is explicitly canonicalized for that order.

## Release 1 Use Cases

### Register-Attribute Search Replacement

The earlier searchable register-attribute idea can be replaced by a binary
index stored in an ordinary class attribute:

```rexx
StringIndex: class
  _arena = .binary

  contains: method = .boolean
    arg key = .string
    return packed_tree_contains(_arena, key)
```

The object still has normal attributes, but search metadata lives in `_arena`.
Lookup code uses offsets, fixed-width fields, and zero-copy compare. This keeps
register attributes simple and makes the searchable structure explicit Rexx
code.

### Fixed-Width Index Records

```text
record_size = 24
record = base + index * record_size

.u64 hash        at record + 0
.u32 key_offset  at record + 8
.u16 key_length  at record + 12
.u32 row_offset  at record + 16
.u32 row_length  at record + 20
```

Binary search over this table needs fixed-width reads and zero-copy key
comparison.

### Balanced Trees

```text
node:
  .u32 left
  .u32 right
  .u32 parent
  .i8  balance_or_color
  .u32 key_offset
  .u32 key_length
  .u32 value_offset
  .u32 value_length
```

Rotations update fixed fields in place. Keys can be compared directly from the
arena.

### Parser And Lexer Tables

Lexer/parser code needs to:

- classify one byte;
- read transition table entries;
- compare source spans against keyword constants;
- advance or rewind an offset;
- report source offsets without materializing substrings.

### Intern Tables And Hash Indexes

```text
bucket[i] -> entry offset
entry.next -> next entry offset
entry.hash -> cached hash
entry.key_len -> byte length
entry.key_bytes -> UTF-8 bytes or arbitrary binary key
```

The hot path reads offsets and hashes, then compares candidate key bytes with a
normal `.string` or `.binary` key.

## RXAS Baseline Status

The RXAS binary-memory surface is no longer tracked in this working note. The
locked Release 1 reference is
`docs/reference/rxas/instructions/07-binary-memory.md`, with implementation
context in `docs/ai-context/RXAS_ASSEMBLER.md`.

Completed RXAS baseline:

- `.const name binary 0x...` and `.const name string "..."` aliases.
- Whole binary constant materialisation with `load rDst,0x...`.
- Register and constant binary lengths with `blen`.
- Mutable buffer operations: `bresize`, `bclear`, `bfill`, `setbyte`,
  `bupdate`, `bconcat`, and `bappend`.
- Target-sized slice copy with `bcopy rDst,rSrc,rOffset`, where the destination
  length defines the copy length.
- Strict fixed-width reads and writes for `.u8`, `.i8`, `.u16`, `.i16`, `.u32`,
  `.i32`, `.i64`/`.int`, `.f32`, `.f64`, and `.float`, using canonical
  little-endian storage.
- Binary constant source forms for strict reads, target-sized copy, length, and
  zero-copy compares.
- Zero-terminated UTF-8 text-field operations: `bgets`, `bsets`, and `bcmps`.
- String-constant extraction with `sget`.
- Different-register and same-register memory moves with `bmove` and
  `bmemmove`.
- Zero-copy compare instructions `bcmpb` and `bcmps`, using an in/out compare
  register for source offset and result.
- Legacy cursor instructions `setbinpos`, `getbinpos`, and `bslice` retained as
  compatibility RXAS only, not as the preferred Rexx lowering target.

Roadmapped or deliberately deferred RXAS work:

- Capacity-only reserve such as `breserve`.
- Direct span fill / `memset(ptr,len)` support, if whole-buffer `bfill` plus
  helper code is not enough for packed update paths.
- Unsigned 64-bit storage forms.
- Explicit big-endian forms for protocol-oriented code.
- Search/hash instructions, after hash persistence and ordering semantics are
  chosen.
- Direct compare-and-branch or boolean compare instructions, only if profiles
  justify them.
- General span-to-span compare forms; Release 1 compares a source span with a
  whole binary or string needle.
- Extra constant-source append/update/concat forms, only if generated Rexx code
  demonstrates a real need.
- mmap/shared-memory/atomic binary operations.

## Rexx Surface Gap List

Implemented Rexx source surface:

- The lexer/parser accepts compiler-recognized intrinsic heads, including
  `<at..type>(...) memory` and `<sizeof..type>`, with case-insensitive `at` and
  storage type names.
- Fixed-width `<at..type>(offset) binary` reads and writes lower directly to
  RXAS for the implemented storage types.
- Variable-size `<at..binary|decimal>(offset, length) binary` reads and
  `<at..string>(offset, codepoints) binary` reads materialize ordinary Rexx
  values through target-sized copy/conversion.
- `.binary` constants and variables share the same read syntax for supported
  forms.
- Unsupported storage types, missing offsets, illegal fixed-width lengths,
  missing variable lengths, non-binary targets, read-only writes, and
  unsupported span writes have localized diagnostics.

Remaining Release 1 Rexx gaps:

1. Intrinsic compare functions.
   Zero-copy compare should be explicit. Do not hide it behind ordinary `=`.
   Source `=` remains a normal Rexx comparison of already-materialized values.
   Working intrinsic shapes:

   ```rexx
   if <compare..binary>(page, pos, key_bytes) = 0 then say "found"
   if <compare..binary>(page, pos, key_len, key_bytes) = 0 then say "found"
   if <compare..string>(page, pos, "index") = 0 then say "found"
   if <compare..u32>(page, node + NODE_HASH, wanted_hash) = 0 then say "found"
   ```

   `<compare..binary>` maps to `bcmpb`: the source memory may be a binary
   variable or binary constant, the needle may be a binary variable or binary
   constant, and the compare length may come from either the needle or an
   explicit source length. `<compare..string>` maps to `bcmps`: the source memory
   may be a binary variable or binary constant, the string may be a string
   variable or string constant, and the source field is NUL-terminated UTF-8.
   Fixed-width typed compare forms can lower through typed read/setup or a
   future direct compare if profiling justifies one. These forms avoid copies
   whenever RXAS has a direct operand form. The source-level intrinsic may have
   more arguments than the final RXAS instruction; the compiler owns that
   lowering.
2. Intrinsic binary transfer coverage.
   Any direct detail read from a binary constant should use an intrinsic, and
   the same intrinsic should work on binary variables for consistency. Release 1
   should cover:

   ```rexx
   bytes = <blen>(memory)
   n = <at..u32>(pos) memory
   key = <at..binary>(pos, len) memory
   text = <at..string>(pos, codepoints) memory
   <at..u32>(pos) memory = n
   ```

   `memory` may be a binary variable or binary constant for reads. Writes require
   a mutable binary variable.
3. Variable-size writes.
   `<at..binary>(pos, len) page = bytes`, `<at..string>(pos, len) page = text`,
   and `<at..decimal>(pos, len) page = amount` are currently rejected. Release 1
   needs lowering or a final decision to keep variable-size writes helper-only.
4. Zero-terminated text-field source surface.
   RXAS has `bgets`/`bsets` for NUL-terminated UTF-8 fields. The docs baseline
   proposes `<at..string>(offset) memory` for NUL-terminated read/write, while
   `<at..string>(offset, codepoints) memory` remains the fixed-codepoint form.
5. Binary memory moves.
   RXAS has `bmove` and `bmemmove`, including the same-buffer `memmove` case,
   but Rexx has no implemented source spelling. These should be ordinary helper
   functions with direct compiler/inliner lowering where needed, not core
   intrinsic syntax unless profiling proves they are lookup hot-path operations.
6. Buffer lifecycle helpers.
   Resize, clear, fill, reserve, and byte length need a coherent Rexx helper
   surface over `bresize`, `bclear`, `bfill`, and `blen`, including how those
   helpers interact with existing `rxfnsb` `bin*` functions.
7. Function fallback and inliner policy.
   Decide which binary-memory helper calls are public fallbacks and which are
   compiler-recognized intrinsics with direct RXAS lowering.
8. Constant scope and import/export proof.
   Scoped constants should make large binary constants usable across
   procedures. Cross-module constant export/import still needs validation or a
   documented Release 2 deferral.
9. Decimal span policy.
   `.decimal` spans currently materialize through text conversion. Canonical
   decimal storage and bytewise ordering rules still need a sharper contract
   before decimal compare/search helpers are promised.
10. Data-structure proof fixtures.
    Build packed sorted-table/tree fixtures and inspect optimized RXAS to prove
    the hot path has no helper-call overhead and no substring/binary-slice
    copies.

### Intrinsic Versus Function Rule

Use an intrinsic only when the compiler must understand the operation as binary
memory:

- direct field/span transfer from or to binary variables;
- direct field/span transfer from binary constants;
- binary/string compares that must avoid copies;
- fixed storage metadata such as `<sizeof..type>` and binary length when the
  operand may be a constant.

Everything else should start as an ordinary Rexx function. The compiler may
still lower selected functions directly to RXAS, but the source surface should
not grow angle-bracket forms just because an RXAS instruction exists.

Release 1 intrinsic set should stay small:

- `<sizeof..type>`
- `<blen>(memory)`
- `<at..type>(offset) memory`
- `<at..binary|decimal>(offset, length) memory`
- `<at..string>(offset [, codepoints]) memory`
- `<compare..binary>(memory, offset [, length], needle)`
- `<compare..string>(memory, offset, string)`
- `<compare..type>(memory, offset, value)` for fixed-width field comparisons, if
  this proves cleaner than spelling a read followed by normal `=`.

Release 1 ordinary helper functions should cover buffer management and mutation:

- resize, clear, whole-buffer fill, and possibly reserve;
- byte copy/move between buffers and same-buffer `memmove`;
- append, overlay/update, insert gap, delete range;
- search/hash only as library functions unless profiles justify RXAS support.

Current hot-path assessment: typed field reads/writes and binary/string compares
are hot for lookup algorithms and deserve intrinsics. `memmove` is important for
insert/delete/update paths, but not usually for the lookup loop; direct-lowered
ordinary function spelling should be enough for Release 1. No other mutation
helper currently deserves a dedicated `<...>` form.

## Release 1 Bounds, Signals, And Validation

Strict binary-memory operations should use predictable signals:

- `OUT_OF_RANGE`: negative offset, negative length, field does not fit in the
  current binary length, or write value outside selected field range.
- `UNICODE_ERROR`: converting memory bytes to `.string` finds invalid UTF-8.
- `CONVERSION_ERROR`: converting memory bytes to `.decimal` or an integer
  value fails under the target type rules.
- `OVERFLOW_UNDERFLOW`: a numeric value is representable in Rexx but not in the
  selected binary field width, or the reverse conversion cannot fit the target.
- `FAILURE`: allocation failure during resize/reserve or other VM failure.

## Release 1 Optimiser And Code Generation

Compiler lowering should prefer direct RXAS over helper calls:

- `<at..type>(offset) buffer` on `.binary` variables lowers to strict typed
  reads and writes.
- `<at..type>(offset) constant` uses the same source syntax but may materialize
  a register in Release 1.
- Compare intrinsics such as `<compare..binary>` and `<compare..string>` lower
  to zero-copy compare RXAS when a direct form exists.
- Writes, resize, reserve, clear, fill, and any generated internal cursor
  movement mutate the binary register and are optimizer barriers for that
  register.
- Read-only constants are immutable compile-time values and can be shared
  through the constant pool.
- Hot-path compare intrinsics must not materialize `binsubstr`, binary slices,
  or temporary strings. Ordinary `=` compares ordinary materialized Rexx values.
- Performance fixtures should inspect optimized `.rxas` so helper calls and
  unnecessary copies do not creep back in.

## Release 1 Work Plan From Here

Completed baseline:

- RXAS binary-memory instructions, constants, tests, and human reference docs.
- Generic intrinsic lexer/parser shape for `<...>` and `<at..type>`.
- `<sizeof..type>`.
- Fixed-width source reads and writes.
- Variable-size source reads.
- Negative and localized diagnostics for the currently rejected source forms.
- Initial Rexx binary-memory reference and programming-guide docs:
  `docs/books/crexx_language_reference/binary_memory.md` and
  `docs/books/crexx_programming_guide/binary_memory.md`.

Documentation is now the next gate. The RXAS surface is coherent and the first
Rexx-facing docs exist, but those docs must be reviewed and approved before more
compiler work. Once approved, the docs become the implementation checklist.

Doc-first closure checklist:

1. Review the Rexx language-reference chapter
   `docs/books/crexx_language_reference/binary_memory.md`. This chapter must be
   normative for:
   - byte offsets and byte lengths;
   - fixed-width storage types and canonical little-endian encoding;
   - `.binary` variables versus `.binary` constants, including read-only writes;
   - `<sizeof..type>`;
   - `<blen>(memory)`;
   - `<at..type>(offset) memory` fixed-width reads and writes;
   - `<at..binary|decimal>(offset, length) memory` materializing reads and the
     final Release 1 decision for variable-size writes;
   - `<at..string>(offset [, codepoints]) memory`, where omitted length means a
     NUL-terminated UTF-8 field and present length means codepoints;
   - `<compare..binary>(memory, offset [, length], needle)`;
   - `<compare..string>(memory, offset, string)`;
   - whether fixed-width compare forms such as `<compare..u32>` are Release 1 or
     deferred;
   - zero-terminated UTF-8 field read/write using `<at..string>(offset)`.
2. Review the programming-guide chapter
   `docs/books/crexx_programming_guide/binary_memory.md`, including the header
   check, sorted-table lookup, and insert/delete path examples. The guide should
   show when to use intrinsics and when to use helpers.
3. Review the BIF/function reference for ordinary binary helpers. Keep these
   separate from compiler intrinsics. Specify names, argument order, mutability,
   zero-based offsets for packed-memory helpers, compatibility with existing
   1-based `BIN*` helpers, and which helpers are eligible for direct lowering.
4. Review the diagnostic contract table covering parser, validation, and runtime
   errors. This must include exact diagnostic keys/messages for:
   - malformed intrinsic heads and argument counts;
   - unsupported storage types;
   - missing offsets or illegal lengths;
   - attempts to write constants;
   - non-binary memory operands;
   - out-of-range reads/writes/compares;
   - integer overflow/underflow and conversion errors;
   - invalid UTF-8 for string reads and string compares.
5. Keep doc examples in a testable format before implementation. Each accepted
   intrinsic/helper form should have at least one positive example, and each
   diagnostic family should have a negative compiler or runtime test expectation.

Implementation steps after the docs are approved:

1. Constant scoping cleanup: reject file-body `constant` declarations with a
   localized diagnostic, migrate existing tests/examples to explicit procedure,
   method, or factory scopes, and add negative coverage proving top-level
   constants no longer create an accidental implicit `main()` path.
2. Parser and AST: ensure generic intrinsic heads can represent
   `<blen>`, `<at..type>`, `<compare..type>`, and any text-field intrinsics with
   ordinary parenthesized expression arguments.
3. Validation and diagnostics: enforce the documented signatures, type rules,
   mutability rules, and localized diagnostics.
4. Code generation: lower binary variables and constants for `<blen>`,
   `<at..type>`, variable-size reads, accepted variable-size writes, and compare
   intrinsics to the existing RXAS instructions.
5. Helper functions: implement or rename the ordinary binary helper surface for
   resize, clear, fill, copy, memmove, append, overlay, insert gap, and delete
   range. Add direct lowering only for helpers that profiles or packed examples
   prove hot.
6. Tests: add focused parser, validation, localization, runtime, constant-source,
   and optimized-RXAS tests. The compare tests must assert no substring, binary
   slice, or helper call appears in the optimized hot path.
7. Examples and performance fixtures: add packed table/tree fixtures and inspect
   generated RXAS to prove the lookup path is direct and zero-copy.
8. Close the working note: once the reference docs, guide, helpers, compiler
   lowering, diagnostics, and fixtures are complete, either remove this file or
   reduce it to a short historical pointer so the reference docs remain the
   source of truth.

## Release 2 Level B Possibilities

Release 2 Level B should stay conservative and explicit. Candidate additions:

- `readonly .binary` or `const .binary` parameter/view type, if measured
  constant materialization is still a problem.
- More polished memory lvalue syntax after Release 1 experience.
- Source-level cursor syntax, if offset/span syntax proves too verbose for
  parsers or streaming encoders.
- Generated layout accessors from a simple declarative format.
- Safer sub-span references if the reference system can express mutable spans
  without copying.
- Import/export rules for binary layout constants if Release 1 keeps them
  module-local.
- Optional checked fixed-size binary arrays, such as `.binary[.u32, count]`, if
  they do not conflict with normal arrays.

Do not add these merely for aesthetics. The Release 2 Level B bar should be:
measured performance, clearer safety, or repeated ugly patterns in Release 1
code.

## Release 2 Level G Possibilities

Level G can afford more expressive syntax once the lower-level model is proven.
Candidate additions:

- `binary struct` declarations that generate offsets, sizes, accessors, and
  validation.
- Field syntax over binary layouts:

  ```rexx
  node.left(arena, pos)
  arena.node[pos].left
  <at..u32>(pos + NODE_LEFT) arena
  ```

- Cursor/view objects that the compiler treats as zero-cost:

  ```rexx
  c = arena.cursor(pos)
  key = c.string(key_len)
  call c.put(.u32, next_offset)
  ```

- Lexer/parser convenience syntax:

  ```rexx
  when <compare..string>(source, pos, "select") = 0 then ...
  when <compare..binary>(source, pos, KW_SELECT) = 0 then ...
  ```

- Schema/version metadata for persistent binary structures.
- Higher-level builders for tries, B-trees, sorted tables, and packed maps.
- Unicode-aware binary/text bridges for Level G string services, while keeping
  arbitrary bytes on the `.binary` side until explicitly decoded.

## Release 2 RXAS Possibilities

Release 2 RXAS changes should be driven by measured Release 1 generated code.
Possibilities:

- A read-only constant/view operand kind that avoids register materialization.
- Extra constant-pool operand forms for operations not covered in Release 1,
  such as append, update, concat, search, or hash, if generated code needs them.
- Four-operand RXAS compare/search forms such as `memory,offset,length,needle`
  if lowering the richer source intrinsic through the Release 1 compare-register
  convention is too costly or noisy.
- Strict cursor and read/write-advance instructions if source-level cursor
  syntax is accepted.
- Span reference instructions if source-level sub-span references are accepted.
- mmap-backed binary payload instructions or flags.
- Atomic fixed-width read/write/update operations for shared-memory work.
- Stable hash instructions once the algorithm and persistence contract are
  selected.

The main Release 2 caution: do not add RXAS surface area for source syntax that
has not proven itself in Release 1 code.

## Open Decisions

1. Final names for compare intrinsics: the current preferred family is
   `<compare..type>(...)`, while shorter aliases such as `<bcmp>`/`<scmp>` or
   flatter names such as `<compare_binary>` remain spelling options.
2. Whether variable-size writes should be Release 1 intrinsic syntax or
   helper-only until there is more usage evidence.
3. Final names for direct-lowered mutation helpers such as copy, memmove,
   append, overlay, insert gap, and delete range.
4. Whether any shorter single-byte write spelling belongs after Release 1.
5. Whether compare returns only `-1/0/1` or also needs first-mismatch offset
   variants.
6. Whether explicit big-endian typed operations are needed in Release 2.
7. Whether decimal memory fields require canonical decimal text before bytewise
   ordering is allowed.
8. Whether current `bin*` helpers remain as compatibility wrappers, are renamed,
   or are superseded before Release 1.
9. Whether cross-module binary constants are required in Release 1 or can wait.

## Working Recommendation

Before more compiler work, review and approve the Rexx language reference,
programming-guide examples, diagnostics table, and binary helper/BIF reference.
Those docs should then be treated as the implementation checklist.

For Release 1, use explicit byte offsets with `<at..type>(offset) memory`,
optional byte length as `<at..type>(offset, length) memory`, the size operator
`<sizeof..type>`, and explicit compare intrinsics such as
`<compare..binary>(memory, offset, needle)` and
`<compare..string>(memory, offset, string)`. Keep constants and variables
identical at the source read level, accept ordinary register materialization for
constants in calls, and use scoped constants plus direct intrinsics to keep hot
lookup code fast.

The first successful milestone is not a full binary-struct language. It is a
packed Rexx lookup structure whose optimized RXAS has no helper-call overhead
and no substring copies in the inner loop.

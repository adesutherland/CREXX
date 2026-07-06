# Level B/G Binary Memory Working Design

Status: working design note, not an approved language specification.

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
- RXAS byte operations include `blen`, `getbyte`, `setbyte`, `bconcat`,
  `bappend`, `setbinpos`, `getbinpos`, `bslice`, `bupdate`, `stobin`, and
  `bintos`.
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
4. Fixed-width layout types use dotted storage names. Slice 2 implements
   `.u8`, `.i8`, `.u16`, `.i16`, `.u32`, `.i32`, `.f64`, and `.float`.
   Future storage widths such as `.u64`, `.i64`, and `.f32` need measured
   Release 1 demand.
5. `.binary` constants and `.binary` variables use the same read syntax.
6. Byte positions are explicit: `<at..type(x)> buffer` means byte offset `x`
   in `buffer`. The unit of the offset expression is always bytes.
7. Typed access uses `<at..type(offset)> buffer`, with
   `<at..type(offset, length)> buffer` when a variable-size span needs an
   explicit byte length.
8. `<sizeof>` is the Release 1 unary size operator for fixed storage types.
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
constant MAGIC = "4352584944583031"x as .binary
constant MAGIC_PREFIX = "43525849"x as .binary

if <at..u32(0)> MAGIC = <at..u32(0)> MAGIC_PREFIX then say "magic"
```

That same syntax should work for variables:

```rexx
page = .binary
if <at..u32(0)> page = <at..u32(0)> MAGIC then say "magic"
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
namespace packedindex expose NODE_LEFT NODE_RIGHT NODE_SIZE EMPTY_NODE

constant NODE_LEFT = 0
constant NODE_RIGHT = 4
constant NODE_SIZE = 32
constant EMPTY_NODE = "00000000000000000000000000000000"x as .binary

find_node: procedure = .int
  arg arena = .binary, key = .string
  /* NODE_* constants are available directly in this procedure. */

insert_node: procedure = .int
  arg expose arena = .binary, key = .string
  /* Same constants are available here too. */
```

Release 1 requirement: binary layout constants should be available across
procedures in the source scope where they are declared. The existing
`namespace ... expose ...` shape should be repeated for constants: constants in
the expose list are visible to importers and to procedures in that namespace in
the same way as other exposed symbols. Cross-module constant export/import can
follow the existing constant metadata path if that path is settled enough;
otherwise it should remain an explicit follow-up.

### Constants And Mutability

The same read syntax does not mean the same mutability:

```rexx
version = <at..u32(8)> HEADER          /* OK for constant or variable */
<at..u32(8)> HEADER = 2                /* error: constant is read-only */
```

For mutable storage:

```rexx
page = .binary
page = bmemnew(4096)
<at..u32(8)> page = 2
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
- `.f64` for fixed binary64 storage, with `.float` accepted as the standard
  cREXX convenience spelling.
- `.u64`, `.i64`, and `.f32` remain future candidates for large offsets,
  64-bit hashes, external ids, dense data, or binary interchange.

Standard cREXX types should be accepted as conveniences only where the
operation has a clear storage contract:

- `.float` as the normal cREXX float view, expected to map to binary64 storage
  unless a future numeric option says otherwise.
- `.int` is not a Slice 2 persistent storage spelling; use `.i32` or `.u32`
  when the byte width matters.
- `.string` for UTF-8 text spans.
- `.decimal` for decimal text spans.
- `.binary` for byte spans.

The working bias is that fixed persisted layouts should use explicit storage
names such as `.u32`, `.i32`, and `.f64`; ergonomic standard aliases can be
added only where persistence and cross-platform byte width remain unambiguous.

## Release 1 Defined Source Syntax

Release 1 uses explicit byte offsets plus a compiler-recognized parameterized
intrinsic operator:

```rexx
<at..type(offset)> memory
<at..type(offset, length)> memory
```

The `at` head names the intrinsic family. `..type` is the Release 1 compact
single-specialization form, chosen to keep the syntax compatible with possible
future generic callable syntax such as `search.[.int, .string](tree, key)`.
The storage type is the same concept as `.u32` or `.string`; in this compact
operator form the leading dot is represented by the `..` specialization
separator, so `<at..u32(...)>` means the `.u32` storage view.
The runtime arguments inside `(...)` are ordinary expressions parsed by the
normal expression grammar. `<sizeof>` remains a unary size operator for fixed
storage types.

### Byte Access

Byte access is ordinary typed access with `u8` or `i8` and an explicit byte
offset:

```rexx
byte = <at..u8(6)> buffer
<at..u8(6)> buffer = 255
```

The byte result is an `.int` in the range `0..255`. Writes signal
`OUT_OF_RANGE` if the offset is outside the current binary length or the value
is not a byte.

Bare `buffer[x]` is not binary byte access in Release 1. That spelling remains
normal array-style indexing, which matters for arrays of `.binary` values:
`buffers[3]` should naturally mean the third binary value, not byte 3 of one
binary buffer.

### Precedence

The parser treats `<at..type(args)> memory` as one primary expression. Offset
and length arithmetic lives inside the normal argument list:

```rexx
b = <at..u8(1 + 1)> data
```

This reads byte offset `2`. Arithmetic after the memory operand applies to the
loaded value:

```rexx
b = <at..u8(1)> data + 1
```

This keeps layout expressions such as `node + NODE_BALANCE` natural inside the
argument list while avoiding the old high-precedence paired-token ambiguity.

### Fixed-Width Typed Access

Fixed-width fields are read by specializing `at` with the storage type and
passing a byte offset:

```rexx
i = <at..i8(6)> buffer
n = <at..u32(pos)> buffer
f = (<at..i8(6)> buffer) as .float
balance = <at..i8(node + NODE_BALANCE)> page
```

The `as .float` in the example is ordinary cREXX cast syntax applied after the
`.i8` load. `at` selects the binary memory object; the first argument supplies
a zero-based byte offset; `as` remains a cast.

Writes use the same left-hand shape:

```rexx
<at..u32(pos)> buffer = next_offset
<at..i8(node + NODE_BALANCE)> page = balance
```

Fixed-width types know their byte width. No length is written for `.u8`, `.i8`,
`.u16`, `.i16`, `.u32`, `.i32`, `.f64`, or the `.float` convenience alias.
`u64`, `i64`, and `f32` remain future candidates until there is a measured
Release 1 need.

### Variable-Size Spans

Variable-size memory views pass a second byte-count argument:

```rexx
b = <at..binary(6, 10)> buffer
s = <at..string(6, 10)> buffer
d = <at..decimal(pos, decimal_len)> buffer
```

When the expression is materialized as a normal Rexx value, `.string` validates
UTF-8 and `.decimal` parses decimal text. `.binary` copies the selected bytes.

Variable-size writes use the same conceptual shape, but are not part of Slice 2
source lowering. The compiler currently rejects them with a specific diagnostic:

```rexx
<at..binary(pos, key_len)> buffer = key_bytes
<at..string(pos, key_len)> buffer = key_text
<at..decimal(pos, amount_len)> buffer = amount
```

Later slices can add the write lowering and decide whether any exact-length
write may omit the length argument. Persistent variable-size fields should
spell the length when a span length matters.

### Size Operator

`<sizeof>` is a unary compile-time operator for fixed storage types:

```rexx
constant NODE_BALANCE = 12
constant NODE_SIZE = 3 * <sizeof> .u32 + <sizeof> .i8

balance = <at..i8(node + NODE_BALANCE)> page
next = node + 3 * <sizeof> .u32
```

`<sizeof>` returns bytes. It does not imply typed-element indexing; offsets are
always byte offsets. `NODE_BALANCE` and similar layout constants should be byte
offsets.

### Zero-Copy Compare

Typed memory expressions should produce an internal span/view:

```text
binary source + byte offset + byte length + interpretation tag
```

The compiler must use that span/view directly for comparison. It must not first
create a substring, binary slice, or string value. This is the planned Slice 3
work; Slice 2 materializes variable-size reads after a strict `bcheckrange`.

```rexx
if <at..binary(pos, key_len)> buffer = key_bytes then say "found"
if <at..string(6, 5)> buffer = "index" then say "found"
if <at..binary(pos, <sizeof> .u32)> buffer = KEY_U32_BYTES then say "found"
```

For compare, `.string` selects compatibility with string operands, but the
comparison itself is byte-wise over the selected span and the other operand's
bytes. UTF-8 validation is required when the span is materialized or explicitly
cast to `.string`; a pure equality compare should not allocate a string just to
validate it.

Exact-length compare may later infer the span length from the other operand
when that operand is a literal, constant, or ordinary variable with a known
runtime byte length:

```rexx
if <at..binary(pos)> buffer = KEY_BYTES then say "found"
if <at..string(6)> buffer = "index" then say "found"
```

Until that compare lowering exists, pass the length for variable-size memory
expressions. Continue to pass the length when it is stored separately, when
comparing a prefix/range, or when the code should document the field width.

### Function Fallbacks

Function helpers remain useful for fallback and tests:

```rexx
version = bmem.u32(buffer, 8)
if bmem.equals(buffer, pos, key_len, key_bytes) then say "found"
```

They are not the hot path unless the compiler recognizes them as intrinsics and
lowers them to the same RXAS as `<at..type(args)>`.

The inliner roadmap should include special lowering for selected binary-memory
functions. That lets users choose a BIF/function spelling where it reads better
while still allowing hot paths to lower to direct RXAS when the call target and
argument shape are known.

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

## Release 1 RXAS Surface

Instruction names below are working names.

Release 1 should prefer a small RXAS core that fits the existing assembler
shape. The current RXAS parser accepts up to three operands, so Release 1 should
avoid depending on four-operand instructions.

### RXAS Principles

1. Binary memory operands are registers in Release 1.
2. A `.binary` constant used by source syntax may be materialized with `load`
   before the typed memory instruction.
3. No Release 1 instruction requires a special constant-pass operand model.
4. Writes, resize, reserve, clear, and fill require mutable binary registers.
5. Strict typed memory operations raise `OUT_OF_RANGE` for invalid ranges.
6. Existing tolerant `getbyte` behavior can remain for compatibility.
7. New instructions should be easy for the compiler to emit from syntax without
   helper calls.

Optional direct literal forms may be useful later, but they are not required for
Release 1 semantics.

### Size And Capacity

Existing:

```rxas
blen rOut,rMem
```

Proposed:

```rxas
bresize  rMem,rLength     ; set byte length, zero-fill growth
breserve rMem,rCapacity   ; ensure capacity, length unchanged
bclear   rMem             ; length becomes 0, capacity may remain
bfill    rMem,rByte       ; fill the whole current binary with one byte
```

`bresize` raises `OUT_OF_RANGE` for negative lengths and `FAILURE` for
allocation failure. Growth should zero-fill.

Slice 1 implements `bresize`, `bclear`, and `bfill`. `breserve` remains a later
capacity-only candidate.

RXAS is the right layer for clearing, initialising, reserving, resizing, and
bulk filling buffers. Source syntax should stay focused on typed access and
compare; buffer lifecycle operations can remain helper/instruction backed.

Existing cursor instructions such as `setbinpos`, `getbinpos`, and `bslice`
remain part of the baseline. New source-level cursor syntax is deferred to
Release 2. Release 1 source span reads use `bcheckrange` before cursor-based
`bslice`, saving and restoring the binary cursor around the internal slice.

### Fixed-Width Reads

```rxas
bgetu8   rOut,rMem,rOffset
bgeti8   rOut,rMem,rOffset
bgetu16  rOut,rMem,rOffset
bgeti16  rOut,rMem,rOffset
bgetu32  rOut,rMem,rOffset
bgeti32  rOut,rMem,rOffset
bgetu64  rOut,rMem,rOffset
bgeti64  rOut,rMem,rOffset
bgetf32  rOut,rMem,rOffset
bgetf64  rOut,rMem,rOffset
```

`bgetu8` is the strict counterpart to current `getbyte`. Current `getbyte`
returns `-1` when out of range and should remain available for compatibility.
Slice 1 implements `bgetu8`, `bgeti8`, `bgetu16`, `bgeti16`, `bgetu32`,
`bgeti32`, and `bgetf64`; `u64`, `i64`, and `f32` remain future candidates.

Unsigned reads return `.int` values when representable. If a value cannot fit
in the configured `rxinteger`, the instruction raises `OUT_OF_RANGE` rather
than silently wrapping.

### Fixed-Width Writes

```rxas
bsetu8   rMem,rOffset,rValue
bseti8   rMem,rOffset,rValue
bsetu16  rMem,rOffset,rValue
bseti16  rMem,rOffset,rValue
bsetu32  rMem,rOffset,rValue
bseti32  rMem,rOffset,rValue
bsetu64  rMem,rOffset,rValue
bseti64  rMem,rOffset,rValue
bsetf32  rMem,rOffset,rValue
bsetf64  rMem,rOffset,rValue
```

Slice 1 implements `bsetu8`, `bseti8`, `bsetu16`, `bseti16`, `bsetu32`,
`bseti32`, and `bsetf64`; `u64`, `i64`, and `f32` remain future candidates.

Writes are strict:

- offset must be non-negative;
- `offset + field_width <= blen(rMem)`;
- integer values must fit the selected signed or unsigned field width;
- float values must be representable in the selected storage format;
- invalid writes raise `OUT_OF_RANGE`, `CONVERSION_ERROR`, or
  `OVERFLOW_UNDERFLOW` as appropriate.

### Span Copy And Fill

Existing `bupdate` overlays a whole binary source at an explicit offset. Release
1 may add:

```rxas
bcopy    rDst,rDstOffset,rSrc    ; copy all bytes from rSrc to dst offset
bsets    rMem,rOffset,rString
bsetd    rMem,rOffset,rDecimal
```

`bsets` copies UTF-8 bytes from a `.string` into a mutable binary value without
normalization. `bsetd` copies a canonical decimal text representation.
Span fill needs offset, length, and byte value, so it is a Release 2 candidate
unless generated code can express it cleanly through a small Release 1 sequence.

### Span Loads

Constructing ordinary Rexx values from memory spans copies out of the binary
memory space:

```rxas
bgets   rOut,rMem,rLen    ; string from current/internal position and length, UTF-8 checked
bgetd   rOut,rMem,rLen    ; decimal from current/internal position and length
bgetb   rOut,rMem,rLen    ; binary slice from current/internal position and length
```

`bgets` raises `UNICODE_ERROR` for invalid UTF-8. `bgetd` raises
`CONVERSION_ERROR` for invalid decimal text.

### Zero-Copy Compare

Lookup algorithms need compare operations that do not allocate temporary string
or binary values:

```rxas
bcmpb   rOut,rMem,rNeedle
bcmps   rOut,rMem,rString
bcmpd   rOut,rMem,rDecimal
```

The first implementation may use the existing binary cursor internally to name
the memory start. The source model is still offset/span based; cursor movement
is not exposed to Release 1 Rexx code. The result should be `-1`, `0`, or `1`
for unsigned-byte lexicographic ordering. This is more useful for binary search
and tree ordering than a first-mismatch-offset result.

Boolean forms may be added if measured code benefits:

```rxas
bseqb   rOut,rMem,rNeedle
bsneb   rOut,rMem,rNeedle
bseqs   rOut,rMem,rString
bsnes   rOut,rMem,rString
```

Release 1 can implement source compare syntax by setting an internal cursor and
using a three-operand compare, avoiding a four-operand RXAS form. If that proves
too costly or too stateful in generated code, direct offset/length RXAS forms
can be considered for Release 2.

The source-level compare operator is required to lower to this family, or to an
equivalent direct span compare. Lowering it by first extracting a string or
binary slice is not an acceptable Release 1 implementation.

### Search And Hash Helpers

Search/hash instructions are useful but can be staged after fixed access and
compare:

```rxas
bfindb  rOut,rMem,rNeedle ; search from current/internal position, return offset or -1
bfinds  rOut,rMem,rString
bhash   rOut,rMem,rLen    ; hash bytes from current/internal position for rLen bytes
```

Do not expose `bhash` for persistent indexes until the hash algorithm is chosen
and documented.

### Byte Order

Release 1 uses a portable fixed-width byte order:

- canonical little-endian storage for fixed-width reads and writes;
- explicit big-endian variants only if needed for protocol work, such as
  `bgetu32be` and `bsetu32be`;
- no host-native struct layout, alignment, or padding in the language contract.

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

- `<at..type(offset)> buffer` on `.binary` variables lowers to strict typed
  reads, writes, or compare.
- `<at..type(offset)> constant` uses the same source syntax but may materialize
  a register in Release 1.
- Writes, resize, reserve, clear, fill, and any generated internal cursor
  movement mutate the binary register and are optimizer barriers for that
  register.
- Read-only constants are immutable compile-time values and can be shared
  through the constant pool.
- Hot-path compare syntax must not materialize `binsubstr`, binary slices, or
  temporary strings. A compare operator that copies is a bug in the selected
  design, not merely a missed optimization.
- Performance fixtures should inspect optimized `.rxas` so helper calls and
  unnecessary copies do not creep back in.

## Release 1 Implementation Slices

### Slice 1: RXAS Core

- Add strict fixed-width reads and writes for the most useful field types:
  `.u8`, `.i8`, `.u16`, `.i16`, `.u32`, `.i32`, and `.f64`.
- Add `bresize`, `bclear`, and whole-buffer `bfill`.
- Add focused RXAS tests for bounds, endian behavior, constants materialized
  through `load`, and writes.

### Slice 2: Source Syntax And Intrinsics

- Implement `<at..type(offset)> buffer`,
  `<at..type(offset, length)> buffer`, and unary `<sizeof>` as compiler
  recognized intrinsics/operators.
- Implement compiler lowering directly to RXAS for fixed-width reads,
  fixed-width writes, and variable-size reads.
- Keep helper functions only as fallback/test API.
- Add tests proving constants and variables use the same read syntax.
- Add negative tests proving constants cannot be write/resize targets and
  variable-size writes are rejected until their lowering is implemented.

### Slice 3: Zero-Copy Compare

- Add RXAS compare operations.
- Lower source compare syntax without substring, binary-slice, or temporary
  string creation.
- Add parser/lexer and binary-search fixtures.

### Slice 4: Rexx Data Structure Proofs

- Build a packed sorted table fixture.
- Build a packed tree fixture.
- Compare correctness and rough performance against array/object versions.
- Record the generated RXAS patterns that must remain stable.

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
  <at..u32(pos + NODE_LEFT)> arena
  ```

- Cursor/view objects that the compiler treats as zero-cost:

  ```rexx
  c = arena.cursor(pos)
  key = c.string(key_len)
  call c.put(.u32, next_offset)
  ```

- Lexer/parser convenience syntax:

  ```rexx
  when <at..string(pos, 6)> source = "select" then ...
  when <at..binary(pos)> source = KW_SELECT then ...
  ```

- Schema/version metadata for persistent binary structures.
- Higher-level builders for tries, B-trees, sorted tables, and packed maps.
- Unicode-aware binary/text bridges for Level G string services, while keeping
  arbitrary bytes on the `.binary` side until explicitly decoded.

## Release 2 RXAS Possibilities

Release 2 RXAS changes should be driven by measured Release 1 generated code.
Possibilities:

- Direct constant-pool memory operands for read-only operations.
- A read-only constant/view operand kind that avoids register materialization.
- Four-operand compare/search forms such as `memory,offset,length,needle` if
  cursor setup is too costly or noisy.
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

1. Whether exact-length compare may omit the length argument whenever the other
   operand's byte length is known, or only for constants/literals.
2. Whether any shorter single-byte write spelling belongs after Release 1.
3. Whether standard cREXX types beyond `.float` should be source conveniences
   only, or whether any of them can name persistent storage widths.
4. Whether compare returns only `-1/0/1` or also needs first-mismatch offset
   variants.
5. Whether explicit big-endian typed operations are needed in Release 2.
7. Exact signal for unsigned reads that do not fit `.int`.
8. Whether decimal memory fields require canonical decimal text before bytewise
   ordering is allowed.
9. Whether current `bin*` helpers remain as compatibility wrappers, are renamed,
   or are superseded before Release 1.
10. Whether cross-module binary constants are required in Release 1 or can wait.

## Working Recommendation

For Release 1, use explicit offsets with `<at..type(offset)> buffer`,
optional length as `<at..type(offset, length)> buffer`, and unary `<sizeof>`,
then lower that syntax directly to a compact RXAS typed-memory core. Keep
constants and variables identical at the source read level, accept ordinary
register materialization for constants in calls, and use scoped constants plus
direct syntax to keep hot lookup code fast.

The first successful milestone is not a full binary-struct language. It is a
packed Rexx lookup structure whose optimized RXAS has no helper-call overhead
and no substring copies in the inner loop.

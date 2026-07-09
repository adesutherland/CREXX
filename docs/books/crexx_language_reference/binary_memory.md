# Binary Memory

This chapter specifies the Release 1 binary-memory source surface for Level B.
It builds on the RXAS binary-memory instruction baseline and defines the forms
that the compiler lowers directly for packed lookup and record code.

Binary memory treats a `.binary` value or `.binary` constant as an indexed byte
space. It is intended for packed lookup structures, indexes, parsers, protocol
records, and other cases where repeatedly copying strings or binary slices would
dominate runtime cost.

## Core Rules

- Offsets are zero-based byte offsets.
- Lengths are byte counts unless the form explicitly says otherwise.
- `.binary` variables and `.binary` constants use the same read syntax.
- Writes require a mutable `.binary` variable; constants are read-only.
- Fixed-width numeric fields use canonical little-endian storage.
- `.string` fields are UTF-8. Invalid UTF-8 raises `UNICODE_ERROR` when a string
  value is materialized or compared as a string field.
- Ordinary `=` compares ordinary materialized Rexx values. Zero-copy binary
  memory compare uses explicit compare intrinsics.

Do not use `[]` to address binary bytes. Brackets remain array indexing. Binary
memory uses intrinsic forms so an array of `.binary` values and a byte offset in
one `.binary` value remain unambiguous.

## Storage Types

Fixed-width storage types are layout encodings, not separate Rexx variable
types. A read returns a normal Rexx value; a write converts a normal Rexx value
into the selected storage encoding.

| Storage type | Width | Rexx value | Notes |
| --- | ---: | --- | --- |
| `.u8` | 1 | `.int` | Unsigned byte. |
| `.i8` | 1 | `.int` | Signed byte. |
| `.u16` | 2 | `.int` | Unsigned little-endian 16-bit integer. |
| `.i16` | 2 | `.int` | Signed little-endian 16-bit integer. |
| `.u32` | 4 | `.int` | Unsigned little-endian 32-bit integer. |
| `.i32` | 4 | `.int` | Signed little-endian 32-bit integer. |
| `.i64` | 8 | `.int` | Signed little-endian 64-bit integer. |
| `.int` | 8 | `.int` | Convenience spelling for `.i64` in binary memory. |
| `.f32` | 4 | `.float` | IEEE binary32 storage. |
| `.f64` | 8 | `.float` | IEEE binary64 storage. |
| `.float` | 8 | `.float` | Convenience spelling for `.f64` in binary memory. |

`.u64` is not part of the Release 1 source surface. Use `.i64`/`.int` or an
explicit byte layout until unsigned 64-bit conversion and ordering rules are
settled.

Variable-size storage views are:

| View | Length argument | Result |
| --- | --- | --- |
| `.binary` | Bytes | A copied `.binary` value. |
| `.string` | UTF-8 codepoints, or omitted for NUL-terminated | A copied `.string` value after validation. |
| `.decimal` | Omitted for NUL-terminated | A `.decimal` value parsed from decimal text. |

For `.string`, the starting position is still a byte offset. The length is a
count of UTF-8 codepoints to extract. The VM scans from the byte offset to find
the corresponding byte length, validates the selected UTF-8, copies those exact
bytes into the destination string, and writes the string safety NUL outside the
logical string length.

For `.decimal`, the starting position is a byte offset to zero-terminated
decimal text. The terminator is not part of the decimal value. Decimal reads
scan to the first zero byte, validate the selected bytes as UTF-8 text, and
parse that text through the active decimal plugin.

## Size And Length Intrinsics

`<sizeof..type>` returns the fixed storage width in bytes:

```rexx
layout_size: procedure = .int
  constant NODE_LEFT = 0
  constant NODE_RIGHT = NODE_LEFT + <sizeof..u32>
  constant NODE_BALANCE = NODE_RIGHT + <sizeof..u32>
  constant NODE_SIZE = NODE_BALANCE + <sizeof..i8>
  return NODE_SIZE
```

`<sizeof>` is valid only for fixed-width storage types. It is a compile-time
operator.

`<blen>(memory)` returns the logical byte length of a `.binary` variable or
`.binary` constant:

```rexx
bytes = <blen>(arena)
header_bytes = <blen>(HEADER_TEMPLATE)
```

## Fixed-Width Reads And Writes

The fixed-width read form is:

```rexx
value = <at..type>(offset) memory
```

`memory` may be a `.binary` variable or `.binary` constant. `offset` is a
zero-based byte offset.

The fixed-width write form is:

```rexx
<at..type>(offset) memory = value
```

`memory` must be a mutable `.binary` variable. The field must fit completely
inside the current logical byte length. Binary-memory writes do not resize the
buffer.

<!-- rexx-example name="binary-memory-fixed-fields" test="pending" -->
```rexx
options levelb

main: procedure = .int
  constant NODE_LEFT = 0
  constant NODE_RIGHT = 4
  constant NODE_BALANCE = 8

  node = .binary
  call binresize node, 16

  <at..u32>(NODE_LEFT) node = 12
  <at..u32>(NODE_RIGHT) node = 44
  <at..i8>(NODE_BALANCE) node = -1

  left = <at..u32>(NODE_LEFT) node
  balance = <at..i8>(NODE_BALANCE) node
  return 0
```

## Variable-Size Reads And Writes

Variable-size reads materialize ordinary Rexx values:

```rexx
key_bytes = <at..binary>(key_offset, key_len) arena
name = <at..string>(name_offset, name_codepoints) arena
amount = <at..decimal>(amount_offset) arena
```

The `.binary` length argument is a byte count. The `.string` length argument is
a UTF-8 codepoint count from the byte offset. The `.decimal` form does not
accept a length argument; decimal fields are zero-terminated decimal text.

Release 1 implements zero-terminated string and decimal writes and rejects
variable-size span writes:

```rexx
<at..binary>(key_offset, key_len) arena = key_bytes
<at..string>(name_offset, name_codepoints) arena = name
```

These span-write forms raise `BINARY_MEMORY_SPAN_WRITE_UNSUPPORTED`.
Programmers should use `binupdate`, `bincopy`, `binfillat`, `binmakegap`, and
`bindrop` for Release 1 byte-span mutation. If span writes are added later, the
complete destination span must already fit inside the current binary length.
Binary-memory writes do not resize the binary buffer.

## Zero-Terminated UTF-8 Fields

Some packed formats store UTF-8 text as bytes terminated by a zero byte. The
program must know which fields use that convention. The terminator is not part
of the logical string.

The Release 1 source spelling is `<at..string>(offset) memory`, with no length
argument:

```rexx
text = <at..string>(offset) memory
<at..string>(offset) memory = text
amount = <at..decimal>(offset) memory
<at..decimal>(offset) memory = amount
```

The read form scans from the byte offset to the first zero byte, validates the
selected bytes as UTF-8, and copies those bytes into a `.string`. The write form
writes the string bytes followed by one zero byte. It does not resize the binary
buffer; the complete string plus terminator must already fit.

The decimal forms use the same zero-terminated field convention. Reads parse
the selected text as `.decimal`. Writes convert the decimal value to decimal
text through the active decimal plugin, then write those bytes followed by one
zero byte. They do not resize the binary buffer.

The fixed-codepoint read form remains
`<at..string>(offset, codepoints) memory`. Release 1 does not support the
matching fixed-codepoint write form.

The compare intrinsic `<compare..string>` uses this zero-terminated field
contract.

## Zero-Copy Compare Intrinsics

Use compare intrinsics when the binary memory location itself is part of the
operation and copying would be the wrong cost model.

The preferred compare family is:

```rexx
result = <compare..binary>(memory, offset, needle)
result = <compare..string>(memory, offset, string)
result = <compare..u32>(memory, offset, value)
```

The result is an integer:

| Result | Meaning |
| ---: | --- |
| `-1` | The selected memory field is less than the comparison value. |
| `0` | The selected memory field is equal to the comparison value. |
| `1` | The selected memory field is greater than the comparison value. |

`<compare..binary>(memory, offset, needle)` compares bytes at `memory + offset`
with the whole binary needle. The source length is the byte length of `needle`.
`memory` and `needle` may each be a variable or a constant.

There is no Release 1 explicit source-length binary compare form. For
variable-length fields, compare the stored length with `<blen>(needle)` first,
then use `<compare..binary>(memory, offset, needle)`. An explicit-length
zero-copy compare can be added later when RXAS has a matching instruction.

`<compare..string>(memory, offset, string)` compares a zero-terminated UTF-8
field in binary memory with a string variable or string constant. It validates
the source field as UTF-8 but must not materialize a temporary string when a
direct compare is available.

Fixed-width typed compare forms such as `<compare..u32>` compare the stored
field with an ordinary Rexx value. They lower through a typed read plus a
tri-state comparison, avoiding any binary slice copy.

<!-- rexx-example name="binary-memory-compare" test="pending" -->
```rexx
options levelb

main: procedure = .int
  arg arena = .binary, key_offset = .int, key_len = .int, wanted_key = .binary, text_offset = .int
  constant MAGIC = "4352584944583031"x as .binary

  if <compare..binary>(MAGIC, 0, "43525849"x as .binary) = 0 then say "CRXI"
  if key_len = <blen>(wanted_key) then
    if <compare..binary>(arena, key_offset, wanted_key) = 0 then say "hit"
  if <compare..string>(arena, text_offset, "select") = 0 then say "keyword"
  return 0
```

Ordinary `=` remains ordinary Rexx comparison. For example, this materializes a
binary slice before comparing:

```rexx
if <at..binary>(key_offset, key_len) arena = wanted_key then say "copied compare"
```

Use that form only when materialization is acceptable or wanted.

## Constants And Scope

Layout constants should be compile-time constants declared in an explicit
procedure scope. Free-floating top-level `constant` declarations are not part of
the Release 1 surface; because `constant` is an instruction form, that shape can
create an implicit `main()` rather than a pure module surface.

When callers need the constants, publish the constant names through the
namespace expose list. The implementation requirement for Release 1 is that a
constant declared in an explicit procedure scope and listed in
`namespace ... expose ...` is available to other procedures in the same
namespace and to importers as a compile-time value. A private declaration
procedure may be used to give the constants an explicit home without making that
procedure part of the public API.

If the same file also has executable script code, put that code in an explicit
`main: procedure`. A declaration procedure is a real procedure boundary; later
statements belong to it until the next procedure/class boundary, not to a fresh
implicit `main()`.

<!-- rexx-example name="binary-memory-constants" test="pending" -->
```rexx
options levelb
namespace packedindex expose find_node NODE_LEFT NODE_RIGHT NODE_SIZE EMPTY_NODE

packedindex_layout: procedure expose NODE_LEFT NODE_RIGHT NODE_SIZE EMPTY_NODE
  constant NODE_LEFT = 0
  constant NODE_RIGHT = 4
  constant NODE_SIZE = 32
  constant EMPTY_NODE = "00000000000000000000000000000000"x as .binary
  return

find_node: procedure = .int
  arg arena = .binary, key = .binary
  if <compare..binary>(arena, NODE_LEFT, key) = 0 then return NODE_LEFT
  return -1
```

Do not put executable setup or `constant` declarations at top level in a
library-style module. Top-level executable statements before a procedure
synthesize an implicit `main()` for script compatibility. Shared binary layout
values should be constants in an explicit procedure scope, exported through
`namespace ... expose ...` when they are part of the module contract.

## Ordinary Helper Functions

Intrinsics are for direct binary-memory access, constants, and zero-copy
compare. Other buffer management should use ordinary functions from `rxfnsb`.
The existing `BIN*` helpers are compatibility helpers that return copied values
and use one-based positions. Packed-memory helpers use zero-based offsets,
mutate exposed arguments, and may be direct-lowered by the compiler/inliner.

The Release 1 packed-memory helper surface is:

| Helper | Effect |
| --- | --- |
| `BINRESIZE(data, length)` | Resize `data` to `length` bytes; growth zero-fills. |
| `BINCLEAR(data)` | Set `data` to the empty binary value. |
| `BINFILL(data, byte)` | Fill the whole current logical byte range. |
| `BINFILLAT(data, offset, length, byte)` | Fill a zero-based byte span. |
| `BINCOPY(dst, dst_offset, src, src_offset, length)` | Copy `length` bytes between different binary values. |
| `BINMEMMOVE(data, dst_offset, src_offset, length)` | Move `length` bytes within one binary value; overlapping ranges are safe. |
| `BINAPPEND(dst, src)` | Append all bytes from `src` to `dst`. |
| `BINUPDATE(dst, offset, src)` | Overlay all bytes from `src` into `dst` at `offset`. |
| `BINMAKEGAP(data, offset, length)` | Insert a zero-filled gap. |
| `BINDROP(data, offset, length)` | Delete a byte range. |

Mutating helpers update the first binary argument through `arg expose` and
return the new logical byte length unless the specific helper documentation says
otherwise.

## Diagnostics And Signals

Compiler diagnostics use stable catalog keys. Tests should normally assert raw
diagnostic keys with `CREXX_DIAGNOSTICS=raw`, while user output is localized
through `messages/diagnostics.*.msg`.

Existing keys that must continue to be used:

| Key | English template | Required use |
| --- | --- | --- |
| `BINARY_MEMORY_AT_REQUIRED` | Binary memory access requires an at intrinsic. | Binary memory access was parsed without an `at` head where one is required. |
| `BINARY_MEMORY_FIXED_FOR_NOT_ALLOWED` | Fixed-width binary memory access must not specify a length argument. | Fixed-width access has an illegal length argument. |
| `BINARY_MEMORY_INVALID_STORAGE_TYPE` | Binary memory access has an unsupported storage type. | The dotted storage type is not supported. |
| `BINARY_MEMORY_LENGTH_NOT_ALLOWED` | This binary memory access form does not accept a length argument. | A zero-terminated form such as `.decimal` was given a length argument. |
| `BINARY_MEMORY_LENGTH_REQUIRED` | Variable-length binary memory access requires a length argument. | A `.binary` variable-size form is missing its length argument. |
| `BINARY_MEMORY_OFFSET_REQUIRED` | Binary memory access requires a byte-position argument. | A binary-memory form is missing its byte offset. |
| `BINARY_MEMORY_READ_ONLY` | Binary memory access target is read-only. | A write targets a binary constant or other read-only storage. |
| `BINARY_MEMORY_SPAN_WRITE_UNSUPPORTED` | Variable-length binary memory writes are not supported yet. | A variable-size write is not implemented in this compiler slice. |
| `BINARY_MEMORY_TARGET_NOT_BINARY` | Binary memory access target must be a scalar .binary value. | The memory operand is not a scalar `.binary` value or constant. |
| `CANNOT_CAST_BINARY` | Binary data cannot be converted to text without an explicit encoding. | Binary bytes would become text without explicit valid UTF-8 conversion. |
| `INVALID_SIZEOF_SYNTAX` | SIZEOF intrinsic has invalid syntax. | `<sizeof>` is malformed. |
| `INTRINSIC_GENERIC_TYPES_UNSUPPORTED` | Intrinsic type parameter lists are not supported in Release 1. | A parsed generic intrinsic type-list form is valid syntax but unsupported in Release 1. |

Compare diagnostics are:

| Key | Required message template |
| --- | --- |
| `BINARY_MEMORY_COMPARE_ARGUMENTS` | Binary memory compare intrinsic has an invalid argument list. |
| `BINARY_MEMORY_COMPARE_TYPE` | Binary memory compare intrinsic has an unsupported comparison type. |
| `BINARY_MEMORY_COMPARE_NEEDLE_TYPE` | Binary memory compare needle has an incompatible type. |

Runtime operations signal:

| Signal | Required use |
| --- | --- |
| `OUT_OF_RANGE` | Negative offset, negative length, field does not fit, missing NUL terminator, or value outside selected field range. |
| `UNICODE_ERROR` | Bytes selected for a `.string` value or string compare are not valid UTF-8. |
| `CONVERSION_ERROR` | Decimal or numeric conversion from selected bytes fails. |
| `OVERFLOW_UNDERFLOW` | A Rexx numeric value cannot be represented in the selected binary field or reverse conversion overflows. |
| `FAILURE` | Allocation or VM failure during resize, append, or other buffer-changing operations. |

## Deferred Items

The Release 1 surface deliberately leaves a few items for later work:

- unsigned 64-bit storage syntax and ordering rules (`.u64`);
- variable-size span writes for `.binary` and fixed-codepoint `.string` fields;
- explicit source-length zero-copy binary compare;
- direct compiler or inliner lowering for selected packed-memory helpers when
  profiling proves the helper call overhead matters;
- read-only constant/view parameter passing, binary struct declarations, and
  mmap/shared-memory views.

## Implementation Acceptance Tests

The implementation test surface covers:

- positive parsing and lowering for every intrinsic in this chapter;
- `.binary` constants and variables using the same read and compare forms;
- read-only writes to constants;
- each diagnostic key listed above, using raw diagnostic output;
- all runtime signals listed above;
- optimized RXAS for lookup examples, proving compare intrinsics do not emit
  `binsubstr`, binary slice materialization, string extraction, or helper calls
  in the hot path.

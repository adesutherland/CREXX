# Packed Binary Memory

Packed binary memory is for code that needs to treat a `.binary` value as a
byte-addressed record space. Typical examples are sorted lookup tables, parser
keyword tables, indexes, protocol records, and compact tree nodes.

Use this feature when a packed layout avoids many ordinary Rexx variables,
temporary substrings, or repeated binary slice copies. Do not use it just to
make simple code look lower-level; normal variables are clearer when the data is
not layout-sensitive.

The exact language rules are in the Language Reference chapter
[Binary Memory](../crexx_language_reference/binary_memory.md).

## Choosing Intrinsics Or Helpers

Use binary-memory intrinsics for hot direct access:

```rexx
hash = <at..u32>(node + NODE_HASH) arena
if <compare..binary>(arena, key_offset, wanted_key) = 0 then say "hit"
```

Use ordinary helper functions for buffer management and mutation:

```rexx
call binresize arena, NODE_SIZE * 128
call binmemmove arena, dst_offset, src_offset, count
call bincopy target, 0, source, source_offset, length
```

The compiler may direct-lower selected helpers, but the source still reads as an
ordinary action. That is deliberate: moving, resizing, and opening gaps are
operations, not field references.

## Layout Constants

Write layout constants in bytes. This makes offset arithmetic visible and keeps
the source independent of the storage type used for each field.

<!-- rexx-example name="packed-binary-layout" test="pending" -->
```rexx
options levelb
namespace packedindex expose find_node NODE_LEFT NODE_RIGHT NODE_HASH NODE_KEY NODE_SIZE

packedindex_layout: procedure expose NODE_LEFT NODE_RIGHT NODE_HASH NODE_KEY NODE_SIZE
  constant NODE_LEFT = 0
  constant NODE_RIGHT = NODE_LEFT + <sizeof..u32>
  constant NODE_HASH = NODE_RIGHT + <sizeof..u32>
  constant NODE_KEY_LEN = NODE_HASH + <sizeof..u32>
  constant NODE_KEY = NODE_KEY_LEN + <sizeof..u16>
  constant NODE_SIZE = 32
  return
```

For reusable modules, expose public procedures and any constants callers need.
Declare constants inside an explicit procedure scope. Top-level executable code
in a library-shaped file can synthesize an implicit `main()`, which is not
usually what a packed-layout module wants.

For script-style examples that need a separate layout-constant declaration
procedure, add an explicit `main: procedure`. Otherwise the statements after the
declaration procedure are part of that procedure body until the next callable
boundary.

## Header Check

Binary constants and binary variables use the same direct-access syntax for
reads and compares.

<!-- rexx-example name="packed-binary-header-check" test="pending" -->
```rexx
options levelb

check_header: procedure = .int
  arg page = .binary
  constant MAGIC = "4352584944583031"x as .binary  /* CRXIDX01 */

  if <compare..binary>(page, 0, MAGIC) = 0 then return 1
  return 0
```

Use `=` only when comparing already-materialized values:

```rexx
magic_copy = <at..binary>(0, <blen>(MAGIC)) page
if magic_copy = MAGIC then say "copied compare"
```

For lookup loops, prefer `<compare..binary>` so the compiler can emit a direct
binary compare.

## Sorted Table Lookup

This example shows the intended pattern for a packed sorted table. The table
header stores the row count. Each row has a hash and a variable-size key area.
The example omits the full binary-search loop to focus on field access.

<!-- rexx-example name="packed-binary-sorted-table-lookup" test="pending" -->
```rexx
options levelb

find_row: procedure = .int
  arg table = .binary, wanted_hash = .int, wanted_key = .binary

  constant HEADER_COUNT = 0
  constant HEADER_ROWS = 8
  constant ROW_HASH = 0
  constant ROW_KEY_LEN = 4
  constant ROW_KEY = 6
  constant ROW_SIZE = 40

  count = <at..u32>(HEADER_COUNT) table
  do i = 0 to count - 1
    row = HEADER_ROWS + i * ROW_SIZE
    hash = <at..u32>(row + ROW_HASH) table
    if hash = wanted_hash then do
      key_len = <at..u16>(row + ROW_KEY_LEN) table
      if key_len = <blen>(wanted_key) then
        if <compare..binary>(table, row + ROW_KEY, wanted_key) = 0 then
          return row
    end
  end

  return -1
```

The hot loop reads fixed-width fields directly, checks the stored key length,
and compares the key without making a binary slice. A good implementation test
should inspect optimized RXAS for this shape and fail if `binsubstr`, `bslice`,
string extraction, or helper calls appear in the inner loop.

## Insert And Delete Paths

Insertion and deletion usually need memory movement. Keep that as helper-style
source. It may still be direct-lowered, but it does not need an angle-bracket
intrinsic unless profiling proves the helper spelling is a problem.

<!-- rexx-example name="packed-binary-insert-gap" test="pending" -->
```rexx
options levelb

insert_gap: procedure = .void
  arg expose table = .binary, row = .int, count = .int
  constant ROW_SIZE = 40

  old_len = <blen>(table)
  new_len = old_len + ROW_SIZE
  call binresize table, new_len

  move_from = row
  move_to = row + ROW_SIZE
  move_len = old_len - row
  if move_len > 0 then call binmemmove table, move_to, move_from, move_len

  call binfillat table, row, ROW_SIZE, 0
```

The planned `binfillat(table, offset, length, byte)` span-fill helper depends on
whether Release 1 adds direct span fill or implements it as a small helper loop.
Whole-buffer `binfill(table, byte)` is backed by the RXAS `bfill` instruction.

## Text Fields

There are two text layouts:

- fixed-codepoint UTF-8 fields, read with `<at..string>(offset, codepoints)`;
- zero-terminated UTF-8 fields, read with `<at..string>(offset)` and compared
  with `<compare..string>`.

For fixed-codepoint fields, the starting position is a byte offset and the
length is a count of UTF-8 codepoints:

```rexx
name = <at..string>(name_offset, name_codepoints) record
```

For zero-terminated fields, the program must know that the binary format stores
a zero byte after the text. Omit the length to read the field, or use
`<compare..string>` to compare it without allocating a string:

```rexx
keyword = <at..string>(keyword_offset) record
if <compare..string>(record, keyword_offset, "select") = 0 then say "keyword"
```

Invalid UTF-8 signals `UNICODE_ERROR`.

## When Not To Pack

Prefer ordinary Rexx values when:

- the data is small and not in a lookup hot path;
- field layout is not externally visible;
- text processing is mostly Unicode-aware string work;
- the code would need many manual offsets but little measurable speedup;
- a class or array would express the model more safely.

Packed binary memory is a performance tool. Use it when it removes real copying
or lets a data structure stay cache-friendly.

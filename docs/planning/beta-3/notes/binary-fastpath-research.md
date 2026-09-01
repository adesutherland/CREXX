# Binary Fast-Path Research

Status: first measurement pass completed on 2026-07-09.

## Question

The binary-memory surface proved usable in lexer and packed treemap trials, but
the treemap variants showed that packed scalar metadata does not automatically
beat the optimized register/array representation. This pass isolated the VM
instruction fast path from tree rotations, string comparisons, and algorithmic
effects.

The questions were:

- Are `bget*` and `bset*` themselves slow?
- Is bounds checking the dominant cost?
- Should endian handling stay inside the VM instruction or be exposed as a user
  concern?
- What advice should users get for Release 1 packed binary memory?

## Implementation Review

The RXAS binary-memory instructions are strict and byte-addressed. Fixed-width
numeric fields use canonical little-endian storage, independent of host byte
order.

Before this pass, the VM read/write helpers assembled every fixed-width field
byte-by-byte:

- `rxvm_binary_read_le_bytes()` looped over `width` bytes.
- `rxvm_binary_write_le()` looped over `width` bytes.
- Each instruction still performed the required range and value checks.

That byte loop is portable, but it is unnecessarily conservative for common
widths. The VM now keeps the same strict semantics but uses fixed-width
`memcpy` load/store paths for 1, 2, 4, and 8 byte fields, byte-swapping only on
known big-endian hosts. This avoids unaligned-load undefined behaviour while
letting optimized builds lower the common little-endian case efficiently.

Endianness remains a storage-format rule, not a host rule:

- `<at..u32>` always means canonical little-endian `.u32` binary-memory storage.
- If a file, protocol, or network payload is big-endian, the Rexx program or a
  future helper must say so explicitly.
- The VM should not infer external byte order from the host architecture.

## Benchmark

`tests/performance/binary_fastpath_compare.crexx` compares:

- `.int[]` indexed attribute-array writes and reads;
- `<at..u8>` writes and reads;
- `<at..u32>` writes and reads;
- `<at..int>` / `bgeti64` writes and reads.

The benchmark is an observational smoke test. It has no performance threshold.

Focused validation:

```sh
cmake --build cmake-build-release --target rxvm performance_binary_fastpath

ctest --test-dir cmake-build-release \
  -R 'binary_fastpath_compare|binary_memory|rxfnsl_tinyexpr_binary_surface_smoke' \
  --output-on-failure
```

## Results

Release build on local macOS arm64, 2,000,000 cells, safe bounds checks enabled
after the fixed-width load/store change:

| Operation | Time |
| --- | ---: |
| `.int[]` write | 93,559 us |
| `.int[]` read | 44,800 us |
| `.u8` write | 16,942 us |
| `.u8` read | 12,539 us |
| `.u32` write | 9,829 us |
| `.u32` read | 13,413 us |
| `.int` / `.i64` write | 10,298 us |
| `.int` / `.i64` read | 12,709 us |

Two earlier safe Release runs after the same change were in the same range for
binary operations:

| Operation | Run 1 | Run 2 |
| --- | ---: | ---: |
| `.u8` write | 16,374 us | 16,488 us |
| `.u8` read | 12,985 us | 12,875 us |
| `.u32` write | 10,001 us | 11,599 us |
| `.u32` read | 12,466 us | 13,203 us |
| `.int` / `.i64` write | 10,698 us | 10,899 us |
| `.int` / `.i64` read | 12,497 us | 13,242 us |

This shows the scalar binary instructions are not inherently weak. In this
isolated loop, they are substantially faster than indexed `.int[]` attribute
array access. The treemap result is therefore best explained by whole-source
shape: extra manual offset arithmetic, repeated field access patterns, and the
algorithm's use of several scalar fields per logical action.

## Bounds-Check Experiment

A temporary local patch removed the upper-bound checks from
`rxvm_memory_range()` while preserving the negative-offset check. This unsafe
experiment was reverted immediately after measurement.

Release build on local macOS arm64, 2,000,000 cells:

| Operation | Safe run | Unsafe run 1 | Unsafe run 2 |
| --- | ---: | ---: | ---: |
| `.u8` write | 16,942 us | 16,345 us | 16,884 us |
| `.u8` read | 12,539 us | 11,916 us | 12,377 us |
| `.u32` write | 9,829 us | 9,612 us | 9,762 us |
| `.u32` read | 13,413 us | 12,012 us | 12,285 us |
| `.int` / `.i64` write | 10,298 us | 9,441 us | 9,737 us |
| `.int` / `.i64` read | 12,709 us | 12,017 us | 12,374 us |

The gain from removing checks was modest. Bounds checks are not the dominant
cost in this benchmark and should remain part of the Release 1 instruction
contract. The safety behaviour is important: packed-memory bugs should signal
cleanly rather than corrupting VM memory.

## Guidance

Use binary memory when it avoids copying or expresses dense byte-oriented data:

- generated lexers and parsers scanning a source buffer;
- token streams and fixed-width record tables;
- lookup tables that compare keys in place with `<compare..binary>` or
  `<compare..string>`;
- large binary constants where direct field reads avoid copying constant data;
- protocol or file structures where the layout is already byte-addressed.

Do not assume binary memory will beat normal values for every scalar structure.
If the data structure repeatedly reads several small fields per logical step,
manual offset calculation can offset the win from direct binary loads. In those
cases, keep an array/register version and measure.

Use source shapes that help both compiler and VM:

- compute row or node base offsets once per loop iteration;
- use `<compare..binary>` and `<compare..string>` instead of extracting slices
  before comparing;
- prefer fixed-width fields that match the real format, such as `.u32` for
  node handles when the range is known;
- resize buffers outside hot loops when practical.

Do not use host endian assumptions in source. Release 1 fixed-width binary
fields are little-endian by definition. Big-endian file or network structures
need explicit source-level conversion or future endian-specific helpers.

## Follow-Up

- Add a generated-lexer or JSON-parser benchmark where binary memory avoids
  string slicing and materialization.
- Consider source-level endian helpers for external formats, for example
  `binbeu32()` or a future intrinsic spelling, if real parsers need them.
- Consider compiler common-subexpression cleanup for repeated
  `row + FIELD_OFFSET` expressions in hot binary-memory loops.
- Keep bounds checks in Release 1; revisit only with a formal unsafe mode and a
  much larger measured win.

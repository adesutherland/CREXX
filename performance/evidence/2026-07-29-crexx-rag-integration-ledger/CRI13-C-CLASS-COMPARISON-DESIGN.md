# CRI-13 Option C class comparison design

Status: **approved benchmark-local comparison active**

Date: 2026-07-30

Governed ID: `CAP-01-J02-C01`

## Decision already made

Adrian accepted headerless raw Option B as the production storage/performance
primitive and approved a subsequent measured comparison with Option C classes.
The classes must use specific typed read and write members with values passed
and returned by value. No in-place comparison operation, case conversion,
metadata header or new serialized format is part of the comparison.

This rung is deliberately benchmark-local. It does not select public class
names, invalid-instance behavior, equality, persistence, future numeric widths
or a `.jsondocument` method returning a class. Promotion remains a later public
API decision informed by the result.

## Compared mechanisms

### B — accepted raw `.binary`

- headerless canonical-little-endian bytes;
- direct `<at..f32>` and `<at..i64>` reads and writes;
- explicit type and count owned by the caller; and
- the accepted `node_f32_array` / `node_i64_array` projection remains unchanged.

### C — benchmark-local typed owning wrappers

Two private benchmark classes own a by-value `.binary`, derive count from byte
length, and provide:

```text
count() = .int
read(index) = .float or .int
write(index, value) = .void
payload() = .binary
```

`read` returns one scalar by value. `write` accepts the scalar by value and
mutates only the wrapper-owned payload. `payload` measures the explicit
by-value bridge back to Option B rather than pretending that ownership has no
cost. The hot methods add no case conversion, string work, comparison surface
or class-level per-access bounds pass; the existing typed binary-memory access
retains its current semantics.

## Evidence and verdict rules

The maintained `rxjson_numeric_materialization_compare.crexx` remains the sole
numeric benchmark. It will report, separately for f32 and i64:

- wrapper construction/ownership;
- by-value payload extraction;
- repeated typed reads and checksum;
- one complete typed write pass and checksum;
- matched raw B read and write controls;
- JSON parse/B projection/raw scan total; and
- JSON parse/B projection/C construction/C scan total.

Run optimized and non-optimized images on `rxvm` and `rxbvm`. Focused
correctness must prove exact count, bytes, read values, caller isolation after
class writes, payload extraction, write checksums and unchanged B/JSON results.
Release evidence uses two warmups and ten serial rotating recorded samples per
cell with no silent outlier removal.

Interpret optimized C/B hot-read and hot-write medians as follows:

1. `<=1.25x` on both VMs: C is competitive enough to consider as the normal
   ergonomic surface while retaining B for interop;
2. `>1.25x` and `<=2x` in any VM: retain B as the documented hot path and
   consider C only as an additional convenience surface;
3. `>2x` in any VM: do not promote C for performance-sensitive use without a
   separately approved compiler/dispatch countermeasure.

Construction, payload-copy, total-time and RSS results remain separately
visible and can make the recommendation more conservative. Exact correctness,
no material RSS increase, and no optimizer-induced inversion above 10% are
required. Stop at the first ordinary Release verdict before public promotion,
further optimization, broad post-C QA, CRI-14 or `PERF2-07-C01`.

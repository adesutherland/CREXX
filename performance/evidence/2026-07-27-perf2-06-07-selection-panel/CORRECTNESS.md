# PERF2-07-V3-R01 correctness disposition

## Reproduction

The retained PERF2-04 source has SHA-256
`5e7d3e0bddc428038d40f567e8f5cbf640ff7e68532257735664ba8c0fb7e65a`.
It initializes a destination string as `""`, copies decimal `"2.2"` into the
same value, executes `DTOS`, then asks `STRLEN` for codepoint length. The
frozen images were:

| Mode | RXAS SHA-256 | RXBIN SHA-256 |
| --- | --- | --- |
| no-opt | `a8b5c1a9e6627d2a0d1a1f571ad6b67f7cd36ce7677d1e240a1b402f31021309` | `721b42f1960d88b759e3adc4a738af64e705432548b77a11236dbf8bf66f854b` |
| optimized | `ac566fc14ac4968d565ddf455121f089218d885ebdf8c7d3284663e2442898a6` | `d8bfcfc6302521338c7467c1580f422eb7205e12beaeb998cc2eff6423162a2d` |

All four cells failed identically:

| VM | no-opt | optimized |
| --- | --- | --- |
| `rxvm` | `FAIL: direct STRLEN result mismatch: 0 source=2.2` | same |
| `rxbvm` | same | same |

The bounded representation trace was also identical. Immediately before
`DCOPY`, the destination contained an empty string with byte length `0`,
codepoint length `0`, and both VM-private UTF-8 validity bits set. `DCOPY`
changed the decimal representation but deliberately preserved the other
representations. `DTOS` then wrote bytes `32 2e 32` (`2.2`) and byte length
`3`, while leaving `string_chars=0`, the cursors at zero and status `0x3`.
`STRLEN` trusted `RXFLAG_VM_UTF8_COUNT_VALID` and returned the stale `0`.

## Correction and exact contract

The correction is representation bookkeeping only. It adds two private VM
completion helpers:

- `finish_string_write(value, length)` sets the byte length, resets both byte
  and codepoint cursors, validates/recounts the resulting byte span in UTF
  builds, and replaces only the VM-private UTF validity bits.
- `finish_ascii_string_write(value, length)` sets byte length and exact
  codepoint count to `length`, resets both cursors and marks the new ASCII span
  valid.

Both helpers preserve compiler/public type flags and all other value
representations: integer, float, decimal, binary/native, object and reference
identity. Existing buffer ownership and capacity are retained. A conversion
through a live reference changes the referenced storage's materialized string
representation without detaching or invalidating that reference identity.

There is no language, compiler syntax, public RXAS, RXBIN or ABI change.

## Sibling in-place audit

The audit covered every current interpreter site that can replace or expose
string bytes while an old codepoint cache might exist.

| Disposition | Operations |
| --- | --- |
| Already complete or safe | `set_string*`, `ITOS`, `FTOS`, `BTOS`, `BINTOS`, `FEXTR`, text reads, stem materialization, concat/append/slice, upper/lower |
| Missing completion, corrected | `DTOS`, `DEXTR`, `XTIME N`, legacy `TRIMR`/`TRIML`, current `TRIMR`/`TRIML`/`TRUNC` |
| Normalized to explicit known-ASCII completion | `FFORMAT`, `HEXCHAR` |

The distinguishing maintained Level B regression covers:

- initially empty and initially non-empty Unicode destinations;
- typed null;
- a live reference alias to the destination;
- integer, float and boolean sibling conversions;
- decimal coefficient extraction; and
- optimized/no-opt images on both VMs, with an RXAS assertion that the
  distinguishing `dcopy; dtos; strlen` sequence survives.

After the correction, all four retained cells report source `2.2` and length
`3`; the trace reports `string_length=3`, `string_chars=3`, both cursors zero
and status `0x3`. The focused regression and its related 12-test compiler/
interpreter group pass. Raw before/after output and bounded traces are under
`raw/correctness/`.

## Performance disposition

V3-R01 is accepted as a correctness prerequisite only. The current VM has no
general numeric/string representation-hit cache: conversion opcodes still
materialize on each execution, so the measured mechanism count for broad cache
retention is zero. A future selective retention proposal would need a separate
numeric-context and memory-growth contract and fresh RexxCPS evidence; it does
not ride on this correctness fix.


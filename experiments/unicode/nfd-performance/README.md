# Unicode NFD Table-Layout Performance Experiment

Status: clean-host experimental evidence captured on the `unicode` branch;
product selection remains open. This experiment compares execution
representations; it does not select a product format or change the compiler,
VM, Level B string contract, or Level G API.

## Question

Does a host-native `<packed..int>` representation materially improve the
existing portable `<at..u8/u16/u32>` NFD proof on the complete Unicode 17.0.0
normalization workload, and how does the resulting Level B implementation
compare in broad magnitude with macOS CoreFoundation normalization?

The packed control preserves the portable proof's logical shape and algorithm:

- direct code-point-to-CCC lookup;
- high-byte mapping pages;
- binary search within one page;
- shared decomposition values;
- recursive canonical and algorithmic Hangul decomposition; and
- stable canonical combining-class ordering.

All packed logical fields, including CCC values, use `<packed..int>`. This is a
deliberately direct representation comparison, not the likely final memory
layout. It makes native access cheap but expands the dense CCC table from one
byte per code point to one host integer per code point. A later hybrid may keep
byte CCC data while packing only mapping indexes and values if the evidence
supports that design.

## Measurement Contract

- Build an ordinary profiling-off `Release` product and compile optimized
  Level B benchmark images through `rxc`, `rxas`, and `rxlink`.
- Use every 20,034 source/result pair from Unicode 17.0.0
  `NormalizationTest.txt`, specifically `c1 -> c3`.
- Preload and decode the corpus outside the timed region.
- Require every result to match `c3` before any timing sample.
- Run two untimed warmups and ten recorded serial samples per cell.
- Report concrete `rxtvm` and `rxbvm` results separately.
- Time one complete traversal of all 20,034 rows per sample and retain the raw
  microsecond values plus an observable checksum.
- Separately join all rows with U+0020, whose CCC is zero and which has no
  decomposition, then repeat that closed block 1, 8, and 64 times. These are
  single-call UTF-8 buffers of 101,506, 812,048, and 6,496,384 input bytes;
  report input-byte throughput and exact Unicode 17 output for both CREXX
  layouts. This sustained-buffer lane complements rather than replaces the
  call-heavy row lane. The semantically current CREXX implementations run all
  three sizes by default.
- Record table byte size, one separately labelled preparation observation,
  process peak RSS, host/build/tool identities, and the exact dirty scope.
- Do not remove outliers.

Two Apple controls keep the representation and integration costs explicit:

- the standalone control prebuilds immutable `CFString` source and expected
  values, then times `CFStringCreateMutableCopy`, `CFStringNormalize` with
  `kCFStringNormalizationFormD`, complete UTF-8 output materialization,
  UTF-8 scalar/byte checksum observation, and release for each row; and
- the RXPA control is called by the same optimized Level B benchmark loop as
  the CREXX implementations. It accepts a real UTF-8 `.string`, pays RXPA's
  ordinary string copy-out, decodes and normalizes through CoreFoundation,
  materializes UTF-8, and returns a real validated CREXX `.string` into the
  same `actual = ...` assignment and `strlen` observation shape.

The RXPA string ABI is currently NUL-terminated rather than length-bearing.
`NormalizationTest.txt` has no U+0000 input, so this is a valid bounded control
for the selected corpus, not a proposed general Unicode API. CoreFoundation's
internal representation remains platform-owned. The standalone result is a
UTF-8-producing library sense-check; the RXPA result is the meaningful
end-to-end native ceiling for this Level B call surface.

The large-buffer RXPA cells normalize the same prebuilt UTF-8 `.string` values
in one call. Their output is checked for repeatability and compared with the
Unicode 17 expected buffer, while the two known CoreFoundation version
differences remain reported rather than accepted as Unicode truth. They run
the 1- and 8-block sizes by default. The 64-block control remains available
through `CREXX_NFD_PERFORMANCE_APPLE_BUFFER_REPETITIONS`, but one retained
exploratory sample took tens of seconds and does not justify dominating the
clean session for an outdated, non-conforming host library.

## Optimized RXAS Audit

The current optimized output confirms that the 25-times raw-library gap is not
a simple string-concatenation mistake:

- the caller receives `normalize()` directly into its `actual` register via
  `settpswapcall`; there is no following `scopy`, so the source-level
  assignment is already result-coalesced;
- both CREXX layouts emit the same 59 executable instructions in
  `normalize()` outside their called helpers;
- `appendchar` grows the VM string buffer geometrically (32 bytes minimum,
  then powers of two), so output construction is amortized linear rather than
  repeated whole-string concatenation; and
- sequential `strchar` uses the VM's private UTF-8 character-position cache,
  so the input scan is linear rather than repeatedly rescanning from byte zero.

The proof algorithm is nevertheless intentionally naive and call-heavy. The
20,034 conformance rows average only 1.553 input scalars and 2.766 output
scalars (maximum eight), so the benchmark performs 20,034 complete public
normalizer calls to process just 31,113 input and 55,422 output scalars. Each
output scalar still enters a separate `_append` method; each non-Hangul
decomposition node enters `_find_mapping` and a separate scalar-validation
procedure; recursive decomposition allocates/recycles bytecode frames; and
every call performs status/error housekeeping plus a canonical-order pass.
`_append` checks and links capacity attributes on all 55,422 output scalars
even though its 64-scalar work buffer never grows for this corpus. CCC helpers
inline, but the optimized form still repeats receiver assertions and attribute
link/unlink operations around table reads.

The large-buffer lane distinguishes that per-scalar work from the public-call
cost. It forces the normalizer's reusable scalar work buffer and returned UTF-8
string to grow well beyond their initial capacities, while the U+0020 boundary
keeps independently normalized corpus rows safe to concatenate.

Returning the bytecode normalizer's locally built string also transfers that
local buffer to the caller and discards the caller's prior result buffer. A
native RXPA procedure writes directly into the caller's return register, whose
string capacity can be reused. That is a real current compiler/VM cost in the
Level B return path, not a table-format result.

These observations make the current algorithm a correctness oracle and useful
surface probe, not an "as good as it gets" NFD implementation. The next
algorithm experiments should be separately measured: pre-expand recursive
canonical decompositions when compiling the table; eliminate trusted-data
scalar/cycle checks from the hot path after table validation; track whether
canonical order is already satisfied; and compare a caller-owned/reusable
UTF-8 output builder with the ordinary string-return form. Any new builder or
return-buffer language/VM surface remains a later design decision.

## Clean AC Observation — 2026-08-25

Adrian confirmed that the host was clear. The profiling-off Release session
ran on AC with no recorded thermal or performance warning, two warmups and ten
serial samples per completed cell. Every CREXX result matched Unicode 17.0.0
exactly. Raw-sample spread was 0.29–4.28% across the CREXX cells and below 1.8%
for every sustained-buffer cell.

| Workload | UTF-8 input | `rxtvm` portable | `rxtvm` packed | `rxbvm` portable | `rxbvm` packed |
| --- | ---: | ---: | ---: | ---: | ---: |
| 20,034 separate rows | 31,113 scalars | 48.797 ms | 45.525 ms | 46.542 ms | 43.940 ms |
| one joined block | 101,506 bytes | 1.61 MB/s | 1.80 MB/s | 1.72 MB/s | 1.92 MB/s |
| eight joined blocks | 812,048 bytes | 1.62 MB/s | 1.82 MB/s | 1.73 MB/s | 1.94 MB/s |
| 64 joined blocks | 6,496,384 bytes | 1.64 MB/s | 1.82 MB/s | 1.75 MB/s | 1.94 MB/s |

The sustained rates remain nearly constant across 64-times input growth. This
confirms amortized-linear string/work-buffer growth and makes per-scalar
decomposition, lookup, validation, method/frame, and ordering work the next
optimization target. Packed tables are an observed 9.7–11.2% faster than the
portable representation on the sustained buffers in this session, and
5.6–6.7% faster on the row workload. This is not yet a paired-interleaved
format-selection verdict.

The packed image is 9,074,136 bytes versus 1,186,472 bytes portable: 7.65
times the table size. Peak process RSS at 64 blocks was about 587 MB packed
versus 576 MB portable. A later hybrid should therefore test packed mapping
indexes with byte-sized dense CCC data rather than treating all-packed as the
obvious product layout.

The Apple controls remain non-conforming sense checks. CoreFoundation missed
two Unicode 17 rows. Through RXPA it took about 11.2 ms for one 101,506-byte
buffer but about 710–711 ms for 812,048 bytes. One retained exploratory
6,496,384-byte sample took 44.574 seconds, so that obsolete control was omitted
from the clean 64-block matrix. The standalone row control, which excludes the
CREXX ABI, had a 3.345 ms median.

Raw output, RSS, pre/post host state, and the consolidated summary are retained
under
`cmake-build-unicode-release/nfd-performance-evidence/2026-08-25-ac-clear-large-buffer/`.

## Prepared Algorithm Rough Comparator

The comparator has an explicit build/run split. `prepare_prepared_compare.sh`
builds the Release product, compiles and links the benchmark modules, generates
the deterministic thin Level B wrapper on both VMs, and records source,
product, and bytecode fingerprints in `prepared.manifest`.
`run_prepared_compare.sh` accepts that prepared directory and invokes only the
two VMs. It refuses missing or fingerprint-mismatched artifacts, so compilation
cannot leak into a timed session.

The default comparison is the original scalar NFD algorithm against the
prepared-symbol NFD implementation. The latter iterates CREXX `.string`
codepoints, reads one dense portable `<at..u32>` classification, and dispatches
exact synthetic kinds through the shared bounded-register Level B executor.
The retained generated wrapper calls that executor; the current route does not
build a UTF-8 byte DFA. Both variants use the same optimized
`.string -> .string` call and result-observation shape.

By default the run-only harness uses one warmup, three serial samples, and three
alternating-order rounds on both Release VMs across 20,034 separate conformance
rows and one joined 101,506-byte buffer. Every invocation checks its complete
result against Unicode 17 before timing. Larger joined buffers are selected
with `CREXX_UNICODE_COMPARE_BUFFER_REPETITIONS`, and contextual `prepared-nfd`
and `prepared-nfc` cells remain available through
`CREXX_UNICODE_COMPARE_VARIANTS`. This is a directional algorithm screen, not a
formal clean-host product-selection verdict.

```sh
experiments/unicode/nfd-performance/prepare_prepared_compare.sh
experiments/unicode/nfd-performance/run_prepared_compare.sh
```

### Prepared-symbol recovery result — 2026-08-27

The complete Level L/Unicode job passes optimized and noopt images on both VMs:
400,680 four-form corpus invariants, 100,170 generated NFD relations, 1,094,978
unlisted-scalar identity checks per form, focused fixtures, and the independent
C++/re2c oracle. Disassembly confirms a 32-local `normalize()` containing
`STRCHAR`, `BGETU32`, `.jtable ... acph`, `JUMPI`, and `APPENDCHAR`. Its NFD
fast path returns before the general string-to-binary conversion.

On AC under variable load, median paired prepared/canonical ratios were:

| Workload | UTF-8 input | `rxtvm` ratio | `rxbvm` ratio |
| --- | ---: | ---: | ---: |
| 20,034 separate rows | 31,113 scalars | 0.8949 | 0.9239 |
| one joined block | 101,506 bytes | 0.6564 | 0.5842 |
| eight joined blocks | 812,048 bytes | 0.5239 | 0.5424 |
| 64 joined blocks | 6,496,384 bytes | 0.5178 | 0.5114 |

The short-row result restores the earlier provisional parity result; the
sustained lanes show the prepared route at roughly twice canonical throughput.
These are indicative rather than clean-host verdicts, but the former 8–100x
slowdown has disappeared. Full samples, fingerprints, host state, and the
recovery explanation are retained in
`evidence/2026-08-27-prepared-symbol-nfd-recovery.txt`.

### Rejected UTF-8 DFA result — 2026-08-26

The preceding experiment generated a 3,440-state UTF-8 DFA. It was conformant,
but its method had 6,455 locals and it was 84–101x slower on rows and about 9x
slower on a 101,506-byte buffer. That historical result describes the rejected
byte-DFA successor, not the prepared-symbol codepoint algorithm above. Its
exact evidence remains in `evidence/2026-08-26-generated-nfd-rough.txt`.

## Acceptance Boundary

Both CREXX layouts must pass the complete selected relation set. Both Apple
controls report every Unicode 17.0.0 mismatch before timing; if the system
Unicode data is older, their timing remains only a same-input native-platform
sense-check and must not be presented as semantically equivalent. A speed
result cannot approve a host-native committed format, cross-architecture
behavior, a compiler/VM change, implicit normalization, NFC, or a public
Unicode API. Any production selection requires a separate design decision
after size, preparation, steady-state, portability, both-VM evidence, and the
algorithm/return-path costs above are considered together.

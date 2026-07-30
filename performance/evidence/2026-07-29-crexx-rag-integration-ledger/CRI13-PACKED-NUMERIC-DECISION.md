# CRI-13 parse-once JSON and packed numeric decision

Status: **decision-blocked**

Date: 2026-07-30

Governed ID: `CAP-01-J02`

## Exact decision boundary

CRI-09 already delivered and validated the final parse-once JSON document,
node traversal, typed getters and noisy-container scanner. CRI-13 does not need
a competing document or parser. Its remaining question is whether CREXX should
add an explicit bulk conversion from a JSON array node to packed numeric bytes,
and what public representation that conversion should select.

The decision must not infer numeric width, vector meaning, normalization,
dimension, or storage from JSON alone. Ordinary JSON traversal remains valid
and unchanged. Any packed conversion is requested explicitly by the caller.

## Existing JSON benchmark: effect of the accepted infrastructure

The primary before/after comparator remains the unchanged
`tests/performance/rxjson_parser_compare.crexx`, SHA-256
`f4ce6bcd3b53f9be3a7ffab3357e99d39b5687562bed43dac1b385276718257b`.
The accepted A2 Release run used its normal 60-row, 4,394-byte, 30-operation
workload with three warmups and 12 balanced samples per product/VM.

Compared with the pre-edit product, the compatibility functions changed as
follows:

| VM | `valid` | deep get | tail get | `count` | `members` |
| --- | ---: | ---: | ---: | ---: | ---: |
| `rxvm` | +17.59% | +6.45% | +12.29% | **-40.18%** | +12.82% |
| `rxbvm` | +17.82% | +9.84% | +12.01% | **-38.77%** | +12.47% |

All cells passed the predeclared 25% compatibility guard. The new retained
surface changes the repeated-use result much more materially:

- one construction plus 30 path gets is **76.25% faster** on `rxvm` and
  **79.05% faster** on `rxbvm` than 30 legacy reparse-per-call gets;
- one construction plus 30 reads of a once-resolved node is **93.87%** and
  **94.11% faster**; and
- noisy structural scanning is 394.5 us and 480.5 us on the two VMs instead of
  the reproduced multi-thousand-parser workaround.

The exact existing-benchmark products, samples, medians and calculations are
in [`CRI09-A2-RELEASE-VERDICT.md`](CRI09-A2-RELEASE-VERDICT.md). They are
reused rather than rerun because the accepted production `rxjson.crexx` and
benchmark are unchanged; CRI-10 through CRI-12 changed tests and documentation,
not that product.

## Current minimized numeric workload

The maintained Level B diagnostic is
`tests/performance/rxjson_numeric_materialization_compare.crexx`, SHA-256
`250789426e3b7ec51f9d4e4e86ed1edfb66babe0a0cb2cfdca74d906dd0c3e03`.
It uses only current public APIs and existing binary-memory intrinsics. Its raw
headerless little-endian buffers are controls, not a selected format.

The exact workload is:

- 3,072 binary32-compatible JSON numbers, using the retained provider-shaped
  spelling `0.5000000000000000`;
- 3,072 exact signed-integer JSON numbers;
- ten repeated typed traversals and ten direct packed scans;
- a separately bounded 128-number legacy extraction, with no extrapolation;
- first parse, sequential `element()` control, `children()` materialization,
  typed materialization, typed copy, raw f32/i64 packing, direct packed scans,
  JSON encoding, parse/pack/scan total, source/index/packed bytes, checksums and
  external peak RSS; and
- nested object metadata, Unicode, null and empty containers in the large float
  source. The maintained CRI-09 functional tests separately cover malformed
  input, missing/null distinctions, arbitrary nested traversal, numeric extrema
  and range failures.

Focused Debug registration passes 5/5: the runtime fixture plus opt/no-opt on
both `rxvm` and `rxbvm`. Raw log:
`/tmp/cri13-benchmark-focused-ctest-final.log`, SHA-256
`dc6c8f0f3206ad1a46ae82b00a636a833fce12aa47fddfe43f9a597b277aa534`.

The ordinary profiling-off Release product is the accepted A2 product at
`/tmp/crexx-cri09-release-a2.N4ELYs/build`. The numeric images are:

| Image | SHA-256 |
| --- | --- |
| non-optimized RXBIN | `b1aed16185bf34f0bdbb5541ac689d2cf10abf893d7a0769eeac620e4f73ab29` |
| optimized RXBIN | `35cf6bdc924f7474350b1b51b3800e04e97f0212a80b9b64d3978b36e568d33e` |
| non-optimized RXAS | `b483f9281573b9881c2de5a368a4af9b26ca2055cdc4a84609830dee9545f354` |
| optimized RXAS | `fcf82ae700d05196addbbabcfa00925818db056828f31d102806943a6c84768b` |

Representative compile/run commands:

```sh
product=/tmp/crexx-cri09-release-a2.N4ELYs/build
scratch=/tmp/crexx-cri13-release.uSmoLW

"$product/bin/rxc" -i "$product/bin" -n -o "$scratch/cri13_noopt" \
  tests/performance/rxjson_numeric_materialization_compare.crexx
"$product/bin/rxas" -n -o "$scratch/cri13_noopt" "$scratch/cri13_noopt"

/usr/bin/time -l "$product/bin/rxvm" "$scratch/cri13_noopt" \
  "$product/bin/library" -a 3072 10
```

Formal absolute diagnostics used two warmups followed by ten serial recorded
samples per opt/no-opt and VM cell. The starting cell rotated each round; no
outlier was removed. All 40/40 recorded runs passed exact checksums. The host
was on AC with low-power mode off. Exact environment, power, tool, product,
source/image hashes and sampling contract are in
`/tmp/crexx-cri13-release.uSmoLW/manifest-final.txt`, SHA-256
`3b8936ef0df16758f9c22a9ca560a7fa45b92dd25bd5ef28412e69926ac9c397`.

### Current public composition medians

Times are microseconds. `total` is one parse, one child-ID materialization, one
direct JSON-node-to-f32 packed conversion and one packed scan. JSON encoding is
reported separately.

| Cell | first parse | `children` | f32 typed materialize | f32 pack after materialize | ten packed scans | JSON encode | total | peak RSS bytes |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| `rxvm -n` | 1,623 | 4,734 | 21,479 | 36 | 290 | 2,956.5 | 27,752.5 | 27,336,704 |
| `rxvm opt` | 1,693 | 4,656.5 | 20,929 | 34.5 | 283.5 | 2,987.5 | 27,138.5 | 27,435,008 |
| `rxbvm -n` | 2,003 | 4,908 | 22,084.5 | 51.5 | 357 | 3,196 | 29,157.5 | 27,361,280 |
| `rxbvm opt` | 2,016.5 | 4,866.5 | 21,724.5 | 50 | 345 | 3,257.5 | 28,207 | 27,418,624 |

There is no optimizer-induced inversion in the total. The optimized cells are
2.21% (`rxvm`) and 3.26% (`rxbvm`) faster than their non-optimized partner.
Raw samples and reduction:

- `/tmp/crexx-cri13-release.uSmoLW/current-formal-final-raw.log`, SHA-256
  `26f7f01c8440ac7181db1a7ddc8cc6c3684ff051396af1c1b01354c4ad2f7dfe`;
- `/tmp/crexx-cri13-release.uSmoLW/current-final-samples.tsv`, SHA-256
  `fe43526b35051114594db3035454a5be12d63e96b80f9f651a7386226931a8d3`;
- `/tmp/crexx-cri13-release.uSmoLW/current-final-medians.tsv`, SHA-256
  `22a7a925728388a32fc5ef44d87eff2530fa6cf7ba727be71e71e66390c14fc4`.

## Root cause and mechanism evidence

The parse-once document is not the bottleneck. The public composition performs
three expensive operations before a hot packed loop can start:

1. sequential `element(parent, index)` calls restart from the first sibling,
   so a complete indexed walk is quadratic; the retained one-pass controls are
   88.0--100.7 ms at the medians;
2. the correct public traversal calls `children()` once, but that materializes
   3,072 document-local node IDs into an ordinary `.int[]`; and
3. conversion then makes 3,072 public `node_float` or `node_int` method calls,
   repeatedly decoding numeric lexemes into ordinary Level B value slots.

The node-ID list is valid only while its owning immutable document is retained.
It is a snapshot of IDs, not an independent iterator or value tree. Ordinary
array/object traversal remains correct; it is simply the wrong representation
for a bulk numeric hot path.

Ordinary typed arrays are also not packed arrays in the VM. On this exact
Apple build, `sizeof(value)` is 248 bytes and a 3,072-element array grows to a
4,096-element capacity. Its value storage plus the two pointer arrays is at
least 1,081,344 bytes before the parent header and allocation bookkeeping.
The raw controls are 12,288 bytes for f32 and 24,576 bytes for i64: about 88x
and 44x smaller respectively. This is an exact current-build calculation but
not a cross-platform `sizeof(value)` promise. The retained size probe and
executable hashes are:

```text
be1bdfd73fefa17cabd350a4e8765ebf50b1679ac83de1bde7a1b9f7c5ca01f8  /tmp/crexx-sizeof-value.c
898074ab56600d9ceec3d0a902e01bf72636ec5ce8c332193e9930e1c9b7312c  /tmp/crexx-sizeof-value
```

Once a typed array already exists, copying it takes only 142.5--204.5 us and
packing it takes 34.5--51.5 us. Directly scanning the packed f32 control ten
times takes 283.5--357 us. The same ten traversals through public document
typed getters take 207,829--221,146 us, a 619x--733x difference. This confirms
the CRI-02 conclusion: obtain the packed payload once, then use direct typed
binary-memory access in the hot loop.

### Integrated-projection prototype

The read-only incubation was mechanically renamed only in scratch so its
`jsondocument` class could coexist with the now-production class. The sister
checkout was not changed. The prototype walks its private sibling chain and
writes packed values inside one document method. It retains the incubation's
16-byte envelope solely as a measured mechanism control; that does not approve
the envelope.

The renamed source and benchmark hashes are:

```text
e7a8d972b9aae3acfad891edafb02ba8daeae615982e2e4acff6965e2c518041  json_document_prototype.crexx
ea6ad9b9c52dfe689e7613b6b4c03888c8d1b7c4c2340209932b675c467d6639  json_document_benchmark_prototype.crexx
```

Current-toolchain correctness passes 4/4 across opt/no-opt and both VMs,
including malformed/truncated payloads, Unicode, null/missing, empty arrays,
signed-i64 extrema and range failure, float32 range failure, corrupt envelope
failure and direct packed access. Raw correctness log:
`/tmp/crexx-cri13-release.uSmoLW/incubation-current/prototype-correctness.log`,
SHA-256
`f4b93836b9be889018111e4aa998a90be9c031e702edac358e50f769c509fb0a`.

The formal prototype also passed 40/40. One 3,072-element f32 projection is
256.65--285.65 us. Combining its measured parse, one projection and one direct
scan is 1,372.75--2,210.95 us, versus 27,138.5--29,157.5 us for current public
composition: a 13.19x--19.77x mechanism difference. Because the prototype has
a separate older parser and an unapproved envelope, this is feasibility and
mechanism attribution, not a candidate verdict.

Raw samples and reduction:

- `/tmp/crexx-cri13-release.uSmoLW/prototype-formal-raw.log`, SHA-256
  `6e8a49cf3624b4f027b4e2f72a847d4a3c991d2b13b9376e94c949b603d94358`;
- `/tmp/crexx-cri13-release.uSmoLW/prototype-medians.tsv`, SHA-256
  `214e5bc423bdcf312f02c4b51009aedc3a2fc16a97434400dcf74929e3a7244c`;
- `/tmp/crexx-cri13-release.uSmoLW/final-comparison.tsv`, SHA-256
  `ed53da9d9f32ac39da572407d24c05bc7ad5248d2084bd46a3c8917ceefc36e1`.

Prototype peak RSS medians are 21.41--22.18 MiB versus 27.34--27.44 MiB for
the current diagnostic. The programs have different module shapes and retain
different simultaneous objects, so this is supporting process evidence, not a
paired memory claim. The exact typed-array versus raw-byte calculation above
is the stronger representation evidence.

## Alternatives

### Option A — retain public composition only

Call `children()`, apply `node_float`/`node_int` for each item, optionally fill
an ordinary typed array, and then pack it with binary-memory writes.

- Correctness: all choices are explicit; current JSON semantics stay intact.
- Compatibility/ABI/format: no change.
- Time: 27.14--29.16 ms for parse/pack/one scan; repeated typed traversal is
  619x--733x slower than direct packed scans in this workload.
- Memory: a 3,072-element ordinary typed array has at least 1,081,344 bytes of
  current-build value/pointer storage, versus 12,288/24,576 raw bytes.
- Maintenance/platform: no new API; portable but every consumer repeats a
  delicate range/error loop and must know the efficient `children()` lifecycle.

This is safe but does not close the reproduced functional/performance gap.

### Option B — explicit bulk projection to existing `.binary` (recommended)

Add two methods to the existing production `.jsondocument`:

```text
node_f32_array(node, expected_count, packed, error) = .int
node_i64_array(node, expected_count, packed, error) = .int
```

`packed` and `error` are exposed outputs. `expected_count` is `-1` to accept
the array's current count, otherwise it is an exact non-negative count,
including zero. The caller therefore chooses width and dimension explicitly;
there is no inference from JSON.

Contract:

- success returns `0`, clears `error`, and replaces `packed` with an owning,
  headerless canonical-little-endian `.binary` of exactly `count * 4` or
  `count * 8` bytes;
- failure returns a stable negative status, clears `packed`, and writes a
  human-readable error including the failing one-based element when relevant;
- invalid document/node, wrong node type, invalid expected count, count
  mismatch, non-number element and numeric conversion/range failure remain
  distinct statuses;
- f32 uses the existing IEEE binary32 storage conversion, rejects overflow and
  a non-zero JSON number that underflows to zero, and never clamps;
- i64 follows the existing `node_int` contract, including exact integral JSON
  forms such as `1.0` and `1e3`, and rejects non-integral or out-of-range input;
- an empty JSON array produces an empty binary when the expected count permits
  it; a nested array, object, string, boolean or null element is rejected;
- no normalization flag, vector meaning, magic, version, reserved field,
  cached specialization or hidden document mutation is added; and
- ordinary document traversal and the existing `jsonarray()` encoder remain
  unchanged. The maintained benchmark demonstrates the reversible generic
  route by reading packed values, materializing JSON numeric text, and calling
  `jsonarray()`; its median encoding cost is 2.96--3.26 ms.

The method name supplies the type at the conversion boundary; byte length
supplies the count afterward. A persisted value must store its type and count
in its owning schema. Raw bytes alone are deliberately not self-describing.

- Correctness: explicit type/count and strict element/range checks; no JSON
  semantic mutation.
- Compatibility: additive Level B methods only. No language syntax, native ABI,
  RXAS/RXBIN or existing serialization changes.
- Time: the integrated prototype establishes a 13.19x--19.77x plausible
  end-to-end mechanism gain. The selected implementation must be measured
  afresh in the production parser.
- Memory: one document plus one exact binary output; no node-ID or typed-value
  array is required on the bulk path.
- Maintenance/platform: two small public methods share current parser numeric
  helpers and canonical cross-platform binary-memory semantics.

### Option C — headerless typed wrapper classes

Add `.packedf32` and `.packedi64` classes that own raw `.binary`, derive count
from byte length, and provide `payload()`, `count()` and diagnostic `value()`;
the document methods return those classes.

- Correctness/type safety: stronger runtime type identity after conversion.
- Compatibility/ABI: additive Level B class metadata, no native ABI or wire
  envelope.
- Time/memory: nearly the same payload cost as Option B, with an object factory,
  ownership/copy and method surface to validate.
- Maintenance: creates a generic packed-numeric library contract beyond the
  JSON need and requires decisions about constructors, invalid instances,
  equality, copying and future widths.
- Platform: portable Level B.

This is viable but broader than the evidence requires. A static `.binary`
boundary plus explicit typed binary-memory access is already the established
Release 1 packed-layout model.

### Option D — adopt the incubation's `F32V`/`I64V` envelope

Promote the 16-byte magic/version/flags/count/reserved header and wrapper
classes, including the historical normalized flag.

- Correctness: self-describing type/count and corrupt/truncated validation.
- Compatibility/format: creates a new public serialized format and versioning
  promise. `normalized` is mathematical-vector metadata that JSON does not
  supply and is not meaningful for a generic i64 sequence.
- Time/memory: the header is only 16 bytes, and the prototype is fast; the
  important gain comes from integrated traversal, not the envelope.
- Maintenance/platform: requires codec evolution, persistent compatibility,
  flag allocation, storage interoperability and migration policy.

This is appropriate only if CREXX itself intends to standardize a durable
portable vector file/wire format. CRI-13 does not provide that requirement.

## Recommendation

Approve **Option B exactly as specified**.

It closes the measured bulk-conversion gap while using the existing public
`.binary` and canonical little-endian binary-memory contract. It keeps type and
dimension explicit, avoids allocating ordinary node-ID and typed-value arrays,
preserves direct packed hot loops proven by CRI-02, and does not smuggle vector
metadata or a new serialized envelope into the JSON library.

Reject Option A because it leaves a 13x--20x mechanism gap and substantial
ordinary-array storage. Defer Option C until a non-JSON consumer demonstrates
that persistent runtime type identity is worth a generic class contract.
Reject Option D for this item because the measured speed comes from bulk
internal traversal, not from its header, and normalization/codec versioning are
separate product choices.

After approval, implementation is governed production performance work. Add
the two methods and focused positive/negative tests, freeze after minimum
correctness, run the mandatory first ordinary Release comparison against the
retained current public composition and prototype ceiling, report the verdict,
and stop again before broad closeout.

Predeclared first Release acceptance:

1. all CRI-09 JSON behavior remains correct in opt/no-opt and both VMs;
2. every new empty/type/count/numeric-boundary/failure byte contract passes;
3. production parse + one f32 projection + one direct scan is at most 25% of
   the retained current public-composition total on each optimized VM;
4. production f32 projection is no more than 2x the renamed prototype's
   same-cell per-projection median, allowing the newer parser/error contract
   without accepting the current 77x--84x materialization gap;
5. no optimizer-induced inversion greater than 10% appears on either VM;
6. raw checksums and bytes match exactly, and peak RSS introduces no material
   regression beyond the current diagnostic range; and
7. the unchanged `rxjson_parser_compare` compatibility and retained-document
   guards still pass.

## Decision-stop preservation audit

At this stop CREXX remains on `develop` at
`d78c6fcfa81ef03fdbea65ff9cc39ed99e8716bf`; `git diff --check` passes and no
work is staged. The read-only `crexx-rag` checkout remains on `main` at
`97cd87e91344d6ac1773a054bd38df23eb128ed2`. Its tracked diff SHA-256 remains
`97443028cfa8e86624fc5fda9ea5fb33d02f4d7413e5a8a848d92ec365288ef9`, its
empty index diff SHA-256 remains
`30cea35503c6dc073f3007218b9458f2bc0c28b2c7661327b9144036d5a7c61d`, and
its no-branch porcelain-v2 status SHA-256 remains
`02a6e4216ff47be3ce7252be2ccb39157bc45fda316c254f7198648109df14f1`.
These exactly match the preceding item-boundary audit, so the sister checkout's
pre-existing state was preserved.

## Smallest exact decision

Adrian should decide only this representation/API choice:

> Approve CRI-13 Option B exactly as specified: add explicit
> `.jsondocument.node_f32_array` and `node_i64_array` bulk projections to
> owning, headerless canonical-little-endian `.binary`; require explicit node,
> type and expected-count policy; preserve strict failure/range semantics and
> ordinary traversal; add no wrapper class, normalization metadata, inference,
> cache, magic, version or new serialized envelope.

## Paste-ready continuation prompt

```text
Resume /Users/adrian/CLionProjects/CREXX at CRI-13 using
performance/CREXX-RAG-INTEGRATION-WORKLIST.md and
performance/evidence/2026-07-29-crexx-rag-integration-ledger/CRI13-PACKED-NUMERIC-DECISION.md.

I approve CRI-13 Option B exactly as specified: implement
.jsondocument.node_f32_array and node_i64_array as explicit bulk projections to
owning, headerless canonical-little-endian .binary. Keep type and expected-count
policy explicit; preserve strict element/range/failure behavior and all ordinary
JSON traversal. Add no wrapper class, normalization metadata, inference, cache,
magic, version or serialized envelope. Keep CRI-13 as the sole active item.
Run focused opt/no-opt, dual-VM correctness, then freeze and run the packet's
mandatory first ordinary Release comparison against the retained current
public-composition evidence and prototype ceiling. Report that first Release
verdict and stop before broad closeout or CRI-14. Do not touch crexx-rag.
```

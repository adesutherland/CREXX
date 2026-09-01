# CRI-13 Option B first Release verdict

Status: **mandatory decision stop — frozen B fails one predeclared rule**

Date: 2026-07-30

Governed ID: `CAP-01-J02`

## Outcome first

The frozen production B candidate is correct and materially faster than current
public composition, but it is not accepted. It passes six of seven predeclared
rules and fails the prototype-ceiling rule in every VM/optimization cell.

One parse, one production f32 projection and one direct packed scan is 4.01 ms
on optimized `rxvm` and 4.86 ms on optimized `rxbvm`. That is 14.81% and
17.23% of the retained current-composition totals, comfortably passing the 25%
end-to-end rule. The f32 projection itself is nevertheless 11.28x--13.05x the
renamed prototype's same-cell per-projection median, versus the allowed 2x.
This is a real validation-path cost, not an optimizer inversion.

The implementation remains provisional and frozen. No C wrapper class and no
CRI-14 work has begun.

## Frozen production contract

The existing `.jsondocument` now provisionally supplies:

```text
node_f32_array(node, expected_count, packed, error) = .int
node_i64_array(node, expected_count, packed, error) = .int
```

Both produce owning headerless canonical-little-endian `.binary`; success is
zero and failure uses stable distinct negative statuses, clears the output, and
reports a one-based element where relevant. There is no wrapper, flag, cache,
inference, magic, version or serialized envelope.

Frozen hashes:

| Artifact | SHA-256 |
| --- | --- |
| `lib/rxfnsb/rexx/rxjson.crexx` | `f9bc3110e672b93e5b428287d846dcb7b30b1fada0087e56dca803e6d6df8a8a` |
| focused projection test | `809c52c2578960a6b5d55297d03268f5c7ff4f0e0a23b5b7d6b48afa50d9aea5` |
| numeric benchmark | `ffe920aed41293f767356a12db398b5a9a10272b50f2a7d1ed2815e15f784932` |
| unchanged JSON benchmark | `f4ce6bcd3b53f9be3a7ffab3357e99d39b5687562bed43dac1b385276718257b` |
| freeze manifest | `2c83c82a1926200961a94e46ff27a1bb00870345db81391360c92e732fe4f8bf` |

## Focused correctness

The dedicated Debug build ran the new functional contract and numeric
benchmark in optimized/non-optimized form on both VMs and passed 9/9. A second
focused matrix combined the CRI-09 document/noisy contracts with the new
projection contract and passed 13/13. The contract covers exact f32/i64 bytes,
owning replacement, success/error clearing,
empty arrays, `-1` and exact expected counts, invalid document/node, wrong
container, count failures, non-numbers and nested values, integral decimal and
exponent i64 forms, signed-i64 extrema, f32 maximum and minimum subnormal, and
f32/binary64 overflow and underflow.

```sh
cmake --build /tmp/crexx-integration-ledger.mZ5jyp/candidate-debug \
  --target ts_rxjson_numeric_projection performance_rxjson_numeric --parallel 10
ctest --test-dir /tmp/crexx-integration-ledger.mZ5jyp/candidate-debug \
  --parallel 8 --output-on-failure \
  -R '^(linked_opt_runtime_artifacts_build|ts_rxjson_numeric_projection_(noopt|opt|rxbvm_noopt|rxbvm_opt)|rxjson_numeric_materialization_compare_(noopt|opt|rxbvm_noopt|rxbvm_opt))$'
```

Raw logs are `/tmp/cri13-optionb-focused-build-r4.log` and
`/tmp/cri13-optionb-focused-ctest-r3.log`. The 13/13 CRI-09 behavior matrix is
`/tmp/cri13-optionb-cri09-focused-ctest.log`, SHA-256
`35ace7653d7b26448287a69a8641557e9f9e1ba00d8df3d6e229a13b14d496be`.

## Ordinary Release product and sampling

The ordinary profiling-off Release product is
`/tmp/crexx-cri09-release-a2.N4ELYs/build`, built with Apple clang 21.0.0.
The VM executables remain byte-identical to the accepted A2 product. The
updated library SHA-256 is
`899872f19c840c6b5173807022cb366868196989cfbbfdd34fbd7735bde7f4e0`.

The numeric verdict used two warmups then ten serial recorded samples in each
opt/no-opt x VM cell, rotating the starting cell each round and removing no
outliers. All 40/40 recorded samples passed. The unchanged optimized JSON
benchmark used three warmups and twelve alternating recorded samples per VM;
all 24/24 passed. The Apple M5 host was on AC with low-power mode off.

Evidence root: `/tmp/crexx-cri13-optionb-release.wDmu2e/`.

| Evidence | SHA-256 |
| --- | --- |
| `numeric-raw.log` | `ce5c76da53a2a7fc19a68b8824177703f7f86378fafd4fff1e9b364c0e79a428` |
| `numeric-medians.tsv` | `483fd20919e3bce420d1c4c2db587c896615d7abb0b3d9b58892def6b47861db` |
| `parser-raw.log` | `bafa23abe0df87ac846a2a8c54f4559b28995ffebfc389a6376ca814306de9d8` |
| `parser-medians.tsv` | `4c8eccfc38b4cce9a6b0275d6a70d2386e12808ef3cee4fdc90b0b6bd2a8b898` |
| `verdict-calculations.txt` | `0967a97fd6d385d3385df949a1fad05af0b714633514e32fb4f3f53e58a451a0` |
| `manifest.txt` | `8b73535c5f4cd1c41eeb28e495c76c3960237f3bc33d2782ebe03b184c50d6e8` |

## Numeric medians

Times are microseconds. `current total` is the current public
children/materialize/pack path. `B total` is one parse, one production f32
projection and one direct scan. Prototype projection is the earlier retained
ten-projection median divided by ten.

| Cell | current total | B f32 projection | B i64 projection | B total | B/current | prototype projection | B/prototype | peak RSS bytes |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| `rxvm -n` | 18,464.5 | 2,907 | 1,582 | 4,011 | 21.72% | 256.65 | 11.33x | 28,499,968 |
| `rxvm opt` | 18,337.5 | 2,918 | 1,552.5 | 4,020.5 | 21.93% | 258.65 | 11.28x | 28,655,616 |
| `rxbvm -n` | 19,691.5 | 3,476 | 1,833.5 | 4,875.5 | 24.76% | 285.65 | 12.17x | 28,688,384 |
| `rxbvm opt` | 19,754.5 | 3,475.5 | 1,797 | 4,860 | 24.60% | 266.35 | 13.05x | 28,688,384 |

All float/current/production and integer checksums were exact in every sample.
Optimized B total is 0.24% slower than non-optimized on `rxvm` and 0.32% faster
on `rxbvm`, well inside the 10% inversion rule. Optimized peak RSS is 4.45%
and 4.63% above the retained pre-B diagnostic on `rxvm` and `rxbvm`; the
candidate benchmark performs and retains the additional projection, so this is
not a material memory regression.

## Unchanged JSON benchmark

All legacy cells remain inside the 25% guard against pre-edit medians:

| VM | valid | deep get | tail get | count | members |
| --- | ---: | ---: | ---: | ---: | ---: |
| `rxvm` | +18.18% | +8.67% | +14.25% | -39.87% | +13.16% |
| `rxbvm` | +18.73% | +9.89% | +15.14% | -38.37% | +12.48% |

Construction plus 30 retained path gets is 78.81%/81.97% faster than matched
legacy repeated parsing on `rxvm`/`rxbvm`. Construction plus 30 resolved-node
gets is 94.60%/94.91% faster. Scanner medians are 393.5/486 us. Thus B did not
regress the accepted CRI-09 infrastructure or its primary benchmark.

## Verdict against the predeclared rules

| Rule | Verdict | Evidence |
| --- | --- | --- |
| CRI-09 behavior remains correct | pass | focused 13/13 includes both modes and VMs |
| New byte/count/type/range/failure contract | pass | exact focused boundary matrix |
| Optimized B total <=25% retained current total | pass | 14.81% `rxvm`; 17.23% `rxbvm` |
| B f32 projection <=2x same-cell prototype | **fail** | 11.28x--13.05x |
| No optimizer inversion >10% | pass | +0.24% / -0.32% total |
| Exact bytes/checksums and no material RSS regression | pass | all 40/40 exact; +4.45%/+4.63% RSS |
| Unchanged JSON guards | pass | all legacy and retained-document cells pass |

## Root cause

The candidate correctly eliminates the intermediate child-ID and typed-value
arrays, but its strict f32 validation is implemented as repeated string work
for every element:

1. materialize the number source slice as `.string`;
2. `_json_number_nonzero` scans the full string;
3. `_json_float_token_in_range` calls `lower(text)`, then uses repeated
   `pos`, `substr`, concatenation and digit scans to derive an adjusted decimal
   exponent;
4. convert the same string to binary64, compare the f32 range, write f32 and
   read it back for underflow.

The renamed prototype performs the slice conversion, binary64 conversion,
range compare and f32 write but not those additional string scans and
allocations. The candidate's 2.91--3.48 ms f32 projection versus the
prototype's 0.26--0.29 ms therefore has a concrete source-level mechanism.
There is no optimizer inversion. In particular, the existing helper's
per-element `lower(text)` is real work; no case normalization is required to
recognize the two JSON exponent bytes `e` and `E`.

## Alternatives

### A — retain frozen B and waive the prototype ceiling

This keeps strict correctness and still gives a 4.1x--6.7x paired end-to-end
improvement. It is the smallest maintenance surface, but it explicitly waives
the approved 2x mechanism gate and leaves avoidable repeated string work in the
hot projection. Not recommended.

### B — private allocation-free f32 source-span classifier (recommended)

Replace the two per-element string helpers on the bulk f32 path with one
private pass over `_source[start,after)`. The pass recognizes `e` and `E`
directly, computes nonzero and adjusted decimal exponent without `lower`,
`substr`, concatenation or a temporary digit string, and rejects binary64
overflow/underflow before the required one source-slice conversion and numeric
cast. The existing maximum comparison and post-write nonzero-underflow check
remain. The public B API, bytes, statuses, ABI and parser index do not change.

This is bounded and directly targets the measured extra work. Re-run the same
focused matrix and mandatory first Release verdict after this one production
countermeasure.

### C — store numeric classification in every parser node

Classify adjusted exponent/integrality while indexing and store metadata for
later projections. It could make repeated projections cheaper, but increases
parse work and private-index footprint for every JSON consumer and risks the
accepted parser benchmark. The current workload needs one projection, so this
is disproportionate evidence. Not recommended now.

### D — use the prototype's permissive validation

Dropping exact overflow/nonzero-underflow behavior would approach the prototype
directly but weaken the approved correctness contract. Rejected.

## Recommendation and smallest exact decision

Approve Alternative B only: one private allocation-free f32 source-span
classifier, with no public API, parser-index, ownership or byte-format change.
Keep the frozen public methods and all boundary tests. C classes remain
deferred until production B passes its mandatory Release gate.

## Paste-ready continuation prompt

```text
Resume /Users/adrian/CLionProjects/CREXX at the CRI-13 Option B first Release
stop using performance/CREXX-RAG-INTEGRATION-WORKLIST.md and
performance/evidence/2026-07-29-crexx-rag-integration-ledger/CRI13-B-RELEASE-VERDICT.md.

I approve CRI-13 countermeasure Alternative B exactly as specified: replace the
bulk f32 path's per-element string-helper validation with one private
allocation-free source-span classifier over the retained JSON bytes. Recognize
e/E directly and compute nonzero plus adjusted decimal exponent without case
conversion, substrings, concatenation or temporary digit strings. Preserve the
public node_f32_array/node_i64_array API, exact statuses, strict range and
nonzero-underflow behavior, canonical bytes, parser index, ABI and ownership.
Keep CRI-13 as the sole active item. Run focused opt/no-opt dual-VM correctness,
freeze, rerun the same mandatory ordinary Release verdict and stop. Do not
start C, CRI-14, broad closeout or any crexx-rag work.
```

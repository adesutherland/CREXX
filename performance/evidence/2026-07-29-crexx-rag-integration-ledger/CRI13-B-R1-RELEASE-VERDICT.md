# CRI-13 Option B R1 repeated Release verdict

Status: **mandatory decision stop — R1 is adverse and fails two rules**

Date: 2026-07-30

Governed ID: `CAP-01-J02`

## Outcome first

The approved allocation-free projection-time byte-span classifier is correct
but slower than the string-helper implementation it replaced. R1 is not
accepted and remains frozen/provisional.

Optimized f32 projection rose from the first B candidate's 2.918/3.4755 ms to
5.4295/5.863 ms on `rxvm`/`rxbvm`: 86.07% and 68.70% slower. End-to-end B total
rose from 4.0205/4.860 ms to 6.586/7.160 ms: 63.81% and 47.33% slower.

R1 therefore fails both:

- the 2x prototype projection ceiling, measuring 20.99x/22.01x optimized; and
- the <=25% retained-current total rule on optimized `rxbvm`, measuring 25.38%
  (`rxvm` remains inside at 24.27%).

No C class or CRI-14 work has begun.

## What R1 changed

R1 preserved the public `node_f32_array`/`node_i64_array` contract and replaced
the bulk f32 path's `_json_number_nonzero` and `_json_float_token_in_range`
string calls with one private `_json_f32_span_classify` pass over the retained
source bytes. The pass recognizes `e` and `E` directly and allocates no strings.
The one required numeric source-slice conversion remains after classification.

Frozen hashes:

| Artifact | SHA-256 |
| --- | --- |
| `lib/rxfnsb/rexx/rxjson.crexx` | `1791e30d2400fba8098b6b774cdce1dfb9f042f1066fa99083cba96209a9dacb` |
| projection contract | `9ded3b044d1a183c77b301624ab937752a214c5546fd5da1ac68beaa4e6fcc32` |
| numeric benchmark | `ffe920aed41293f767356a12db398b5a9a10272b50f2a7d1ed2815e15f784932` |
| unchanged JSON benchmark | `f4ce6bcd3b53f9be3a7ffab3357e99d39b5687562bed43dac1b385276718257b` |
| freeze manifest | `df0094aa816f1a1959890f1ed3c20ae61946ac32dcda642776af95367dfe4a1f` |

## Correctness and sampling

Focused Debug passes 17/17 across the CRI-09 document/noisy contracts, the new
projection contract, the numeric benchmark, opt/no-opt and both VMs. New cases
prove direct lower/uppercase exponent handling and exact zero with extreme
positive/negative exponents, alongside all earlier type/count/byte/range and
failure-clears-output cases.

The repeated ordinary profiling-off Release verdict used the same protocol as
the first verdict: two warmups and ten serial recorded numeric samples in each
of four cells with rotating starts and no removed outliers, plus three warmups
and twelve alternating optimized unchanged-JSON samples per VM. Results are
40/40 numeric and 24/24 parser passes. The Apple M5 host was on AC with
low-power mode off.

Evidence root: `/tmp/crexx-cri13-optionb-r1-release.CD6iZo/`.

| Evidence | SHA-256 |
| --- | --- |
| focused CTest | `379d66a611fa99c0a5cac206a1a1d97cfbc089737a83d2963ea64302b20c6f79` |
| `numeric-raw.log` | `a83411860aeea18c03b21a9c0f21f5a8924237888b0ff04237d7fa1e904b751e` |
| `numeric-medians.tsv` | `67891fc16ed0d0b20d299a4111923e1abb663cc1e5960bd19b0c1ba2e423c240` |
| `parser-raw.log` | `0c954298da6f006f454d77b40ec51b3d95e700f97db71cba1874fa0f07707cd0` |
| `parser-medians.tsv` | `1991340ecb83e74894bc8d5de1bbf9f207a2f0cdc9223b4e6ad8faa9990cdc0f` |
| `verdict-calculations.txt` | `6a6f593d177f681a1e793d82ccb4657dab44da73abed3169d71c800a2bec548f` |
| `manifest.txt` | `4bd91bd5a9d7249438a3e6b8b2cab4eacff8a3fc00bfb50dc08b8c43365ea581` |

## Numeric medians

Times are microseconds. Prototype projection is the retained ten-projection
median divided by ten.

| Cell | current total | R1 f32 projection | R1 i64 projection | R1 total | R1/retained current | prototype projection | R1/prototype | peak RSS bytes |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| `rxvm -n` | 18,503.5 | 5,551.5 | 1,596 | 6,344.5 | 22.86% | 256.65 | 21.63x | 28,655,616 |
| `rxvm opt` | 18,460.5 | 5,429.5 | 1,584.5 | 6,586 | 24.27% | 258.65 | 20.99x | 28,745,728 |
| `rxbvm -n` | 19,702 | 5,848.5 | 1,785.5 | 7,104 | 24.36% | 285.65 | 20.47x | 28,819,456 |
| `rxbvm opt` | 19,665.5 | 5,863 | 1,805.5 | 7,160 | 25.38% | 266.35 | 22.01x | 28,770,304 |

All checksums and bytes remain exact. Optimized total is 3.81% slower than
non-optimized on `rxvm` and 0.79% slower on `rxbvm`, within the 10% inversion
limit. Optimized RSS is 4.78%/4.93% above the retained pre-B diagnostic and is
not a material memory regression.

## Unchanged JSON benchmark

The accepted JSON surface remains within every legacy guard:

| VM | valid | deep get | tail get | count | members |
| --- | ---: | ---: | ---: | ---: | ---: |
| `rxvm` | +17.17% | +5.82% | +13.37% | -39.03% | +14.69% |
| `rxbvm` | +17.94% | +9.75% | +12.82% | -38.34% | +12.80% |

Retained path use remains 78.79%/81.50% faster than matched repeated parsing,
and resolved-node use remains 94.60%/94.76% faster. Scanner medians are
393/486 us. R1 did not regress the primary CRI-09 benchmark.

## Verdict against the unchanged rules

| Rule | Verdict | Evidence |
| --- | --- | --- |
| CRI-09 behavior remains correct | pass | focused 17/17; parser 24/24 |
| New byte/count/type/range/failure contract | pass | exact focused boundary matrix |
| Optimized total <=25% retained current total | **fail** | 24.27% `rxvm`; 25.38% `rxbvm` |
| f32 projection <=2x same-cell prototype | **fail** | 20.47x--22.01x |
| No optimizer inversion >10% | pass | +3.81% / +0.79% total |
| Exact bytes/checksums and no material RSS regression | pass | all 40/40 exact; <5% RSS |
| Unchanged JSON guards | pass | every legacy and retained-document cell passes |

## Root cause

Allocation-free is not equivalent to low-cost in this Level B path, but the
classifier procedure call is not the cause: optimized RXAS fully inlines it
into `node_f32_array`.

The dominant regression is two hidden whole-source copies per element. The
non-local `_source` class attribute crosses the inlined classifier's by-value
`.binary` boundary as `linkattr1; copy; unlink`; inlined `binlength(data)` then
copies the same binary again. For the 3,072-element, 58,479-byte benchmark this
is 6,144 copies or 359,294,976 logical bytes per projection. The loop also
executes about 61,441 strict `bgetu8` reads, each with a VM range check.

A scratch one-local-snapshot/one-length variant removes the projection-loop
payload copies and measures 1.311/1.755 ms on `rxvm`/`rxbvm`, substantially
faster than first B's 2.918/3.4755 ms. It still measures about 5.07x/6.59x the
same-cell prototype ceiling because it executes about 1.28 million interpreted
RXAS instructions. Bounds checks are redundant after the span proof, but even
the entire measured `bgetu8` handler cost cannot close that remaining gap.

The stable 68.70%--86.07% projection regression, dual-VM shape and lack of
optimizer inversion therefore reflect a real mechanism, not timing noise.
Exact static/dynamic traces and scratch controls are retained in
[`CRI13-R1-RXAS-TRACE.md`](CRI13-R1-RXAS-TRACE.md).

The private node is already 40 bytes and initializes an unused u32 field at
offset 36. The parser's `_json_scan_number` already visits every number byte
once. This provides a route that avoids both string-helper rescans and a new
projection-time byte loop without increasing index size.

## Alternatives

### A — retain R1

Correct, allocation-free and simple in concept, but slower than the first B
candidate and fails two rules. Rejected.

### B — return to the first B and waive the prototype ceiling

The first B candidate passed the end-to-end rule at 4.0205/4.860 ms but retained
2.918/3.4755 ms projections, 11.28x--13.05x the prototype. This is a valid
correctness-first stopping point only if Adrian explicitly relaxes the approved
performance gate. Not recommended.

### C — remove only the payload-copy explosion

R1 is already fully inlined. A source-local snapshot plus one precomputed
length removes the repeated copies and produces 1.311/1.755 ms diagnostic
medians, but remains about 5.07x/6.59x the prototype ceiling. This is valuable
systemic evidence for `PERF2-07-B02`, not a sufficient CRI-13 countermeasure.
Do not accept CRI-13 on this basis alone.

### D — classify during the existing parser scan and store private flags
(recommended)

While `_json_scan_number` is already consuming the token, compute only two
facts required by f32 projection: whether the number is nonzero and whether its
adjusted decimal exponent is safe for binary64 conversion. Store those facts in
the already-zeroed private u32 node field at offset 36. The node remains 40
bytes, `index_bytes()` is unchanged, and the class documentation already states
that this index is private, ephemeral, and not an ABI or serialized format.

`node_f32_array` then reads the flags, materializes the source span exactly once
for the required numeric cast, compares against the f32 maximum, writes the
value, and retains the post-write nonzero-underflow check. There is no
projection-time validation rescan. Query/boundary behavior, parser grammar and
the legacy benchmark must remain within their existing guards.

This moves a few scalar updates into a byte loop that already exists instead of
creating a second Level B loop. It is the smallest mechanism consistent with
the evidence and does not enlarge the private index.

The residual-copy issue is separately queued as `PERF2-07-B02`. RXAS already
has CFG/liveness/effects/copy-flow machinery but deliberately rejects generic
full-value copies as `full-value-ownership-unproved`. A bounded RXAS PoC can
first target register-local post-inline copies; the harder
`linkattr/copy/unlink` attribute case must also preserve alias lifetime and
exceptional cleanup. This follow-on must not be used to waive R1's failed
CRI-13 ceiling.

### E — weaken range or underflow validation

This could match the prototype but violates the approved correctness contract.
Rejected.

## Recommendation and smallest exact decision

Approve Alternative D only: replace R1 with parse-time nonzero/binary64-safe
flags stored in the existing unused private node field, with no node-size,
public API, ABI or serialized-format change. Keep all current tests and rerun
the same focused and mandatory ordinary Release verdict. C remains deferred.

## Paste-ready continuation prompt

```text
Resume /Users/adrian/CLionProjects/CREXX at the CRI-13 B R1 repeated Release
stop using performance/CREXX-RAG-INTEGRATION-WORKLIST.md and
performance/evidence/2026-07-29-crexx-rag-integration-ledger/CRI13-B-R1-RELEASE-VERDICT.md.

I approve CRI-13 R2 Alternative D exactly as specified: replace the adverse R1
projection-time byte pass by computing nonzero and binary64-safe classification
while the existing parser number scan is already consuming the token, and store
only those private flags in the already-unused u32 node field at offset 36.
Keep the node at 40 bytes and preserve index_bytes, grammar, public
node_f32_array/node_i64_array API, statuses, strict f32 range/nonzero-underflow
semantics, canonical bytes, ownership, ABI and serialized formats. Eliminate
all projection-time validation rescans. Keep CRI-13 as the sole active item,
run focused opt/no-opt dual-VM correctness, freeze, rerun the same mandatory
ordinary Release verdict and stop. Do not start C, CRI-14, broad closeout or
crexx-rag work.
```

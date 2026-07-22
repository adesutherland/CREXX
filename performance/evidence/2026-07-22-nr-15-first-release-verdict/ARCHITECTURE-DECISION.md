# NR-15 architecture selection packet

Status: stopped for Adrian's explicit architecture selection
Date: 2026-07-22
Starting branch/commit: `develop` at
`240b29f456e995928206f04285a7c319612ff022`
Starting local and remote upstream: `origin/develop` at the same commit
Starting worktree: clean

This is the mandatory pre-integration architecture stop, not the governed
ordinary-Release verdict for a frozen production implementation. The fastest
measured ceiling changes the serialized instruction set and binds a narrower VM
operation to the stem representation. Formal paired Release sampling therefore
has not begun.

## Semantic gate

The focused Level B matrix passes optimized and `-n` in both `rxvm` and
`rxbvm`. It covers missing/default values, omitted versus explicit empty keys,
new and existing entries, repeated generations and selective writes,
method/property/bracket access, multi-tail separators, integer/dynamic tails,
ASCII/Unicode/empty/long/collision keys, indexed/extracted values, live and
snapshot iterators, and reference mutation. The existing focused stem,
multi-tail, TRACE/source-map and signal/error surfaces remain in the test set.

The separate Classic comparison passes in Regina and ooRexx. It records the
important difference that `DROP item.a` makes the compound uninitialized as
`ITEM.A`; Level B `rxfnsb.stem` has no drop/remove method and is not being made
to emulate a surface it does not expose.

Candidate A passes the final 17-test focused optimized/`-n`, dual-VM and TRACE
set. The provisional native-opcode semantic control also passes in both
VMs for missing/default behavior, generation reset, empty and Unicode keys,
new/existing updates, and a collision chain. Candidate B is rejected because
its optimized executions fail the semantic gate.

## Exact current baseline

The retained baseline is the exact starting product, not the historical
programme percentage. Its linked access image is 17,142 bytes and its linked
library is 858,896 bytes. The current post-NR-18 profiling smoke attributes
75,000 calls and 302,253,782 ns self time to `stem.get`, and 49,800 calls and
181,331,705 ns self time to `stem.set`, in the `rxvm` diagnostic run. These are
profiling diagnostics, not ordinary-Release timing claims.

The current emitted path retains `settpcall` method calls plus caller argument,
return and receiver bookkeeping. The imported inline body is rejected as an
unportable class-attribute shape. This is necessary today: removing the array
shape restriction in Candidate B grows and miscompiles the optimized product.

## Candidate dispositions

### A - existing Level B table with native whole-string hash: accepted fallback

Candidate A keeps the fixed 256-bucket, parallel-array, separate-chaining
representation and the existing O(1) generation reset. Public `stem.hash()`
retains its documented codepoint polynomial result. Only internal `get` and
`set` bucket selection use existing `RXHASH` over the complete UTF-8 payload
and reduce its low byte with one immediate `IAND`.

This is semantically invariant: both insertion and lookup apply the same
deterministic function to the exact UTF-8 bytes, equal keys therefore select
the same bucket, and exact string equality still resolves arbitrary collisions.
No normalization or codepoint-equivalence rule is lost because the existing
stem equality contract is byte-exact string equality. Public callers of
`hash()` continue to observe the old polynomial bucket.

All measured profile cells execute fewer instructions than the exact baseline.
Representative optimized reductions are:

| Cell | Baseline | Candidate A | Reduction |
| --- | ---: | ---: | ---: |
| get hit, ASCII, 16 entries / 1,000 | 105,696 | 48,470 | 54.142% |
| get hit, ASCII, 256 entries / 1,000 | 158,214 | 72,188 | 54.373% |
| get hit, ASCII, 1,024 entries / 500 | 259,812 | 137,625 | 47.029% |
| get miss, ASCII / 1,000 | 168,125 | 32,423 | 80.715% |
| existing set / 500 | 64,904 | 32,254 | 50.305% |
| new set / 100 | 28,849 | 12,806 | 55.610% |
| Unicode get / 500 | 91,232 | 29,818 | 67.316% |
| long-key get / 500 | 396,250 | 29,556 | 92.541% |
| collision get / 200 | 812,438 | 71,814 | 91.161% |
| multi-tail / 1,000 | 140,457 | 67,197 | 52.158% |
| histogram / 1,000 | 297,032 | 190,181 | 35.973% |
| default reset / 1,000 plus final read | 33,225 | 31,471 | 5.279% |

The linked access image is 17,134 bytes, 8 bytes smaller; the linked library is
858,880 bytes, 16 bytes smaller. No new call or allocation is introduced.

A fixed 1,024-bucket variant improves some large-table paths but increases
per-stem attribute storage and loses the one-shot general RexxCPS comparison:
5,369,059 versus 5,658,302 CPS in `rxvm`, and 5,303,670 versus 5,316,321 CPS in
`rxbvm`. A load-sensitive table adds a mask/capacity load on every small-table
hot path plus resize scans and copies; it cannot satisfy the strict-fewer gate
there. Both sizing variants are rejected for this panel.

### B - generic array-bearing method inlining: rejected

The guarded compiler PoC removes 13 of 23 stem get/set call references but
grows the optimized source RXAS from 69,433 to 116,492 bytes (+67.8%) and the
unlinked RXBIN from 24,634 to 39,111 bytes (+58.8%). Optimized executions fail
dotted stem access, multi-tail access, default generations, explicit empty keys,
reference mutation and the workload checksum in both VMs. `-n` remains correct.
The generic array-shape exclusion is therefore retained.

### C - segmented multi-tail lookup: deferred into the architecture choice

Under Candidate A, the matched 64-entry/1,000-lookup multi-tail profile executes
67,197 instructions and 4,454 concat operations. The prejoined single-key
control executes 53,874 instructions and 134 setup concats: 13,323 fewer
instructions (19.8%) before any streamed lookup improvement.

The current Level B/RXAS surface has no segment-aware hash/equality operation.
A source/compiler-only rewrite either materializes the same canonical key or
cannot compare against stored strings. A correct streamed hit/miss path, with
one canonical materialization only after insertion is proved, therefore belongs
with the selected runtime/representation architecture rather than Candidate A.

### D - native stem operations: choose architectural candidate

The provisional ceiling adds local-only opcodes 641 `STEMGET` and 642
`STEMSET`. They hash once, traverse once, and copy directly between final
registers while using the current stem representation. They are not integrated
compiler output and are removed after measurement.

The ceiling is intentionally not production-complete. A selected D design must
add a versioned receiver/type contract, preserve operand aliasing with snapshots
where storage can grow, define allocation-failure atomicity, and integrate the
existing source-step, TRACE and signal boundaries. Those obligations are why D
stops at comparative evidence even though its focused primitive semantics pass.

Matched optimized dynamic counts versus Candidate A are:

| Cell | Candidate A | Native ceiling | Further reduction |
| --- | ---: | ---: | ---: |
| get hit, ASCII, 16 entries / 1,000 | 48,470 | 16,465 | 66.031% |
| get hit, ASCII, 256 entries / 1,000 | 72,188 | 36,703 | 49.156% |
| get hit, ASCII, 1,024 entries / 500 | 137,625 | 115,476 | 16.094% |
| get miss, ASCII / 1,000 | 32,423 | 12,418 | 61.700% |
| existing set / 500 | 32,254 | 11,917 | 63.053% |
| new set / 100 | 12,806 | 6,421 | 49.859% |
| Unicode get / 500 | 29,818 | 13,045 | 56.251% |
| long-key get / 500 | 29,556 | 13,131 | 55.572% |
| collision get / 200 | 71,814 | 57,448 | 20.004% |

Single-shot profiling-off Release pilots agree for every lookup cell and for
existing update in both VMs. For example, the 64-entry/10,000-hit `rxvm` cell
falls from 1,400 to 547 microseconds and the 1,024-entry/5,000-hit cell from 970
to 525 microseconds. The `rxbvm` new-insertion pilot is 754 versus 721
microseconds despite 49.859% fewer instructions, so no wall-clock claim is made
for that noisy allocation-sensitive cell. These are decisive PoC pilots, not
governed production samples.

Default reset is already O(1) and scans no entries. Removing its remaining
method-call scaffolding and adding segmented lookup are part of the prospective
narrow intrinsic family; they are not necessary to establish that the
architecture ceiling beats Candidate A.

## Register-binary storage variant

Adrian noted that a native hash/stem implementation could use a register's
binary payload as storage. This is recorded as Candidate D2 and is recommended
for the next bounded architecture comparison.

D2 could pack bucket heads and entry metadata into contiguous binary storage,
eliminating most parallel `value.attributes[]` pointer walks and improving
cache locality. A viable design would keep a compact bucket/entry index plus a
key arena or key references, while string values remain GC/reference-safe VM
values or use an explicit ownership table. It must preserve generation reset,
Unicode byte equality, insertion order, live/snapshot iterators, reference
lifetime, failure atomicity and cross-platform serialized widths/alignment.

The next architecture panel should compare:

1. D1: narrow native get/set/reset/segmented operations over the current
   parallel arrays;
2. D2: the same operation contract over register-binary bucket/entry metadata;
3. D2-hybrid: binary bucket/chain metadata with VM `value` slots for keys and
   values, avoiding copied string arenas where ownership costs dominate.

Each design must include construction, growth/rehash, lifecycle, iterator and
RSS/allocation costs, not only steady-state lookup.

## Recommendation and stop

Recommendation: **choose the architectural candidate** and authorize the
bounded D1/D2/D2-hybrid comparison before production integration. Candidate A
is a semantically safe, image-neutral fallback, but the direct ceiling proves
that accepting it now would leave 16.1-66.0% of the matched executable work in
place. Candidate B is rejected; Candidate C should share the selected D-family
contract.

Because this choice changes ISA/RXBIN and may change the stem representation,
the provisional opcodes are not left in production source and the governed
paired ordinary-Release verdict has intentionally not begun. No broad CTest,
sanitizer, install/package, documentation closeout, commit or push has run.

## Evidence map

- `baseline/`: exact starting images, RXAS, hashes, pilots and profiles.
- `panel/candidate-a-native-hash/`: fallback images, profiles and deltas.
- `panel/candidate-a-fixed1024/`: rejected capacity variant.
- `panel/candidate-b-generic-inline-arrays/`: rejected image and semantic proof.
- `panel/candidate-c-segmented-multitail/`: materialized/prejoined profile pair.
- `panel/candidate-d-native-stem-opcodes/`: provisional source, linked controls,
  dual-VM profiles and primitive semantic control.
- `raw/`: captured command outputs for every retained cell.
- `ENVIRONMENT.md`: exact build configuration, command forms and final focused
  checks.
- `SHA256SUMS`: recursive checksums for all 550 other retained files.

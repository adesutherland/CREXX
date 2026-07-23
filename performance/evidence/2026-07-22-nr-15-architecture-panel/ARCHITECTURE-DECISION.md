# NR-15 D1/D2/D2-hybrid architecture decision

## Decision requested

Select the production representation before any permanent RXBIN/ISA/compiler
integration or governed Release sampling. The panel recommends **D2-hybrid as
the mutable general stem representation**, while retaining D1 as the safe
non-architectural fallback. Fully packed D2 should not be the sole mutable
representation on this evidence; it is the strongest future snapshot/frozen
or copy-heavy representation.

No candidate is a universal winner:

- D1 wins most steady-state miss, existing update, reset and segmented-access
  pilots, but its parallel arrays are much slower and more allocation-heavy
  when inserting and copying.
- D2 has the smallest object graph, fastest growth and overwhelmingly fastest
  copy, but packed value/default handling loses steady-state update/reset and
  long/Unicode access.
- D2-hybrid keeps most of D1's access performance, removes most of D1's new-key
  penalty, and materially reduces the object graph. It is the best one-layout
  compromise for a mutable general stem.

## Common contract and semantic result

All candidates implement the same direct operations: get, set, generation
reset, two-segment get/set, insertion-order key/value extraction, FNV-1a byte
hashing and byte-exact UTF-8 equality. A segmented hit/miss hashes and compares
`left || "." || right` without constructing that string; insertion constructs
the canonical key once.

The linked semantic control passes on both `rxvm` and `rxbvm` for empty keys,
Unicode, initial and assigned defaults, stale generations, selective update,
same-bucket collisions, plain/segmented equivalence, insertion order,
generation-aware `valueAt`, deep-copy isolation and ordinary VM move transfer.
The common 36-cell smoke matrix also passes every candidate/mode/VM cell.

The packed layout uses explicit little-endian fixed-width fields in an ordinary
register/value `binary_value`; it is an ephemeral runtime layout, not a
serialized RXBIN format. D2 value/default slots reserve power-of-two capacity
and overwrite repeated updates in place. D2-hybrid keeps header, buckets,
hash/next/generation records in the binary payload while keys, values and the
default remain ordinary VM-owned strings.

## Profiling-off Release tie-breakers

Times are microseconds for the complete timed loop. Each cell is one run;
`rxvm/rxbvm` are shown separately. Core modes use 200,000 operations except
new-key insertion, which uses 20,000.

| Mode | D1 | D2 | D2-hybrid | Interpretation |
| --- | ---: | ---: | ---: | --- |
| get hit | 6,513 / 6,294 | 6,776 / 6,714 | **5,998 / 6,072** | hybrid wins this tie-break |
| get miss | **3,433 / 3,157** | 3,673 / 3,559 | 3,711 / 3,445 | D1 wins |
| set existing | **11,419 / 12,194** | 12,776 / 13,702 | 12,397 / 12,998 | D1 wins; hybrid is second |
| set new | 12,828 / 13,738 | **5,523 / 5,810** | 5,657 / 5,864 | binary metadata is about 2.3x faster than D1 |
| reset + stale read | **10,132 / 10,209** | 12,107 / 12,146 | 11,287 / 11,433 | D1 wins; all remain O(1) |
| two-segment get | **4,272 / 4,044** | 5,141 / 4,962 | 4,680 / 4,303 | D1 wins; no key materialisation in any candidate |

These pilots support direction only. The final 84 diagnostic profiles provide
exact operation, call and allocation evidence but their per-handler timing is
not a substitute for the required governed ordinary-Release verdict.

## Exact work, calls and allocation evidence

The common source executes one native operation per hot get/set and two per
reset/read iteration. At the representative 64-entry/20,000 get-hit cell:

| Candidate | Executed instructions | Direct bytecode calls | Binary allocation bytes | Value-slot bytes | Attribute-pointer bytes |
| --- | ---: | ---: | ---: | ---: | ---: |
| D1 | 321,700 | 4 | 0 | 217,992 | 16,960 |
| D2 | 321,623 | 3 | 12,288 | 86,056 | 4,544 |
| D2-hybrid | 321,623 | 3 | 6,144 | 119,784 | 8,704 |

The 77-instruction/one-call D1 difference is construction-only: D1 invokes the
existing stem factory, while the two binary candidates initialise their
minimal receiver in the native operation. Hot-loop bytecode work is otherwise
the same, so the representation comparison is inside the measured handler and
allocation rows.

For 20,000 new inserts, cumulative profiler allocation bytes were:

| Candidate | Binary buffers | String buffers | Value slots | Attribute pointers |
| --- | ---: | ---: | ---: | ---: |
| D1 | 0 | 640,000 | 32,660,360 | 4,203,584 |
| D2 | 8,384,512 | 640,000 | 86,056 | 4,544 |
| D2-hybrid | 2,095,104 | 640,000 | 16,340,968 | 2,102,016 |

These are cumulative allocation records, including realloc growth, not RSS or
retained-live bytes. They show why D1 loses new insertion and why full D2 is
the memory/copy ceiling. Tracking-unavailable, invalid-event and overflow
counters are all zero.

## Copy, move, iteration and image

Exclusive profiled `COPY_REG_REG` averages in nanoseconds were:

| Entries | VM | D1 | D2 | D2-hybrid |
| ---: | --- | ---: | ---: | ---: |
| 64 | `rxvm` | 3,066 | **86** | 793 |
| 64 | `rxbvm` | 3,143 | **87** | 793 |
| 1,024 | `rxvm` | 26,308 | **1,189** | 13,303 |
| 1,024 | `rxbvm` | 27,354 | **1,083** | 13,597 |

D2 deep-copies one contiguous binary buffer. D2-hybrid still recursively
copies VM string slots, so its copy advantage narrows as the stem grows.
Ordinary `MOVE_REG_REG` transfer passes for all representations. Entry records
are appended in insertion order; native `keyAt`/`valueAt` proof passes with
generation semantics. Normal process teardown completes cleanly, but the
current profiler has allocation rather than deallocation counters; independent
destructor/sanitizer proof belongs to the selected production implementation.

All candidates use identical instruction widths: get/set/keyAt/valueAt occupy
four RXBIN cells, reset three, and two-segment get/set five. There is therefore
no candidate-specific call-site image delta. The retained common linked images
are 22,449 bytes (control), 15,038 bytes (semantics) and 14,064 bytes
(lifecycle); they intentionally contain all three candidates and are evidence,
not production-size forecasts.

## Recommendation and production gate

Recommended selection: **D2-hybrid**, with these production requirements:

1. Make the binary metadata a private, versioned runtime representation; do
   not serialize it into RXBIN or expose its offsets as public ABI.
2. Keep keys/default/values under ordinary VM ownership and preserve byte-exact
   Unicode, empty/omitted distinction, generation reset and insertion order.
3. Integrate direct single-key and segmented get/set/reset/extraction through a
   reviewed ISA/compiler contract; the numeric opcode assignments in this PoC
   are not reserved.
4. Specify allocation-failure atomicity and receiver/key/value alias behavior,
   then add focused failure injection, TRACE/source-step and reference tests.
5. Freeze the complete production panel, run the mandatory smallest ordinary
   Release verdict, and stop again before broad closeout.

If Adrian instead weights steady-state miss/update/reset above insertion,
memory and lifecycle, select D1. If copy/snapshot throughput and compactness
dominate, select D2 and accept its mutable-access regressions or restrict it to
a frozen/snapshot representation. Candidate A remains the reversible safe
fallback if no architectural change is selected.

The provisional opcode/effects/interpreter integration is removed before this
decision stop. `provisional-integration.patch` plus the retained helper,
handler, sources, binaries, 84 profiles, raw logs and summaries reconstruct the
panel. No formal sampling, broad QA, commit or push has occurred.

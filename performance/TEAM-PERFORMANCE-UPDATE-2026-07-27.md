# Team performance update — formal Mac scorecard

Date: 2026-07-27

Scope: PERF2-08 benchmark-equivalence gate and PERF2-09 formal Apple ARM64
scorecard

## Executive summary

The current cREXX product is now formally more than twice as fast as ooRexx on
the approved five-workload aggregate under the threaded VM (`rxvm`) and about
1.84 times as fast under the bytecode VM (`rxbvm`). That is a substantial
portfolio-level result, measured in one controlled Mac session against the
same work and with the ordinary profiling-off Release product.

It is not yet a claim that cREXX is faster on every workload:

- Richards remains the largest qualified gap. cREXX currently achieves only
  about 27% of ooRexx throughput and 16% of NetRexx throughput.
- Base64 reaches about 72% of ooRexx throughput. Its two cREXX timing series
  also remained noisy after the one permitted additional sample set.
- The newly qualified object-equivalent Towers comparison is about three times
  slower than ooRexx.
- RexxCPS is close to ooRexx parity under `rxvm` and about 7% behind under
  `rxbvm`, but it remains short of the programme's stronger 1.5-times target.
- cREXX is below NetRexx on the five-workload aggregate. NetRexx results include
  the normal JVM and JIT environment and must be understood separately from
  the ooRexx closure target.

The main positive result is therefore broad and real, but not uniform. Sieve,
Permute and Bounce are strong cREXX wins against ooRexx. Richards, Towers and
Base64 define the most important remaining Mac outcome gaps.

## What was measured

The formal common score contains exactly five workloads:

- **Sieve:** integer arrays, indexing and loops;
- **Permute:** recursion, calls and object mutation;
- **Bounce:** small objects, references and method calls;
- **Richards:** a larger state-machine and scheduler-style call graph; and
- **Base64:** byte/string processing and conversion-heavy loops.

Every runtime received one equal work argument for each common workload. Each
cell used two warmups followed by ten recorded serial samples. The runtime
order was rotated to avoid always giving the same implementation the first or
last position. Peak memory and startup/lifecycle phases were captured
separately so they could not distort steady-state throughput.

The tested cREXX executables were ordinary optimized Release builds with VM
profiling disabled. `rxvm` is the threaded-dispatch interpreter; `rxbvm` is the
bytecode/switch-dispatch interpreter. They execute the same cREXX program but
use different VM dispatch implementations, so the scorecard reports them
separately rather than choosing a winner or default.

Other terms used below:

- **Work per second** is the amount of the agreed logical workload completed
  each second. It lets runtimes be compared even when their raw internal loop
  counts differ.
- **Median** is the middle recorded result after sorting the samples. It is less
  sensitive than an average to one unusually slow or fast run.
- **Parity** means a ratio of 1.0: cREXX and the reference complete the same
  work at the same rate. The **strong band** is the programme's more demanding
  target of at least 1.5 times the reference rate on each qualified workload.
- **Noise-labelled** means the sample spread crossed the governed stability
  threshold even after the one allowed append. The median remains reported,
  but small changes should not be selected from that cell.
- **Peak RSS** is peak resident set size: an operating-system measure of the
  process memory resident in RAM. It is reported separately from speed.
- **RXAS** is cREXX's assembly representation; **RXBIN** is the assembled
  bytecode image consumed by the VMs. Hashing both proves which generated
  program was actually measured.
- **Profiling-off Release** is the normal optimized product without internal
  event-count instrumentation. Profiling builds help explain costs but are not
  used for the headline speed verdict.

## Headline scorecard

Throughput ratios are cREXX divided by the reference runtime, so values above
1.0 mean cREXX is faster. The aggregate is a geometric mean: it combines
ratios without allowing a very large win on one workload to dominate merely
because that workload has a larger numeric rate.

| Comparison | `rxvm` | `rxbvm` | Meaning |
| --- | ---: | ---: | --- |
| Five-workload aggregate versus ooRexx | 2.125x | 1.843x | Both VMs are ahead in aggregate; only `rxvm` clears the 2.0-times aggregate objective. |
| Five-workload aggregate versus NetRexx | 0.743x | 0.644x | Both VMs remain behind the decimal-semantics NetRexx portfolio. |

| Workload | `rxvm / ooRexx` | `rxbvm / ooRexx` | Current reading |
| --- | ---: | ---: | --- |
| Sieve | 7.214x | 5.339x | Large guarded win. |
| Permute | 8.005x | 7.015x | Large guarded win after the accepted direct-placement work. |
| Bounce | 3.903x | 2.963x | Both VMs are comfortably ahead of ooRexx. |
| Richards | 0.267x | 0.264x | Largest qualified common deficit; needs about 3.7–3.8x more throughput for parity. |
| Base64 | 0.720x | 0.725x | Needs about 1.38–1.39x for parity; cREXX measurements remain noise-labelled. |

These figures do not mean that Permute has no remaining work. It is far ahead
of ooRexx but only reaches 0.537x/0.470x of NetRexx. The reference runtime and
the mechanism being compared therefore matter when selecting future work.

## What “canonical” and the other labels mean

Performance comparisons become misleading when programs print the same answer
but perform materially different work. The scorecard uses explicit labels to
prevent that.

- **Canonical** means the recognized workload form and semantic contract for a
  runtime. It does not mean “fastest possible” or “identical source text.” For
  example, canonical Classic Rexx arithmetic is decimal, whereas forcing an
  imported binary arithmetic helper would measure a different mechanism.
- **Equivalent port** means language-specific spelling differs, but the
  algorithm, logical work, important object/allocation behavior and observable
  result are equivalent. The five common workloads use qualified equivalent
  ports where literal source sharing is impractical.
- **Qualified** means the cell passed its correctness and equivalence review
  and may enter its named score lane. A qualified workload is not automatically
  added to the five-workload aggregate; aggregate membership is versioned and
  deliberately stable.
- **Disclosed adaptation** means the implementation makes a material,
  documented substitution needed by that language or product. It may be a
  useful performance target, but it is reported separately rather than being
  presented as identical-source evidence.
- **Control** means the result deliberately measures a different mechanism or
  ceiling. Controls help explain the system but are not fair headline ratios.
- **Not comparable / diagnostic exclusion** means the program may be correct
  and informative, but its timed work differs too much to support a fair
  cross-runtime ratio. Exclusion is a correctness decision about the claim,
  not a failed benchmark.

“Common” therefore means common comparable work, not merely a benchmark that
exists in every language.

## Why the separate and excluded items are not all canonical common cells

### Towers — qualified separately

The old ooRexx Towers port represented disks with numeric identifiers and
stems. It produced the correct move count but removed the intended object
allocation and method-dispatch work. PERF2-08 replaced it with an ordinary
ooRexx object graph: one benchmark object and 14 disk objects per repetition,
mutable object links and the same 8,191 recursive moves.

That makes the cREXX/ooRexx object comparison qualified. It remains outside the
five-workload aggregate because the aggregate membership was already frozen
and the NetRexx Towers cell is a binary/JVM control rather than the approved
decimal-Rexx comparison mode. On the qualified cREXX/ooRexx lane, cREXX needs
about 3.05x (`rxvm`) or 3.11x (`rxbvm`) to reach parity.

### RexxCPS — canonical reference plus disclosed adaptations

ooRexx and Regina run canonical RexxCPS 2.2. cREXX uses the disclosed typed
2.2d port, while NetRexx uses the disclosed 2.2n adaptation. These retain the
benchmark's intended clause mix and native rate reporting but are not a common
source form, so RexxCPS is a separately governed target rather than a member of
the common-five aggregate.

The current cREXX rates are 37.929/35.546 million nominal clauses per second
versus canonical ooRexx at 38.091 million. This is near parity, but the
programme target is at least 1.5 times canonical ooRexx, not parity alone.

### Mandelbrot — excluded because the arithmetic contract differs

cREXX and the retained binary NetRexx form produce the required binary64
checksums. Ordinary ooRexx decimal arithmetic produces different results at
the decision sizes. Increasing `NUMERIC DIGITS` does not turn that decimal
model into binary floating-point arithmetic.

Using a foreign binary helper merely to obtain matching output would time the
helper rather than canonical ooRexx numerics. Mandelbrot therefore has no
published cross-runtime ratio. This is a more honest result than accepting a
program that prints the expected answer through different timed work.

### Storage — excluded because cREXX must build a different ownership shape

All implementations build the same logical tree and report the same node
count. Current Level B cREXX, however, needs both a `StorageNode` owner and an
`.object[]` for each logical array node. ooRexx and NetRexx need one container.
The additional owners, allocations and dispatches are material to an
allocation benchmark, so the cells are diagnostic rather than comparable.

Closing this honestly requires a separately approved ownership/container
design, currently associated with post-Release 1 Level G work. It is not a
small benchmark patch.

### List — excluded because ownership work differs

cREXX references are intentionally weak. The List port therefore uses an
arena to keep referenced nodes alive; the comparator object links own their
targets directly. Both forms produce the same list result, but the arena adds
real allocation and lifetime-management work. The current timings remain
useful diagnostics, not a common ownership score.

### JSON — excluded because the APIs measure different jobs

cREXX currently exposes a string/path API that reparses queries. ooRexx and
NetRexx build different retained document object models. Matching the final
count of eight operations does not make parsing, allocation, result ownership
or access behavior equivalent.

A common JSON score first needs an approved parse-once document and access
contract. That is a library/runtime and possibly language-design decision, not
an optimization to infer from the current timings.

### Lifecycle — honestly named, not imputed

The public tools do not expose one common “load but execute nothing” boundary.
The scorecard therefore reports compile or translate, cREXX assemble, and
load-to-first-result. It does not rename the last phase as pure load. Most
lifecycle series remained noisy after the governed append, so they are useful
orientation rather than a fine-grained selection result.

## Memory, artifacts and measurement noise

cREXX peak resident memory stayed around 16.6–18.2 MiB across the timed
workloads; ooRexx stayed around 17.1–17.3 MiB. NetRexx/JVM medians were much
higher for the comparable cells, but JVM memory is a separate product
dimension and is not blended into throughput.

All passing observations remain in the result. The process allowed exactly one
unchanged additional sample set for cells crossing the noise thresholds.
cREXX Base64 timing, four NetRexx RSS series and seven of eight lifecycle
series remained labelled noisy afterwards. No convenient high or low sample
was deleted, and there was no second retry.

The evidence bundle records exact source, RXAS, RXBIN, executable, library,
generated NetRexx, runtime and tool hashes. Its 54-entry recursive checksum
manifest verifies independently.

## Suggested future direction — not yet an approved plan

The scorecard suggests the following order, but it does not select or authorize
an implementation:

1. **Re-attribute Richards on the accepted product.** The accepted compiler
   placement change already removed a large proved receiver-copy expansion and
   improved Richards by about 21%. Richards is still the largest qualified gap,
   so the next useful question is which residual copy, call, value or ownership
   shapes remain and which have a mathematically safe static owner. Previously
   rejected frame-reset, mapping-ledger, pooling, slab and broad-layout designs
   should not be revived without materially different evidence.
2. **Profile the now-qualified Towers object lane.** Capture current payload
   shapes, object lifetimes, allocation size classes, reuse and high-water
   behavior before proposing pooling or value-layout work. The 3x gap is a
   machine ceiling, not proof of its cause.
3. **Stabilize and attribute Base64.** Reconfirm the noisy Mac lane only when it
   becomes decision-critical, then prove exact reductions in string copy,
   length scanning or materialization before timing a candidate. A reusable
   Base64 API is a separate product feature and should not be confused with a
   benchmark-speed shortcut.
4. **Keep the NetRexx gaps visible.** Richards is the largest, followed by
   Permute and Base64. Permute's excellent ooRexx result shows why a competitor
   gap alone cannot select a VM mechanism; the current cREXX work must still be
   attributed first.
5. **Treat RexxCPS as a representation/conversion guard.** It is close to
   ooRexx parity, and its conversion stream remains visible. Any selective
   representation retention needs an exact validity/invalidation contract and
   a measured hit/miss ceiling before implementation.
6. **Keep capability work separate.** JSON parse-once ownership, nested owned
   containers, a standard Base64 library API and a public load-only boundary
   each need their own semantic/product decision. None is implicitly approved
   by this scorecard.
7. **Continue the platform sequence only when authorized.** Apple results are
   valuable screening and handover evidence. Final selection still requires
   the planned Intel Linux GCC/Clang work, the separate supported Linux ARM64
   lane and supported Windows validation. Mac results cannot select the final
   VM/default stream or close the open code-layout decision.

Until a next slice is explicitly selected, Sieve, Permute and Bounce should be
retained as guards, Richards as the leading common outcome target, Towers as a
separate qualified object target, and Base64/RexxCPS as representation and
conversion-sensitive controls.

Primary evidence: [formal scorecard](evidence/2026-07-27-perf2-09-mac-closure/scorecard.md),
[workload dossiers](evidence/2026-07-27-perf2-09-mac-closure/workload-dossiers.md),
[PERF2-08 qualification](evidence/2026-07-27-perf2-08-qualification/) and
[live roadmap](ROADMAP.md).

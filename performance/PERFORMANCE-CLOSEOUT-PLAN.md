# cREXX performance closeout plan

Status: **Stages 1-4 complete; Stage 5 Apple scorecard complete with formal
Linux QA-C pending; Stage 6 in progress; Stage 7 remains plan-only**

Approved: 2026-08-18

Starting candidate: `0fe5618eb` (`Merge remote-tracking branch
'origin/develop' into develop`)

## 1. Purpose and authority

This plan closes the completed PERF3 and POSTPERF performance programme around
the current product. It does not create POSTPERF-06 and does not authorize a
new compiler, RXAS, VM, language, RXBIN, ABI or architecture optimization.

The closeout must leave:

- one current, correctness-gated performance result set;
- a bounded DECIMAL-01 retain/change decision;
- a truthful cross-runtime capability matrix;
- enduring as-built, results, roadmap, lessons and rejected-idea records; and
- a compact retained evidence set without historical checksum and build-log
  noise in the current tree.

Adrian approved execution of Stages 1-3, Stage 4 and, conditional on a green
Stage 4 commit, Stage 5 on 2026-08-18. Stage 4 was committed as `81f159186`,
and the Apple Stage 5 scorecard has run against that exact fresh source.
Stages 6-7 are fully specified here so the closeout boundary is not lost.
Stage 6 began with the current, less-technical [`RESULTS.md`](RESULTS.md)
scorecard report; the wider documentation-authority consolidation remains
open. Stage 7 remains plan-only.

## 2. Standing measurement and claim rules

1. Correctness precedes timing.
2. Formal timing uses ordinary profiling-off Release products, AC power,
   low-power mode off, a quiet host and serial samples.
3. Formal absolute cells use two warmups and ten recorded observations.
   Candidate comparisons use at least one warmup and 12 paired,
   balanced/interleaved rounds.
4. `rxbvm` and `rxtvm` are reported separately. Product `rxvm` is not counted
   as an independent third engine.
5. Throughput, lifecycle, RSS and artifact size remain separate scorecards.
6. No passing sample is removed without an independently demonstrated fault.
7. A 3% comparable-workload adverse result, 1% governed aggregate regression,
   correctness mismatch or invalid host state is a hard stop.
8. Canonical, adapted, control, not-comparable and unsupported cells are never
   pooled or silently relabelled.
9. Historical results remain available through Git. The current closeout
   reports current results rather than treating old checksum identity as a
   substitute for measurement.

## 3. Stage 1 - freeze the combined closeout candidate

Status: **complete - exact combined candidate frozen on 2026-08-18**

1. Fetch and merge the current remote `develop` without rebasing or losing the
   seven completed local POSTPERF commits.
2. Record the exact combined commit, branch relation and dirty-worktree scope.
3. Confirm that POSTPERF-05 is complete and that no further production
   performance stage is authorized.
4. Inventory the incorporated concurrency, Windows, Linux and macOS evidence
   and distinguish completed qualification from remaining work.
5. Freeze production implementation while Stages 2 and 3 run. Benchmark,
   scratch-candidate and evidence work must not alter ordinary production
   behaviour.

Exit: one named combined candidate and no unreported source divergence.

## 4. Stage 2 - clear-host current-provider and concurrency measurements

Status: **complete - DECIMAL current-provider blocks retained; scorecard test
guards transferred to Stage 4; redundant Mac concurrency replay waived**

### 4.1 Host gate

Before each timing block:

- confirm Adrian's reservation is still current;
- capture AC/battery, low-power, thermal, load, CPU and competing-process
  state;
- invalidate rather than statistically repair any block exposed to competing
  work; and
- run DECIMAL and concurrency campaigns serially, never concurrently.

### 4.2 DECIMAL-01 Gate 1

Run the already-qualified D0s, D0d, D1a and D1b L1-L3 matrix:

- current static/default `mc_decimal` at Common 18 and Classic 9;
- dynamic `mc_decimal` packaging control;
- unrestricted Common-18 `db_decimal` diagnostic ceiling, with mismatches
  visible and excluded from correctness-qualified ranking;
- qualified Classic-9 `db_decimal` speed-control cells only where checksums
  match D0;
- adapter arithmetic, conversion, comparison, copy/clear and context-sync;
- VM arithmetic, conversion, compare, context, ledger and compound kernels;
- product optimized, optimized-compiler/RXAS-noopt and full-noopt images kept
  separate.

Calibrate equal work so the fastest like-for-like cell lasts at least one
second. Retain raw results and separately report process/lifecycle cost, RSS
and artifacts.

### 4.3 Concurrency Mac QA-B disposition

The retained battery campaign remains diagnostic and is not relabelled as
formal AC evidence. It found no confirmed adverse guard; earlier accepted AC
baselines remain valid, Mac correctness/sanitizer/stress/Release/install and
package qualification pass, and later concurrency production changes retained
clean single-thread Release guards. On 2026-08-18 Adrian judged an unchanged
quiet-AC replay unnecessary and waived it as a closeout requirement. Reopen
Mac concurrency timing only for a relevant source change, a concrete evidence
inconsistency or a new governed performance question.

The prepared decimal optimizer-integrity and canonical RexxCPS guards are
test-set qualification rather than a standalone baseline. They transfer to
Stage 4 so all pre-release tests are prepared and verified together.

Exit: a valid Gate 1 cost baseline, with no outstanding Mac concurrency replay
or separately timed RexxCPS obligation.

## 5. Stage 3 - bounded DECIMAL alternative-library decision

Status: **complete - no change; current `mc_decimal` retained; no production
provider selection or integration authorized**

### 5.1 Candidate batch

The complete first batch is:

1. D2 tuned `decNumber`: a composed, isolated grid covering `DECDPUN` 3, 4, 8
   and 9; `DECNUMDIGITS` 18, 34 and 64; and relevant `DECBUFFER` values near
   18, 34, 64 and 128. Individual winners are not composed without measuring
   the composed build.
2. D4 `libmpdec`: hash-pinned external source, direct-core and CREXX-adapter
   PoCs covering arbitrary precision and the current context/rounding
   contract.
3. D3 `decQuad`: the already-vendored fixed-34-digit direct and adapter
   comparator. It is a fixed-precision ceiling, not an arbitrary-precision
   replacement.

Third-party source and builds remain in an external scratch directory. Record
the exact version or commit, URL, archive/source digest, licence, local patch,
compiler and build command. Do not vendor candidate source during this stage.

### 5.2 Layers and progression

1. Run L0 direct-core diagnostics for attribution only.
2. Run like-for-like L1 CREXX adapter cells including allocation, conversion,
   context and lifecycle.
3. A candidate proceeds beyond the screen only when it is semantically clean
   for the claimed context and improves at least two representative L1 modes
   by 15% or otherwise demonstrates enough attributable headroom to make a 10%
   L3 result plausible.
4. Qualifying candidates may run isolated L2/L3 VM/application comparisons
   using the frozen Gate 1 work and exact checksums. This does not make the
   candidate a production provider.
5. Retain neutral and negative results and the reasons for rejection.

### 5.3 Materiality rule

The final classification is based on correctness-qualified L3 application
results, not a library microkernel alone:

- **immaterial**: under 5% representative L3 improvement; retain current
  provider and record the candidate as closed;
- **roadmap only**: 5% to under 10%; record the evidence and reopen only if a
  real application or later platform result strengthens it;
- **material**: at least 10% improvement on the predeclared representative L3
  result, with at least two workloads favourable, no greater-than-3% adverse
  workload guard, acceptable lifecycle/RSS/artifact cost and no semantic
  mismatch.

A fixed-only candidate must additionally repay the future tag, promotion,
fallback and code-size costs. No hybrid design is inferred from a fast
`decQuad` result. If a result is material, stop and ask Adrian whether to open
the separately governed integration gate. If no candidate is material, record
the no-change decision in the enduring decisions/roadmap record.

Exit: one evidence-backed `no change`, `roadmap only` or `material - decision
required` verdict. Stop before production integration.

### 5.4 2026-08-18 D4 decision

The narrowed libmpdec 4.0.1 screen is complete. Its opt-in pointer-free
adapter passed the current provider contract and lifecycle proof, but the
formal L1 result lost 41.66% Common-18 and 26.28% Classic-9 arithmetic
throughput and about 42% conversion throughput. Comparison improved about 56%,
but that is only one representative mode and does not meet the two-mode L1
progression rule. A lean-capacity adapter repeat remained 38.04% and 25.65%
slower in arithmetic; a direct-core diagnostic still put libmpdec 22.26% and
6.61% behind decNumber. A final matched-capacity repeat narrowed those losses
slightly to 21.90% and 5.30% but did not change direction. This attributes the
result to both the library on this kernel and the view/commit cost imposed by
the current raw-copy VM value contract.

D4 is therefore rejected at L1 and did not run L2/L3. It did not classify the
separate D2 tuned-decNumber or D3 fixed-34 decQuad experiments. Evidence:
[`2026-08-18-decimal-01-libmpdec-screen`](evidence/2026-08-18-decimal-01-libmpdec-screen/).

### 5.5 2026-08-18 D2/D3 decision and Stage 3 exit

After Adrian confirmed the host clear, D2 ran all 48 composed tuning builds
against the current provider for one warmup and eight balanced calibration
rounds. The best balanced configuration was 0.23%/1.67% slower at
Common-18/Classic-9. The largest isolated gain was only 2.70% and carried a
1.74% loss in the other context. No D2 build demonstrated the 10% credible
headroom needed to justify a formal L1 screen. The first attempt was invalidated
in full because XProtect overlapped its final 11 seconds; the decision uses the
clear-host replacement capture only.

D3's adapter arithmetic was 44.40%/64.76% slower at Common-18/Classic-9, while
matched direct-core arithmetic was 49.28%/66.73% slower. The compact 16-byte
representation helped isolated copy/clear and context-sync work, but conversion
and comparison were also slower. Following Adrian's stated arithmetic stop
rule, D3 does not proceed to a formal L1 screen or L2/L3.

Stage 3 exits as **no change**. D2, D3 and D4 are all rejected with their
negative evidence retained. `mc_decimal` remains the current provider; no
default, plugin ABI, hybrid design or production integration is selected.
The enduring rejected-experiment lessons and reopening triggers are recorded
in [`DECISIONS.md`](DECISIONS.md); this bounded addition does not start the
broader Stage 6 documentation consolidation.
Evidence:
[`2026-08-18-decimal-01-stage3-calibration`](evidence/2026-08-18-decimal-01-stage3-calibration/).

## 6. Stage 4 - portfolio v3 and final source-quality review

Status: **complete - reviewed, calibrated and correctness-qualified
portfolio-v3 frozen on 2026-08-18**

### 6.1 Complete benchmark inventory

The v3 inventory must explicitly cover every current cREXX lane:

- AWFY/SOM: Sieve, Permute, Bounce, Richards, Towers, Storage, List,
  Mandelbrot, Json, DeltaBlue, CD, Havlak, Queens and NBody;
- Rexx/community and binary: RexxCPS and Base64 v2;
- capability/attribution: JSON parser/query/path controls, PARSE controls and
  compile/load/first-result lifecycle; and
- decimal: the complete CDB-1 core/application set and canonical RexxCPS
  decimal guard.

Stage 4 also qualifies the canonical RexxCPS whole-product rows, native CPS
provenance, decimal-string attribution and the prepared compiler-on/RXAS-on,
compiler-on/RXAS-off and full-noopt integrity controls. These are test and
equivalence gates for the scorecard, not Stage 4 performance results.

Canonical Java and CPython controls are required where an upstream canonical
source exists. They are separate language controls and never members of a
Rexx aggregate.

### 6.2 NetRexx capability labels

Every NetRexx row must receive exactly one evidence-backed classification:

- `qualified NetRexx capability`: the timed algorithm and state are expressed
  using genuine NetRexx language facilities, including `Rexx` decimal state
  where the common contract requires it;
- `not NetRexx capability - Java/JVM control`: primitive Java numerics,
  arrays, collections, libraries or hand-written/generated Java perform the
  material timed work, even if NetRexx launches or generates the class;
- `not supported by NetRexx language`: the required semantics cannot be
  expressed through the supported NetRexx language surface; or
- `not comparable`: the language can express a program, but preserving the
  timed algorithm/work contract is not feasible; or
- `failed`: a completed port attempt reached build, execution or qualification
  but did not pass the declared correctness/equivalence gate; or
- `unresolved`: no qualified genuine-NetRexx port was completed after the
  Stage 4 review, without misreporting the absence as a language limitation.

Generated Java and HotSpot are the normal substrate of a genuine NetRexx
program, but that fact does not make a primitive-Java implementation evidence
of NetRexx language capability. Java controls and genuine NetRexx results must
be presented in different columns and aggregates.

### 6.3 ooRexx attempt requirement

ooRexx must receive a real attempted port and correctness review for every
benchmark in scope. Each row is then classified as:

- `qualified ooRexx capability`;
- `not supported by ooRexx language/runtime`, naming the concrete missing
  capability;
- `not comparable`, naming the unavoidable algorithm/work change; or
- `failed`, for a completed attempt that did not pass build, execution or the
  correctness/equivalence gate; or
- `unresolved`, where no qualified port was completed and no concrete
  language/runtime prohibition was proved.

An unsupported/not-comparable result needs retained source notes or a minimal
reproducer. Performance alone is not a reason to call a language unsupported.

### 6.4 Final cREXX benchmark review

Before final timing, review every cREXX benchmark one last time for the best
fair use of the current language and library:

1. Compare algorithm, work count and observable result with the pinned
   canonical source or documented contract.
2. Review typed values, classes, interfaces, references, containers, binary
   storage, numeric context, string/JSON APIs, calls and allocation for an
   idiomatic cREXX implementation.
3. Remove accidental translation overhead where a normal cREXX facility
   expresses the same operation, but do not specialize away required work,
   precompute results, weaken checks, change algorithmic complexity or import
   host-native work unavailable to ordinary programs.
4. Preserve opaque inputs and observable checksums so the optimized result
   cannot disappear.
5. Classify every difference as a fair spelling/adaptation, a disclosed
   capability-specific implementation, a diagnostic control or not
   comparable.
6. Re-run opt/noopt and both concrete VMs after any source revision. Version a
   benchmark if its work or timing boundary changes; do not pool old and new
   results.

### 6.5 Calibration and adverse cREXX screen

Stage 4 may run bounded pilot measurements to choose Stage 5 work counts and
to identify a cREXX implementation that is plainly dominated by accidental
translation overhead. These pilots are calibration/diagnostic evidence, not
the pre-release scorecard: they use a small declared sample count, retain all
results and are not published as formal cross-runtime rankings.

Aim for approximately one second in the fastest Stage 5 cell without making a
very slow language cell impractical. Record any deliberately asymmetric work
or native-rate metric rather than pretending equal work. If a cREXX pilot is
catastrophically adverse, first confirm algorithm, work, result and timing
boundary, then look for ordinary current cREXX language/library facilities
that remove accidental overhead. A fair source-only improvement is allowed in
Stage 4, but required work must not be specialized away or moved outside the
timed boundary. Reversion or a disclosed diagnostic/adaptation label is
preferred when equivalence cannot be proved.

Any changed cREXX benchmark must repeat the source review, optimized and
unoptimized correctness, both concrete-VM checks, optimizer-resistance gate
and calibration. A production compiler, assembler, VM or library performance
change remains outside Stage 4 and requires separate approval plus the
mandatory first Release verdict.

Exit: a reviewed and correctness-qualified manifest-v3 test set plus
capability/equivalence matrix and calibrated Stage 5 work counts, with no
silent missing cell and no Java control presented as NetRexx capability. Stage
4 produces runnable scorecard inputs and diagnostic pilots, not formal
scorecard timings.

## 7. Stage 5 - one pre-release current-product scorecard

Status: **Apple scorecard complete on 2026-08-18; exact-commit formal Linux
QA-C pending**

1. Build the ordinary profiling-off Release product from the final frozen
   closeout commit.
2. Qualify checksums, expected results and optimizer resistance before timing.
3. Run two warmups and ten recorded serial observations per absolute cell.
4. Report cREXX `rxbvm`/`rxtvm`, ooRexx, genuine NetRexx, canonical Java and
   canonical CPython according to the Stage 4 labels.
5. Keep the common Rexx aggregate membership versioned and unchanged unless a
   separate governance decision changes it.
6. Report lifecycle, RSS, artifacts, concurrency and DECIMAL separately.
7. Complete formal Linux QA-C. Do not repeat Windows QA-D without a concrete
   evidence inconsistency or a relevant source change.

The Apple execution qualified all 89 cells and retained a complete timing,
RSS, lifecycle and current-artifact scorecard. The named cREXX/ooRexx
common-five is 5.467915x/5.679203x on `rxtvm`/`rxbvm`; the separately named
genuine-NetRexx common-four is 1.318977x/1.358087x. Honest separate-lane
findings, including CD, DeltaBlue, Towers, Storage and Havlak, remain visible
for a bounded final-release review. The unchanged Mac concurrency replay stays
waived and DECIMAL retains `mc_decimal`. Evidence:
[`2026-08-18-performance-closeout-stage5`](evidence/2026-08-18-performance-closeout-stage5/).

Linux local repair validation is strong but explicitly does not issue the
formal exact-commit QA-C result. No Linux host was attached to the Apple
scorecard task, so the required runner on `81f15918676d92a7d3e88954d94779ed759e9db8`
remains the only Stage 5 exit item. This is an external execution dependency,
not a request to repeat Mac timing or Windows QA-D.

Exit: one pre-release scorecard and compact raw-sample bundle for the frozen
source candidate. If the scorecard exposes a correctness, equivalence,
optimizer-resistance or capability-label problem, return the affected test to
the Stage 4 review rules, version any changed work/timing boundary and rerun the
affected scorecard scope. A final-release review may revisit tests for concrete
findings, but must not silently pool pre- and post-revision results.

## 8. Stage 6 - enduring documentation consolidation

Status: **in progress — current results report created; wider consolidation
pending**

Leave these authorities:

- `performance/README.md`: as-built operating guide and navigation;
- `performance/RESULTS.md`: current product/platform results only;
- `performance/DECISIONS.md`: accepted mechanisms, lessons, rejected ideas,
  reasons and explicit reopening triggers;
- `performance/PERFORMANCE-GOVERNANCE.md`: enduring measurement method;
- `docs/ROADMAP.md`: future work only; and
- dated reports plus Git history: historical snapshots.

Retire contradictory “live PERF3” and “active POSTPERF-05” wording. Do not
rewrite the dated 2026-07-15 charter as though it described the final product.

Exit: as-built, current results, future roadmap and historical lessons agree.

## 9. Stage 7 - evidence clean-down and final verification

Status: **plan only**

Keep in the current tree:

- final manifests and source/tool/runtime identities;
- consolidated raw sample CSVs and summaries;
- compact host/build provenance;
- correctness outcomes and benchmark-source digests; and
- negative decisions needed to avoid repeating failed ideas.

Remove after conclusions and links are consolidated:

- superseded calibration and duplicate raw bundles;
- generated RXAS/RXBIN/classes/binaries that are reproducible build products;
- configure/build logs and transient CMake caches;
- recursive historical checksum forests; and
- obsolete handoff prompts/worklists whose durable decision has moved to the
  decisions record.

The final compact bundle may checksum its current manifests, sources, tools
and consolidated raw tables. It must not require ignored or host-transient
files. Run link/inventory checks and reproduce the retained result boundary
from a fresh checkout before declaring closeout complete.

Exit: a clean current tree, reproducible current evidence, no lost negative
decision and no stale authoritative status.

## 10. Final completion condition

Performance closeout is complete only when:

- the current combined product is named and qualification-clean;
- DECIMAL has a recorded retain/roadmap/material decision;
- Mac concurrency performance and Linux qualification have dispositions;
- every portfolio-v3 cell has an honest runtime/capability label;
- every cREXX benchmark has passed the final fair-idiomatic source review;
- one current cross-runtime scorecard exists;
- enduring documents agree; and
- the compact retained evidence can be verified from a fresh checkout.

Commit and publication remain separate actions. Nothing in this plan
authorizes a push.

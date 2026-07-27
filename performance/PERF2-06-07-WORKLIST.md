# PERF2-06/07 combined Mac value and VM-ownership worklist

Status: planned; start in the next session after live-state verification

Planned: 2026-07-27

Purpose: finish the Apple-relevant value/reference, copy/move/clear,
representation, conversion, allocation and payload/frame ownership work as one
evidence-led slice. This joins only the still-relevant PERF2-06 ownership
boundary to PERF2-07. It does not reopen rejected frame reset/allocation PoCs,
complete `PERF2-06-D01`, select a final execution stream/default VM, or make a
cross-platform release claim.

## Approved programme sequence

1. Execute this combined PERF2-06/07 slice on Adrian's Apple ARM64 Mac.
2. Complete the PERF2-08 equivalence/capability gate on the Mac.
3. Run PERF2-09 closure, bounded PERF2-10 Apple controls and the PERF2-11 Apple
   pre-handover scorecard on the Mac.
4. Hand a frozen, hash-complete package to an Intel x86-64 Linux machine for
   GCC/Clang testing, native counters and final evidence-selected tuning.
5. Obtain the separately required supported Linux ARM64 validation evidence.
6. Boot the same Intel x86-64 hardware into the supported Windows environment
   and run the final matrix against the frozen candidate.

The Intel Linux stage is the principal final tuning stage. Windows is the final
same-hardware validation and scorecard stage. Any Windows regression reopens the
cross-platform decision; it does not authorize a Windows-only shortcut.

## Planning state

- Repository: `/Users/adrian/CLionProjects/CREXX`.
- Branch at planning time: `develop`.
- Planning-time HEAD and `origin/develop`:
  `53f7757c5` (`Merge remote-tracking branch 'origin/develop' into develop`).
- Planning-time main worktree: clean and aligned with `origin/develop`.
- The roadmap/worklist/handover documentation commit will follow that planning
  state. The execution session must record its actual live HEAD/upstream and
  must not assume `53f7757c5` remains current.
- Accepted VM-C1b implementation:
  `a39608426e2c1bb84d5fc0c4f767f4c9492339a9`.
- PERF2-06 tactical closeout:
  `d4dd0d84a5093f5530a86e63b75a943e707eaed1`.
- Publication state must be rechecked live; do not infer it from this file.

## Authority and scope guards

- `performance/ROADMAP.md` is live status; the dated programme report and
  closed evidence remain historical authority for their exact observations.
- `performance/PERFORMANCE-GOVERNANCE.md` governs sampling, uncertainty,
  regression budgets, aggregates and claim language.
- Canonical RXBIN, public RXAS, ABI, language behavior, TRACE/source/profile
  identity, signals/unwind, plugin/native ownership, late load and both VMs stay
  unchanged unless Adrian separately approves a decision paper.
- Maintained analysis and orchestration must be cREXX Level B. Native host
  sampling may be diagnostic input.
- Preserve clean/dirty user work. Use isolated source/build roots for PoCs and
  keep accepted baseline and each candidate independently identifiable.
- Do not rerun a retained valid baseline merely for freshness. Use a bounded
  same-session drift control when the decision requires it.

## Explicit negative boundary

The following are closed on the current Apple product and must not be retried:

- C2-A segmented allocation while retaining the full pointer map;
- C2-B or any ledger/bookkeeping on ordinary mapping writes or operand reads;
- fixed-core copies, reset-needed flags, exact reset lists or quickened
  link/use/unlink clearing derived from C2R01;
- changed-only numeric/plugin synchronization in the current frame model;
- cleanup-only reshaping, source deduplication or smaller-text edits in the
  flattened interpreter; and
- a generic allocator, mutable-object/string COW or hot/cold `value` rewrite as
  the first intervention.

A rejected form reopens only if a materially different ownership or
representation contract changes its machine mechanism, or the supported
cross-platform counter matrix selects it with zero-work drift controls.

## Numbered execution plan

1. Freeze live Git, accepted evidence, host/power/toolchain and ordinary
   Release/diagnostic build identities.
2. Reproduce and classify the open V3-R01 representation-validity regression;
   audit sibling in-place string-producing conversions before proposing a fix.
3. Refresh only the current value/reference/frame attribution needed to rank
   the combined panel after all accepted PERF2-02 through PERF2-06 changes.
4. Attribute each selected cost to the earliest safe owner: compiler/inliner,
   RXAS, private VM/helper, representation, allocator or frame/control boundary.
5. Complete the full bounded candidate panel and machine ceilings before the
   first production performance edit; present it to Adrian and stop for
   selection.
6. For an approved candidate, use a small isolated PoC, exact semantic tests
   and reduced machine work before ordinary Release product timing.
7. Take one accepted production slice at a time through the mandatory first
   Release verdict, report it and stop. Do not batch neutral or adverse work
   behind a winner.
8. After accepted closeout, refresh only the affected Apple portfolio cells,
   reconcile PERF2-06/07 dispositions, and build the Mac-to-hardware handover
   package.

## Stage 0 — live freeze and evidence audit

- [ ] Read repository and performance instructions plus the relevant
      interpreter/value/compiler documentation.
- [ ] Record branch, HEAD, upstream, dirty scope and all linked worktrees.
- [ ] Verify checksums for the accepted PERF2-01 baseline, PERF2-02 direct
      reference result, PERF2-05 value/relink assists, PERF2-06 VM audit,
      VM-C1b verdict, C2/reset rejections and C3 tactical rejection actually
      used.
- [ ] Record Apple model/CPU, OS, logical CPUs, AC/low-power/thermal/load state,
      compiler, CMake, Ninja and build options.
- [ ] Identify clean ordinary profiling-off Release, profiling/count and any
      native-sampling builds without sharing mutable artifacts.
- [ ] Verify current `sizeof(value)`, alignment, important field offsets,
      `stack_frame` size and plugin/native/reference side structures on Apple
      ARM64. Record target-ABI-dependent facts rather than generalizing them.
- [ ] Freeze exact optimized/no-opt workload and library hashes for selected
      cells.

## Stage 1 — correctness prerequisite and representation audit

### PERF2-07-V3-R01

Reproduce the exact empty-string, decimal `2.2`, same-destination `dcopy; dtos`,
then `strlen` sequence. The required result is string `"2.2"` and codepoint
length `3`; the current retained observation returns stale length `0`.

- [ ] Run optimized/no-opt on `rxvm` and `rxbvm` with byte-identical authored
      source/RXAS where expected.
- [ ] Audit every in-place operation that can replace string bytes while leaving
      cached byte/codepoint, numeric, decimal, binary or object validity state.
- [ ] Separate the correctness fix from any performance claim. Define which
      representation-validity bits/caches must be invalidated or preserved.
- [ ] Produce a distinguishing regression matrix, including typed-null,
      non-empty destination, Unicode/codepoint and alias/reference controls.
- [ ] Include V3-R01 in the selection panel. Do not silently install it while
      measuring unrelated value work.

## Stage 2 — current combined attribution

Use current optimized and no-opt images. Start with Bounce, Richards, Base64
and RexxCPS because prior accepted evidence assigned reference, whole-value,
conversion and residual call work there. Include Sieve and Permute as generic
VM/layout and call guards; add Storage, Towers, List or JSON only when their
current comparability status and exact mechanism make them valid diagnostics.

For each selected workload and both VMs record:

- general and typed copy/move/clear/reset counts and bytes by payload shape;
- reference creation, root/tree/cell, link/relink, invalidation and release
  operations, with owner frame/procedure/caller where available;
- string/binary/decimal/object conversion and materialization counts, retained
  representation hits/misses and invalidations;
- allocation/free requests and bytes by object, value payload, attributes,
  references, frames and plugins; lifetime/high-water/reuse and size class;
- frame entry/return work only where it is caused by the selected payload or
  reference ownership contract;
- top native helper/caller stacks and procedure/opcode paths;
- source/RXAS/RXBIN/linked-image and product text/data sizes; and
- steady-state, lifecycle, RSS and teardown as separate dimensions.

The refreshed panel, not pre-PERF2-02 numbers, decides whether Bounce reference
storage or Richards copying still dominates. A zero current mechanism count is
a rejection, not permission to optimize a historical profile.

## Stage 3 — candidate and placement panel

Every candidate needs exact mathematical/semantic correctness, a machine-work
ceiling, earliest safe owner, lifecycle/ownership rules, both-VM behavior,
fallback/failure behavior and a decisive end-to-end cell.

| Stable ID | Candidate | Required pre-timing proof | Initial disposition |
| --- | --- | --- | --- |
| PERF2-06-07-V1R01 | Eliminate a dead/full value operation or place a result directly using compiler/RXAS flow and typed copy/move facts. | Final-stream liveness/alias/exception/return proof; fewer exact operations/bytes; no hidden payload release. | preferred first owner when current profiles select it |
| PERF2-06-07-V1R02 | Reduce residual direct reference-helper/root/tree work without runtime site state. | Exact reference identity, owner discovery, invalidation, escape and teardown; lower helper/heap work in current Bounce-like cells. | panel only; PERF2-02 forbids assuming quickening |
| PERF2-06-07-V2R01 | Payload-shape fast copy/move/clear/reset for proved scalar, string, binary or simple non-native shape. | Complete destination cleanup and validity flags; no object/native/reference shortcut; exact operation/byte reduction. | panel only after V1 ceiling |
| PERF2-06-07-V3R01 | Correct and then selectively retain valid string/numeric/decimal representations across conversions. | Distinguishing stale-cache regression; mutation invalidation; numeric context and memory-growth accounting. | correctness prerequisite plus bounded performance panel |
| PERF2-06-07-V5R01 | Size/shape-specific payload capacity reuse or pooling. | Allocation-stack/lifetime/size-class/high-water proof; teardown, plugin/native and sanitizer visibility; beats narrower V1-V3. | conditional; request counts/RSS alone cannot select it |
| PERF2-06-07-C2R03 | Procedure-affine non-moving value slabs with a compact linear control stack and fixed-call embedded arguments. | A new payload-capacity mechanism beyond rejected pointer-map/reset forms; reference/native/object/decimal/signal/thread ownership; no hot mapping ledger; high-water comparison. | architecture option only if current V5 evidence selects it; Adrian decision required |
| PERF2-06-07-V6R01 | Hot/cold `value` representation or broad layout split. | Demonstrated cache/footprint owner on current products; ABI blast radius; complete payload semantics; supported-platform plan. | defer to hardware architecture matrix unless narrower work cannot close a measured cost |

Required comparisons include status quo, earliest safe static owner, narrow
helper/representation form and a hand-equivalent/native ceiling where useful.
Do not implement every row merely because it is listed.

## First mandatory stop — panel selection

- [ ] Present current attribution and the complete disposition table.
- [ ] Name the smallest candidate with a material machine ceiling and its exact
      target/guard cells.
- [ ] Identify candidates rejected without timing and why.
- [ ] Identify any language, public RXAS/RXBIN, ABI or architecture decision and
      stop for Adrian.
- [ ] Do not install a production optimization, run broad QA, polish a rejected
      PoC, start PERF2-08/09 or commit implementation work before selection.

## Stage 4 — approved PoC and production ladder

For each Adrian-approved slice:

1. create an isolated source/build identity and retain the exact patch;
2. add/run only the focused semantic and ownership fixtures needed for safe
   measurement;
3. prove fewer exact operations/bytes/allocations or a selected native-cost
   mechanism;
4. freeze implementation and build the ordinary profiling-off Release product;
5. run at least 12 balanced paired rounds on the smallest decisive target plus
   both-VM and unrelated/layout guards;
6. report and stop for accept/rework/revert;
7. after acceptance only, run proportional Debug/Release/sanitizer/lifecycle/
   package/RXBIN compatibility closeout and commit as directed; and
8. update the roadmap/evidence before proceeding to another slice.

Formal portfolio guards remain zero correctness failures, no worse than 1%
common aggregate, no worse than 3% comparable Tier A cell, plus the separate
lifecycle, RSS and artifact budgets in governance.

## Stage 5 — Mac closeout and hardware handover

PERF2-06/07 Apple completion requires:

- [ ] every high-cost current value/reference shape has an accepted slice or an
      evidence-backed reject/defer/owner transfer;
- [ ] V3-R01 has a correct explicit disposition;
- [ ] rejected C2/C3 forms remain closed and C2R03/V6 are either selected for a
      later architecture decision or explicitly deferred;
- [ ] accepted Apple product commits have focused and proportional broad QA;
- [ ] affected Apple Tier A cells, lifecycle, RSS and artifacts are retained;
- [ ] roadmap, worklist, technical docs and evidence checksums reconcile; and
- [ ] no cross-platform or final-superiority claim is made.

The hardware handover bundle must contain:

- exact source commit/tag candidate and clean-tree proof;
- source/product/workload/library/tool hashes and build options;
- accepted Apple scorecard plus raw samples and zero-work/layout controls;
- open `PERF2-06-D01`, PERF2-10 and Gate E/F decision ledger;
- reproducible GCC/Clang Linux x86-64 build, correctness, timing, RSS, artifact,
  `perf stat` and focused `perf record` commands;
- supported Linux ARM64 validation instructions;
- supported Windows build/test/timing commands for the same x86-64 hardware;
- regression budgets and exact stop/escalation rules; and
- a list of all rejected forms that must not be rediscovered as new ideas.

## Resumption rule

Reread repository/performance instructions and this worklist, verify the live
roadmap and Git state, replay only the accepted evidence actually used, and
resume at the first unchecked item. Keep verbose builds/profiles in temporary
logs and inspect focused slices. Never allow an isolated PoC or diagnostic build
to become the timing baseline by accident.

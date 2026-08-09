# PERF3-05-R3 handler code-generation and dispatch-tail analysis

Status: **in progress — approved through Clang/GCC diagnosis, proved internal
repairs and comparative remeasurement**

Approved: 2026-08-09

Purpose: explain why the R2 profile-30 handler panel regresses even though six
governed workloads execute no outlined handler, distinguish source/ABI defects
from compiler-specific layout behaviour, and repair only defects proved by
controlled Clang and GCC evidence.

## Authority and boundary

Adrian approved all proposed R3 steps on 2026-08-09: isolated source and build
locations, Clang source/expansion review, real GCC builds and formal timing,
cross-compiler output analysis, bounded diagnostic panels, internal defect
repairs, comparative remeasurement, documentation, retained evidence and local
commits.

This authority does not include a public RXAS/RXBIN change, plugin/public ABI
change, language change, install, push, merge, product-default selection or
cross-platform closure. Preserve the completed R2 checkpoint. Stop for a
correctness failure that cannot be repaired inside the approved internal
handler-placement architecture or for any required public/architectural
decision.

## Exact sources and tools

- R3 branch: `codex/perf3-05-r3-handler-codegen-analysis`.
- R3 worktree:
  `/Users/adrian/CLionProjects/CREXX-perf3-05-r3-handler-codegen-analysis`.
- Framework source: R2 commit
  `fd54b616764ef880270f4bce9dd202b476bf559c`.
- Untouched pre-refactor control: detached
  `6a65b9c685b3776da211bcd209af14fcf23be445`.
- Scratch root: `/private/tmp/crexx-perf3-05-r3.IvLL06`.
- Clang: Apple clang 21.0.0.
- GCC: Homebrew GCC 16.1.0 at
  `/opt/homebrew/opt/gcc/bin/gcc-16`; `/usr/bin/gcc` is Apple clang and is not
  an eligible GCC identity.
- GCC macOS builds use `CREXX_ENABLE_TLS=OFF` because the NETWORK backend uses
  Clang blocks. This limitation is recorded and is not a product-policy
  recommendation.

## Frozen observations from R2

- All-inline changes the compiled owner from the untouched source:
  `rxtvm` 535,556 to 532,512 bytes and `rxbvm` 530,528 to 531,868 bytes.
- Profile-30 places 176/588 non-reserved public handlers inline and emits 475
  outlined wrapper symbols. It loses 9.35%/12.08% geometric-mean normalized
  throughput on Clang `rxtvm`/`rxbvm` against all-inline.
- Sieve, Permute, Bounce, Richards, Base64 and Towers execute zero outlined
  handlers in the profile-30 policy; RexxCPS executes eight in roughly 23
  million instructions.
- All-inline direct-threaded `rxtvm` is 1.44% faster geometrically than
  switch-dispatch `rxbvm` when noisy Base64 is excluded, reversing the recent
  historical tendency and strengthening the compiler-shape question.

## Competing hypotheses

### H0 — intended compiler layout response

The framework is semantically and mechanically correct; merely changing the
owner population changes layout, register allocation and branch placement.
This remains a valid negative result if no avoidable source defect is found.

### H1 — pointer-facade alias and escape defect

Making any outlined call reachable keeps `rxvm_handler_state` live. The state
contains pointers to most mutable `run()` locals. Address escape and possible
mutation may force stack homes, spills, reloads and alias barriers across hot
inline paths even when no outlined call executes.

### H2 — interrupt/signal dispatch-tail defect

Every instruction polls the volatile pending-signal word and may enter the
owner `INTERRUPT` path. Outlined handlers additionally return a continuation
enum through one owner funnel. The combined CFG may cause Clang to merge or
reorder normal dispatch, interrupt detection and signal continuations, adding
hot loads/branches or changing indirect-branch prediction despite zero actual
signal entries.

### H3 — helper sub-inlining or compiler-budget effect

Helpers used from both retained wrappers and owner-inline bodies may receive
different inline/clone decisions. The 176 inline wrapper copies disappear from
the linked product, but retained cold wrapper call-graph size may still change
the compiler's decisions for hot owner code.

### H4 — handler-map/switch expansion defect

Policy expansion may accidentally change an opcode-to-label mapping, introduce
an extra lookup/continuation, or reshape the switch/computed-goto target for an
inline handler. Normalized preprocessing and label/case ledgers must prove or
reject this before timing is interpreted.

## Candidate repair designs

No design is selected before the diagnostic controls.

### A — current full pointer facade

Retain one pointer-rich state and the shared continuation enum. This is the R2
control and is acceptable only if the measured loss is irreducible layout
behaviour rather than avoidable alias/CFG damage.

### B — call-site snapshot with explicit commit

Construct an outlined-handler value snapshot only on the cold call edge and
return an updated snapshot/continuation. Commit mutable fields after the call.
This avoids pointers to owner locals but may copy excessive state and reload
more than a cold handler needs.

### C — narrowed handler ABI

Generate or group callable signatures/state views by the fields a handler can
read or modify. This can avoid global aliasing and reduce call cost, but it adds
an auditable access-classification surface that must remain complete as
handlers evolve.

### D — canonical execution-state owner

Move hot execution state into one struct used by inline and outlined forms.
This makes aliasing explicit and stable but changes all-inline register
allocation and was rejected in R2 as an equivalence control. Retain only as a
measured fallback if narrower forms cannot preserve performance.

### E — separate cold dispatch/continuation owner

Keep hot owner dispatch and interrupt polling independent of cold callable
continuations, possibly through a cold trampoline. This targets H2 but must not
add a hot branch, lookup or poll and must preserve direct-thread label
ownership, signal coordinates and early next-target resolution.

## Execution gates

### Gate 1 — Clang source and expansion sanity

- [ ] Capture normalized preprocessed all-inline, profile-30 and all-outline
      owners under identical Release flags.
- [ ] Prove every profile-selected inline handler has identical implementation
      tokens and opcode/label mapping between all-inline and profile-30.
- [ ] Prove actual interrupt selection/entry counts for governed workloads.
- [ ] Compare address-taken locals, facade lifetime and owner continuation CFG.
- [ ] Capture Clang inline/optimization records and owner assembly.

### Gate 2 — frozen GCC baseline

- [ ] Configure untouched, all-inline, profile-30 and all-outline Release builds
      with exact GCC 16 identity and TLS limitation.
- [ ] Build both concrete engines and pass focused dispatch, signal, interrupt,
      late-load and instrumentation tests for every shape.
- [ ] Record owner/text/artifact size, stack/build cost and wrapper symbols.
- [ ] Run one same-session balanced formal matrix containing all four shapes,
      both engines and the governed seven workloads.

### Gate 3 — cross-compiler output comparison

- [ ] Compare Clang/GCC hot handler tails, stack/spill shape, helper inlining,
      indirect dispatch sites, signal branches, label order and alignment.
- [ ] Attribute the profile-30 movement to H0-H4 with direct evidence and name
      every residual uncertainty.

### Gate 4 — bounded diagnostic controls

- [ ] Retain wrappers with no reachable wrapper call.
- [ ] Compare one/few/increasing outlined call-site panels while preserving the
      hot inline set.
- [ ] Isolate facade escape from the continuation funnel and interrupt fields.
- [ ] Use poll removal only as a labelled semantic-invalid ceiling; do not
      select it as a repair.
- [ ] Compare candidate B/C/E forms only where the preceding controls justify
      them.

### Gate 5 — proved repair and first Release verdict

- [ ] Implement only the smallest internal repair supported by Gates 1-4.
- [ ] Pass minimum focused correctness for both engines and compilers.
- [ ] Freeze implementation and run the smallest decisive profiling-off Clang
      and GCC Release comparison before broad closeout.
- [ ] Revert or retain the repair according to correctness, portfolio guards,
      owner size and compiler evidence; do not select a default panel here.

### Gate 6 — evidence and stop

- [ ] Run proportional broad validation for the retained repair.
- [ ] Retain one checksum-closed bundle with raw timing and compiler outputs.
- [ ] Update this worklist, `ROADMAP.md` and `RXVM_INTERPRETER.md`.
- [ ] Commit locally without push and stop before product/default selection.

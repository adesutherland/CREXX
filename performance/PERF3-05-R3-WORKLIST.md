# PERF3-05-R3 handler code-generation and dispatch-tail analysis

Status: **locally complete — compiler-specific repair retained; no product
default selected**

Approved: 2026-08-09

Purpose: explain the R2 profile-30 regression and its incorrect claim that six
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
- R2 reported that Sieve, Permute, Bounce, Richards, Base64 and Towers execute
  zero outlined handlers, with RexxCPS executing eight in roughly 23 million
  instructions. R3 retracts that completeness claim: public-opcode attribution
  hid hot process-private fused dispatch, most visibly `PRIVATE_R1_RELINK` in
  Bounce.
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

- [x] Capture normalized preprocessed all-inline, profile-30 and all-outline
      owners under identical Release flags.
- [x] Prove every profile-selected inline handler has identical implementation
      tokens and opcode/label mapping between all-inline and profile-30.
- [x] Prove actual interrupt selection/entry counts for governed workloads.
- [x] Compare address-taken locals, facade lifetime and owner continuation CFG.
- [x] Capture Clang inline/optimization records and owner assembly.

Gate 1 passed its source-integrity purpose. The normalized checker passed all
368 checks: both engines contain the same 651 handlers in the same order in all
three shapes; all 176 selected inline handler bodies are token-identical between
all-inline and profile-30; the non-handler owner skeleton is identical; and the
threaded 650-entry label map is identical. All seven governed profiles for both
engines record zero interrupt selection, entry and transition events, so H2 can
only act through polling/CFG shape in this workload set, not through a taken
signal path. H4 is rejected at source-expansion level.

The compiled evidence identifies two later-stage effects. First, the pointer
facade keeps owner locals addressable whenever wrapper calls are reachable,
preventing the all-inline scalar/register form even when no wrapper executes.
Second, on `rxtvm` the facade also makes the 650-entry local label-address map
escape. Clang therefore materializes the map on the owner stack: the total
frame grows from about 2,192 bytes all-inline to 6,832 bytes profile-30 and adds
a Darwin stack-probe call. `rxbvm`, which has no label table, does not have this
frame expansion, so it is a threaded-specific addition to the shared facade
cost rather than the whole explanation.

Clang's optimization records do not support H3 as a primary cause. Across the
176 retained inline handlers every successful helper-inlining decision is
unchanged. Six non-inlined helper sites are reported twice in profile-30 by the
remark pipeline, identically for both engines, but linked owner assembly has
fewer actual helper calls because the other handlers moved out. The owner
assembly also confirms that Clang already funnels normal computed-goto dispatch
through a central indirect branch in all-inline; outlined calls add a call and
continuation-result switch only when those cold labels execute.

### Gate 2 — frozen GCC baseline

- [x] Configure untouched, all-inline, profile-30 and all-outline Release builds
      with exact GCC 16 identity and TLS limitation.
- [x] Build both concrete engines and pass focused dispatch, signal, interrupt,
      late-load and instrumentation tests for every shape.
- [x] Record owner/text/artifact size, stack/build cost and wrapper symbols.
- [x] Run one same-session balanced formal matrix containing all four shapes,
      both engines and the governed seven workloads.

The eligible compiler is Homebrew GCC 16.1.0 targeting
`aarch64-apple-darwin25`; every tree records `CREXX_ENABLE_TLS=OFF`. All four
complete Release trees built. Each shape passed 14/14 focused dispatch, worker,
reentrancy, signal, instrumented-signal, breakpoint and late-load tests, plus
an explicit `METALOADMODULE` run under both concrete engines.

GCC's all-inline owners are much larger than Clang's: 1,493,900 bytes for
`rxtvm` and 1,478,368 bytes for `rxbvm`. Profile-30 reduces them to 537,568 and
539,648 bytes; all-outline reduces them to 104,512 and 110,304 bytes. GCC also
responds differently to the source refactor: untouched owner stack frames are
about 33,408/28,112 bytes for `rxtvm`/`rxbvm`, whereas all-inline is about
7,328/2,016 bytes. Profile-30 is about 7,040/1,776 bytes. The threaded 5,200-byte
label table is materialized in every GCC threaded shape, unlike Clang which
eliminates it only when the facade does not escape.

The formal same-session matrix ran on AC power with no thermal/performance
warning. It passed all 784 executions and retained all 672 recorded samples
(two warmups plus twelve recorded rounds per cell), with zero wrong outputs,
empty runner stderr and no discarded sample. Relative to all-inline,
profile-30 is +8.999% geometric-mean throughput on GCC `rxbvm` (+7.387%
without Base64). GCC `rxtvm` is mixed: +1.238% including Base64 but -2.239%
without it, ranging from +18.834% Sieve to -25.819% Bounce. All-outline is
-5.225%/-23.539% for `rxbvm`/`rxtvm` including Base64. Therefore outline count
is not intrinsically proportional to loss; compiler lowering and workload code
shape dominate.

### Gate 3 — cross-compiler output comparison

- [x] Compare Clang/GCC hot handler tails, stack/spill shape, helper inlining,
      indirect dispatch sites, signal branches, label order and alignment.
- [x] Attribute the profile-30 movement to H0-H4 with direct evidence and name
      every residual uncertainty.

H1 is confirmed directly in both optimized IRs. The all-inline facade and its
address-taking assignments disappear; profile-30 retains the facade plus
23/27 address-taking assignments in GCC `rxbvm`/`rxtvm` and corresponding
owner allocas and member pointers in Clang. The threaded form additionally
copies the 650-entry constant label map into a 5,200-byte owner alloca under
Clang profile-30.

H3 is rejected for the retained inline handlers. GCC has 478 normalized helper
inline/missed decisions across the 176 handlers with exactly zero differences
between all-inline and profile-30, for both engines. Clang has no changed
successful decision; six non-inlined sites are reported twice by its remark
pipeline but do not become extra linked calls. H4 was already rejected by the
368-check expansion ledger.

H0 is strongly supported as a material effect. GCC `rxtvm` retains 1,520
distributed indirect branches in all-inline and 247 in profile-30; Clang emits
only six/eight indirect branches in the entire owners and funnels normal
dispatch through one central branch. GCC `rxbvm` lowers the opcode switch to a
direct conditional decision tree with no indirect dispatch branch, so owner
reduction can improve that tree. Clang instead retains one central switch
dispatch plus facade stack traffic.

H2 is narrowed but not yet rejected: actual interrupt paths are never taken,
and the continuation-result switch is reached only after an outlined call, but
volatile polling and the cold continuation predecessors can still influence
Clang's hot CFG/register allocation. Gate 4 isolates this residual from H1 and
the threaded label-table escape. The remaining uncertainty is causal size of
each H1/H2 component, not source semantics, opcode mapping or helper inlining.

### Gate 4 — bounded diagnostic controls

- [x] Retain wrappers with no reachable wrapper call.
- [x] Compare one/few/increasing outlined call-site panels while preserving the
      hot inline set.
- [x] Isolate facade escape from the continuation funnel and interrupt fields.
- [x] Use poll removal only as a labelled semantic-invalid ceiling; do not
      select it as a repair.
- [x] Compare candidate B/C/E forms only where the preceding controls justify
      them.

Retaining all 651 unused wrapper definitions is neutral, as is a reachable
continuation funnel with no handler call. In contrast, one never-executed
outlined public site is sufficient to slow Clang materially; panels of one,
eight and 49 such sites remain adverse but are not ordered by count. A facade-
only snapshot recovers most of the owner size yet leaves workload loss. Moving
all outlined public identities to one shared cold owner entry and taking the
value snapshot only there recovers Sieve and Permute to roughly two percent of
all-inline in the bounded pilot. A narrower per-handler ABI is therefore not
justified at this gate.

The semantic-invalid poll-removal ceiling is deliberately rejected. It recovers
much of the switch loss but makes threaded dispatch substantially worse on some
workloads, proving that the interrupt poll participates in compiler shape while
providing no valid replacement for it. No governed timing run takes an actual
interrupt.

The remaining Bounce loss exposed a profiling defect rather than an interrupt
defect. Native samples show hot execution of `PRIVATE_R1_RELINK`; the R2 outlined
call census attributed that work to public `UNLINK` and therefore incorrectly
reported zero outlined handlers. Inlining both private fused handlers recovers
the pilot: relative to all-inline, Sieve is +0.891%/-1.329%, Permute
+1.140%/-0.521%, and Bounce +1.203%/-1.192% for `rxtvm`/`rxbvm` (positive is
slower elapsed time). The Clang repair is therefore candidate B plus the
shared-cold part of E, with the two proved-hot private identities explicit in
the inline policy. A first cross-compiler verdict then rejected applying that
lowering universally: GCC `rxtvm` lost 5.33% geometric-mean throughput without
Base64 versus its rebuilt all-inline control, while GCC `rxbvm` gained 13.67%.
The retained implementation therefore preserves GCC's faster R2 per-identity/
pointer-facade lowering and uses the value snapshot/shared-cold lowering only
for Clang. This is internal experimental-panel code generation, not a
product-default selection.

### Gate 5 — proved repair and first Release verdict

- [x] Implement only the smallest internal repair supported by Gates 1-4.
- [x] Pass minimum focused correctness for both engines and compilers.
- [x] Freeze implementation and run the smallest decisive profiling-off Clang
      and GCC Release comparison before broad closeout.
- [x] Revert or retain the repair according to correctness, portfolio guards,
      owner size and compiler evidence; do not select a default panel here.

The first formal verdict exposed two additional code-shape defects and was not
accepted. Removing all-inline facade scaffolding that optimized away changed
threaded layout: without Base64 the rebuilt all-inline `rxtvm` lost 5.09% under
Clang and 5.66% under GCC versus R2. Restoring the exact R2 all-inline source
shape recovered equivalence. The universal shared-cold form also caused the GCC
reversal described above, so the final compiler split was frozen and rerun.

The final balanced matrix contains 84 cells, two warmups and twelve recorded
rounds per cell: 1,176 executions, 1,008 recorded samples, no discarded sample,
no wrong output and empty runner stderr. Relative to the rebuilt R3 all-inline
control, geometric-mean normalized throughput is:

| compiler | engine | all seven | without Base64 |
|---|---:|---:|---:|
| Clang | `rxtvm` | -0.541% | -0.341% |
| Clang | `rxbvm` | +0.274% | +0.201% |
| GCC | `rxtvm` | +3.841% | +1.379% |
| GCC | `rxbvm` | +8.053% | +6.422% |

The rebuilt all-inline control is itself equivalent to R2 without Base64:
Clang `rxtvm`/`rxbvm` is -0.220%/-0.280% and GCC is -0.127%/+0.520%. The
profile panel contains 178 of 590 non-reserved public-plus-private definitions
inline (30.17%). Clang reduces `run()` to 205,548/205,444 bytes for
`rxtvm`/`rxbvm`; GCC retains its faster legacy shape at 547,808/549,632 bytes.
This completes the requested 30% measurement checkpoint without selecting it
as the default product shape.

### Gate 6 — evidence and stop

- [x] Run proportional broad validation for the retained repair.
- [x] Retain one checksum-closed bundle with raw timing and compiler outputs.
- [x] Update this worklist, `ROADMAP.md` and `RXVM_INTERPRETER.md`.
- [x] Commit locally without push and stop before product/default selection.

Both final profiling-off Release trees pass the 14-test focused signal,
breakpoint, dispatch, late-load, worker and reentrancy suite under each compiler.
The complete profile-30 Release suite then passes 2,002/2,002 under Clang and
2,002/2,002 under real GCC. Fresh complete all-outline trees also pass the
focused 14/14 suite under both compilers. Evidence is retained in
[`2026-08-09-perf3-05-r3-handler-codegen-analysis`](evidence/2026-08-09-perf3-05-r3-handler-codegen-analysis/).

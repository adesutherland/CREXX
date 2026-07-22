# NR-26 typed semantic flow analysis worklist

Status: complete; Adrian accepted the frozen three-transformation panel, the
closeout correctness gates pass, and the exact 41-instruction saving is
preserved

Started: 2026-07-21

This is the resumable control plane for a reusable, fail-closed `rxc` flow
analysis followed by bounded evidence-selected transformations. It is not a
string-comparison, conversion, copy-only or benchmark-specific activity. The
dated performance-programme report remains a historical charter and is not
edited by this work.

## Verified starting state

- Repository: `/Users/adrian/CLionProjects/CREXX`
- Branch: `develop`
- Starting HEAD: `4ab5f3d8da673c10b81af4249757763d052dda34`
- Upstream: `origin/develop`
- Relationship at start: `HEAD == origin/develop`; zero ahead and zero behind
- Starting worktree: clean
- Predecessors used as evidence:
  - NR-05 call census: 675,554 attributed defensive argument copies in the
    historical 22-image census, kept separate from return and general copies;
  - NR-08: accepted scalar `ENDLIFE` removal left `NULL` unchanged because it
    lacked must-initialize-before-read proof;
  - NR-09: exact compiler-owned temporary combinations are complete and remain
    separate from source-level path analysis;
  - NR-12 inspection: unconditional overwrite exposes a dead eager formal
    copy, while conditional and loop writes were explicitly deferred for
    path-correct flow facts;
  - NR-14: current clean accepted baseline at the starting commit above.
- Publish boundary: Adrian authorized the local gate-to-commit closeout on
  2026-07-22. Do not push without a separate explicit request.
- First-verdict boundary: after minimum focused correctness for the first
  production batch, freeze implementation, run the smallest decisive ordinary
  profiling-off Release comparison, report it and stop before broad CTest,
  sanitizer, package/install, documentation polish, commit or push.

## Hard boundaries

- Preserve language semantics, source/TRACE observation, reference lifetime,
  allocation/cleanup, signal behavior, ISA, RXBIN 007 and public ABI.
- Emit only ordinary self-contained RXAS. Do not add optimizer annotations,
  hints, pseudo-instructions or sideband metadata for RXAS.
- Keep source variables, compiler-created symbols/temporaries and eventual
  physical registers as distinct identities. The first analysis runs before
  register assignment.
- Unknown facts reject only the affected transformation at the affected site.
  They do not globally disable analysis for unrelated variables or blocks.
- Exposure, references, calls, objects, arrays, optional arguments, signals and
  TRACE are semantic fact changes, not permanent global exclusions.
- Any future language, public ABI, serialized RXBIN or VM architecture change
  remains a separate Adrian decision. None is selected here.

## Design selection

### A. Attach facts directly to the existing typed AST

This is the smallest storage change, but path facts would be scattered over a
mutable tree whose nodes are rewritten by inlining and dispatch lowering.
Joins, backedges, exceptional predecessors and repeatable fixed-point analyses
would need ad-hoc per-node state. It is suitable for local facts, not as the
primary reusable flow representation.

Disposition: rejected as the primary representation. AST nodes remain anchors
for source identity and transformations.

### B. Procedure-local CFG overlay over the final typed AST

Build basic blocks and edges after typed AST optimization/dispatch lowering and
before register assignment. Blocks point back to their AST anchors; values
point to source or compiler symbols, with expression temporaries represented
separately where needed. Run reachability, definition/use, definite-assignment,
liveness and reaching-definition fixed points over explicit bitsets.

Advantages: smallest path-correct reusable layer; no new language/emission IR;
natural branch/loop/early-exit/signal joins; analysis can be rebuilt after a
tree rewrite; transformations can remain AST/emitter-owned; no RXAS contract
change.

Risks: the structured-AST-to-CFG builder must fail closed for unsupported
control shapes and must model exceptional handlers without assuming that an
operation completed before it signalled.

Disposition: selected.

### C. Small typed lowering IR between validation and RXAS emission

This could make eventual compiler temporaries and instruction selection more
explicit, but it duplicates mature typed emitter behavior, cleanup fragments,
TRACE/source metadata and register allocation before evidence shows that
multiple transformations require the duplicate layer.

Disposition: deferred. Reconsider only if the CFG overlay repeatedly cannot
express selected transformations without parsing completed RXAS.

### D. SSA-like form

SSA would make single definitions, value equivalence and some dead-code rules
convenient, but phi placement, renaming, alias memory state, exceptional edges
and lowering back to mutable Rexx storage add substantial machinery.

Disposition: not selected. Reconsider only if several accepted transformations
need value-version joins that the reaching-definition overlay cannot support.

## Selected stage-one analysis contract

Per procedure, the first implementation will model:

- basic blocks, normal control-flow edges, early exits and structured signal
  handler edges;
- reachable blocks;
- value definitions and uses, including use-before-definition within a block;
- safe writes that complete without a language-visible signal before the
  destination update;
- definite assignment and must-write-before-first-read;
- first-read/first-write class, read/write counts, single-use and last-use
  anchors;
- live-in/live-out sets;
- reaching definition sites with per-value kill sets;
- known declared type and constant/copy definition anchors;
- exposed, argument, reference-target, dereference-alias, caller-owned,
  procedure-owned and compiler-generated identity facts;
- branch/loop joins and uncertainty/opaque-site masks; and
- the procedure numeric context and TRACE/source anchors needed by later
  transformations.

Stage-one limitations are implementation backlog, not permanent policy:

- nested `BLOCK_EXPR`, dispatch ASTs and any unrecognized control shape may be
  represented as an affected-value opaque site until their exact edge model is
  implemented;
- value equivalence and constants are recorded at definition sites first;
  propagation through arbitrary joins is deferred until a selected transform
  needs it;
- precise compiler expression-temporary lifetimes after register assignment
  are deferred; the overlay initially owns symbol-level facts and keeps AST
  expression identities separate;
- escaping exceptional edges are irrelevant to the first non-throwing scalar
  writes, while structured handlers must still be represented explicitly.

## Initial evidence-selected production batch

### Transform F1 - overwritten scalar default initialization

Suppress only the `NULL` operation for a procedure-owned scalar local when the
CFG proves that every path to every first read contains a prior safe write.
Keep variable metadata and source anchors in their existing order. Reject the
site if the value is exposed, caller-owned, reference-targeted, dereference
aliased, aggregate/object/reference storage, compiler-generated scaffolding, or
affected by an opaque control site.

Proof obligation: removing the default value cannot change any read, signal
handler observation, reference lifetime, cleanup, allocation, TRACE/source
event or final state. A definition whose RHS may signal before the write does
not satisfy the safe-write proof.

### Transform F2 - NR-12 overwritten scalar by-value entry copy

For a non-optional, non-reference scalar by-value formal, suppress the private
entry copy only when every path to every first read contains a prior safe write
to the private formal. The caller-owned incoming `aN` remains untouched; the
formal retains its distinct procedure-owned local register and all existing
metadata/status/call behavior.

Proof obligation: pass-by-value isolation is preserved because no read can see
the incoming value and all writes target the existing private local. Optional
presence/default work, large values, references/exposure and uncertain writes
retain the current path.

### Deferred first candidates

- inline formal/result scaffolding copy coalescing needs expression-temporary,
  ownership and block-result equivalence beyond F1/F2;
- general dead pure temporary/store elimination needs precise AST-temporary or
  final typed-emission facts;
- repeated conversions, compare results and numeric-context propagation wait
  for retained evidence after the foundational facts exist.

## Focused proof matrix

- [x] straight-line safe overwrite before first read
- [x] both sides of a branch overwrite before the join
- [x] only one branch overwrites and the other reads the default/input
- [x] zero-iteration and positive-iteration loops
- [x] nested loop and early-return paths
- [x] structured signal handler reads and handler-independent positive case
- [x] unrelated call, reference, exposure and TRACE constructs still permit an
      eligible independent scalar to optimize
- [x] reference-target, exposed and caller-visible negative cases
- [x] optimized removes only proved work; no-opt retains the reference form
- [x] exact RXAS before/after instruction/register/copy counts
- [x] identical outputs in `rxvm` and `rxbvm`
- [x] focused analysis-contract coverage for reachability, liveness, reaching definitions,
      joins and affected-value uncertainty

## Implementation and verdict ledger

- [x] Verify branch, HEAD, upstream and clean worktree.
- [x] Read repository/performance instructions, live roadmap, historical Strand
      4, compiler architecture/emitter/optimizer docs and predecessor worklists.
- [x] Audit retained NR-05/NR-08/NR-09/NR-12 evidence and current compiler
      pass/emitter/register-allocation boundaries.
- [x] Record A-D design comparison and select B before production editing.
- [x] Add the CFG/dataflow module and lifecycle ownership.
- [x] Add focused analysis and compiler fixtures.
- [x] Implement F1 and F2 behind their positive proof predicates.
- [x] Build only target `rxc` and focused dependencies during iteration.
- [x] Retain exact baseline/candidate RXAS and count deltas.
- [x] Run focused optimized/no-opt correctness in both VMs.
- [x] Freeze implementation after minimum correctness.
- [x] Build ordinary profiling-off Release and run the smallest decisive paired
      current-product comparison against a validated baseline.
- [x] Report the first Release verdict and stop for Adrian.
- [x] Record Adrian's ACCEPT decision and authorization for a local commit.
- [x] Harden counted-loop block ownership and generated semantic-reference
  reads exposed by the broad correctness gate.
- [x] Rebuild the affected Debug and ordinary profiling-off Release products.
- [x] Pass the final focused 8/8 regression set and full Debug CTest
  (1877/1877).
- [x] Reproduce the frozen 19-image census exactly and retain the narrow
  same-session RexxCPS artifact-drift control.
- [x] Remove disposable intermediate reductions and retain the final closeout
  evidence bundle.

## Frozen focused evidence

The implementation froze after `git diff --check`, target-only Debug `rxc`
builds and these passing focused CTests:

- `nr26_flow_contract`: positive and negative CFG cases, optimized/no-opt RXAS,
  assembly, and identical `ok`/RC 0 under both `rxvm` and `rxbvm`;
- `inline_test_byvalue_arg_reuse_{noopt,opt,run_noopt,run_opt}`: predecessor
  argument-isolation structure and runtime behavior;
- `nr09_codegen_contract`: existing optimized/no-opt benchmark structures and
  both VM paths; and
- eight focused reference-source optimized/no-opt tests across both VMs before
  the final must-loop correction. The post-correction NR-26 and NR-09 tests
  cover the corrected top-initialized loop fixed point directly.

The NR-26 fixture's exact procedure-local deltas are ordinary RXAS changes only:

| Transformation evidence | no-opt | optimized | Delta |
|---|---:|---:|---:|
| F1 eligible procedures | 7 `null`, 83 executable ops, unchanged `.locals` | 0 `null`, 76 executable ops, unchanged `.locals` | -7 ops |
| F2 eligible procedures | 7 copy ops, 17 executable ops, unchanged `.locals` | 4 copy ops, 14 executable ops, unchanged `.locals` | -3 copy ops |

The F1 total covers straight-line, both-branch, nested-loop, early-return,
unrelated-reference, TRACE-inserted and signal-handler-positive procedures.
The negative one-branch, zero-trip, reference-target and handler-read
procedures retain their defaults. F2 removes exactly the entry copy in the
straight-line, both-branch and early-return procedures; conditional, zero-trip
and read-before-write procedures retain it.

## First Release verdict

Verdict: **noisy/inconclusive; REVISE recommended, with REVERT still live.**
The implementation remains provisional, uncommitted and frozen. It is not
accepted for closeout.

The ordinary `cmake-build-release` product built successfully with `Release`,
`-O3 -DNDEBUG` and `CREXX_VM_PROFILING=OFF`. The accepted NR-14 baseline `rxc`
hash is `d34e19e573017b77785d1b5f739dc8912575dd4cc70c2ebeb36d1a99e5e7924d`;
the frozen NR-26 `rxc` hash is
`2c20d57825d7ac28c8fc80de6c5b0d89dd286d1d6cc42d1b0b1c7a4bdb3f4c61`.

The bounded portfolio audit found an F1 footprint in optimized RexxCPS but no
F2 footprint. Canonical optimized RexxCPS removes 16 `null` source-RXAS lines
and shrinks from 228,784 to 228,594 bytes; its no-opt RXAS is byte-identical.
The exact accepted linked image was reproduced at SHA-256
`e63425d71ab296ffdb4e8925f1c546e2d210fde311d1d8769c8b5687bb9ac440`
and 195,599 bytes. The candidate is
`392fbae1c826d584990758d02c913fbbd18a067e1eea4f758dad8a16dfcfca1f`
and 195,527 bytes, a 72-byte reduction. Disassembly shows four fewer packed
initialization operations in `main` and two fewer at `cps_subroutine` entry.

Formal same-session evidence used the canonical default RexxCPS contract,
serial balanced/interleaved cells, one warmup per cell and three retained
12-pair blocks. All 148 executions (four warmups and 144 recorded cells) had
RC 0, the expected PASS marker and exact `100 x 100` provenance. Percentages
are `(candidate / baseline - 1) * 100`; positive CPS and negative elapsed are
favorable. Intervals are two-sided 95% Student-t intervals around the mean.

| Lane | VM | Pairs | Paired median | Mean 95% interval | Favorable |
| --- | --- | ---: | ---: | ---: | ---: |
| native CPS | `rxvm` | 36 | +0.601% | -0.294% to +1.310% | 24/36 |
| native CPS | `rxbvm` | 36 | +0.317% | -0.331% to +1.150% | 20/36 |
| process elapsed | `rxvm` | 36 | -0.596% | -1.256% to +0.352% | 23/36 |
| process elapsed | `rxbvm` | 36 | -0.339% | -1.086% to +0.345% | 20/36 |

All four absolute CPS cells crossed the approved noise rule. A first append
left both paired intervals crossing zero, so the final 12-pair append reached
the 36-pair cap. The direction remains favorable, but neither VM proves a
nonzero effect. A final-block attempt was aborted before retaining any sample
when the pre-start host check showed CLion at 281% CPU; the distinct retry is
the valid third block.

The current F1/F2 production benefit is therefore too small to accept on
performance evidence alone. Recommended next decision: revise the selected
consumer set around a stronger evidence-backed transformation while retaining
the frozen worktree for review, or revert NR-26 if no such consumer is wanted.
Do not land the current scope as a completed performance change. The compact
evidence is in `performance/evidence/2026-07-21-nr-26-first-release-verdict/`.

## Adrian's revision direction

Selected: 2026-07-21.

NR-26 will continue by assembling a stronger coherent transformation panel.
During panel construction the acceptance test is semantic correctness plus
exact instructions avoided; do not run a separate performance campaign after
each transformation. Once the panel is complete and its individual static
deltas remain attributable, freeze the combined product and run one formal
performance sweep under the agreed governance methodology.

### Development gate

Each panel candidate must retain:

- an exact semantic proof stated as necessary predicates rather than a broad
  construct blacklist;
- positive and negative optimized/no-opt structural cases;
- runtime-equivalent output and status under both VMs for the focused case;
- exact source RXAS instruction counts before and after, with register and
  TRACE/source-observation deltas where relevant; and
- a bounded current-product occurrence census. Retained dynamic counts may be
  used when already available, but no timing result is required during panel
  construction.

Unknown facts reject only the affected value, definition or use. Exposure,
aliasing, calls, signals, TRACE and aggregate storage must be modelled as
specific memory/observation effects wherever that is sufficient; they are not
grounds for disabling an otherwise independent procedure or block.

### Stronger-candidate discovery queue

1. Compact TRACE-correct `ILOADSETUNLINKN`: route the existing wide
   trace-temporary form to the already-defined compact opcode when equality,
   dead-temporary and trace-retargeting facts are proved. Retained NR-09
   evidence: 364,203 bounded executions.
2. Dead pure compiler-owned stores and single-use copy/result scaffolding:
   use per-definition liveness and exact ownership rather than source-symbol
   category exclusions, preserving RHS effects and source/TRACE events.
3. Flow-sensitive constant/copy propagation: replace a use only when its
   reaching-definition set proves one equal value, then rerun existing folding;
   model only mathematically possible indirect writes as kills.
4. Result-only arithmetic fusion: retained `FDIVSUB` evidence covers 501,000
   bounded executions, but changing the serialized opcode's observable
   quotient write is an ISA/RXBIN semantic decision and remains outside the
   current no-architecture-change panel until Adrian selects that design.

The discovery census will rank these by instructions avoided and current
portfolio reach before the next production transformation is selected.

## Stronger panel slice P1 - must-copy and dead-store fixed point

Status: **passes the construction gate; retained provisionally in the panel.**
No timing verdict has been run for this slice.

### Exact transformation predicates

The consumer is limited to plain local/procedure boolean, integer and float
variables with equal source/target types. A value is rejected only when it is
an attribute, aggregate, reference/exposed/global store, reference target,
known dereference alias, or opaque at the affected definition/use.

For each reachable block, the must-copy lattice records a direct `x == y` fact
only after an exact scalar `x = y`. A write kills the written value and direct
dependants; predecessor facts meet only when the same equality reaches every
path. Equality chains are resolved at the use with cycle rejection. The
authored AST symbol and TRACE name are retained; only the physical read
register is retargeted.

Scalar promotions are destructive in the current emitter. A promoted read may
share the source register only when that register has one resolved read in the
statement and its old value is not live out. Otherwise that use alone retains
the copy. This is covered by positive dead-source and negative live-source
conversion cases.

After substitution, liveness is rebuilt. An assignment destination store is
omitted only when its value is not live out and is not opaque; RHS evaluation,
signals, cleanup and assignment TRACE remain. Copy propagation and dead-copy
marking iterate monotonically. Only a skipped exact `x = y` is treated as an
absent physical definition during the next iteration—computed RHS operations
may already emit into `x` and therefore keep their definition.

A semantically writable small-scalar formal shares its incoming `aN` slot only
when every reachable definition is one of those skipped exact copies, the
assignment contains exactly one write to the formal, and no opaque definition
exists. A computed write retains the private formal and defensive entry copy.

### Exact bounded result

The final 19-image optimized census is 50,965 to 50,924 source-RXAS
instructions: **41 instructions avoided**. All count changes are copies:
`copy -11`, `icopy -30`, with no other mnemonic delta.

| Surface | Images with savings | Instructions avoided |
|---|---:|---:|
| benchmark workloads | 5 | 35 |
| performance-tool selftests | 3 | 6 |
| total bounded 19-image set | 8 | 41 |

The benchmark reductions are Mandelbrot 1, Permute 8, Richards 12, Towers 10
and Base64 4. Sieve and both RexxCPS images have intended equal-register/TRACE
retargeting but no instruction-count reduction, so they receive no credit.
The exact 19 rows and both RXAS hashes are retained in
`evidence/2026-07-21-nr-26-panel-construction/static-instruction-census.csv`.

The focused contract passes optimized/no-opt structural cases, loop and join
proofs, all enabled scalar types, computed-overwrite negatives, destructive
promotion guards, effectful RHS retention and incoming-slot sharing. It emits
exact `ok`/RC 0 under both VMs. The 11 changed/retargeted final Release images
pass their registered `rxvm` checks and manual linked `rxbvm` checks; adjacent
`nr09_codegen_contract` and `rxc_inline_byvalue_arg_reuse` also pass.

### Discovery dispositions after P1

- Compact `ILOADSETUNLINKN` is not currently a no-architecture transformation.
  The compact form unlinks the alias before the following register-backed
  literal TRACE event, while the wide form deliberately retains the loaded
  register for that observation. Without an equal live register, immediate
  TRACE representation or an opcode/metadata change, retargeting would observe
  the restored pre-store value. This is a mathematically necessary rejection,
  not a whole-procedure fail-close.
- General flow constants have the same register-backed variable-TRACE boundary
  when replacing the final value with an immediate. The equal-live-register
  subset is P1 copy propagation; immediate propagation remains deferred until
  it proves additional instructions without losing authored variable events.
- Result-only `FDIVSUB` remains outside the no-architecture panel because the
  serialized instruction's quotient write is observable.
- Counted-loop source-register reuse exposes only four nearby count copies in
  the current 19-image census. It is not selected: destructively reusing an
  authored count register adds a new loop/handler/TRACE proof surface for a
  smaller static result than P1. The idea remains available for a separately
  ranked later slice.

## Combined panel freeze

Frozen: 2026-07-21.

The bounded no-architecture panel is now the requested smallest three
production transformations:

1. F1 proved-overwritten scalar default initialization;
2. F2 proved-overwritten scalar by-value entry copy; and
3. P1/F3 small-scalar must-copy propagation, dead-copy fixed point and the
   consequent guarded incoming-slot share.

Implementation is frozen. Compact literal-store fusion and constant
immediates are blocked by the current register-backed TRACE representation;
result-only arithmetic needs an ISA decision; the four counted-loop setup
copies do not justify a fourth transformation. The combined formal
profiling-off Release sweep compares the accepted NR-14 product with the frozen
panel on the five benchmark workloads where P1 removes instructions plus
canonical RexxCPS for F1, under both current VMs.

## Combined panel first Release verdict

Verdict: **ACCEPT recommended on Adrian's correctness-plus-instruction-
avoidance gate; stopped for explicit direction.** The implementation remains
provisional, uncommitted and frozen.

The formal ordinary profiling-off Release sweep used serial,
balanced/interleaved cells, one warmup per cell and three governed 12-pair
blocks. All 888 executions—24 warmups and 864 recorded samples—returned zero
and matched their required marker. Both variants ran under the same current
`rxvm` and `rxbvm`; timing images were linked source/TRACE-stripped with `-s`.

At the 36-pair cap, Permute/rxbvm is the one statistically clear elapsed
improvement: paired median -0.662%, mean 95% interval -2.438% to -0.375%,
28/36 favorable. The other elapsed and RexxCPS native-rate intervals cross
zero and are retained as noisy/inconclusive. No interval is wholly
unfavorable, no workload reaches the 3% regression guard, and every candidate
linked image is 8 to 128 bytes smaller.

This is a narrow first-verdict sweep over the exact five statically affected
benchmarks plus canonical RexxCPS, not the complete Tier A portfolio or the
normative five-common aggregate. It therefore supports accepting the panel on
the selected construction gate without claiming a release-wide speedup. The
compact raw and reduced evidence is in
`evidence/2026-07-21-nr-26-panel-first-release-verdict/`.

## Accepted closeout

Adrian selected **ACCEPT** on 2026-07-22 and authorized the shortest local
gate-to-commit path. The broad Debug gate exposed two correctness defects in
the provisional analysis: a counted-loop header traversal crossed into its
separately modelled initializer/body, and generated `VAR_REFERENCE` selector
nodes were not always counted as semantic reads. The implementation now keeps
each traversal within its exact CFG block, restores the synthetic latch's
implicit control use, and treats a semantic variable-reference node as a read
even when its legacy connector flag is clear. Focused regressions cover both a
loop-backedge kill and opaque integer jump dispatch.

The final focused set passes 8/8 and the required full Debug CTest passes
**1877/1877**. The ordinary profiling-off Release surface rebuilds, and all 19
optimized source-RXAS images exactly reproduce the frozen census: 50,965 to
50,924 instructions, **41 avoided**, exactly `copy -11` and `icopy -30`. The
five changed benchmark linked images reproduce their accepted hashes.

The corrected standard library made the canonical RexxCPS linked image 24
bytes smaller, so the closeout ran a narrow same-session old/corrected drift
control under both VMs. All 52 executions pass; both paired intervals cross
zero and no individual elapsed pair reaches the 3% guard. This preserves the
accepted no-regression decision without adding a new performance claim. Final
evidence is in `evidence/2026-07-22-nr-26-closeout/`.

Sanitizer and install/package proof remain outside this approved shortest
closeout path. No push is authorized.

# PERF3-10 trace-safe redundant conversion elimination

Status: **complete — C1/T1 accepted and closed on Apple ARM64**

Approved: 2026-08-01

Purpose: prove whether the locked RXAS storage-identity/typed-continuation
infrastructure can eliminate repeated integer-to-string materialization in
ordinary compiled code while preserving every ordered TRACE value event.  The
first target is the canonical RexxCPS hot path; the design must be reusable
and must fail closed outside the proved component, storage and signal model.

## Authority and boundary

Adrian approved a production proof of the algorithm and, if the proof is
successful, making it a core flow capability rather than another tactical
RexxCPS rule.  Adrian also approved fixing the TRACE boundary so an executed
trace event delivers all past ordered value events without requiring a `CNOP`
solely as a metadata separator.

This activity may:

- extend the RXAS flow model with component-aware, storage-identity facts;
- make TRACE result-event delivery ordered and lossless at a shared reached
  instruction boundary;
- remove an `ITOS` only when its string representation is already proved for
  the same storage identity and numeric context; and
- add focused compiler, assembler and both-VM tests.

It must not change the public RXAS/RXBIN format, public VM ABI, benchmark
source, clause counts, TRACE source-step contract or ordinary semantics.  It
must not infer events from a numeric address range across a branch, call,
signal, skip or retry transition.  No push is authorized.

## Exact starting point

- Branch/HEAD: `develop` at
  `6759a8cef543ca496be246402fa623edbb101c56`.
- Upstream comparison point: `21fdcf529d0e51ea264bf0c92ccfbdc06dea8200`;
  the branch is four local commits ahead.
- Five pre-existing untracked lifecycle `lifecycle_probe.rxbin` evidence files
  remain protected and outside this activity.
- An ordinary profiling-off Release rebuild completed before any production
  edit.  Exact C0 artifacts are retained outside the repository at
  `/tmp/crexx-perf3-10-baseline.pkyQyD`:

  | Artifact | SHA-256 |
  | --- | --- |
  | `rxas` | `7cec2f1245da24cf24ffd37ec94faba0309dfc41f088d5b94c382522c0e9258d` |
  | `rxvm` | `94d5202292ea6ec6b0ce019f86f572501a5a54820b2a0d16b7c96f0e24a0befd` |
  | `rxbvm` | `f4e853f5865a54a2f1c97cd09c4a2fb093cf1779059086e10cb996ba971d3f18` |
  | `benchmark_rexxcps_levelb_opt.rxbin` | `d85192dab1d74e54a71029f530992396b8f073b75624a10e4eac85dc96a4d1c7` |
  | `library.rxbin` | `51e42735d5e3c58ebd80bef48c9b1d52532d809f961e2099094b4f0dd3a42ea8` |

The pre-edit build log is
`/tmp/crexx-perf3-10-baseline-build.XXXXXX.log`.  Temporary retention is
sufficient through the mandatory first verdict; accepted closeout will create
a compact checksum-closed repository evidence bundle.

## Measured entry evidence

- Canonical RexxCPS executes 26,448,435 VM instructions.
- Conversion instructions account for 3,009,511 (11.379%): 1,184,406 `ITOS`,
  780,200 `STOD` and 1,043,400 `DTOS`.
- The timed `main` loop calls `cps_subroutine` 131,600 times and contains six
  compiler-emitted `ITOS` sites on the same `lvar` integer register with only
  one integer update at the loop latch.  The retained C0 assembler already
  removes one of those sites, leaving five in the binary.
- The same conversion population is negligible in Base64, Richards, Sieve and
  Towers, so a RexxCPS improvement must be guarded against neutral-work
  regressions rather than extrapolated to the portfolio.
- Current TRACE metadata attaches to the next instruction boundary.  The
  controller currently returns after the first visible value event, so two
  events sharing a reached boundary lose later events.  This is the actual
  correctness blocker to removing an executable separator.

## Options retained for replay

### C0 — current unconditional materialization

Every compiler-emitted `ITOS` executes.  TRACE values remain separated by
executable addresses where the existing pipeline happens to provide them.
This is the exact retained baseline.

### C1 — static storage/component proof plus ordered TRACE batching

Selected for implementation.  Track a representation fact by RXAS storage
identity, source component and numeric-context generation.  Transfer it
through proved direct link/swap/unlink mapping, meet it at joins, and kill it
on relevant component writes, context changes, unproved aliases, indirect
writes, calls or opaque/signal effects.  Keep all TRACE events and drain every
visible event at the reached boundary in emitted order.

### C2 — compiler loop-invariant conversion hoisting

Retained, not selected.  It can move a conversion to a loop preheader when
the compiler proves both value and context invariant, but it duplicates a
lower-level reusable representation fact and covers fewer non-loop cases.

### C3 — private cached-conversion VM operation

Retained fallback.  A private opcode or internal dispatch helper could check
runtime representation provenance and avoid formatting.  It requires a wider
runtime contract and may add a check to every conversion.

### C4 — universal runtime validity flags

Retained last resort.  Register/value flags could invalidate and reuse cached
representations broadly, but add state and likely overhead to unrelated
programs.  Reopen only if static proof leaves a material common case.

TRACE delivery also keeps two explicit states: T0 is the current one-event
boundary behavior; T1 is the selected ordered batch at one reached boundary.
An executed canonical-span T2 for private VM superinstructions is compatible
future infrastructure, but is not required to prove C1 and must not be
implemented by scanning skipped numeric addresses.

## Semantic obligations

1. A removed conversion's destination string component equals the current
   product's conversion for the same integer storage and numeric context.
2. Any integer or string write to that storage, numeric-context mutation,
   ambiguous identity, reference/indirect mutation, unproved call effect or
   unsupported signal phase kills the fact.
3. Facts survive a join only when true on every executable incoming normal
   path.  Typed signal skip/retry/handler continuations fail closed unless
   their exact phase is proved.
4. `LINK`, `SWAP` and `UNLINK` may transfer a fact only through the locked
   storage-identity service; raw register-number equality is insufficient.
5. TRACE metadata is an ordered event stream.  Every visible result event at a
   reached boundary is emitted once, in metadata order, even when its producer
   instruction was statically removed.
6. TRACE component observation constrains only the component named by its
   value type.  It is not automatically an executable anchor or an all-view
   barrier.
7. Debugger `.srcstep` identity and instruction stepping remain distinct from
   TRACE result batching.
8. Canonical benchmark source and workload are immutable.

## Work stages

### Stage A — control and red proofs

- [x] Preserve exact ordinary Release C0 binaries and record hashes.
- [x] Record C0-C4 and T0-T2 so rejected alternatives remain replayable.
- [x] Add a regression that exposes multiple ordered TRACE result events at
      one reached boundary and fails under T0.  The optimized fixture places
      assignment, caller-value, inlined-argument and callee-value events on
      one boundary; the no-opt fixture independently shares assignment and
      caller-value events.  Current T0 emits only the assignment.
- [x] Add RXAS fixtures for same-storage success, direct link/swap transfer,
      integer and string writes, numeric context, missing/dominating joins,
      unescaped/tainted indirect writes, calls and retained ordered TRACE
      metadata before enabling the rewrite.

### Stage B — ordered TRACE event delivery

- [x] Replace the scalar first-event return with an ordered pending-event
      cursor/batch.
- [x] Drain all visible result events from the trace exit on both VM variants.
- [x] Prove event count, order, value, mode filtering and no-event behavior.

### Stage C — reusable component/storage fact

- [x] Represent exact component read/write/derivation information at the
      RXAS flow boundary without changing serialized opcode formats.
- [x] Key the `ITOS` fact by storage identity and numeric-context generation.
- [x] Transfer through proved mapping and joins; fail closed at all unproved
      writes, calls, references, opaque operations and signal phases.
- [x] Remove only redundant `ITOS`; retain adjacent TRACE events unchanged.
- [x] Confirm the canonical optimized RexxCPS image removes proved sites.  The
      compiler emits 24 static `ITOS`; retained C0 contains 17 and C1 contains
      14.  The hot `main`-loop `lvar` sites are six in emitted RXAS, five in C0
      and two in C1.  The incremental C1 effect is three static sites.  Wider
      portfolio drift remains deliberately outside the first-verdict gate and
      no portfolio performance claim is made.

### Stage D — minimum correctness

- [x] Focused RXAS optimizer positive/negative fixtures pass.
- [x] TRACE batching and optimized-away value tests pass under `rxvm` and
      `rxbvm`, optimized and no-opt where applicable.
- [x] Canonical RexxCPS result, clause count and workload output match C0 under
      both VMs.
- [x] Zero-work applicability reviewed.  The conditional first-verdict guard
      was not triggered because neither VM executable nor dispatch path was
      changed; no zero-work timing claim is made.

### Stage E — mandatory first ordinary Release verdict

Once the minimum focused correctness gate passes, freeze implementation.
Build the ordinary profiling-off Release product and run a balanced,
interleaved, paired C0/C1 comparison using the preserved binaries/images:

- canonical RexxCPS on `rxvm` and `rxbvm`, initially 12 pairs per cell; and
- one zero-work control on both VMs if the Release image or dispatch path is
  touched outside the exact RexxCPS artifact.

Report paired medians, pair direction, spread, exact correctness counts,
emitted `ITOS` delta and dynamic-instruction delta.  Stop for Adrian's verdict
before broad Debug CTest, sanitizer, install/package proof, documentation
polish, T2 superinstruction work, wider conversions, scorecard refresh or
commit/push.

### First Release verdict — 2026-08-01

The minimum gate passed and the implementation is frozen.  Focused validation
passed 53/53 checks, including the complete selected TRACE set, opcode
metadata, positive and fail-closed RXAS flow cases, both-VM optimized/no-opt
runtime cases and canonical Debug RexxCPS under both VMs.  The new TRACE batch
test was a genuine red proof: T0 failed all 4/4 optimized/no-opt and
`rxvm`/`rxbvm` combinations before the implementation, then T1 passed 4/4.

The ordinary profiling-off Release product built successfully.  The balanced,
interleaved first-verdict schedule passed all 52/52 initial executions (four
warmups and 48 recorded samples).  RXVM crossed the standing spread threshold,
so the governed ten-pair append ran without removing samples; it passed 20/20.
The combined verdict is therefore 72/72 correct executions, 68 recorded:

| VM | Pairs | Favourable | Median CPS change | Q1 / Q3 | Mean 95% interval |
| --- | ---: | ---: | ---: | ---: | ---: |
| `rxvm` | 22 | 21/22 | **+10.376%** | +9.429% / +11.426% | +7.320% to +12.869% |
| `rxbvm` | 12 | 12/12 | **+10.612%** | +9.839% / +11.042% | +9.684% to +11.488% |

No sample was removed.  RXVM retained one adverse -7.711% pair and one
favourable +31.247% pair; the predeclared append resolved the noisy span and
the combined interval remains wholly favourable.  Both VMs are decisive and
clear of the regression guard.

A separate counts-only diagnostic used identical noncanonical fixed work of
200 x 100 iterations for C0 and C1; both runs passed without calibration.
C0 executed 55,900,921 VM instructions including 2,520,006 `ITOS`; C1 executed
54,501,316 including 1,120,006 `ITOS`.  C1 therefore removes 1,399,605 dynamic
instructions (2.504%) and 1,400,000 dynamic `ITOS` executions (55.555%).
Adrian accepted C1/T1 on 2026-08-01.  The counts, raw timing, exact artifact
hashes and closeout validation are retained in
[`2026-08-01-perf3-10-trace-safe-itos-closeout`](evidence/2026-08-01-perf3-10-trace-safe-itos-closeout/).

### Proportional closeout — 2026-08-01

- The affected Debug product rebuilt successfully.
- The closeout-focused selection passed 59/59.
- The first broad Debug run passed 1,981/1,982.  Its one mismatch was the
  pre-existing `trace_stem_sugar` intermediate-mode expectation: it expected
  the factory event but not the newly correct same-boundary assignment event.
- The ordered expected stream was updated, the exact test passed 1/1, and the
  clean broad rerun passed 1,982/1,982.
- The five protected untracked lifecycle RXBINs retain their starting hashes.
- No sanitizer, install/package, cross-platform or wider timing sweep was
  added; those were outside the shortest accepted closeout path.

### Tactical-rule and residual-guard review

T1 removes the old assumption that two visible TRACE results require two
executable addresses, but it does not make every existing TRACE guard
redundant.  The older whole-procedure load, `null`, one-register `itof`, copy
and producer-forward consumers still use `flow_has_trace_after()`.  Each needs
the same component/storage/value-generation proof as C1 before its guard can be
removed; ordered delivery alone proves event transport, not value equivalence.

The local duplicate link/read and swap/call-window rules also remain live.
The storage service follows those mappings and reports swap round trips, but it
does not yet replace the transformations.  The adjacent `cnop` rule removes
only a redundant second zero-operand `cnop`, doubles as the wide-map regression
and is not a TRACE-address anchor workaround.  No tactical rule or guard is
therefore deleted in this closeout.  PERF3-11 records the migration path to one
component-generation fact engine and requires an exact replacement proof per
consumer.

## Acceptance and rejection ledger

| Option | Correctness | Runtime | Disposition |
| --- | --- | --- | --- |
| C0/T0 | accepted oracle | retained baseline | immutable comparison |
| C1/T1 | 59/59 focused and 1,982/1,982 broad | decisive on both VMs | accepted production result |
| C2 | not implemented | not measured | retained compiler alternative |
| C3 | not implemented | not measured | retained runtime fallback |
| C4 | not implemented | not measured | retained last resort |
| T2 | design-compatible only | not measured | deferred beyond first verdict |

C1/T1 was accepted only after its correctness gate and mandatory paired
ordinary Release verdict both passed.  C2-C4 and T2 remain replayable future
options rather than dormant production switches.

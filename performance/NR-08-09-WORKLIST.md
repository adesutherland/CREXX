# NR-08/NR-09 resumable design and bounded-PoC worklist

Status: **NR-08 accepted and fully closed out; NR-09 expanded production scope
is recorded and queued next**

Started: 2026-07-17

This is the resumable control plane for the combined NR-08 then NR-09
programme. The original bounded PoCs remain recorded below. Adrian subsequently
selected NR-08 for production evaluation and clarified that NR-09 must proceed
beyond candidate mining: implement the mechanically obvious candidates, retain
the rest in the decision records, and assess synthesized instructions and
procedure-entry/default state when that removes repeated setup. The dated
programme report remains the historical charter and must not be edited.

## Verified starting state

- Repository: `/Users/adrian/CLionProjects/CREXX`
- Branch: `develop`
- Starting HEAD: `e748621d1f9242f68f30fe8aa3f58fb804be68fb`
- Upstream: `origin/develop`
- Relationship at start: `HEAD == origin/develop`; zero ahead and zero behind
- Starting worktree: clean
- Required predecessor state:
  - NR-04 and NR-04A complete
  - NR-05 complete, with historical 22-image schema-4/RXSEQ evidence
  - NR-06 complete and NR-07 rejected/removed
  - NUMERIC-01 complete; canonical and opaque RexxCPS are version 2.2d
- Session authority: reversible design/PoC edits and retained evidence only;
  no commit, push, formal production Release verdict, broad Debug CTest,
  sanitizer, package/install, cross-platform or formal portfolio timing work

## Mandatory orientation

- [x] `AGENTS.md`
- [x] `performance/AGENTS.md`
- [x] `performance/ROADMAP.md`
- [x] `performance/README.md`
- [x] Historical NR-08/NR-09 charter and related instruction/RXSEQ findings
- [x] `docs/ai-context/CREXX_ARCHITECTURE.md`
- [x] `compiler/docs/emitter_architecture.md`
- [x] `docs/ai-context/RXAS_ASSEMBLER.md`
- [x] `docs/ai-context/RXVM_INTERPRETER.md`
- [x] `docs/books/crexx_programming_guide/profiling.md`, especially RXSEQ
- [x] Opcode definitions, effects sidecar/API, VM handlers and metadata tests
- [x] Current emitter/allocation/optimization paths named in the handover
- [x] NR-04 effects and NR-04A graph boundaries relevant to safe dataflow
- [x] NR-05 manifest, driver and retained sequence-ranking contract
- [x] Current canonical/opaque RexxCPS sources and NUMERIC-01 decision evidence
- [x] Existing compiler, reference, alias, lifetime and dual-VM test inventory
- [x] `docs/ai-context/CREXX_LEVELB_AUTHORING.md` before any NR-09 tool edit

## Hard boundaries

- Run NR-08 before NR-09. NR-09 must not promote patterns removed by NR-08.
- Preserve language semantics, ISA, RXBIN 007, public ABI, source/TRACE behavior
  and both VM contracts.
- Do not weaken or reclassify `ENDLIFE_REG` effects to enable elimination.
- Treat `NULL` and `ENDLIFE` independently:
  - `NULL` clears the entire value and needs must-initialize-before-read proof;
  - `ENDLIFE` invalidates reference identity/lifetime without clearing ordinary
    contents and needs reference/alias/escape proof.
- Reference-bearing procedures, exposed storage, arguments, receiver/factory
  pseudo-locals, reference targets, dereference aliases, inline scaffolding and
  exceptional/signal paths fail closed.
- The deterministic NR-09 ledger is its input and review queue, not its exit
  criterion. NR-09 must implement candidates whose exact operands and opcode
  effects make the transformation mechanically obvious, and record why every
  other reviewed family is deferred or rejected.
- NR-09 must not assume that existing instructions are the only answer.
  Synthesized instructions, private-runtime superinstructions and procedure
  setup/default-state designs belong in the comparison when they retire a
  repeated semantic unit. Any selected canonical opcode/ISA/RXBIN/public-ABI or
  architectural change still requires Adrian's explicit design selection
  before production implementation.
- Historical NR-05 RexxCPS 2.2c rows remain revision-labelled historical
  evidence and must never be merged with 2.2d or an NR-08 candidate revision.

## Status-quo semantic record

### VM and opcode effects

| Operation | Status quo | PoC consequence |
| --- | --- | --- |
| `NULL_REG` | Calls `value_zero`; clears owned contents/reference payload. Effects prove no read and a kill of operand 1. | Removal requires a separate must-initialize-before-read proof on every relevant path. Reference-freedom alone is insufficient. |
| `ENDLIFE_REG` | Calls `release_value_reference_lifetime`; invalidates references to the storage and nested attributes, releases a reference payload, preserves ordinary contents. Effects read/write operand 1, prove no kill, and carry reference-release, lifetime-end, may-throw and opaque semantics. | Removal is legal only where the storage cannot have reference identity, cannot contain a reference payload, cannot be alias-observable, and cannot be observed by exceptional control flow. |
| `COPY_REG_REG` | Copies the complete value and status; can carry owned payload and reference state. | Copy removal is a separate value-flow proof and is not implied by an `ENDLIFE` result. |
| `UNLINK_REG` | Restores the frame's base-local pointer after a link/dereference alias. | Dereference-linked storage and scopes with unlink obligations fail closed. |

### Current compiler rules

- `add_initiator()` emits `null` once for a symbol with
  `needs_default_initiation` when its initialization metadata is first emitted.
- Scoped register recycling currently admits local variable symbols of
  unknown, Boolean, integer, float, decimal, string, binary, object and
  reference type.
- Recycling already excludes exposed symbols, arguments, reference arguments,
  receiver/factory pseudo-locals, symbols marked `has_reference_target`, and
  compiler-generated `__inline*`/`__rxtrace*` symbols.
- `emit_scope_reference_lifetimes()` nevertheless emits `endlife` for every
  ordinary local general-register variable in a recyclable local scope; it
  does not inspect declared type or the stricter recycling eligibility helper.
- Scope exits emit dereference unlinks first, then `endlife`, then metadata
  closeout. `BLOCK_EXPR` has the same order.

## NR-08 design and baseline ledger

### Current-source cells

| Cell | Source | Compiler mode | VM modes | Source hash | RXAS hash | RXBIN hash | Status |
| --- | --- | --- | --- | --- | --- | --- | --- |
| canonical-noopt | `tests/benchmarks/rexxcps_levelb.crexx` | no-opt | rxvm/rxbvm | `2970c3d73fe2` | `c09530e9bca8` | `afbe9e787892` | baseline pass |
| canonical-opt | same | optimized | rxvm/rxbvm | `2970c3d73fe2` | `aba5b72b9e7d` | `c885dcf9d6a3` | baseline pass |
| opaque-noopt | `tests/benchmarks/rexxcps_levelb_opaque.crexx` | no-opt | rxvm/rxbvm | `72a7c2d1d284` | `8f637935e7c8` | `694585c2b4f1` | baseline pass |
| opaque-opt | same | optimized | rxvm/rxbvm | `72a7c2d1d284` | `4ea0046261f6` | `4925de43f301` | baseline pass |

For every cell retain authored/generated/imported procedure separation, static
opcode counts, dynamic instruction counts, correctness output, source/image
hashes, exact commands and source/TRACE mode.

### Per-procedure/symbol inventory requirements

- [x] Separate authored procedures from generated TRACE helpers and imported
  library modules.
- [x] Record declared storage/value type for each authored local.
- [x] Record reference type, reference target, exposure, argument,
  receiver/factory, alias/link, escape and exceptional-path facts.
- [x] Record static `NULL`, `ENDLIFE`, unlink, reference-op and relevant-copy
  sites by procedure and symbol/register.
- [x] Record exact image-wide dynamic counts for every current cell and VM;
  per-procedure/static symbol sites are retained separately because schema 4
  has no instruction-site row and aggregate profile rows must not be
  mislabelled as authored-only or module-only counts.
- [x] Prove or disprove zero reference-typed and alias-observable authored
  RexxCPS values; list every exception and fail closed if any exists.

Baseline proof:

- Authored `main`, `cps_subroutine` and `fail` contain no reference declaration,
  `reference`, `dereference`, `snapshot`, `refvalid`, `ARG expose` or procedure
  `expose` surface in either source.
- All four exact RXAS forms contain zero `mkref`, `deref`, `linkref`, `setref`,
  `refvalid` or `unref` instructions and zero reference-typed metadata.
- Authored metadata uses `.int`, `.float`, `.decimal`, `.string`, arrays,
  `.binary` and `.stem`; the selected scalar gate below excludes the latter
  three conservative ownership shapes.
- No authored storage is marked as a reference target because the only source
  construct that sets `has_reference_target` is absent. Inlining/cloning
  preserves that flag, so this is also true after optimization.
- Normal `link`/`linkattr` plus `unlink` operations used by stem/array access are
  distinct from reference identity. Array/object/stem/binary locals remain
  outside the PoC gate so nested storage and ownership stay fail closed.
- Generated `__rxtrace*` locals and imported library modules remain a separate
  population. No aggregate dynamic count below is presented as authored-only.

Exact baseline dynamic `NULL_REG` / `ENDLIFE_REG` counts are:

| Cell | `rxvm` | `rxbvm` |
| --- | --- | --- |
| canonical-noopt | 1,359,142 / 1,301,497 | 1,359,142 / 1,301,497 |
| canonical-opt | 1,359,142 / 1,379,697 | 1,359,145 / 1,379,705 |
| opaque-noopt | 14,325 / 13,507 | 14,325 / 13,507 |
| opaque-opt | 14,328 / 14,326 | 14,328 / 14,326 |

The canonical cell is explicitly noncanonical (`--smoke-count 1` with
`averaging=100`); opaque uses `A Off 1 1`. Small cross-VM/cross-run differences
come from bounded control/TRACE-support paths and reinforce that candidate
deltas must be compared per exact VM/cell rather than assumed from one total.

### Candidate gates to compare before implementation

#### Gate A: symbol-local type/alias eligibility

Suppress only the operation whose target symbol independently proves all
required facts. For `ENDLIFE`, the initial candidate is an ordinary local
scalar/value symbol with a statically non-reference type, no reference target,
no exposure/argument/receiver/factory role, no dereference link, no escape and
no inline/TRACE generated role. `NULL` remains unchanged unless a distinct
must-initialize rule is proved.

Strengths: narrow blast radius; preserves lifetime operations for every
reference-capable or alias-observable symbol; supports mixed procedures.

Risks/costs: the compiler must expose one authoritative predicate to both
allocation and lifetime emission; object/array/general-value ownership needs a
deliberately conservative classification; incomplete symbol facts fail closed.

#### Gate B: procedure-level certified-reference-free eligibility

Certify a whole authored procedure only when no symbol, operation, metadata
surface or exceptional path can create, carry, expose, alias or observe a
reference lifetime; suppress eligible operations within the certified body.

Strengths: easy to audit for a bounded reference-free workload; one explicit
fail-closed procedure fact can guard all local decisions.

Risks/costs: unnecessarily rejects safe symbols in mixed procedures; inlining,
generated helpers, imports, objects/arrays and future compiler rewrites make
the certification wider and more fragile; one missed surface affects a whole
procedure.

#### Selected PoC gate

Select Gate A. Omit `ENDLIFE` only for an ordinary local variable symbol when
all of these facts hold at final emission:

1. it is in a recyclable local scope and uses an ordinary `r` register;
2. it is not exposed, an argument/reference argument, receiver/factory
   pseudo-local, reference target, inline-generated or TRACE-generated;
3. it has no array dimensions; and
4. its final declared type is exactly Boolean, integer, float, decimal or
   string.

Unknown, binary, object, reference and every array/stem shape keep `ENDLIFE`.
The selected types cannot contain a reference value under Level B's explicit,
non-assignment-compatible reference boundary. `has_reference_target` is set on
the target storage of every valid source `reference target` expression and is
preserved by cloning/merging; excluding it prevents escaped weak aliases from
being invalidated late. Excluding arrays/objects/binary and generated
scaffolding keeps nested storage, plugin/ownership payloads and transformation
internals conservative.

Reject Gate B for this PoC because one whole-procedure certificate is wider
than the needed fact, rejects safe scalars in mixed procedures, and would have
to absorb future inlining/import/generated-helper changes. It may remain a
diagnostic assertion later, but it is not the elimination authority.

### Separate transform decisions

| Transform | Initial design disposition | Evidence needed to change it |
| --- | --- | --- |
| `ENDLIFE` | PoC candidate, subject to Gate A proof | unchanged reference negative controls; exact RXAS site delta; dual-VM correctness; dynamic reduction |
| `NULL` | held unchanged | must-initialize-before-read proof covering every normal and exceptional predecessor, plus positive/negative fixtures |
| redundant copies | held unchanged | value identity, ownership, alias, liveness and exceptional-path proof independent of lifecycle elimination |

### NR-08 implementation/validation checklist

- [x] Record final selected gate and rejected alternative before compiler edit.
- [x] Retain exact baseline sources, RXAS/RXBIN, profiles, commands, hashes and
  raw results; retain the candidate patch after implementation.
- [x] Add focused positive structural coverage for eligible locals.
- [x] Add negative structural coverage for reference values/targets, exposed
  storage, arguments, objects/arrays where ownership is uncertain,
  dereference aliases, inline scaffolding and signal paths.
- [x] Build only the required compiler/benchmark/focused targets with deliberate
  high parallelism.
- [x] Inspect optimized and unoptimized RXAS.
- [x] Run focused correctness on `rxvm` and `rxbvm`.
- [x] Capture exact current/candidate dynamic instruction counts where they
  establish removal.
- [x] Retain source/RXAS/RXBIN/tool hashes, commands, patch and raw results.
- [x] Freeze the PoC; do not run the formal profiling-off Release verdict.

### NR-08 PoC result table

| Dimension | Result |
| --- | --- |
| Semantic/correctness status | PASS for the bounded PoC: final focused reference/structural matrix 26/26; focused stem regression matrix 3/3; exact canonical/opaque outputs pass on both VMs. This is not a broad correctness or formal Release verdict. |
| Exact gate | Gate A: omit scope-exit `ENDLIFE_REG` only for ordinary recyclable local `r`-register scalars of exact Boolean/int/float/decimal/string type; reject exposed/argument/ref-argument/receiver/factory/reference-target/dereference/array/generated inline or TRACE shapes. `NULL`, copies, ISA and effects are untouched. |
| Static `NULL` change | Program cells unchanged. The corrected label-aware imported-library audit is also unchanged at 4,293. The earlier apparent 3,908 to 3,897 delta was an analysis-script undercount, not generated-code DCE. |
| Static `ENDLIFE` change | Canonical no-opt 33 to 11; canonical opt 152 to 99; opaque no-opt 32 to 11; opaque opt 154 to 99. Authored `cps_subroutine` 4 to 0; `main` canonical 19 to 1 / 52 to 3 and opaque 18 to 1 / 54 to 3. Generated TRACE remains exactly 10 no-opt and 96 opt. Corrected label-aware imported-library count is 8,006 to 151. |
| Static copy change | Program cells unchanged. The corrected imported-library count is also unchanged at 8,478; unlink remains 1,758 and reference ops remain 7. The earlier apparent copy cascade was the same label-parsing undercount. |
| Dynamic count change | Canonical no-opt: rxvm 1,301,489 to 1,516 and rxbvm 1,301,497 to 1,516. Canonical opt: rxvm 1,379,697 to 5,616 and rxbvm 1,379,705 to 5,616. Opaque no-opt: both 13,507 to 31. Opaque opt: rxvm 14,326 to 72 and rxbvm 14,318 to 72. |
| Reference negative controls | PASS. Optimized/no-opt reference-source inline and block lifetime RXAS hashes are byte-identical baseline/candidate; object/reference/binary/array structural cases retain cleanup while eligible scalars omit it. |
| RXAS/RXBIN size change | RXBIN: canonical no-opt 50,632 to 50,536 bytes; canonical opt 79,933 to 79,853; opaque no-opt 54,291 to 54,227; opaque opt 84,614 to 84,438. Imported library 890,992 to 881,192 bytes. |
| Exploratory timing | None. Only exact correctness, static/dynamic instruction and image-size evidence was collected; no timing is presented as the mandatory profiling-off Release verdict. |
| Recommendation | **Selected by Adrian.** The mandatory ordinary profiling-off Release verdict below is positive. The imported-library audit concern is resolved: only `ENDLIFE` changes in the corrected exact per-procedure comparison. |

## NR-08 mandatory first Release verdict

Status: **accepted positive verdict; production candidate retained and broad
closeout complete**.

The candidate was frozen, the complete ordinary Release product was rebuilt
with `CREXX_VM_PROFILING=OFF`, and only the minimum focused Release correctness
gate was run before timing. The same canonical RexxCPS 2.2d source SHA-256
`2970c3d73fe2537ec8f81295c585495c4668b442d5b9a2335b1ee453a13bbdd6`
was used for the retained accepted baseline and candidate. The retained
NUMERIC-01 accepted-C baseline is valid: it is the exact final 2.2d source and
ordinary profiling-off Release product that became current commit
`e748621d1`; the intervening commit records those already-measured library and
benchmark changes and contains no later VM implementation change.

### Minimum correctness and audit gate

- full ordinary Release build: pass;
- focused Release CTest including fixture, scalar/reference structure and stem
  behavior: 4/4 pass;
- prior bounded final reference/structural matrix: 26/26 pass;
- prior stem matrix: 3/3 pass;
- optimized/no-opt reference source controls remain byte-identical;
- all eight candidate benchmark executions pass the canonical-default
  correctness and provenance contracts.

The mandatory imported-library audit found and corrected a defect in the PoC
analysis script: an instruction following a branch label was parsed from field
1 (the label) rather than field 2, so labelled operations were omitted from
the earlier totals. The retained RXBINs and dynamic profiles were unaffected.
After label-aware replay, all 629 procedures reconcile and the exact result is:

| Imported optimized library operation | Baseline | Candidate | Delta |
| --- | ---: | ---: | ---: |
| `NULL` | 4,293 | 4,293 | 0 |
| `ENDLIFE` | 8,006 | 151 | -7,855 |
| typed/general copies | 8,478 | 8,478 | 0 |
| `UNLINK` | 1,758 | 1,758 | 0 |
| reference operations | 7 | 7 | 0 |

Exactly 263 procedures have a change, every change is an `ENDLIFE` reduction,
no procedure gains `ENDLIFE`, and no `NULL`, copy, unlink or reference operation
changes. The suspected downstream DCE cascade therefore did not occur.

### Ordinary profiling-off Release comparison

Lifecycle: canonical optimized RexxCPS, retained source/TRACE metadata, default
`100 x 100` contract, serial processes, one candidate warmup plus three
recorded candidate samples per VM. The already-retained valid baseline has the
same lifecycle and three recorded samples per VM.

| VM | Baseline median CPS (range) | Candidate median CPS (range) | CPS delta | Baseline/candidate median elapsed | Elapsed delta |
| --- | ---: | ---: | ---: | ---: | ---: |
| `rxvm` | 1,145,721 (1,138,702-1,172,036) | 1,195,649 (1,180,932-1,203,585) | **+4.358%** | 8.74 s / 8.38 s | **-4.119%** |
| `rxbvm` | 1,114,685 (1,098,014-1,132,910) | 1,180,487 (1,179,414-1,184,294) | **+5.903%** | 8.99 s / 8.48 s | **-5.673%** |

Both VM modes improve, the candidate ranges do not overlap the retained
baseline ranges, native CPS and process elapsed agree in direction, and every
sample passes. This is a clear positive first Release verdict for Gate A. It
is not a portfolio-wide forecast.

Evidence:
`performance/evidence/2026-07-17-nr-08-first-release-verdict/`.

### Accepted closeout

Adrian accepted the first verdict and approved the shortest required closeout.
The complete Debug build passed. The first broad Debug CTest then found 71
compiler-output golden mismatches, while the corresponding runtime coverage
continued to pass. An exact pre-update diff audit proved that the only changes
were 170 removed scalar `ENDLIFE` instructions: no additions and no unrelated
line changes. The documented `crexx_test_driver --update-gold` path updated
exactly those 71 files. The failed selection then passed 71/71, and the final
required `ctest --test-dir cmake-build-debug --parallel 30
--output-on-failure` gate passed 1,851/1,851 in 217.35 seconds.

The implementation, tests, intentional goldens, records and retained evidence
are staged together. Per the approved closeout scope, no sanitizer,
install/package, cross-platform or additional portfolio/timing campaign was
added. No commit or push was performed.

## NR-09 design and bounded ledger PoC

NR-09 starts only after the NR-08 table is frozen.

### Input revisions

| Revision | Scope | Expected rows | Use |
| --- | --- | ---: | --- |
| NR-05 historical | 22 exact no-opt/opt portfolio images, including RexxCPS 2.2c | 14,009 | revision-labelled portfolio evidence only |
| current RexxCPS | exact 2.2d baseline no-opt/opt N=2/3/4 cells | 5,306 | current baseline comparison |
| NR-08 candidate | exact frozen PoC no-opt/opt N=2/3/4 cells | 5,005 | identify removed/demoted patterns as `subsumed` |

### Implementation shapes to compare

#### Shape A: separate deterministic Level B post-processor

Read one or more retained `sequence-ranking.csv` files plus an explicit
revision manifest and emit the durable ledger. Keep evidence capture unchanged.

Pros: independent, replayable over historical bundles, small input contract,
easy deterministic repeat and revision separation.

Cons: needs its own manifest/parser/verification path and duplicates a few CSV
helpers from the evidence driver.

#### Shape B: extend `run_evidence_bundle.crexx`

Build the ledger while the exact-image bundle is produced.

Pros: image metadata and raw candidate rows are already in memory; one command
can capture and summarize new evidence.

Cons: couples cross-revision portfolio decisions to evidence capture, makes
historical replay awkward, and broadens an already multi-purpose proof driver.

#### Selected implementation shape

Select Shape A: a separate deterministic Level B post-processor with an
explicit revision/input manifest. Reuse local style and helpers, but do not
make evidence capture depend on semantic-review policy. This keeps exact-image
capture and semantic review independently replayable. The implementation uses
short `StringTreeMap` indexes plus parallel typed row arrays; aggregation keys
are stable IDs whose full normalized identities are collision-checked. Index
variables are declared outside Level B branch scopes so new-row accumulation
cannot fall back to index zero.

### Required ledger contract

Each row must carry a stable identity, normalized pattern, evidence revision,
workload/entry/mode/window, module/site example, dynamic count, static sites,
module count, optimized/unoptimized occurrence, semantic-review status,
opcode-effect completeness, control-flow/liveness/alias/reference/exception/
interrupt risks, likely owner, proposed lowering/fusion, prerequisite,
measured disposition/evidence and status.

The tool must mechanically join opcode-effect facts where available and mark
incomplete/unknown semantics unsafe or unreviewed rather than guessing.

### Bounded review rule

- At least the top 25 global patterns by a deterministic generality-aware
  ordering plus the top 10 per workload after stable-identity deduplication.
- Keep dynamic count, workloads, sites, modules, modes and window length as
  independent visible facts; do not collapse the rank to raw RexxCPS count.
- Mark patterns removed/demoted by NR-08 as `subsumed`.

### NR-09 validation checklist

- [x] Audit all 14,009 historical rows and record exact revision metadata.
- [x] Capture minimal exact-hash current/NR-08 RexxCPS N=2/3/4 evidence.
- [x] Implement the selected Level B tool without Python.
- [x] Produce machine-readable CSV and concise Markdown decision view.
- [x] Run the same processing twice and prove byte-identical output/stable IDs.
- [x] Verify aggregate counts, deduplication, revision separation and examples
  against raw RXSEQ candidate CSV/output.
- [x] Record reviewed/unreviewed/unsafe/subsumed/candidate counts.
- [x] Identify cross-workload and workload-specific leaders separately.
- [x] Freeze the pipeline and record the recommended production follow-up.

### NR-09 PoC result table

| Dimension | Result |
| --- | --- |
| Input revisions and rows | `nr05-6a064499f327-rxseq-schema4`: 14,009 historical rows; `rexxcps-2.2d-baseline-e748621d1`: 5,306 current rows; `rexxcps-2.2d-nr08-63743eb90e97`: 5,005 candidate rows. Historical and current revisions remain separate ledger rows. |
| Unique ledger entries | 11,332 revision-pattern rows; 14,189 revision/workload aggregates; 76 stable identities selected by global/per-workload deduplication. |
| Reviewed/unreviewed/unsafe/subsumed/candidate | 9,928 / 1,404 / 9,558 / 322 / 48. `unsafe` is a mechanical fail-closed review state, not a permanent rejection. |
| Cross-workload leaders | Numeric-context setup families lead generality but are unsafe due state/exception/opaque effects. The strongest mechanically clean cross-workload row is `LOAD_REG_INT | LOAD_REG_INT`: 1,948,338 observations, 11 workloads, 22 entries, both modes, maximum 45 sites and 4 modules per entry. |
| Workload-specific leaders | The per-workload view keeps lifecycle, unlink/link, SETTP/SWAP, load/copy and branch families visible without merging sites across images. Example: `SWAP_REG_REG | SWAP_REG_REG` has 40,867,553 observations across 8 workloads but remains only a proof queue. Current RexxCPS `ENDLIFE` families are explicitly `subsumed` by NR-08. |
| Deterministic repeat | PASS. Recompiled no-opt Level B tool produced byte-identical CSV, Markdown, run-state and checksum files on `rxbvm` and `rxvm`; rerun overwrites rather than appends. Historical/current input line counts, 11,332 unique ledger IDs and a representative 20-row source aggregate reconcile exactly. |
| Strongest next candidates and proof obligations | Generality leader: `LOAD_REG_INT | LOAD_REG_INT`. Plausible direct-lowering leader: `LOAD_REG_INT | ICOPY_REG_REG` at 3,380,680 observations across 6 workloads/10 entries. SETTP/SWAP and repeated SWAP families are higher-count alternatives. Every one still needs operand/destination liveness, alias, exception and VM interrupt-boundary equivalence proof plus compiler-vs-RXAS ownership selection. |
| Production recommendation at PoC freeze | The ledger pipeline is accepted as the NR-09 input, not as completion. The first narrow no-new-opcode proof remains `LOAD_REG_INT | ICOPY_REG_REG`, but the production scope is superseded by Adrian's expanded direction below. |

### NR-09 production scope after Adrian's selection

Recorded before starting the NR-08 formal Release verdict so the NR-09 queue
cannot regress to “ledger only.” No NR-09 production edit or timing is mixed
into the NR-08 verdict.

#### A. Implement the obvious existing-contract candidates

1. **Procedure numeric setup via existing `NUMSCI`/`NUMENG`.** The compiler
   currently emits as many as five constant `SETNUM*` instructions at procedure
   entry. RXBIN 007 and both VMs already define `NUMSCI digits,case,standard`
   and `NUMENG digits,case,standard`, which also set fuzz to zero and form to
   scientific/engineering while synchronizing the decimal plugin once. When
   all five effective procedure values are compile-time constants, fuzz is
   zero and no setting is inherited, emit the matching existing combined
   instruction instead of five setters. Preserve validation/signal behavior
   with compiler-valid constants and add explicit inherited/nonzero-fuzz/
   decimal-plugin negative cases.
2. **Direct constant destination lowering.** For the stable
   `LOAD_REG_INT temporary,const | ICOPY_REG_REG destination,temporary` family,
   emit/load directly into the destination when the temporary is dead, neither
   register is alias-observable, and no source/debug/TRACE or interrupt contract
   requires the intermediate instruction boundary.
3. **Exact local redundancies.** Mine exact operands, not normalized family
   names, for identity typed copies, overwritten pure loads and an exact second
   swap of the same pair. Implement only occurrences proved redundant by
   read/write/kill data and liveness; the high-count normalized
   `SWAP | SWAP` family with distinct pairs is not cancellation.

Each implemented rule needs generated positive/negative structural tests,
dual-VM correctness, exact static/dynamic count and image-size deltas, and its
own ordinary profiling-off Release verdict. If an “obvious” rule has no exact
current occurrences or no measurable product benefit, record that result and
move on rather than manufacturing a benchmark-specific case.

#### B. Compare synthesized/procedure-entry designs

1. **General numeric-context instruction.** Compare the existing
   `NUMSCI`/`NUMENG` fast path with one synthesized full-context operation that
   carries digits, fuzz, form, case and standard and performs one validation/
   plugin synchronization. This covers valid nonzero-fuzz combinations that
   the two existing instructions cannot encode.
2. **Procedure-owned default context.** Compare executable prologue setters
   with storing a procedure's non-inherited numeric context in procedure/runtime
   metadata and installing it during frame activation. The current VM copies
   the caller context into a child frame and the emitted prologue then
   overwrites declared values; a procedure-owned default could retire the
   repeated dispatches entirely. The proof must preserve explicit `INHERITED`,
   recycled frames, decimal-plugin ownership/synchronization, signals, inline
   context compatibility, late loading and both VM modes.
3. **Private-runtime versus canonical synthesis.** Compare a canonical new
   opcode only if it materially beats existing `NUMSCI`/`NUMENG`; also compare
   a link/load-time or private-runtime superinstruction that leaves canonical
   RXBIN unchanged. Record dispatch reduction, code size, preparation/startup,
   dequickening/source coordinates and cross-workload benefit.
4. **Other semantic units.** Use the ledger to propose instructions that
   perform a complete repeated operation rather than merely concatenating
   adjacent encodings. Keep compare/branch, call/window, stem and parse families
   linked to their existing accepted/rejected work so NR-09 does not repeat a
   disproved design under a new name.

#### Revised NR-09 exit criterion and ordering

- The NR-08 acceptance/closeout prerequisite is now satisfied. On resumption,
  establish a post-NR-08 exact baseline first.
- Implement and verdict the obvious rules above one at a time.
- Produce a measured comparison for numeric-context procedure setup covering
  the existing combined instruction, a generalized synthesized instruction and
  procedure-owned default context.
- Leave every other stable candidate in the ledger with explicit owner,
  semantic obligation and disposition; “not yet selected” is not deletion.
- NR-09 is complete only when the obvious queue is exhausted and the remaining
  synthesized-instruction register is sufficiently evidenced and prioritized
  for Adrian's design selections.

## Evidence locations

- NR-08 bundle:
  `performance/evidence/2026-07-17-nr-08-lifetime-poc/`
- NR-09 bundle:
  `performance/evidence/2026-07-17-nr-09-sequence-ledger-poc/`
- Historical NR-05 input:
  `performance/evidence/2026-07-16-nr-05-call-census/`

## Stop-point checklist

- [x] NR-08 PoC implementation and result are frozen.
- [x] NR-09 pipeline and bounded result are frozen; the expanded production
  scope and resume order are recorded before the NR-08 Release verdict.
- [x] Roadmap reflects PoC states without claiming charter completion.
- [x] Every changed file, disposable PoC component and retained evidence path
  is listed.
- [x] Correctness, generated-code/dynamic-count, exploratory performance and
  production recommendations are reported separately.
- [x] No formal Release verdict, broad closeout, commit or push occurred during
  the bounded PoC session.
- [x] Adrian selected NR-08 for the mandatory first Release verdict; NR-09
  remains paused until the accepted NR-08 closeout is complete.
- [x] NR-08's first ordinary profiling-off Release verdict is positive and the
  corrected imported-library audit is clean.
- [x] Adrian accepted NR-08; complete Debug build and final 1,851/1,851 CTest
  pass, and the intentional 71-golden/170-`ENDLIFE` audit is retained.
- [x] NR-08 is closed out and staged without commit or push; NR-09's obvious
  candidate and synthesized/procedure-default scope is queued next.

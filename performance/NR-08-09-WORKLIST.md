# NR-08/NR-09 resumable design and bounded-PoC worklist

Status: **NR-08 and NR-09 Rule 1 accepted and fully closed out; NR-09's
corrected 26-form pruning and four mapping selections are implemented and have
passed broad Debug, sanitizer, install, old-RXBIN compatibility and final
ordinary-Release performance QA; the production-batch commit remains pending**

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
- Treat the remaining low-risk mappings as one predeclared instruction batch,
  not as an instruction-by-instruction sequence of production verdicts.
- Produce a measured comparison for numeric-context procedure setup covering
  the existing combined instruction, a generalized synthesized instruction and
  procedure-owned default context.
- Leave every other stable candidate in the ledger with explicit owner,
  semantic obligation and disposition; “not yet selected” is not deletion.
- NR-09 is complete only when the obvious queue is exhausted and the remaining
  synthesized-instruction register is sufficiently evidenced and prioritized
  for Adrian's design selections.

### NR-09 production resumption: Rule 1 numeric setup

Started: 2026-07-17

#### Exact post-NR-08 baseline

- Branch `develop` was clean at
  `7b93bef73267ee1542295616db5a0148e7766a43`, exactly equal to
  `origin/develop`, before the NR-09 resumption record was added.
- The ordinary profiling-off Release artifacts already in
  `cmake-build-release` were audited rather than silently treated as current.
  Their `rxc` output, RXBIN, linked library, `rxvm` and `rxbvm` SHA-256 values
  exactly match the retained accepted NR-08 candidate values. This proves the
  starting product is the accepted post-NR-08 implementation, not historical
  NR-05 or the pre-NR-08 ledger baseline.
- The exact canonical optimized RexxCPS source remains 2.2d at SHA-256
  `2970c3d73fe2537ec8f81295c585495c4668b442d5b9a2335b1ee453a13bbdd6`.
  Its post-NR-08 optimized RXAS/RXBIN sizes are 226,793/79,853 bytes. The
  linked library is 881,192 bytes.
- The canonical optimized RXAS contains five `SETNUMDGTS`, five `SETNUMFUZ`,
  five `SETNUMFRM`, five `SETNUMCAS` and five `SETNUMSTD` sites: 25 static
  procedure-entry setters and no `NUMSCI`/`NUMENG`.
- The retained exact post-NR-08 schema-4 profiles execute 542,500 numeric
  setters on `rxvm` and 542,500 on `rxbvm`. The accepted ordinary Release
  timing baseline remains the NR-08 candidate median: 1,195,649 CPS / 8.38 s
  on `rxvm` and 1,180,487 CPS / 8.48 s on `rxbvm`, three recorded serial
  samples per VM after one warmup. It remains valid and must not be merged
  with historical NR-05/pre-NR-08 evidence.
- Baseline record:
  `evidence/2026-07-17-nr-09-numctx-first-release-verdict/`.

#### Status quo and plausible implementation designs

**Status quo.** `rxc` emits up to five immediate procedure-entry setters in
digits, fuzz, form, case, standard order. Each VM setter validates its one
field, updates the frame context and synchronizes the loaded decimal plugin.
Inherited fields omit only their corresponding setter. The existing RXBIN 007
`NUMSCI digits,case,standard` and `NUMENG digits,case,standard` handlers validate
digits/case/standard, install fuzz zero plus the selected form, and synchronize
the plugin once, but `NUMSCI`/`NUMENG` require digits >= 5 while the individual
digits setter and Level B validator accept digits >= 1.

**Design A - compiler-owned existing-instruction lowering.** At procedure
emission, use `NUMSCI` or `NUMENG` only when digits, fuzz, form, case and
standard are all non-inherited compile-time values; fuzz is zero; digits is at
least 5; and form/case/standard are valid existing enum values. Otherwise emit
the current per-field setters unchanged.

- Advantages: one dispatch and one plugin synchronization; typed procedure
  context is already available; no new ISA, RXBIN, ABI or runtime contract;
  applies equally to optimized and no-opt compiler output.
- Risks/controls: digits 1-4 must retain setters because the combined opcode
  has a stricter historical validation contract; any inherited or nonzero-fuzz
  case fails closed; generated structural tests keep exact operands visible;
  runtime tests cover scientific/engineering, inherited, nonzero-fuzz and both
  decimal plugins on both VMs.

**Design B - RXAS five-instruction peephole.** Recognize the exact immediate
setter sequence and replace it during assembly.

- Advantages: could also optimize hand-written RXAS and keeps emitter code
  unchanged.
- Rejected for Rule 1: RXAS has less direct evidence that the sequence is the
  compiler-owned effective procedure context, must reconstruct source/TRACE
  and exceptional-boundary obligations, and would transform arbitrary authored
  sequences. It is wider than the mechanically proved compiler lowering.

**Design C - generalized full-context opcode.** Add one instruction carrying
digits, fuzz, form, case and standard so nonzero fuzz can also use one semantic
operation.

- Advantage: covers every fully constant context and retains a single plugin
  synchronization.
- Deferred at the Rule 1 decision point because it changes the canonical
  ISA/RXBIN contract. Adrian subsequently selected new large instructions as
  the intended NR-09 mechanism on 2026-07-18; this full-context form is now in
  the 67-mapping batch below.

**Design D - procedure-owned frame-entry default.** Store a non-inherited
context with procedure/runtime metadata and install it during frame activation.

- Advantage: removes executable prologue dispatch entirely.
- Deferred: it changes procedure/frame architecture and must first compare
  recycled frames, inherited fields, inline compatibility, plugin ownership,
  late loading, source coordinates and both VM modes. Production adoption
  requires Adrian's explicit selection.

**Design E - private runtime synthesis.** Quickening or link/load preparation
could replace canonical setter sequences only in the private execution image.

- Advantage: leaves serialized RXBIN unchanged and could cover non-compiler
  producers.
- Deferred: preparation/startup, dequickening, source-coordinate and late-load
  costs are unjustified until the compiler-owned existing-opcode ceiling is
  measured.

#### Rule 1 selection and proof boundary

Select Design A for the first independently verdictable production candidate.
This is the user-directed no-new-ISA fast path and does not select Designs C-E.
The stricter `digits >= 5` gate is mandatory for existing-handler equivalence.
Compiler validation already proves fuzz zero, form and enum values valid, so
the removed fuzz/form setter validations cannot signal; the combined handler
preserves the remaining digits/case/standard validation order and performs the
same final context update before execution of the authored body. Reducing five
procedure-entry dispatch/interrupt boundaries to the existing combined
instruction's one boundary is the intended existing ISA contract and will be
tested in both VM modes.

Rule 1 remains provisional and revertable. After generated structure,
scientific/engineering context, inherited, nonzero-fuzz, digits-1-to-4 and
decimal-plugin controls pass, capture exact static/dynamic instruction and
image-size deltas, freeze production edits, and run the mandatory ordinary
profiling-off Release verdict. Stop for Adrian immediately after reporting it.

#### Rule 1 focused proof and mandatory first Release verdict

Status: **accepted and fully closed out**.

The first-verdict candidate was frozen. The new generated contract covers
optimized and no-opt structure; exact scientific/engineering operands;
inherited digits; nonzero fuzz; digits 1-4; both VM modes; and default,
`mc_decimal` and `db_decimal` plugin execution. The focused Debug selection
passes 12/12, including existing RXAS combined-opcode, decimal and
numeric-library tests. The focused ordinary Release contract passes 1/1. No
broad CTest, sanitizer, package/install, cross-platform, golden refresh, next
candidate or synthesized design implementation had run at the mandatory stop
point.

Exact post-NR-08 to Rule 1 deltas for canonical optimized RexxCPS:

| Dimension | Baseline | Candidate | Delta |
| --- | ---: | ---: | ---: |
| static numeric setup instructions | 25 setters | 5 `NUMSCI` | -20 (-80.000%) |
| dynamic numeric setup, `rxvm` | 542,500 | 108,508 | -433,992 (-80.000% rounded) |
| dynamic numeric setup, `rxbvm` | 542,500 | 108,508 | -433,992 (-80.000% rounded) |
| optimized RXAS bytes | 226,793 | 226,493 | -300 |
| optimized RXBIN bytes | 79,853 | 79,861 | +8 |
| linked library bytes | 881,192 | 880,384 | -808 |

The candidate dynamic total is 108,498 `NUMSCI`, four residual `SETNUMFUZ`,
two residual `SETNUMCAS` and four residual `SETNUMSTD` executions in each VM;
the exact candidate profile image and library hashes equal the ordinary
Release candidate hashes. The retained library disassembly changes 3,123
numeric setup instructions (622 digits, 627 fuzz, 623 form, 624 case and 627
standard) to 651 (618 `NUMSCI` plus 33 residual setters), a reduction of 2,472.
The standalone optimized RXBIN grows by eight bytes despite the instruction
reduction; the full linked library shrinks, so no broader code-size claim is
made.

Ordinary profiling-off Release timing reuses the exact accepted NR-08
candidate as baseline and runs one candidate warmup plus three serial recorded
candidate samples per VM. Higher CPS and lower elapsed time are better:

| VM | Baseline median CPS (range) | Candidate median CPS (range) | CPS delta | Baseline/candidate median elapsed | Elapsed delta |
| --- | ---: | ---: | ---: | ---: | ---: |
| `rxvm` | 1,195,649 (1,180,932-1,203,585) | 1,203,145 (1,194,385-1,213,159) | +0.627% | 8.38 s / 8.33 s | -0.597% |
| `rxbvm` | 1,180,487 (1,179,414-1,184,294) | 1,183,390 (1,176,634-1,189,443) | +0.246% | 8.48 s / 8.46 s | -0.236% |

All samples pass, native CPS and process elapsed agree in direction, but both
candidate ranges overlap their retained baselines and the median changes are
small. This is neutral-to-slightly-positive evidence, not a material product
win. Adrian accepted that improvement on 2026-07-18. Evidence:
`evidence/2026-07-17-nr-09-numctx-first-release-verdict/`.

#### Rule 1 accepted closeout

The complete Debug build passed. Initial broad CTest reported 226 failures:
222 compiler RXAS goldens (111 no-opt and 111 optimized) and four RXPA
signal-address expectations. An exact pre-update replay proved that every
generated/golden difference was one or more five-setter-to-`NUMSCI`
substitutions: 517 substitutions across 222 files and no unrelated changes.
The documented golden driver updated those files. Post-update diff audit found
517 added `NUMSCI` lines and 2,585 removed `SETNUM*` lines, with no other added
or removed lines.

The four RXPA cases retained the same `SIGNAL ERROR` and source location; the
shorter prologue moved the expected bytecode address from 15 (`0xf`) to 9
(`0x9`). The rerun-failed selection passed 227/227 including its linked
fixture. Final `ctest --test-dir cmake-build-debug --parallel 30
--output-on-failure` passed 1,852/1,852 in 205.57 seconds. No sanitizer,
install/package, cross-platform or additional timing campaign was added. No
commit or push was performed.

### NR-09 low-risk mapping batch selected by Adrian

Recorded 2026-07-18. “Low risk” is a semantic-proof classification, not a
no-new-ISA restriction:

1. **Class 1 / RXAS-backed:** RXAS can prove the exact bounded dataflow, such
   as a destination being overwritten before any read, or can replace an
   effect-clean adjacent sequence while preserving its final register state.
   RXAS is the required backstop. rxc may also emit the large instruction
   directly when the complete mapping belongs naturally to one AST-node
   emission; it should not add cross-node coordination merely because two AST
   nodes happen to emit adjacent instructions.
2. **Class 2 / rxc-owned:** rxc knows that an intermediate register or alias is
   compiler-temporary and that otherwise visible intermediate state, cleanup
   or side effects are irrelevant to the authored program. The emitted large
   instruction must retain any relevant validation, failure ordering and final
   state. Direct rxc lowering should prefer the compact semantic form: omit
   irrelevant intermediate side effects and do not allocate or assign dead
   result registers merely to mirror the expanded RXAS sequence. A separate
   wider RXAS form may retain intermediate writes when the keyhole proof shows
   that they are overwritten before any read.

Adding new, larger instructions is the intended NR-09 mechanism, not a design
disadvantage. Exact opcode operands/effects and owner still have to be recorded
before production editing, but the batch is not constrained to existing ISA
forms. The 76 selected stable patterns in the retained ledger classify as:

| Disposition | Patterns | Meaning |
| --- | ---: | --- |
| Class 1 | 20 | RXAS/keyhole proof is plausible; four are the already-measured and rejected NR-07 compare/branch family. |
| Class 2 | 51 | Requires rxc temporary/alias/side-effect knowledge. |
| Not low risk | 5 | Three `ENDLIFE` families are NR-08-subsumed/reference-observable; two branch-into-alias/capacity families retain observable control/throw boundaries. |

The active queue is therefore **67 low-risk mappings**: 16 Class 1 plus all 51
Class 2 after removing the four already-rejected NR-07 mappings. N=2/3/4 rows
overlap and their counts must not be summed. For implementation the 67 rows
collapse into 12 coherent families. The exact stable-ID register, including
all active, rejected and deferred mappings, is
[`NR-09-MAPPING-REGISTER.md`](NR-09-MAPPING-REGISTER.md).

| Active mapping family | Class | Selected patterns | Leading retained observation | Intended batch direction |
| --- | --- | ---: | ---: | --- |
| Numeric-context prologue | 2 | 9 | 16,434,838 | General full-context operation; Rule 1 is the accepted fuzz-zero subset. |
| Multi-swap | 1 | 3 | 40,867,553 | Two/three/four-pair or descriptor-backed swap unit. |
| Call-window preparation | 1 | 7 | 26,311,205 | Combine `LOAD`/`SETTP`/`SWAP` setup sequences. |
| Call-window through call | 2 | 3 | 14,787,936 | rxc-owned prepared/mapped call instruction. |
| Multi-null | 1 | 3 | 12,521,800 | Clear two/three/four proved destinations per dispatch. |
| Constant load and direct destination | 1 | 3 | 3,380,680 | Multi-load and load-direct-to-final-destination mappings. |
| Unlink/cleanup chains | 2 | 7 | 10,866,847 | rxc-owned cleanup unit for compiler-created aliases/temporaries. |
| Attribute/index/link scaffolding | 2 | 23 | 7,113,611 | Replace compiler-generated capacity/index/link/read/write scaffolding by complete semantic operations. |
| Typed conversion/copy | 2 | 3 | 1,002,000 | Convert directly into the required final result(s). |
| Arithmetic chains | 2 | 3 | 501,000 | Preserve per-step arithmetic semantics inside one handler. |
| String conversion/concatenation | 2 | 2 | 14,000 | Consume compiler-temporary text conversions directly. |
| Load/get-attributes | 2 | 1 | 14,012 | Combine the compiler-owned temporary load/access sequence. |

The four NR-07 compare/branch patterns remain visible but rejected; their
prior Release evidence must not be rerun under a new name. The three
`ENDLIFE` rows and the two branch/alias-capacity rows remain explicit deferred
controls rather than silently disappearing.

#### Batch execution rule

- **Stage 1 - instructions and direct tests.** Define the exact canonical
  instruction names, operand forms, effects metadata and VM handlers for all
  selected families. Add direct assembler/VM instruction tests before any
  automatic producer uses them. Deduplicate overlapping windows and choose
  non-explosive operand forms.
- **Stage 2 - RXAS.** Implement every Class 1 mapping in RXAS, with positive
  and negative peephole tests. RXAS remains the backstop even where rxc can
  emit the new instruction directly.
- **Stage 3 - rxc.** Add direct Class 1 rxc emission only where
  one AST node already owns the complete semantic unit and doing so is simpler
  than emitting the expanded sequence. When a mapping spans two merely
  adjacent AST nodes, keep rxc simple and rely on the RXAS backstop. Then add
  the rxc-owned Class 2 lowerings, proving compiler-temporary/alias facts and
  retaining structural compiler coverage.
- Implement the selected Class 1 and Class 2 mappings as one coherent
  production batch. Do not stop for a Release verdict after each instruction.
- Keep per-mapping static/dynamic deltas and positive/negative structural
  controls so a neutral or bad family can be identified within the batch.
- **Stage 4 - verdict gate.** Once the batch's minimum focused correctness
  passes, freeze it and run one mandatory ordinary profiling-off Release
  verdict against this accepted Rule 1 product baseline. Stop for Adrian at
  that batch verdict.
- **Stage 5 - accepted closeout and full QA.** After Adrian accepts the batch
  verdict, run the proportional full QA, audit any required golden refresh,
  retain final evidence and commit when requested.
- Procedure-owned numeric defaults remain a separate architectural comparison;
  they are not silently folded into this mapping batch.

#### Batch implementation record

- [x] Isolate arbitrary opcode operand transport in prerequisite commit
  `32bf7e76f`, including assembler, disassembler, linker/relocation, VM decode,
  rxc `ASSEMBLE`, instruction metadata/database and focused pipeline tests.
- [x] Record its mapping benefit: 38/67 selected mappings require more than
  three operands (11/16 Class 1 and 27/51 Class 2). At least the 11 wide Class
  1 backstops and up to all 38 exact fusions would otherwise be blocked or
  require descriptor/partial alternatives; three additional TRACE-preserving
  production forms also use the wider transport.
- [x] Add 60 instruction forms with effects metadata, both VM handlers and
  direct assembler/VM coverage. The total includes three wider
  TRACE-preserving forms in addition to the 57 forms representing the selected
  mappings.
- [x] Add every Class 1 RXAS backstop with optimized, `-n` and boundary
  controls.
- [x] Add simple direct rxc producers plus the compiler-owned final exact
  template combiner for Class 2. Source-step/label boundaries fail closed and
  TRACE references are retained or retargeted only where proved safe.
- [x] Update the instruction database, RXAS reference/inventory, assembler AI
  context, VM specification example and compiler-emitter architecture.
- [x] Pass the final minimum focused correctness selection and freeze code:
  effects metadata and combiner unit checks, 9/9 selected CTests, standalone
  docs example, exact 600-form/388-mnemonic inventory and exact 60-row new
  instruction source/database agreement.
- [x] Run the ordinary profiling-off Release comparison and exact static/dynamic
  attribution against the accepted Rule 1 product, then stop for Adrian. The
  original unmatched-session result was -2.233%/-2.289% median CPS despite
  -8.595% dynamic dispatch; its interpretation is superseded by the
  drift-controlled rerun below.
- [x] Run the new VMs against exact retained old optimized/no-opt RXBINs and
  matching old library. All smoke cells pass. Its original cross-session
  timing was neutral at -0.060%/-0.069% CPS; the same-session A/B control below
  supersedes its performance interpretation while confirming it.
- [x] Reconstruct accepted commit `847e62f04` and run the approved
  same-session A/B/C rebaseline: accepted product, current VM on accepted
  RXBIN/library, and complete fused product. All six warmups and 72 recorded
  samples pass; every product order occurs twice per VM. Infrastructure paired
  median CPS is neutral at -0.152%/+0.022%. Fusion is slightly positive at
  +0.617%/+0.805%, and the complete batch is +0.586%/+0.671%; elapsed direction
  agrees, but uncertainty intervals cross zero. The retained candidate
  reproduces within +0.165%/-0.120%, while the retained accepted baseline was
  2.577%/2.364% faster than its rerun. The former slowdown was unmatched-session
  baseline drift. Corrected verdict: neutral-to-slightly-positive; stop for
  Adrian before broad QA or production commit.
- [x] Add the rerunnable all-form diagnostic report. Its versioned manifest
  covers all 60 forms and records exact component expansions, coherence,
  temporary-register policy, review bar and pending implementation/decision
  fields. The canonical profile observes 28 forms; the macro expansion model
  explains the exact dispatch reduction within +16/-14 instructions and
  identifies `SETTPCALL` as the first material possible-slowdown signal.
- [x] Review every form on ordinary-Release saving and relevance, semantic
  coherence, temporary-register/side-effect cost and handler implementation.
  The full 22-image census records 76,122,675 executions across 33/60 forms;
  every exercised form has dual-VM profiling-off Release isolation. The
  first balanced scorecard proposed 30 outright removals, two clean
  replacements and 28 retained forms. Its all-enabled zero-use inference was
  then corrected by a controlled post-RXAS replay: three masked forms are
  measurably faster than the currently selected overlapping path, and one
  two-register `ITOF` use is required by the winning arithmetic schedule. The
  corrected proposal is 26 outright removals, two clean replacements and 32
  retained forms. All six caller/trace-temporary forms are still removed or
  replaced. Production remains frozen pending Adrian's approval of the revised
  exact disposition in `balanced-review/README.md` and
  `balanced-review/overlap-review/README.md`.
- [x] After Adrian's approval: apply the 26 selected removals and four corrected
  mapping selections across opcode inventory/database, VM, RXAS, rxc, tests,
  docs and examples. Numeric IDs remain reserved. The two caller-temporary
  replacements are recorded as designs, not silently folded into this edit.
  Target-scoped Debug runtime regeneration passes, the focused selection is
  9/9, the standalone example prints `42` on both VMs, and both new Debug VMs
  run the accepted old RexxCPS RXBIN/library successfully.
- [x] Freeze the corrected implementation and rerun the mandatory first
  ordinary profiling-off Release verdict before broad closeout. The
  equal-path, same-session A/B/C campaign passes 78/78 executions. Corrected
  product C/B paired median CPS is +0.376%/+0.836% (`rxvm`/`rxbvm`); complete
  C/A is +0.262%/+0.937%. Both complete-product intervals cross zero, while
  the `rxbvm` C/B interval is wholly positive. Verdict: no regression,
  neutral-to-slightly-positive. Evidence: `evidence/2026-07-18-nr-09-large-instruction-batch-first-release-verdict/prunedrun1/`.
- [x] After acceptance: complete broad Debug QA, sanitizer, package/install
  proof, audited golden refresh and final evidence. Full Debug CTest passes
  1,864/1,864. The supported Apple ASan run reports no sanitizer diagnostic;
  its sole test failure was a shared-scratch-file race, and the locked pair
  then passes 2/2. Apple ASan does not support leak detection, so LSan is an
  explicit platform limitation. The isolated 112-file install executes its
  shipped example and both installed VMs run the exact accepted old RXBIN and
  library. The final 78/78 ordinary-Release refresh is positive at
  +1.385%/+2.868% complete-product paired median CPS (`rxvm`/`rxbvm`), with
  only the `rxbvm` complete-product interval wholly positive. Evidence:
  `evidence/2026-07-18-nr-09-large-instruction-batch-first-release-verdict/qa-closeout/`
  and `finalrun01/`.
- [x] Audit the final documentation against the live product. All 574 accepted
  forms map exactly to 367 mnemonic headings and Forms tables; all 373 tagged
  RXAS examples assemble, with 328 executed and 45 intentionally
  assemble-only. The emitter guide records the typed-instruction safety
  boundary, the VM guide records fused-call cold unwind, and this register now
  distinguishes the original 60-form candidate from the final 34-form public
  addition.
- [ ] Commit the production batch when requested; do not push without explicit
  authority.

The selected clean replacement designs and their proof boundaries are recorded
in `NR-09-MAPPING-REGISTER.md`. In short, result-only `FDIVSUB` must hold the
quotient in a VM-local C value and be selected only when rxc proves the old
quotient temporary dead; wide `ILOADSETUNLINKN` should collapse to the existing
compact form only when rxc proves the generated TRACE observation can be
retargeted without changing value or source-step semantics. Both remain
design-only until separately implemented and measured.

#### SETTPCALL regression review and design selection

Status: **review complete; production handler unchanged; diagnostic report
corrected**.

Question: why does `SETTPCALL_REG_FUNC_REG_REG_INT` show a material
instrumented slowdown at 56,968 canonical calls when it replaces
`SETTP_REG_INT | CALL_REG_FUNC_REG`, and what is the smallest implementation
change that restores an ordinary-Release win without weakening call-window,
flag, signal, profiling, RXBIN or VM-mode contracts?

Machine-level ceiling: the fused path should perform one masked status write
plus the existing mapped-call body and one dispatch/transition. Its five
operands must be direct runtime-image loads; arbitrary-width decode, prepared
image layout or generic operand access must not add work comparable with the
dispatch being removed.

Review hypotheses:

1. The five-operand RXBIN/runtime-image path or `REG_OP(4)`/`INT_OP(5)` address
   generation costs materially more than the original two- plus three-operand
   stream in one or both VM modes.
2. Expanding `RXVM_MAPPED_CALL_BODY` separately into each fused call handler
   duplicates a large hot body, changes compiler code generation or creates an
   instruction-cache/layout regression relative to the original call handler.
3. Operand order, next-PC selection, call-window base/count capture, native
   argument-vector construction, frame activation or signal/unwind metadata
   differs subtly from the established `CALL_REG_FUNC_REG` implementation.
4. The profile signal is dominated by call-population mismatch or profiler
   bookkeeping and does not reproduce in an exact ordinary-Release
   expanded-versus-fused cell.

Candidate implementations, to be selected only after the decode/codegen and
focused Release evidence:

- **A — specialised fused handler:** retain the five-operand instruction but
  cache its decoded operands explicitly and hand-specialise the body to match
  the established call implementation. This is smallest if generic macro
  expansion alone produces inferior code.
- **B — shared mapped-call tail:** make ordinary and fused call handlers cache
  the same call state and branch to one in-function mapped-call body. This
  removes duplicated hot code and keeps fused setup/next-PC semantics, but
  adds one direct internal branch and needs careful native/signal/profile
  validation.
- **C — withdraw this fusion:** have rxc emit `SETTP | CALL` again while leaving
  the opcode available for compatibility. Select this if five-operand decode
  or the fused code shape cannot beat the established pair reliably; saved
  implementation effort is not evidence for retaining a regression.

Decisive sequence: inspect RXAS/RXBIN and loaded/prepared cells; compare
generated Release assembly and code size for both VM modes; run an exact
expanded/fused call-loop cell; select one candidate; run minimum direct-call,
native-call, signal/unwind and dual-VM correctness; then freeze and run the
smallest ordinary profiling-off Release product verdict against retained valid
baseline evidence. Stop for Adrian at that verdict.

Selection evidence before the first handler edit:

- RXBIN and both runtime modes consume fixed eight-byte cells. The fused image
  is one cell smaller (`0x21` versus `0x22` code cells); neither VM runs a
  generic five-operand decode loop. Hypothesis 1 is rejected.
- A matched 20,000,000-call fixture reproduces no material 15%/6.7% loss. The
  original report compared the macro's call population with a global average
  over different `CALL` sites. Exact instrumented totals are 73 ns fused versus
  74+11 ns expanded in `rxvm`, and 71 ns fused versus 71+12 ns expanded in
  `rxbvm`.
- An initial fixed-order profiling-off Release pair appeared to show
  fused-minus-expanded at +0.281 ns/call in `rxvm` and -0.273 ns/call in
  `rxbvm`. Generated ARM64 showed different macro/legacy operand live ranges,
  which motivated candidate A, but the direct trial and later balanced
  alignment controls reject that difference as the cause.
- **A was tested and rejected.** Copying the established call source shape into
  only `SETTPCALL` worsened the matched cell to +0.394 ns/call in `rxvm` and
  +1.468 ns/call in `rxbvm`; the trial was reverted. The current cached macro
  is the faster implementation.
- Runtime-image position is the remaining sensitivity. With equal call-header
  cache offsets the fused result is +0.278/-0.593 ns/call (`rxvm`/`rxbvm`);
  with equal post-call return-target offsets it is -0.562/-0.320 ns/call.
  Removing one eight-byte cell necessarily changes one of those offsets.
- The decisive 16-offset sweep averages 64 balanced 5,000,000-call samples per
  VM: fused saves 0.129 ns/call (-0.458%) in `rxvm` and 0.418 ns/call (-1.507%)
  in `rxbvm`. **Final decision: retain the opcode and current handler.** A
  padding operand would discard the compact image and merely select one
  alignment; B and C are not justified.
- The standard profile report now labels all call-bearing forms
  `exact-cell-required` instead of classifying a macro from a population-mixed
  global `CALL` average. The manifest records `SETTPCALL` as coherent,
  implementation-reviewed and retained. Focused evidence is under
  `settpcall-review/` in the batch verdict bundle.

## Evidence locations

- NR-08 bundle:
  `performance/evidence/2026-07-17-nr-08-lifetime-poc/`
- NR-09 bundle:
  `performance/evidence/2026-07-17-nr-09-sequence-ledger-poc/`
- NR-09 large-instruction verdict and corrected rebaseline:
  `performance/evidence/2026-07-18-nr-09-large-instruction-batch-first-release-verdict/`
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
- [x] Adrian accepted NR-09 Rule 1; audited 222-golden/four-address closeout and
  final Debug CTest 1,852/1,852 pass.
- [x] The remaining selected ledger is reclassified as 67 active low-risk
  mappings across 12 batch families; new large instructions are the intended
  mechanism and the next verdict is batch-level, not instruction-level.
- [x] The arbitrary-operand prerequisite is isolated and committed; its
  concrete benefit is recorded as 38/67 mappings needing more than three
  operands.
- [x] The 67-mapping batch is implemented across instructions/tests, RXAS, rxc,
  instruction database and docs/examples.
- [x] The batch reached the mandatory first Release verdict stop with a
  negative cross-session result. Adrian selected a fact-based per-form review;
  the subsequent drift-controlled rebaseline supersedes that negative sign
  with a neutral-to-slightly-positive result. Production is still frozen for
  Adrian's corrected-verdict decision while the reusable timing/coherence/
  side-effect/implementation ledger remains available.

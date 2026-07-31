# cREXX PERF3 performance roadmap

Approved: 2026-07-31

Status: **approved live control plane for PERF3**. Adrian approved the roadmap
and transfer boundary on 2026-07-31, then accepted the PERF3-01 current-product
evidence boundary and ranked panel on 2026-07-31. PERF3-02 is the authorized
next evidence/design activity. This does not authorize a production
implementation, candidate installation or push.

PERF2 is closed and preserved in
[`ROADMAP-PERF2-2026-07-31.md`](ROADMAP-PERF2-2026-07-31.md). The initial
`NR-*` sweep remains closed in
[`ROADMAP-INITIAL-SWEEP-2026-07-23.md`](ROADMAP-INITIAL-SWEEP-2026-07-23.md).
The original dated charter remains a historical snapshot in
[`performance-programme-report-2026-07-15.md`](../docs/planning/release-1/performance-programme-report-2026-07-15.md).
Standing measurement, regression and publication rules remain normative in
[`PERFORMANCE-GOVERNANCE.md`](PERFORMANCE-GOVERNANCE.md) and
[`AGENTS.md`](AGENTS.md).

The approved transition is local `develop` at
`3f43a0014be10c930a12b8a636297b60f294c0a6`, one commit ahead of
`origin/develop` at `21fdcf529d0e51ea264bf0c92ccfbdc06dea8200`.
The pre-existing worktree also contains five untracked
generated `lifecycle_probe.rxbin` files under retained evidence directories;
they are outside this documentation transition and must not be overwritten or
deleted casually. This is a planning baseline, not a benchmark baseline.

Status values are `queued`, `in progress`, `decision required`, `blocked`,
`deferred`, `complete`, `rejected` and `superseded`. `Complete` means the
activity's stated exit criterion and retained evidence both exist. A useful
prototype or a closed predecessor programme does not make an unresolved
mechanism complete.

## PERF3 mission

PERF3 turns the accepted PERF2 Mac, Linux x86-64 and Windows evidence into a
small number of evidence-selected, independently revertible product decisions
on the faster Apple ARM64 development host. It does not repeat PERF2 wholesale,
resume every historical idea or treat platform-dependent compiler movement as
a production mechanism.

The approved operating rule is:

> Re-establish exact current-product truth, remove the largest proved semantic
> or ownership cost at its earliest safe owner, and escalate to VM layout or a
> new execution architecture only when the remaining evidence requires it.

The PERF2 north star is retained as a programme target, not as permission to
change semantics or benchmark work:

1. the selected default VM should reach at least 1.50x ooRexx median throughput
   on every qualified equal-work common cell;
2. the common-five geometric mean should reach at least 2.00x ooRexx;
3. the separately governed cREXX RexxCPS 2.2d diagnostic should reach at least
   1.50x same-session canonical Classic ooRexx RexxCPS; and
4. the alternate VM should itself be clearly faster than ooRexx on every
   qualified cell.

Only semantically qualified, correctness-passing, same-session results count.
`rxvm` and `rxbvm`, throughput, lifecycle, RSS and artifacts remain separate.

## What PERF2 established

### Accepted product and evidence foundation

- The initial sweep, current-product profiling schema, governance, repeatable
  evidence tools and common-five comparison contract are complete.
- PERF2-01 through PERF2-05 are closed with accepted compiler, inlining, BIF,
  private execution-image and reference-path work.
- The Apple PERF2-06/07 slice is closed: V3-R01 corrected stale string-length
  state and V1R01-R1 removed the proved receiver-copy explosion.
- PERF2-08/09 closed the Mac qualification and same-session scorecard. The
  common aggregate is exactly Sieve, Permute, Bounce, Richards and Base64;
  RexxCPS remains a separately governed diagnostic.
- The initial Linux x86-64 GCC/Clang correctness, sanitizer, formal baseline,
  schema-5 and native-PMU campaign is checksum-closed and sufficient for
  mechanism selection.
- The supported Windows x86-64 baseline and bounded GCC, Clang and MSVC
  controls are retained. They selected no compiler, CRT, VM or production
  optimization.
- CAP-01's indexed `rxjson` document and accepted numeric projection work are
  closed. Their general residual mechanisms were extracted below rather than
  left hidden inside the capability activity.

### Current accepted Mac outcome snapshot

The 2026-07-27 PERF2-09 Mac scorecard at `d5f0827ca` is the last formal
same-session Apple comparison, not an automatic current-HEAD baseline.
Product-affecting compiler, VM and library work has landed since its source
snapshot.

| Workload | Qualification | `rxvm / ooRexx` | `rxbvm / ooRexx` | PERF3 meaning |
| --- | --- | ---: | ---: | --- |
| Sieve | common | 7.214291x | 5.338790x | Established win and zero-work/code-layout guard. |
| Permute | common | 8.005043x | 7.015322x | Established win; guard accepted call/value placement. |
| Bounce | common | 3.902513x | 2.963270x | Established win; do not reopen reference work without a new exact reduction. |
| Richards | common | 0.267262x | 0.264171x | Largest qualified common deficit and strongest current copy/value lead. |
| Base64 | common | 0.719817x | 0.724922x | Deficit remains noisy; require exact work reduction and stable same-session evidence. |
| RexxCPS | separate diagnostic | 0.995754x | 0.933193x | Near Mac parity but below the separate 1.50x band. |
| Towers | qualified separate lane | 0.328060x | 0.321343x | Large object/allocation deficit; not part of the common aggregate. |

The exact PERF2 Mac common-five geometric means were 2.125260x/1.842840x
versus ooRexx and 0.742985x/0.644251x versus decimal NetRexx for
`rxvm`/`rxbvm`. These remain historical observations until PERF3-01 decides
what current-HEAD refresh is required.

### Mechanisms selected by retained cross-platform evidence

| Evidence | Observation | Planning consequence |
| --- | --- | --- |
| Linux Richards | 56.9 million copy operations and 451.7 MB copied in the bounded profile; `copy_value` accounts for roughly 55-57% of GCC and 77% of Clang sampled cycles, with Clang also exposing attribute-storage trimming. | Make full-copy elimination/ownership the first planned Mac design panel. Do not assume the VM handler is the right owner. |
| Linux Towers | 26.8 million copies, 31.3 million clear/reset/destroy operations and 5.86 GB of allocation requests; front-end and indirect-branch pressure are also visible. | Separate exact copy/clear/attribute shapes from allocator or global-value-layout hypotheses. |
| Linux Base64 | 46.7 million VM instructions; `SCOPY_REG_REG` is the third-ranked opcode and 92-96% of sampled cycles remain in `run`. | Compare a semantic string/copy ceiling with code-layout alternatives before selecting either. |
| Linux RexxCPS | Decimal conversion/formatting and string movement remain visible; the GCC `rxvm` native cell is 42% front-end bound. | Keep conversion and layout as separate hypotheses; do not infer a single cause. |
| CRI-13 residual | The retained projection executes 6,144 full-source copies totalling 359,294,976 logical bytes; RXAS currently reports `full-value-ownership-unproved`. | Carry the byte-weighted proof question into PERF3-02. |
| Concrete scalar access | Current final/concrete wrapper reads are 4.56x-5.10x raw access and writes are 2.41x-3.67x across both VMs. | Preserve a generic accessor-proof lead, not JSON/vector-specific opcodes. |
| Apple/Linux/Windows compiler controls | Compiler direction reverses by workload; zero-work Apple controls moved with code layout, Clang helps some Linux/Windows cells and hurts others, and MSVC `/MT` improves one Windows control without closing the gap. | Treat compiler, CRT and layout results as qualified leads only. Require paired candidate evidence and semantic zero-work guards. |

## PERF2 to PERF3 transfer register

The following items are deliberately transferred. The old stable ID remains in
the source column so no unfinished item disappears during renumbering.

| PERF2 source | PERF3 owner | Closing disposition carried forward | PERF3 entry condition |
| --- | --- | --- | --- |
| `PERF2-07-B02` | PERF3-02 | queued evidence/design | Current byte-weighted copy census confirms a material general full-copy cost. |
| Linux Richards/Towers copy and attribute-trim findings | PERF3-02 | selected mechanism family, no candidate | Exact caller/payload/lifetime shapes and machine ceilings distinguish compiler, RXAS and runtime ownership. |
| `PERF2-05-F01` | PERF3-02 | evidence-gated | A fresh profile attributes material residual reference-descriptor payload cost after accepted R2a. |
| `PERF2-03-F01` and `PERF2-03-F02` | PERF3-02 | evidence-gated | Current hot sites prove residual accessor/ownership/escape cost and exact alias/lifetime obligations. |
| `PERF2-07-C01` | PERF3-03 | queued evidence/design | Conversion allocation/copy and semantic-contract review shows a material current ceiling. |
| `PERF2-03-F06` | PERF3-04 | queued evidence only | Current profile plus hand-equivalent ceiling selects generic final/concrete scalar access. |
| `PERF2-06-D01` | PERF3-05 | open accepted debt | Paired Mac zero-work and target controls distinguish native code layout from semantic work. |
| PERF2-06 compact/hot-cold private stream and PERF2-10 LTO/PGO/layout | PERF3-05 | unstarted, no option selected | A bounded Mac panel identifies a repeatable supported mechanism before production selection. |
| PERF2-09 qualified gaps | PERF3-06 | outcome lane | An accepted product slice exists or PERF3-01 changes the ranking. |
| `CAP-02`, `CAP-03` and `CAP-04` | PERF3-07 | deferred or independent product/evidence tracks | Separate capability/API/use-case approval; they do not block qualified common cells. |
| PERF2-11 Gate E and final VM recommendation | PERF3-08 | incomplete late gate | A Mac-selected candidate is accepted and ready for batched platform validation. |
| PERF2-12 JIT/AOT/native backend | PERF3-09 | deferred | The accepted non-JIT programme cannot meet the target economically and Adrian approves a separate architecture decision. |

### Preserved conditional triggers, not queued work

These points remain discoverable but do not consume PERF3 capacity unless
their recorded trigger fires:

| Source | Preserved disposition |
| --- | --- |
| `PERF2-03-F03` | Admit remaining inline-exit/result/temporary cleanup only as a bounded companion to a currently selected hot site. |
| `PERF2-03-F04` | Reopen dynamic vararg/association/effect reconstruction only with a measured multi-site deficit. |
| `PERF2-03-F05` | Standing producer/consumer consistency requirement owned by any change that consumes new summary facts. |
| `PERF2-05-F02` | Reopen result forwarding only with mathematical equivalence and stable multi-workload dual-VM benefit. |
| `PERF2-06-C2R02` | Deferred; rejected reset evidence gives no reason to advance quickened clearing. |
| `PERF2-06-C2R03` | Analysis-only architecture candidate; it must first pass a current payload-capacity/high-water entrance gate. |
| Higher-arity call/frame forms | No `CALL5+` or embedded-argument work without a refreshed dynamic residual census after accepted inlining. |
| Legacy `FDIVSUB`, `ILOADSETUNLINKN` and frequency-only fusion ideas | Archive-only unless a current exact profile selects the mechanism. |
| Windows MSVC `/MT` | Experimental validation lead only; plugin/API allocator ownership must be proved before any product selection. |
| RexxCPS timer cross-check | Cheap cross-OS validation lead for a later platform campaign, not a Mac optimization. |

### Closed or rejected work that PERF3 must not silently repeat

- Do not retry C2-A/B, fixed-core reset R1/R2, exact reset lists, quickened
  clearing, C3R01 numeric synchronization or cleanup-only flattened-interpreter
  reshaping without materially different ownership evidence and zero-work
  controls.
- Do not reopen selector caches: accepted profiles observed zero attempts.
- Do not add a public RXAS/RXBIN form merely because a private or compiler
  form has a useful ceiling.
- Do not rerun Linux x86-64 or Windows baselines for questions answerable from
  the retained immutable products, profiles, samples or small external
  harnesses.
- Do not reopen CAP-01's accepted API or benchmark-local class probe while
  investigating the extracted generic copy, conversion or accessor questions.
- Do not edit the dated charter or either closed roadmap to reflect PERF3.

## Activity register

| ID | Priority | Activity | Status | Exit / next gate |
| --- | --- | --- | --- | --- |
| PERF3-00 | P0 | Archive PERF2 and approve the transfer boundary | complete | Adrian approved the roadmap and transfer boundary on 2026-07-31; no production work was bundled with approval. |
| PERF3-01 | P0 | Current-HEAD Mac evidence and baseline-validity gate | complete | Adrian accepted the current-product evidence boundary and ranked panel on 2026-07-31. No production edit was made. Evidence: [`2026-07-31-perf3-01-current-mac`](evidence/2026-07-31-perf3-01-current-mac/); control: [`PERF3-01-WORKLIST.md`](PERF3-01-WORKLIST.md). |
| PERF3-02 | P0 | Full-copy, ownership and attribute-storage panel | queued | Compiler, RXAS and runtime ceilings are compared; one bounded candidate or an evidence-backed defer/reject recommendation is presented. |
| PERF3-03 | P1 | Bounded string-to-number conversion review | queued | Current semantics and costs are frozen; no-copy/locale-independent/correct-rounding options are compared without silently changing public behavior. |
| PERF3-04 | P1 | Generic final/concrete scalar accessor proof | queued evidence only | A general proof and hand-equivalent ceiling justify a candidate, or the lead is deferred. |
| PERF3-05 | P1 | Compiler, native layout and private-stream panel | queued | D01, semantic string/copy controls, hot/cold layout, LTO/PGO and compact private representation receive bounded paired dispositions; no flag or stream is preselected. |
| PERF3-06 | P0 | Qualified-deficit closure and Mac scorecard | queued | Accepted slices are reflected in a formal same-session common-five plus RexxCPS/Towers scorecard; every guard and exclusion is explicit. |
| PERF3-07 | P2 | Capability and lifecycle side lanes | deferred/independent | Each approved product/capability use case has its own scope and does not distort the common benchmark programme. |
| PERF3-08 | P1 | Selected-candidate platform validation and default-VM decision | queued late gate | Apple ARM64, Linux x86-64, supported Linux ARM64 and Windows evidence support an explicit default/private-stream recommendation or a named defer. |
| PERF3-09 | P3 | JIT/AOT/native-backend architecture decision | deferred | Reopen only under the recorded economic and architecture gate. |

## Approved execution order

1. **Approve the control plane.** Review PERF3-00 and amend scope, priorities,
   target bands or transfer dispositions before activity work begins.
2. **Establish current Mac truth.** PERF3-01 audits every product-affecting
   change since the accepted Mac scorecard, replays the evidence manifests
   actually used, and decides exactly which current-HEAD timing/profile cells
   must be refreshed. It stops with a ranked panel and no production edit.
3. **Lead with proved work, not subsystem preference.** Unless PERF3-01
   overturns the retained evidence, PERF3-02 is the first design/PoC activity.
   It compares compiler, RXAS and runtime ownership for exact copy/value shapes.
4. **Keep orthogonal risks separate.** PERF3-03 conversion semantics,
   PERF3-04 accessor proof and PERF3-05 native layout/stream work do not ride
   inside a copy candidate. Each receives its own entry criterion and stop.
5. **Apply the mandatory first Release verdict.** After Adrian selects one
   production candidate, run only the minimum focused correctness needed,
   freeze implementation, build the ordinary profiling-off Release product,
   run the smallest decisive paired target plus guards, report and stop.
6. **Close accepted slices proportionally.** Broad QA, sanitizer,
   install/package, compatibility and documentation follow only after the
   first verdict is accepted and in proportion to the changed surface.
7. **Refresh outcomes deliberately.** PERF3-06 runs the formal Mac scorecard
   after an accepted group of slices or when a ranking decision requires it,
   not after every small edit.
8. **Validate, then select architecture.** PERF3-08 reuses retained Linux and
   Windows evidence, adds the selected candidate and still-required supported
   Linux ARM64 lane, and only then recommends the default VM/private stream.

The dependency shape is:

```text
PERF3-00 roadmap approval
└── PERF3-01 current Mac truth and accepted ranking
    ├── PERF3-02 copy / ownership / attribute storage
    ├── PERF3-03 conversion contract and ceiling
    ├── PERF3-04 concrete scalar accessor proof
    └── PERF3-05 compiler / layout / private stream

accepted independently gated production slices
└── PERF3-06 formal Mac outcome scorecard
    └── PERF3-08 Linux x86-64 -> Linux ARM64 -> Windows validation
        └── default VM / private execution recommendation

PERF3-07 capability lanes remain independent.
PERF3-09 remains deferred unless the non-JIT economic gate fires.
```

## PERF3-01 — current-HEAD Mac truth gate

Started 2026-07-31 at local `develop` commit `3f43a0014`. The resumable
evidence-only control plane is
[`PERF3-01-WORKLIST.md`](PERF3-01-WORKLIST.md). No production edit or candidate
selection is authorized in this activity.

Evidence collection is complete and Adrian accepted it on 2026-07-31. Clean current
Release timing records a common-five `2.139811x/1.818954x` versus ooRexx and
`0.779920x/0.662974x` versus decimal NetRexx for `rxvm`/`rxbvm`. Richards
remains the dominant common deficit; current deterministic counts plus retained
Linux native attribution keep PERF3-02 first. The evidence panel ranks
PERF3-05 second, PERF3-03 third and keeps PERF3-04 evidence-gated. See the
[`decision summary`](evidence/2026-07-31-perf3-01-current-mac/decision-summary.md).

### Question

Which retained PERF2 baseline and attribution conclusions remain valid for
exact current HEAD, and which mechanism should receive the first PERF3 design
panel?

### Required work

1. Freeze branch, exact commit, dirty scope, host/power/toolchain and ordinary
   profiling-off Release product identity.
2. Audit product-affecting changes since the 2026-07-27 Mac scorecard. Compare
   compiler, VM, library, workload, tool and manifest hashes before deciding a
   retained cell is reusable.
3. Replay only the retained checksum manifests actually used for decisions.
   Preserve and reconcile the untracked generated lifecycle files as a named
   repository-state issue; do not normalize, delete or regenerate evidence
   casually.
4. If the old Mac scorecard is not valid for current-product ranking, capture
   the smallest governed refresh that restores authority. A representative
   multi-workload set includes RexxCPS; common aggregate claims require all
   five common workloads.
5. Refresh diagnostic counts/native samples only where existing Linux and Mac
   artifacts cannot distinguish the candidate owners. The expected focused
   set is Richards, Towers, Base64, RexxCPS and Sieve as a zero-work/layout
   guard, in both VMs; this is the expected set, not an automatic requirement
   to rerun every profile.
6. Produce a current gap/mechanism ledger with exact operation counts, bytes,
   native footprint, machine ceiling, semantic risk and earliest safe owner.

### Exit and stop

Adrian accepts the current-product evidence boundary and a ranked PERF3-02/03/
04/05 panel. No performance production source is edited, no candidate is
silently selected and no broad platform rerun occurs in PERF3-01.

## PERF3-02 — full-copy, ownership and attribute-storage panel

### Question

Can cREXX remove or narrow the current high-cost full-value copies before they
reach `copy_value`, while preserving by-value isolation, reference identity,
recursive attributes, native payloads, unwind and observable intermediate
state?

### Required comparison

| Variant | Owner | Question |
| --- | --- | --- |
| C0 | current product | Exact caller, payload-shape, byte and lifetime baseline. |
| C1 | `rxc` semantic proof | Can typed flow/inline facts eliminate the copy or directly place the result? |
| C2 | RXAS whole-procedure proof | Can existing CFG/liveness/effects prove a full-copy projection without compiler-only knowledge? |
| C3 | narrow typed/payload operation | Can a proved scalar, binary or no-payload shape avoid general recursive copy work? |
| C4 | runtime machine ceiling | What is the exact direct operation cost after every eligible semantic decision is preproved? |

Richards, Towers and the CRI-13 residual are evidence sources, not permission
for benchmark-specific handling. Compare `rxvm` and `rxbvm`; include existing
Sieve/Permute/Bounce wins as appropriate guards. Keep copy elimination,
attribute-storage trimming and teardown independently attributable.

Before any production edit, create a resumable PERF3-02 worklist with at least
two viable production approaches, exact semantic obligations, machine-level
ceilings and a declared first Release stop. Present the PoC panel to Adrian for
selection; do not install a production ladder automatically.

## PERF3-03 — bounded conversion review

Freeze the current `rx_string_to_double` and `string2integer` allocation,
grammar, whitespace/sign, locale, range, exception and rounding contracts.
Compare the existing temporary NUL-terminated copy with bounded no-copy,
locale-independent and correctly rounded controls across signed zero, halfway
rounding, normal/subnormal, finite/overflow, long input and `inf`/`nan`
compatibility.

This activity starts as evidence/design only. A public RXAS operation, changed
conversion signal, ABI or language contract is a separate decision. It may
advance only if PERF3-01 confirms material current cost and the candidate is
general beyond CRI-13 or RexxCPS.

## PERF3-04 — generic final/concrete scalar access

Test whether statically resolved final/concrete scalar getters and setters can
approach direct typed-memory cost using generic compiler proof. Cover receiver
initialization and identity, writable ownership, signals, source/debug
identity, imports, optimized/no-opt output and both VMs. The hand-equivalent
ceiling precedes any production form.

Do not add JSON-, vector- or numeric-width-specific methods/opcodes. Missing
proof rejects only the affected candidate site; the ordinary call and
materialized path remain valid.

## PERF3-05 — compiler, layout and private-stream panel

This activity owns `PERF2-06-D01`, the unstarted PERF2-10 options and the
compact/hot-cold private execution question. It does not own semantic copy or
conversion work that can be removed earlier.

The bounded Mac panel may compare:

- current source/product as a zero-work drift control;
- semantic Base64 string/copy ceilings before native layout changes;
- hot/cold handler/helper outlining and source ordering;
- supported LTO/interprocedural and representative PGO controls;
- private compact operand/stream or decoded hot/cold representations while
  canonical RXBIN remains unchanged; and
- `rxvm` and `rxbvm`-specific forms only when a common semantic fallback and
  maintenance boundary are explicit.

Record build time/memory, `run()` text/data, retired instructions, front-end,
branch/I-cache evidence, startup/preparation, RSS, artifacts and both VM modes.
A compiler or flag movement without an exact causal mechanism is a control, not
a candidate. A compact stream requires representative benefit on at least two
architectures before adoption.

## PERF3-06 — qualified-deficit closure

Track each accepted product slice against the qualified score rather than one
headline benchmark:

- guard Sieve, Permute and Bounce;
- close Richards and Base64 only with general mechanisms;
- keep RexxCPS separate and first-class in sampling;
- keep Towers as a qualified separate object/allocation lane; and
- retain Mandelbrot, Storage, List and JSON under their approved explicit
  no-ratio dispositions unless a separately approved equivalence decision
  changes them.

The formal Mac refresh uses the ordinary profiling-off Release product, exact
current sources/images, two warmups and ten serial recorded observations per
absolute cell. A before/after verdict uses at least one warmup and twelve
balanced/interleaved pairs, with governance append rules. No unmatched
historical ratio becomes a regression claim.

## PERF3-07 — capability and lifecycle side lanes

- `CAP-02` owned heterogeneous/nested containers remains a separate
  post-Release 1 Level G or explicitly approved library/runtime decision.
- `CAP-03` a standard Base64 API remains a separate pure-Level-B product track;
  it does not replace the qualified common codec-loop benchmark.
- `CAP-04` pure-load lifecycle remains a measurement/API-use-case question and
  enters public API work only with an approved product need.

These lanes can proceed under their own authority but do not borrow PERF3
performance approval or alter benchmark equivalence silently.

## PERF3-08 — platform validation and architecture selection

The approved order is:

1. Apple ARM64 design, PoC and mandatory first Release verdict;
2. retained Linux x86-64 evidence audit, followed by a batched selected
   candidate run only when required;
3. supported Linux ARM64 correctness, timing and relevant counter coverage;
4. supported Windows x86-64 correctness, scorecard and artifact validation;
5. whole-scorecard default VM/private representation decision.

The existing Windows GCC/Clang/MSVC and `/MT` results remain controls. A static
CRT cannot become the default until allocator ownership across executables,
DLLs, plugins and public APIs is proved. A later cross-OS RexxCPS validation
should compare the benchmark-reported timer with an external monotonic timer.

The final decision must state whether `rxvm`, `rxbvm` or a platform-specific
choice is the default; which private execution representation is selected;
which supported compilers/options are normative; and which deficits remain.
Canonical RXBIN portability, public ABI, TRACE/source/debug identity, late
load, plugins, lifecycle, RSS and artifacts remain hard dimensions.

## PERF3-09 — JIT/AOT/native-backend decision

Keep this deferred. Reopening requires current residual gaps after accepted
PERF3 non-JIT work, a comparison of native AOT, baseline JIT, tracing,
quickening and existing-VM maintenance, and explicit treatment of debugger/
TRACE, signals, dynamic loading, plugins, portability, packaging, sandboxing
and reproducible builds. Adrian must approve a separate architecture programme
before implementation.

## Worklist and evidence contract

Before the first production edit in any PERF3 activity, create a resumable
worklist recording:

1. exact baseline commit, branch/upstream/dirty state and artifact hashes;
2. one falsifiable hypothesis and named current mechanism footprint;
3. status quo plus at least two plausible implementation owners/forms when
   two exist, or a recorded reason why only one is viable;
4. machine-level ceiling and exact semantic proof obligations;
5. focused correctness and regression-guard matrix;
6. ordinary profiling-off Release comparison and hard first-verdict stop;
7. proportional closeout only after Adrian accepts the verdict;
8. accepted, rejected, neutral and invalidated evidence; and
9. dated status links back to this roadmap.

All maintained performance orchestration remains cREXX Level B under
`performance/tools/`. Temporary host-side analysis may be used outside the
repository, but it does not become the maintained control plane.

## Approval record

Adrian approved these five points on 2026-07-31:

1. PERF2 is closed at its recorded state and this file is the live PERF3
   control plane.
2. PERF3-01 is the only authorized next activity, with no production edit and
   a hard evidence/ranking stop.
3. Full-copy/ownership is the provisional first design panel unless PERF3-01
   current-product evidence overturns it.
4. Conversion, accessor, layout/stream, capability and JIT/AOT work remain
   behind their separate entry and decision gates.
5. Mac iteration is followed by selected Linux x86-64, required Linux ARM64
   and Windows validation before the default-VM/final architecture decision.

Adrian subsequently accepted the PERF3-01 current-product evidence boundary
and ranked PERF3-02/03/04/05 panel on 2026-07-31. This closes PERF3-01 and
opens PERF3-02 for its bounded evidence/design and PoC comparison only.

## Authoritative references

- standing instructions and governance:
  [`AGENTS.md`](AGENTS.md) and
  [`PERFORMANCE-GOVERNANCE.md`](PERFORMANCE-GOVERNANCE.md)
- closed PERF2 register:
  [`ROADMAP-PERF2-2026-07-31.md`](ROADMAP-PERF2-2026-07-31.md)
- closed initial register:
  [`ROADMAP-INITIAL-SWEEP-2026-07-23.md`](ROADMAP-INITIAL-SWEEP-2026-07-23.md)
- retained Mac scorecard:
  [`2026-07-27-perf2-09-mac-closure`](evidence/2026-07-27-perf2-09-mac-closure/)
- retained Linux x86-64 attribution:
  [`2026-07-28-perf2-10-11-intel-linux`](evidence/2026-07-28-perf2-10-11-intel-linux/)
- retained Windows baseline and controls:
  [`2026-07-29-perf2-11-windows-x86-64`](evidence/2026-07-29-perf2-11-windows-x86-64/),
  [`2026-07-30 compiler comparison`](evidence/2026-07-30-perf2-11-windows-compiler-comparison/) and
  [`2026-07-30 MSVC rxbvm`](evidence/2026-07-30-perf2-11-windows-msvc-rxbvm/)
- extracted copy/conversion/accessor evidence:
  [`CRI-13 RXAS trace`](evidence/2026-07-29-crexx-rag-integration-ledger/CRI13-R1-RXAS-TRACE.md),
  [`bounded conversion decision`](evidence/2026-07-29-crexx-rag-integration-ledger/CRI13-BOUNDED-NUMERIC-CONVERSION-DECISION.md) and
  [`class-access verdict`](evidence/2026-07-29-crexx-rag-integration-ledger/CRI13-C-CLASS-RELEASE-VERDICT.md)

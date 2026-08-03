# cREXX PERF3 performance roadmap

Approved: 2026-07-31

Status: **approved live control plane for PERF3**. Adrian approved the roadmap
and transfer boundary on 2026-07-31, then accepted the PERF3-01 current-product
evidence boundary and ranked panel on 2026-07-31. PERF3-02's clean-host panel,
C1b detached-guard proof, C2E2 storage-identity infrastructure and
infrastructure-enabled R1 repanel are complete. Adrian selected the composed
C1abc production ladder on 2026-08-01. The disposable option mask and broad
correctness-negative branch are removed from the ordinary compiler while the
checksum-closed replay source preserves every investigated option.

C1abc emits the exact measured C1a+C1b Richards program and C1c Towers
program. The accepted R1 paired medians are 53.55%/52.57% lower Richards
elapsed and 18.92%/18.97% lower Towers elapsed on `rxvm`/`rxbvm`, with every
pair favorable. Production closeout passes 11/11 focused Release checks,
16/16 reviewed object structural/runtime pairs and 1,972/1,972 broad Debug
tests. The old clean-host C0/C1a/C1c timings remain historical authority. C2
still lacks ownership/lifetime proof, C3 is immaterial, C4 is met by the
selected C1 rows. P1A is complete: A1 demand-driven storage attachment is
retained, A3 is a correct replayable negative, and accepted closeout passes
24/24 focused plus 1,972/1,972 broad Debug tests.
PERF3-05 is complete: Adrian accepted retaining the ordinary L0 product,
rejecting the tested LTO/PGO/no-flatten runtime forms and leaving L4 unopened.
The separate VM-library link-interface cleanup is queued as PERF3-05-B1.
Adrian selected and accepted PERF3-03 C4 v3. The narrow private locale-aware
loose-comparison prefilter passes its 6/6 minimum gate and mandatory first
ordinary Release verdict: Base64 improves by 4.86%/5.78% on `rxvm`/`rxbvm`;
RexxCPS is +2.52% on `rxvm` and a guard-clean noisy -0.61% on `rxbvm`.
Accepted Apple closeout passes 1,972/1,972 full Debug tests, 6/6 focused ASan,
complete Release build/install and installed VM smoke 2/2. LSan is unavailable
on this macOS runtime; Windows/MSVC validation is queued before publication.
Adrian authorized the combined local closeout commit on 2026-08-01; push
remains a separate user-authorized action.
Adrian subsequently accepted PERF3-10: an ordered TRACE result-event batch and
a storage-identity/component-aware RXAS proof for redundant integer-to-string
materialization. Its first ordinary Release verdict improves RexxCPS median
CPS by 10.38% on `rxvm` and 10.61% on `rxbvm`, with 21/22 and 12/12 favourable
pairs. Closeout passes 59/59 focused and 1,982/1,982 broad Debug tests.
No tactical-rule deletion or public format change was made. The complete
PERF3-02/C1abc slice is committed locally as `4a3940395`; push remains a
separate user-authorized action.

PERF3-11 Gates 1-6 are now locked.  The reusable per-epoch proof service is
the sole authority for repeated `ITOS`; its stronger write-once/component SSA
proof produces a 19-`ITOS` RexxCPS image versus the retained old solver's 21.
Adrian accepted the mandatory Release verdict: median CPS improves 7.469% on
`rxvm` and 6.866% on `rxbvm`, with 12/12 favourable pairs on each VM.  A broad
test failure exposed and drove a precise caller-owned call-window argument
model; the corrected closeout passes 1,987/1,987 Debug tests.  The remaining
legacy-proof inventory and stable replay baseline are now locked.  M01 is
complete: the old one-register `ITOF` authority is deleted and the generic
metadata-driven `XTOY` consumer covers all 20 one-register conversions.  It
recovers the old `ITOF` floor and proves 11 additional focused deletions,
including four `ITOD` flow shapes; signalling and same-component-idempotence
gaps remain closed.  `ITOD` and `BTOD` now have a coherent total
non-signalling runtime/plugin contract.  M02 is also complete: repeated scalar
constants now require write-once value equivalence and already-absent reference
and native payloads.  It recovers the old floor and proves four stronger
focused deletions through equal phis, exact float bits, linked storage and
ordered TRACE.  Adrian accepted its output-neutral Release verdict and bounded
procedure-local 30.1 MB peak RSS on 2026-08-03.  Canonical images remain
unchanged, focused replay passes 53/53 and broad Debug passes 1,991/1,991.
M03 is also complete: repeated `NULL` now requires known storage and all eight
component leaves already absent.  It recovers the old floor and adds equal-phi,
linked-storage and ordered-TRACE cases while preserving scalar, reference and
native cleanup.  Adrian accepted the output-neutral verdict; broad Debug
passes 1,993/1,993.  M04 exact self-copy is next.  Exact old/new parity is not
the gate: the new service must preserve the old valid safe domain and may
prove a larger separately validated domain.

PERF2 is closed and preserved in
[`ROADMAP-PERF2-2026-07-31.md`](ROADMAP-PERF2-2026-07-31.md). The initial
`NR-*` sweep remains closed in
[`ROADMAP-INITIAL-SWEEP-2026-07-23.md`](ROADMAP-INITIAL-SWEEP-2026-07-23.md).
The original dated charter remains a historical snapshot in
[`performance-programme-report-2026-07-15.md`](../docs/planning/release-1/performance-programme-report-2026-07-15.md).
Standing measurement, regression and publication rules remain normative in
[`PERFORMANCE-GOVERNANCE.md`](PERFORMANCE-GOVERNANCE.md) and
[`AGENTS.md`](AGENTS.md).

The committed baseline for this closeout tranche is
`4a3940395980dc40ea45917d71d99caa080e89bb`; the PERF3 planning baseline
remains `e38e514bf611ae3873513368c44742e2ae7332d1`, whose product-code parent is
`3f43a0014be10c930a12b8a636297b60f294c0a6`. The exact new local tip is kept in
repository history rather than self-referenced by the commit that contains
this roadmap. Push remains unauthorized.
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
| `PERF2-07-B02` | PERF3-02 | C1abc selected and Apple closeout complete | Closed by the selected C1a+C1b Richards and C1c Towers production ladder; retain timing and replay evidence. |
| Linux Richards/Towers copy and attribute-trim findings | PERF3-02 | exact C0 attribution and selected C1abc proof complete | Closed for Apple; reuse retained evidence in the later platform-validation gate. |
| `PERF2-05-F01` | PERF3-02 | evidence-gated | A fresh profile attributes material residual reference-descriptor payload cost after accepted R2a. |
| `PERF2-03-F01` and `PERF2-03-F02` | PERF3-02 | evidence-gated | Current hot sites prove residual accessor/ownership/escape cost and exact alias/lifetime obligations. |
| `PERF2-07-C01` | PERF3-03 | Apple closeout complete | The material owner is implicit loose-comparison parsing. Adrian selected and accepted private C4 v3; first verdict and proportional Apple closeout pass. C1/C2/C3/cache/public-span alternatives remain rejected or deferred. Windows/MSVC validation is retained as PERF3-03-W1. |
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
| PERF3-02 | P0 | Full-copy, ownership and attribute-storage panel | complete | Adrian selected C1abc. The ordinary compiler emits the exact measured C1ab Richards and C1c Towers programs; 11/11 focused Release and 1,972/1,972 broad Debug tests pass. Broad C1a-R1 remains rejected and replayable; C2 is ownership-deferred and C3 immaterial. Control: [`PERF3-02-R1-WORKLIST.md`](PERF3-02-R1-WORKLIST.md); timing: [`2026-08-01-perf3-02-r1-repanel`](evidence/2026-08-01-perf3-02-r1-repanel/); closeout: [`2026-08-01-perf3-02-c1abc-closeout`](evidence/2026-08-01-perf3-02-c1abc-closeout/). |
| PERF3-02-C1B | P0 | Multi-return receiver-link ownership feasibility | complete — correct and decisively faster | The exact detached scalar receiver-guard rule preserves canonical `23246/9297`, all fail-closed boundaries and link/unlink state. The R1 panel measures 44.28%/44.01% lower Richards elapsed alone and proves clean composition with C1a. The broad no-write route remains rejected. Control: [`PERF3-02-C1B-WORKLIST.md`](PERF3-02-C1B-WORKLIST.md); correctness: [`2026-08-01-perf3-02-c1b-correctness`](evidence/2026-08-01-perf3-02-c1b-correctness/); timing: [`2026-08-01-perf3-02-r1-repanel`](evidence/2026-08-01-perf3-02-r1-repanel/). |
| PERF3-02-C2E2 | P0 | RXAS symbolic register-storage identity | complete — core-infrastructure candidate | The diagnostic PoC safely follows direct link/swap/unlink identity, recovers exact point state at all 55 globally tainted Richards full-copy sites and 13/56 Towers sites, and proves a consolidated swap-round-trip route. P1 subsequently supplied the required split normal/signal-skip/signal-retry edges; no rewrite or tactical-rule deletion is selected. Control: [`PERF3-02-C2E2-WORKLIST.md`](PERF3-02-C2E2-WORKLIST.md); evidence: [`2026-07-31-perf3-02-c2e2-storage-identity`](evidence/2026-07-31-perf3-02-c2e2-storage-identity/). |
| PERF3-02-C2E2-P1 | P0 | Core storage identity and signal continuations | locked — infrastructure retained | Typed normal/signal-skip/signal-retry edges and the graph-owned bounded storage service are the frozen R1 foundation. No rewrite consumer or tactical rule is bundled. Control: [`PERF3-02-C2E2-P1-WORKLIST.md`](PERF3-02-C2E2-P1-WORKLIST.md). |
| PERF3-02-C2E2-P1A | P1 | Recover storage-analysis assembler cost | complete — A1 retained, A3 rejected | Adrian accepted the bounded disposition on 2026-08-01. A1 demand-driven storage attachment is retained; A3 remains a correct, replayable negative and is removed from production. Exact A1 restoration passes 24/24 focused and 1,972/1,972 broad Debug tests. Control: [`PERF3-02-C2E2-P1A-WORKLIST.md`](PERF3-02-C2E2-P1A-WORKLIST.md); A1 verdict: [`2026-08-01 A1 evidence`](evidence/2026-08-01-perf3-02-c2e2-p1a-first-release-verdict/); A3 verdict: [`2026-08-01 A3 evidence`](evidence/2026-08-01-perf3-02-c2e2-p1a-a3-first-release-verdict/); closeout: [`2026-08-01 closeout`](evidence/2026-08-01-perf3-02-c2e2-p1a-closeout/). |
| PERF3-02-R1 | P0 | Infrastructure-enabled copy/ownership option re-investigation | complete — C1abc selected | All positive, combined and rejected masks remain replayable in retained evidence; correctness and 156/156 formal timing executions pass for eligible rows. Production contains only C1a+C1b+C1c and passes proportional closeout. Control: [`PERF3-02-R1-WORKLIST.md`](PERF3-02-R1-WORKLIST.md); evidence: [`2026-08-01-perf3-02-r1-repanel`](evidence/2026-08-01-perf3-02-r1-repanel/); closeout: [`2026-08-01-perf3-02-c1abc-closeout`](evidence/2026-08-01-perf3-02-c1abc-closeout/). |
| PERF3-03 | P1 | Bounded string-to-number conversion review | complete on Apple — C4 v3 retained | Adrian selected and accepted the private locale-aware C4 v3 prefilter. Minimum validation, 212/212 first-verdict executions, 1,972/1,972 full Debug tests, 6/6 focused ASan, complete Release build/install and installed VM smoke 2/2 pass. Base64 improves 4.86%/5.78% and RexxCPS is +2.52%/-0.61% on `rxvm`/`rxbvm`; no cell hits the 3% guard. LSan is unsupported locally and Windows is separately queued. Control: [`PERF3-03-WORKLIST.md`](PERF3-03-WORKLIST.md); first verdict: [`2026-08-01-perf3-03-c4-first-release-verdict`](evidence/2026-08-01-perf3-03-c4-first-release-verdict/); closeout: [`2026-08-01-perf3-03-c4-closeout`](evidence/2026-08-01-perf3-03-c4-closeout/). |
| PERF3-03-W1 | P1 | C4 v3 Windows/MSVC validation | queued pre-publication gate | No Windows cross-toolchain is installed on the accepted Mac host. Before publication, build both VM variants under MSVC, run focused logic/conversion correctness and confirm material Base64/RexxCPS behavior. Do not reopen C4 design without a correctness or guard failure. |
| PERF3-04 | P1 | Generic final/concrete scalar accessor proof | queued evidence only | A general proof and hand-equivalent ceiling justify a candidate, or the lead is deferred. |
| PERF3-05 | P1 | Compiler, native layout and private-stream panel | complete — retain L0 | Adrian accepted the 2026-08-01 panel. Exact C1abc+A1 baseline/drift products match; effective ThinLTO, merged/per-VM PGO and no-flatten layout fail representative or zero-work guards; L4 remains unopened. No production VM change was made. Control: [`PERF3-05-WORKLIST.md`](PERF3-05-WORKLIST.md); evidence: [`2026-08-01-perf3-05-compiler-layout-panel`](evidence/2026-08-01-perf3-05-compiler-layout-panel/). |
| PERF3-05-B1 | P2 | VM library link interface and static API granularity | queued build/API hygiene | Current Mac links complete in 61-71 ms, so the reported large delay is not reproduced. Export leakage is real but not causal in the isolated relink. Rework should make `crexxsaa` implementation archives/includes private, publish only the supported header/export surface, split the static phase API if narrow clients are supported, and remeasure on the reporting host. Evidence: [`link diagnostic`](evidence/2026-08-01-perf3-05-compiler-layout-panel/link-diagnostic/). |
| PERF3-06 | P0 | Qualified-deficit closure and Mac scorecard | queued next accounting gate | PERF3-02, PERF3-03 and PERF3-10 now form an accepted tranche. Capture their exact current-product same-session common-five plus RexxCPS/Towers scorecard before another production candidate changes the baseline; every guard and exclusion remains explicit. |
| PERF3-07 | P2 | Capability and lifecycle side lanes | deferred/independent | Each approved product/capability use case has its own scope and does not distort the common benchmark programme. |
| PERF3-08 | P1 | Selected-candidate platform validation and default-VM decision | queued late gate | Apple ARM64, Linux x86-64, supported Linux ARM64 and Windows evidence support an explicit default/private-stream recommendation or a named defer. |
| PERF3-09 | P3 | JIT/AOT/native-backend architecture decision | deferred | Reopen only under the recorded economic and architecture gate. |
| PERF3-10 | P0 | Trace-safe storage/component conversion proof | complete — C1/T1 accepted | Closeout passes 59/59 focused and 1,982/1,982 broad Debug tests. Paired RexxCPS median CPS improves 10.38%/10.61% on `rxvm`/`rxbvm`; equal-work profiling removes 1,399,605 dynamic instructions and 1,400,000 `ITOS`. Control: [`PERF3-10-WORKLIST.md`](PERF3-10-WORKLIST.md); evidence: [`2026-08-01-perf3-10-trace-safe-itos-closeout`](evidence/2026-08-01-perf3-10-trace-safe-itos-closeout/). |
| PERF3-11 | P0 | Scalable RXAS flow, signal policy and sparse component SSA | in progress — M01-M03 complete; M04 exact self-copy next | Gates 1-6 are locked. The per-epoch proof service is the sole repeated-`ITOS` authority and its accepted Release verdict improves RexxCPS 7.469%/6.866% on `rxvm`/`rxbvm`. M01 generalizes the old `ITOF` floor to metadata-driven `XTOY`; M02 proves equivalent scalar constants plus hidden-cleanup absence; M03 proves repeated all-component absence for known storage and deletes the old repeated-`NULL` authority. Adrian accepted each output-neutral migration verdict. M03 adds equal-phi, linked-storage and ordered-TRACE deletions while retaining scalar/reference/native cleanup; canonical images remain byte-identical and broad Debug passes 1,993/1,993. Control: [`PERF3-11-WORKLIST.md`](PERF3-11-WORKLIST.md); migration: [`PERF3-11-MIGRATION-WORKLIST.md`](PERF3-11-MIGRATION-WORKLIST.md); M01: [`2026-08-02-perf3-11-m01-xtoy`](evidence/2026-08-02-perf3-11-m01-xtoy/); M02: [`2026-08-03-perf3-11-m02-constant-write`](evidence/2026-08-03-perf3-11-m02-constant-write/); M03: [`2026-08-03-perf3-11-m03-absent-write`](evidence/2026-08-03-perf3-11-m03-absent-write/). |
| PERF3-12 | P1 | Current RexxCPS clause-lowering rereview | queued evidence after current scorecard | Re-profile current accepted code and audit general compiler clause shapes, conversion/loop hoisting, inactive TRACE, PARSE, stems and ADDRESS. Separate compiler lowering from reusable RXAS consumers and reject benchmark-specific rewrites. |

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

Started 2026-07-31 at local `develop` commit `e38e514bf`; its product-code
parent remains `3f43a0014`. The resumable control plane is
[`PERF3-02-WORKLIST.md`](PERF3-02-WORKLIST.md). Adrian authorized the bounded
evidence/design and isolated C1-C4 PoC comparison, not a production candidate,
architecture/ISA/ABI change, commit of later evidence or push.

The initial panel completed with C1a-R2 removing 4,910,249 total Richards copy operations
and authoritative clean-host paired Release elapsed improves 9.18%/9.33% on
`rxvm`/`rxbvm`. C1c-R1 removes 7,140,440 total Towers copy operations,
55,158,560 logical bytes and 202,314 attribute blocks, improving 19.42%/19.65%.
Every target pair is favorable and all target mean intervals exclude zero.
Both pass the focused optimized/no-opt dual-VM matrix and keep their opposite
workload image byte-identical; those four guards pass the +3% budget at the
36-pair cap while remaining noisy around zero. C1c-R1 is recommended first;
C1a-R2 remains an independent alternative. The original remote-terminal timing,
exact products, rejected variants and every clean-host sample are preserved.
C1a-R1 is correctness-invalid, C2 accepts zero full copies with current facts,
and C3's zero-byte scalar residual is immaterial. At that initial gate no
production candidate was selected. See the
[`decision summary`](evidence/2026-07-31-perf3-02-copy-ownership-panel/decision-summary.md).

The approved follow-on analysis-only C1b gate is also complete. It proves the
C1a-R1 failure is caused by four taken Boolean-guard returns bypassing
receiver-derived `unlinkn` cleanup; later register reuse writes through stale
aliases into the scheduler object. Common-exit normalization is already
present and insufficient, while retaining private storage with per-exit
copyback removes no target copy. A narrow candidate-local rewrite that first
snapshots every receiver-derived scalar guard, then directly binds the
receiver only under a fail-closed structural and same-frame-signal gate, is a
bounded isolated PoC candidate. Its ceiling is two static/172,394 dynamic full
receiver copies, 25,341,738 recursive operations and 201,354,752 bytes, with
small scalar snapshot/cleanup work retained. Adrian approved the isolated
correctness PoC on 2026-08-01. Its exact structural recognizer and post-clone
validation preserve canonical `23246/9297` on `rxvm` and `rxbvm` with
optimization on and off, reduce the total static copy count by two with both
target receiver copies absent, retain six scalar snapshots, and leave Towers
plus the class-method control
byte-identical. The independent P1 storage proof keeps exact link/unlink
balance, removes one full-copy event from each target procedure and reduces
unknown join state to zero. That correctness-only gate performed no timing;
the later R1 repanel measures C1b alone at 44.28%/44.01% lower Richards elapsed
and the safe C1a+C1b composition at 53.55%/52.57%, with 12/12 favorable pairs
in both VMs. See the
[`C1b analysis`](evidence/2026-07-31-perf3-02-c1b-analysis/analysis.md) and
[`R1 evidence`](evidence/2026-08-01-perf3-02-r1-repanel/README.md).

The R1 replay retains eight build masks and rejects the broad rule both alone
and in combination. C1abc emits the exact C1ab Richards image and exact C1c
Towers image. The locked storage service finds 59 remaining exact-base full
copies in combined Richards and 18 in C1c Towers, but independent ownership,
destruction, value, lifetime, TRACE and continuation proofs remain absent, so
C2 installs no rewrite. C3 remains a zero-byte scalar residual and C4 is met
by the correct C1 paths. Adrian subsequently selected C1abc; the ordinary
compiler now contains that one ladder, the disposable replay mask is removed,
and the checksum-closed replay source preserves all investigated options.

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

The 2026-08-01 panel retains the ordinary L0 product. ThinLTO changes `__text`
by roughly -6.3% but fails Sieve and Base64 guards with VM reversals.
Representative PGO changes `__text` by roughly -25% but over-specializes the
layout, regressing Sieve 33.7-40.4% and Base64 27.7-48.8%. The isolated
no-flatten PoC improves focused compile time and build RSS but regresses the
non-noisy `rxbvm` Sieve guard by 3.639%. These are retained replayable
negatives, not production candidates. The semantic Base64 controls instead
show a 2.69-10.69x ceiling in generated work, so L4 is not opened without a
new mechanism.

The adjacent VM-library link question is separated from runtime layout. The
Mac build does expose implementation details: `libcrexxsaa` exports 367 globals
for a 16-symbol intended surface, `crexxsaa` propagates static implementation
archives and internal include paths as `PUBLIC`, and the static phase-API
object pulls the monolithic interpreter even for lifecycle-only use. Export
filtering does not change measured dylib relink time; removing propagated
archives reduces the downstream test link from a 35.095 ms median to 21.706
ms. `PERF3-05-B1` therefore owns optional build/API cleanup and must first
reproduce the reported slow client/host.

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

On 2026-08-01 Adrian approved P1A followed, after its mandatory Release
verdict, by the bounded PERF3-05 and PERF3-03 evidence/design gates. Each
production candidate still requires its recorded selection and first-Release
stop; this approval does not preselect a public format, layout or changed
conversion contract.

The first P1A A1 verdict was correct but neutral. Adrian authorized the bounded
A3 rework on 2026-08-01; A1 evidence and binaries remain retained as an oracle,
and PERF3-05 stays blocked until the repeated A3 Release verdict is accepted.

The repeated A3 verdict is correct but shows no selectable gain at the governed
36-pair maximum. It remains frozen for Adrian's revert-or-accept decision;
PERF3-05 has not begun.

Adrian accepted the recommended disposition on 2026-08-01: retain A1, reject
and remove A3, complete P1A closeout, then open PERF3-05.

P1A closeout is complete and PERF3-05 is open at its bounded Mac
evidence/design gate. This authorizes isolated comparators, not a production
architecture edit or default/private-stream selection.

The PERF3-05 bounded panel is complete and stopped at selection on 2026-08-01.
Its recommendation is to retain the ordinary product, reject LTO, PGO and the
no-flatten runtime form, and leave L4 unopened. The separately diagnosed VM
library link-interface cleanup is queued as `PERF3-05-B1`; no CMake/API change
is bundled into the runtime decision. PERF3-03 has not begun.

Adrian accepted that PERF3-05 disposition on 2026-08-01. PERF3-05 is closed
with no production VM edit, PERF3-05-B1 remains independently queued, and the
already approved PERF3-03 bounded evidence/design gate is open.

The PERF3-03 evidence/design gate reached selection on 2026-08-01. The
recommendation is the private locale-aware C4 v3 loose-comparison prefilter:
retain the exact current converter on numeric/uncertain spans, add no public
opcode/API/serialized form, and defer the value cache. C4 v1 is rejected for a
confirmed Base64 `rxvm` guard regression after lost caller inlining; v2 is
rejected unchanged for incomplete active-locale proof. No production edit has
been made at that selection gate. A subsequent governed common-layout guard retained 348/348 correct
executions. Permute, Bounce and Richards execute zero loose comparisons; their
six VM cells show four favourable results, one -0.446% small adverse median
inside guard and one +0.147% neutral median at the 36-pair cap.

Adrian selected C4 v3 on 2026-08-01. The production edit is limited to the
portable no-inline private rejector and exact current-converter fallback.
Minimum Debug validation passes 6/6. The mandatory ordinary Release verdict
retains 212/212 correct executions: Base64 paired medians are +4.859%/+5.780%
and RexxCPS is +2.517%/-0.609% on `rxvm`/`rxbvm`; the latter is noisy/neutral
at cap and no cell reaches the -3% guard. The provisional implementation is
stopped for Adrian's verdict acceptance before broad closeout.

Adrian accepted the first Release verdict on 2026-08-01. Proportional Apple
closeout then passed 1,972/1,972 full Debug tests, 6/6 focused ASan tests, the
complete ordinary Release build and 136-file isolated install, plus installed
VM smoke 2/2. The leak-enabled sanitizer attempt is retained as unsupported on
this macOS runtime, and no local Windows cross-toolchain is available; real
MSVC validation is queued as PERF3-03-W1 before publication. PERF3-03 is
complete on Apple and PERF3-04 remains queued. Adrian subsequently authorized
the combined local closeout commit; no push is authorized.

Adrian then approved PERF3-10 on 2026-08-01. The selected C1/T1 candidate is
ordered TRACE result-event batching plus a reusable storage-identity and
component-aware RXAS fact for redundant `ITOS`. C0-C4 and T0-T2 remain
recorded for replay. The candidate must preserve TRACE event count/order/value,
fail closed across unproved writes, calls and signal phases, and stop after its
minimum correctness gate and mandatory first ordinary Release runtime verdict.

That stop gate was reached and Adrian accepted C1/T1 on 2026-08-01. The
ordinary Release comparison passes 72/72 executions. Combined paired median
CPS changes are +10.376% on `rxvm` (21/22 favourable) and +10.612% on `rxbvm`
(12/12 favourable). An equal 200 x 100 counts-only diagnostic removes
1,399,605 total dynamic instructions (2.504%) and 1,400,000 `ITOS` executions
(55.555%). No sample was removed. The affected Debug product then passes 59/59
focused and 1,982/1,982 broad tests. Exact C0/C1 artifacts, timing, profiles,
the reviewed legacy TRACE expectation update and closeout logs are retained in
the checksum-closed
[`PERF3-10 evidence bundle`](evidence/2026-08-01-perf3-10-trace-safe-itos-closeout/).
T2 and wider conversions remain outside the accepted slice; no push is
authorized.

## PERF3-11 — component generations and signal-phase flow

Adrian approved the scalable per-procedure graph, dedicated signal-policy and
sparse component-SSA architecture on 2026-08-02.  The complete staged plan,
semantic-change boundary, clean-base oracle, scaling gates and mandatory first
consumer verdict are controlled by
[`PERF3-11-WORKLIST.md`](PERF3-11-WORKLIST.md).  Production execution is
authorized subject to its mandatory gates; no push is authorized.

PERF3-10 proves the architectural direction but intentionally implements one
consumer. The reusable metadata now distinguishes register components,
derivation context and an explicit signal-phase type; the production consumer
uses storage identity and invalidates an `ITOS` fact when its integer/string
components or numeric context change. Two limitations are now the entry gate
for the next mechanism slice:

1. derivation availability is solved separately for each candidate generator,
   so it should become one worklist-driven multi-fact forward analysis before
   adding many conversions or paying avoidable assembler cost;
2. signal phase is currently proved only as `NONE` for the safe `ITOS` case
   and otherwise fails closed as unknown. Common pre-write, post-write and
   partial-write locations must be classified against actual VM handlers so
   normal, skip, retry and handler continuations receive the right component
   generation.

The proposed fact is `(storage identity, component, value generation,
derivation, context generation)`. A component write creates a new generation;
derived string/number views name the source generation they represent. Direct
link/swap/unlink mapping, joins, loop entries/backedges and typed signal edges
then operate on the same fact instead of accumulating tactical exceptions.

The current equal-work RexxCPS diagnostic leaves 1,120,006 `ITOS`, 1,660,000
`STOD` and 2,220,000 `DTOS` executions. These are opportunity counts, not
authorization to remove them. PERF3-11 first measures assembler processing
time, attributes the two retained hot-loop `ITOS` sites and proves signal
locations for the decimal handlers. Only then should it compare remaining
`ITOS`, decimal/string round trips, compiler loop hoisting and consolidated
swap/swap cleanup as separate consumers. No public format, ABI or production
rewrite is selected by this queue entry.

Stage 1 closes the signal-contract entry gate. Adrian selected S1-S5 and
retirement of `RXOP_SEM_MAY_THROW` on 2026-08-02. Generic effect flags now
describe only call/alias/reference/indirect/opaque behavior and retain their
existing numeric values; the separate aligned signal inventory is authoritative
for capability, phase, source, dependencies and continuations. The selected
total `DCOPY`, non-signalling/plugin-consistent `DTOS`, portable pre-write
checked `INC`/`DEC`, and pre-write invalid literal `SETNUMFUZ` contracts pass a
permanent four-way runtime matrix. Focused correctness is 68/68, both ordinary
Release VMs build, the live decision ledger consumes 650 effect plus 650 signal
rows, and all three Gate 0 benchmark RXBIN hashes are unchanged. Evidence:
[`Stage 1 analysis`](evidence/2026-08-02-perf3-11-stage1-signal-contract/) and
[`Stage 1 lock`](evidence/2026-08-02-perf3-11-stage1-contract-lock/).

Stage 2 closes the immutable graph gate. The new consumer-free sidecar owns
stable queue-record, instruction, code-block and pre-emission address IDs for
one epoch, plus typed normal, branch, signal skip/retry, handler, unwind,
terminal and unknown edges through synthetic roots/exits. Its label index and
edge construction are expected linear in records plus edges. The first cost
check exposed and rejected a duplicate opcode-resolution pass; the locked
orchestration reuses the final legacy `OpInfo` view, frees the legacy graph and
then constructs the sidecar. Focused correctness is 113/113, all three Gate 0
images are exact, and final same-session 30-round assembler medians are
+0.411% Richards, +0.463% Towers and -2.784% RexxCPS with no RSS escalation.
Evidence: [`Stage 2 flow graph`](evidence/2026-08-02-perf3-11-stage2-flow-graph/).

Stage 3 closes the reusable structural-analysis gate. The procedure epoch now
owns a demand-driven cached result containing unique predecessor sets,
multi-root RPO, dominators and sparse frontiers, SCC/backedge classification
and a loop hierarchy that distinguishes signal-retry cycles from source-loop
candidates. Work and retained-memory budgets fail closed; deterministic dumps
make the scale auditable. The first eager integration was rejected after it
crossed the Richards RSS guard by 1,155,072 bytes. The accepted demand-driven
form retains identical analysis results under `-d` and future consumers while
ordinary consumer-free assembly stays guard-clean. Correctness passes 113/113
and all Gate 0 images remain exact.

Stage 4 closes the signal-policy/effect gate. Handler policy is an inherited
procedure parameter with sparse writes and edge-multiset phis; normal, skip,
retry, handler and exit edges select policy versions using the authoritative
failure phase. Calls do not leak callee-local copy-on-write handler changes,
but do advance call/reference/external/plugin/locale identities because VM
argument slots point at caller-owned values. TRACE and numeric-context effects
remain independently versioned. Silent `sigpush` allocation failure prevents
an invented exact `sigpop` restoration. Strict GNU90 checks pass, focused
correctness is 113/113, all Gate 0 images are exact and the ordinary assembler
cost/RSS guard passes. Stage 5 now consumes these edge-state APIs.
Evidence: [`Stage 4 signal policy`](evidence/2026-08-02-perf3-11-stage4-signal-policy/).

Stage 5 closes the symbolic storage/component gate.  The third demand-driven
epoch cache uses sparse persistent mapping/value definitions and lazy phis;
local, argument and global registers are names for `StorageId`s rather than
the identity itself.  Link/swap/unlink and fused failure edges preserve exact
mapping state, caller arguments remain caller-owned, and reference/effect
versions prevent calls or indirect writes from becoming false unchanged-value
proofs.  Null/absent is distinct from unknown, copies retain presence, and
ITOS/FTOS/DTOS plus two-register ITOF name their actual source `ValueId` and
effect dependencies.

The initial recursive dynamic-storage query was rejected at 82.51 s and the
first eager component materialization was rejected at about 305 MB peak RSS.
The locked generation-marked, derivation-site-demanded form completes canonical
RexxCPS diagnostics in 0.28 s at 18.7 MB peak RSS.  Adrian explicitly accepted
a seconds-scale proof-analysis budget rather than requiring the roughly 50 ms
ordinary baseline.  Focused correctness is 113/113, all Gate 0 images are
exact, and ordinary RexxCPS assembly remains 54.526 ms (+0.009%).  Evidence:
[`Stage 5 sparse SSA`](evidence/2026-08-02-perf3-11-stage5-sparse-ssa/).

Stage 6 closes the first proof-authority gate.  The fourth per-epoch cache
provides dominated-success repetition, speculatability, loop must-execute and
component-invariance queries with cached diagnostic reasons.  Value/effect phi
reduction permits safe proofs through joins without source-order assumptions.
The private ITOS availability solver is deleted and the service is sole
authority.  Its 19-`ITOS` RexxCPS image removes two more operations than the
retained 21-`ITOS` old-solver image; Adrian accepted the resulting +7.469% and
+6.866% median CPS verdict on `rxvm`/`rxbvm`.

The first broad run exposed an unsound proof across a caller-owned range-call
argument.  Sparse SSA now gives explicit and range-call actual arguments
unknown component definitions on normal and failure paths, while preserving
unrelated locals.  Final focused correctness is 10/10, broad Debug is
1,987/1,987, and the accepted RexxCPS image hash is unchanged by the fix.
Diagnostic proof analysis completes in 0.39 s at 20.2 MB peak RSS.  Evidence:
[`Stage 6 proof service`](evidence/2026-08-02-perf3-11-stage6-proof-service/).

The migration principle is basic-to-advanced, not like-for-like parity.  The
old solver is a retained minimum safe-capability baseline.  Each remaining
legacy proof must be inventoried and replayed, then replaced one authority at a
time; a stronger new acceptance is valid only with its own positive proof,
adversarial correctness and output-changing Release gate.

The inventory is locked in
[`PERF3-11-MIGRATION-WORKLIST.md`](PERF3-11-MIGRATION-WORKLIST.md).  M01 is
complete: the old repeated-`ITOF` authority is deleted and the generic
one-register `XTOY` consumer selects all 20 conversions from canonical
metadata.  Twelve focused deletions prove: the old `ITOF` floor plus 11
stronger cases, including four `ITOD` flow shapes.  `ITOD` and `BTOD` now have
a coherent total non-signalling plugin/runtime contract; signalling families
and same-component `BTOI`/`ITOB` normalization remain closed.  Richards,
Towers and RexxCPS are byte-identical to Stage 6, focused replay passes 51/51,
broad Debug passes 1,989/1,989, and diagnostic RexxCPS assembly remains within
the seconds-scale budget. Evidence:
[`M01 XTOY migration`](evidence/2026-08-02-perf3-11-m01-xtoy/).

M02 is complete.  The old repeated integer/bitwise-float load availability
solver is deleted and the proof service now requires equal storage, equal
write-once scalar leaves and already-absent reference/native payloads.  The
focused image recovers the old floor and adds equal-phi, exact-float,
linked-storage and ordered-TRACE deletions; different phis, signed zero and
hidden cleanup remain closed.  Canonical images are byte-identical, ordinary
RexxCPS assembly retains a 0.05 s median, and the accepted procedure-local peak
is 30.1 MB.  Focused replay passes 53/53, Release hidden-cleanup execution is
4/4 and broad Debug passes 1,991/1,991.  Evidence:
[`M02 equivalent constants`](evidence/2026-08-03-perf3-11-m02-constant-write/).
M03 is complete in
[`M03 repeated absence`](evidence/2026-08-03-perf3-11-m03-absent-write/).
Its known-storage/all-component proof recovers the old repeated-`NULL` floor
and adds equal-phi, linked-storage and ordered-TRACE cases while preserving
cleanup.  Canonical images are unchanged and broad Debug passes 1,993/1,993.
M04 exact full/typed self-copy is the next one-authority migration.

The PERF3-10 closeout also audited surviving tactical guards. T1 removes the
address-separation concern but does not itself prove that an observed `null`,
copy or forwarded producer has the same component value; their
`flow_has_trace_after()` checks stay until migrated to the new fact. Loads and
one-register XTOY repetition now use the proof service. The local
duplicate-link and swap/call-window rules still transform code while storage
identity currently only analyses their mappings. The adjacent `cnop` rule is
not a trace-anchor workaround. PERF3-11 must replace these one consumer at a
time with structural/runtime equivalence, rather than deleting guards in a
batch.

## PERF3-12 — current RexxCPS clause-lowering rereview

This is a compiler-facing evidence lane, not a benchmark-specific tuning
licence. Start from the accepted PERF3-06 current-product scorecard and fresh
dual-VM profiles. Map each material RexxCPS clause family to compiler RXAS,
library work and reusable flow facts; include variable/representation hoisting,
string conversions, PARSE, stem-tail construction, ADDRESS and the inactive
TRACE path. T1 already supplies the correct anchoring model: a reached trace
event drains prior ordered events, so `CNOP` or executable conversion work is
not retained merely to separate metadata.

The deliverable is a ranked general-shape ledger with dynamic ceiling,
semantic proof owner and guard workload. Compiler lowering, RXAS flow and
runtime fallback remain separate candidates. Any production edit requires its
own selected design and mandatory first ordinary Release verdict.

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

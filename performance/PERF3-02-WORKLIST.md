# PERF3-02 full-copy, ownership and attribute-storage worklist

Status: **panel complete — superseded by selected C1abc R1 composition**

Started: 2026-07-31

Closeout note: the later locked-infrastructure R1 repanel composed the safe
C1a, C1b and C1c proofs. Adrian selected C1abc on 2026-08-01; production status
and closeout evidence are recorded in
[`PERF3-02-R1-WORKLIST.md`](PERF3-02-R1-WORKLIST.md). The original panel and
decision gates below remain its historical record.

Purpose: attribute the remaining high-cost full-value copies on the accepted
current Apple ARM64 product, compare the earliest safe compiler, assembler and
runtime owners, measure exact no-copy and narrow-copy ceilings, and present a
bounded C1-C4 panel to Adrian. This activity does not authorize a production
compiler, assembler, VM, ISA, ABI, language or value-layout change, a commit of
later evidence, or a push.

## Decision gate and mandatory stop

PERF3-02's current authorization ends when Adrian receives:

1. a current-product C0 site/payload/lifetime baseline for Richards and
   Towers, reconciled with the retained CRI-13 byte-weighted projection;
2. isolated C1-C4 proof or ceiling results, including neutral, rejected and
   inapplicable variants;
3. at least two viable production approaches with their exact proof owner,
   semantic obligations, expected scope and maintenance cost;
4. a focused dual-VM correctness and performance guard matrix; and
5. one recommended bounded production candidate, or an evidence-backed
   defer/reject decision.

**Panel stop:** present that package and stop. Do not copy an isolated PoC into
the main checkout, select a new instruction/value representation, or begin a
production implementation without Adrian's explicit candidate selection.

**Later first-Release stop:** if Adrian selects a production candidate, run
only the minimum focused correctness checks required for safe measurement,
freeze implementation, build the ordinary profiling-off Release product, run
the smallest decisive paired target plus guards against retained valid C0
evidence, report the verdict, and stop again. Broad CTest, sanitizer,
install/package, platform work and documentation polish follow only if Adrian
accepts that first verdict.

## Exact starting state

### Repository

- Repository: `/Users/adrian/CLionProjects/CREXX`
- Branch: `develop`
- Start HEAD: `e38e514bf611ae3873513368c44742e2ae7332d1`
  (`perf: accept PERF3-01 Mac evidence`)
- Product-code parent: `3f43a0014be10c930a12b8a636297b60f294c0a6`;
  `e38e514bf` changes only retained PERF3-01 documentation/evidence
- Upstream at start: `origin/develop` at
  `21fdcf529d0e51ea264bf0c92ccfbdc06dea8200`; relation `+2/-0`
- No push is authorized or performed
- Pre-existing dirty scope: five untracked generated lifecycle RXBIN files;
  no tracked change at activity start

| Existing file | SHA-256 at start |
| --- | --- |
| `performance/evidence/2026-07-15-nr-02-portfolio-expansion/lifecycle/crexx/lifecycle_probe.rxbin` | `447350ecd62f33d441b1ffe82600fa50acc23f318724d87e4ec079a207c187ed` |
| `performance/evidence/2026-07-20-nr-10-formal-baseline/lifecycle-decimal/crexx/lifecycle_probe.rxbin` | `19216070c4921764404dc48d2c018ad54c3e67e401cd27cd5084f501e219b2df` |
| `performance/evidence/2026-07-23-nr-16-17-closeout/final-baseline/lifecycle/crexx/lifecycle_probe.rxbin` | `19216070c4921764404dc48d2c018ad54c3e67e401cd27cd5084f501e219b2df` |
| `performance/evidence/2026-07-23-perf2-01-current-baseline/04-lifecycle-rss/lifecycle/crexx/lifecycle_probe.rxbin` | `19216070c4921764404dc48d2c018ad54c3e67e401cd27cd5084f501e219b2df` |
| `performance/evidence/2026-07-27-perf2-06-07-selection-panel/raw/lifecycle/crexx/lifecycle_probe.rxbin` | `19216070c4921764404dc48d2c018ad54c3e67e401cd27cd5084f501e219b2df` |

These files remain protected. PERF3-02 must not delete, normalize, rebuild or
stage them.

### Host and toolchain

- Capture: `2026-07-31T14:12:32Z`
  (`2026-07-31T15:12:32+0100`, BST)
- Host: MacBook Air `Mac17,3`, Apple M5, Apple ARM64
- OS: macOS 26.5.2 build 25F84; Darwin 25.5.0 arm64
- CPU: 10 physical / 10 logical CPUs
- Memory: 25,769,803,776 bytes (24 GiB)
- Power: AC attached; battery 80%; not charging
- Start load average: `2.65 2.26 1.78`
- Apple clang: 21.0.0 (`clang-2100.1.1.101`)
- CMake: 4.3.2; Ninja: 1.13.2

### Accepted C0 authority

PERF3-01's checksum-closed current-product bundle is the timing and
deterministic-count authority:

[`evidence/2026-07-31-perf3-01-current-mac/`](evidence/2026-07-31-perf3-01-current-mac/)

The exact current product is the clean detached build at product commit
`3f43a0014`. Because start HEAD `e38e514bf` is evidence-only, rebuilding its
product is not required merely to change the commit label. Any experimental
build must record its full source diff, cache and executable/library hashes.

Current focused C0 facts to explain, not silently remeasure, are:

| Workload | VM instructions | Calls | generic `COPY_REG_REG` | recursive value copies | logical bytes | dominant object copies |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| Richards | 8,615,245 | 119,157 | 416,260 | 56,902,732 | 451,730,841 | 6,906,745 copies / 448,172,040 bytes |
| Towers | 7,143,456 | 163,950 | 982,781 | 26,789,582 | 206,454,410 | 6,410,681 copies / 205,142,088 bytes |

Both VMs report the same deterministic instruction/value mechanics for these
cells. The generic copies remain profiler-classified as `unclassified`;
PERF3-02 must attribute them to exact static/dynamic sites before choosing an
owner. Retained Linux native evidence says `copy_value` accounts for roughly
55-77% of Richards samples and 46-48% of Towers, while Towers also exposes
clear/reset/attribute-trim work. CRI-13 contributes a separate retained
projection of 6,144 full copies and 359,294,976 logical bytes with the RXAS
stop reason `full-value-ownership-unproved`; it is corroboration, not a current
Mac timing cell.

## Falsifiable hypothesis

A material share of the residual recursive copy bytes comes from
compiler-generated full-object materialization around inlined method receivers
and values whose exact source, mutation and exit behavior are already
recoverable before `copy_value`. In particular, an inlined receiver proved
unused or read-only should not require a private object copy or copyback merely
because the method has multiple explicit returns; source/TRACE identity can be
retargeted to the original direct receiver.

The hypothesis fails for production selection if exact site attribution shows
that eligible sites are immaterial, if direct use changes any observable
identity/value/unwind/metadata behavior, if the ceiling does not move ordinary
Release wall clock, or if a target gain causes a guard regression beyond the
approved budget.

## Scope and invariants

- [x] Preserve cREXX by-value isolation: any value that can be written through
      one binding must not silently alias another binding that language
      semantics require to remain isolated.
- [x] Preserve reference identity and reference-descriptor payload semantics;
      reference values are not ordinary shallow object pointers.
- [x] Preserve recursive attributes, arrays, binary/string/decimal/native
      payloads, capacity/length state and object/class identity.
- [x] Preserve all normal, explicit-return, signal, error and unwind paths,
      including receiver-owned link/unlink and destructor balance.
- [x] Preserve observable intermediate state at calls, native/plugin boundaries,
      dynamic dispatch, callbacks, TRACE/debug events and source projection.
- [x] Preserve canonical RXBIN, public RXAS/ABI and language behavior. A new
      opcode, serialized form, public ABI or value layout is a separate Adrian
      decision and cannot be smuggled into this panel.
- [x] Preserve no-opt and optimized correctness. An optimization may fail
      closed at an unproved site; the ordinary materialized copy path remains.
- [x] Keep copy elimination, attribute-storage trim and teardown independently
      attributable. Do not credit a combined change without isolated rows.
- [x] Keep benchmark sources and work unchanged. Richards, Towers and CRI-13
      expose general mechanisms; they do not authorize benchmark-specific code.
- [x] Report `rxvm` and `rxbvm` separately; use profiling/counts only for
      mechanism attribution and ordinary profiling-off Release for verdicts.

## Candidate panel

### C0 — current product

Retain the exact PERF3-01 ordinary Release timing and schema-5 counts. Add only
the missing dynamic site/payload/lifetime attribution required to explain
generic full copies. RXSEQ or isolated diagnostic instrumentation may be used;
it must not become a new timing baseline.

### C1 — compiler semantic proof

#### C1a: read-only/unused inlined receiver direct use

Extend the compiler's existing inline summaries so a direct receiver that is
proved not written, escaped or rebound may remain the original receiver across
arbitrary control flow and multiple explicit returns. Retarget cloned
receiver TRACE/source metadata to that original register. No private receiver
copy or copyback should remain at an eligible site.

This is the leading narrow compiler candidate because it extends the accepted
V1R01-R1 owner while removing the multiple-return restriction only where
receiver mutation and link-balance obligations are absent.

#### C1b: per-exit placement for mutating multiple-return receivers

Prove direct placement or balanced copyback independently on every normal
return/fallthrough exit for a receiver that is mutated but does not escape.
Reject paths with signals, dynamic aliases, unresolved calls or receiver-owned
link balance that cannot be established. This is broader and higher risk than
C1a; it remains a separate comparison rather than scope silently added to C1a.

#### C1c: direct use of isolated by-value object-formal storage

Prove that an already-materialized, non-reference object formal owns stable
local storage for the whole inline instance, then use that storage directly as
the receiver of a nested method call. Reject formals coalesced through an
inline value alias, generated/captured storage, reference arguments, array
shape, or any path whose ordinary argument binding has not already established
by-value isolation. This route targets Towers' hot `disk.size()` and
`disk.setNext(top)` receiver copies without weakening the separate
multiple-return `§this` exit rule.

C1c is a distinct compiler-storage proof, not an extension silently folded
into C1a or C1b. A mutating nested method remains safe only if its changes are
supposed to land in that formal's private local value and every existing
copyback/cleanup obligation is retargeted to the same proven storage.

#### C1 obligations

- compiler summary must distinguish receiver read, attribute write, rebind,
  address/reference escape, dynamic call exposure and normal/unwind exits;
- every eligible use/definition and metadata event must be retargeted without
  changing register lifetime, class identity or debug visibility;
- aliasing through arguments, attributes, callbacks, native/plugin code or
  references must fail closed;
- recursive/nested inline reconstruction must preserve the same proof; and
- optimized and no-opt outputs must retain the ordinary path where the proof
  is absent.

### C2 — RXAS whole-procedure full-copy projection

Use existing CFG, liveness, effects and typed-copy projection to prove that a
generic `copy destination,source` can be replaced by direct source use or
register projection when all destination reads, writes, cleanup, metadata and
exit paths are known. This can catch compiler-independent exact register-local
copies, but it lacks compiler semantic ownership facts and therefore must fail
closed at reference/native/dynamic/escape or ambiguous TRACE boundaries.

The first C2 PoC may annotate/report eligible sites without rewriting them. A
rewriting PoC must retain a before/after RXAS mapping and prove destination
lifetime/cleanup removal. It must not introduce a public instruction.

### C3 — narrow typed/payload copy

Compare a runtime or compiler-selected operation that copies only a proven
scalar, binary/string or no-payload shape without invoking general recursive
attribute copying. The proof must include exact type/payload shape and exclude
objects or values whose attributes/native/reference payload can become
observable. Scalar, empty and object rows remain separately counted.

C3 is a fallback/companion, not permission to accelerate avoidable copies. It
may be selected only for a material residual after C1/C2 eligibility is
subtracted and must state whether its form requires an ISA/ABI decision.

### C4 — exact direct-operation ceiling

Hand-prove the selected sites in an isolated artifact so eligible values use
the original bound source/register and perform no allocation, recursive copy,
attribute traversal or search on the success path. Preserve only the exact
metadata/lifetime operations that semantics require. This is a machine ceiling,
not a shippable benchmark patch.

For a C3 residual, the paired ceiling is the exact known-shape field/payload
operation with all type/shape decisions preproved; allocation, graph traversal
or runtime ownership search on the success path fails the ceiling gate.

## Design-selection table

| Route | Earliest owner | Plausible production scope | Principal risk | Panel disposition |
| --- | --- | --- | --- | --- |
| C1a read-only receiver direct use | `rxc` inline analysis/binding/rewrite | General proved inline receivers, including multi-return | incomplete escape/metadata proof | C1a-R1 rejected; narrower unused-receiver C1a-R2 authoritative positive and independently viable |
| C1b mutating per-exit placement | `rxc` inline rewrite/control flow | General proved mutating or link-owning multi-return receivers | link/copyback balance on every exit/unwind | material ceiling but proof not bounded; deferred unbuilt |
| C1c isolated object-formal receiver | `rxc` inline binding/storage proof | Nested calls on already-isolated by-value object formals | hidden argument alias or lifetime/copyback mismatch | correct with largest deterministic and timing effect; recommended first candidate for Adrian selection |
| C2 whole-procedure projection | `rxas` flow/liveness/effects | Compiler-independent register-local full copies | semantic ownership and TRACE information may be insufficient | zero eligible with current facts; rejected as standalone owner |
| C3 known-shape copy | compiler plus existing/private runtime form | material scalar/binary/no-payload residual | hides avoidable copies; type/shape contract or ISA expansion | current residual immaterial; deferred/rejected |
| C4 direct ceiling | isolated exact-site control | no production scope | hand proof is not general | met by C1a-R2 and C1c-R1 at their exact sites |
| global shallow copy/COW/value-layout change | VM/value system | broad object graph | identity, mutation, native/reference, lifetime and ABI expansion | rejected from this panel absent new evidence and separate architecture approval |

The two viable independent production approaches are C1a-R2 and C1c-R1. C2's
current fact engine proves no full copy, C3 is immaterial, and C1b needs a
separate higher-risk exit/link-ownership proof. The authoritative clean-host
rerun ranks C1c-R1 first. No candidate is selected or authorized until Adrian
makes that choice.

Follow-on note: Adrian subsequently authorized the separate analysis-only
[`PERF3-02-C1B-WORKLIST.md`](PERF3-02-C1B-WORKLIST.md). That activity found a
bounded `C1b-R1 detached receiver-guard snapshot` PoC shape without changing
this panel's historical C1b disposition or selecting a production candidate.
Evidence:
[`2026-07-31-perf3-02-c1b-analysis`](evidence/2026-07-31-perf3-02-c1b-analysis/).

## Completed panel outcome

The current decision package is
[`evidence/2026-07-31-perf3-02-copy-ownership-panel/decision-summary.md`](evidence/2026-07-31-perf3-02-copy-ownership-panel/decision-summary.md).
It is checksum-closed after the clean-host rerun.

- C0 attributes 416,260 Richards and 982,781 Towers generic copy executions;
  those sites explain 99.18%/99.21% and 99.39%/99.36% of total copy
  operations/bytes respectively.
- C1a-R2 removes 4,910,249 total copy operations and 39,016,032 logical bytes
  from Richards. Its authoritative paired Release result is 9.18%/9.33% lower
  elapsed on `rxvm`/`rxbvm`; both mean intervals exclude zero and all 22/22
  target pairs are favorable.
- C1c-R1 removes 7,140,440 total copy operations, 55,158,560 logical bytes and
  202,314 attribute blocks from Towers. Its authoritative paired Release result
  is 19.42%/19.65% lower elapsed on `rxvm`/`rxbvm`; both mean intervals exclude
  zero and all 22/22 and 12/12 target pairs are favorable.
- Both successful PoCs pass optimized/no-opt dual-VM correctness. Opposite
  workload images are byte-identical guards; their direction remains
  noisy/inconclusive at 36 pairs but every mean-interval upper bound is below
  the +3% workload guard.
- C1a-R1 fails correctness, C2 proves zero current-fact full-copy candidates,
  and C3's residual is only 32,557 scalar operations with zero logical bytes.
- C1c-R1 is the recommended first candidate for Adrian's selection. C1a-R2
  remains independently reproducible; do not combine them before the mandatory
  first Release verdict.

## Machine-level success criteria

- C1/C2/C4 eligible success path: zero `COPY_REG_REG`, zero recursive
  `copy_value`, zero destination allocation and zero attribute traversal for
  the eliminated value; only proved metadata/lifetime work remains.
- C3 eligible success path: one exact known-shape operation with no recursive
  attribute traversal, native/reference dispatch or ownership search.
- Attribution: every removed top-level site maps to static RXAS/source context
  and its dynamic count; recursive operations/bytes are not falsely counted as
  independent compiler sites.
- Benefit: ordinary Release target timing must move in the direction and order
  predicted by the removed instruction/copy bytes. Profile-only movement does
  not select a design.
- Guards: no correctness failure and no accepted guard regression beyond the
  governance budget; noisy/neutral/negative results remain recorded.

## Focused matrix

### Attribution and isolated PoCs

| Workload | Purpose | Modes |
| --- | --- | --- |
| Richards | primary recursive object-copy target; exact inline receiver sites | `rxvm`, `rxbvm` |
| Towers | independent object/allocation/copy target; separates copy, trim and teardown | `rxvm`, `rxbvm` |
| CRI-13 retained projection or bounded external harness | byte-heavy compiler/RXAS ownership corroboration | only the exact available path; no unsupported score claim |
| Permute | accepted inline/value-placement guard | `rxvm`, `rxbvm` |
| Bounce | accepted reference/receiver guard | `rxvm`, `rxbvm` |
| Sieve | zero-work/layout guard when a candidate changes broad compiler/RXAS output | `rxvm`, `rxbvm` |

RexxCPS is not required for a genuinely single-mechanism attribution PoC and
such a panel is explicitly non-representative. It returns in any
multi-workload candidate verdict required by governance or if the selected
change affects broad compiler/RXAS/runtime paths. Base64 is added if C3 touches
string/binary copy mechanics.

### Correctness proof set

- focused compiler inline-analysis/binding/rewrite tests for direct, computed,
  class-attribute, reference and nested receivers;
- zero, one and multiple explicit return, fallthrough, signal and unwind
  cases, with receiver read/write/escape and dynamic/native call variants;
- assembler CFG/liveness/effects tests for exact eligible and fail-closed
  generic copies, TRACE/source metadata and cleanup lifetime;
- existing copy/value/reference/native-payload fixtures appropriate to the
  selected owner;
- optimized and no-opt benchmark correctness for every measured target/guard;
  and
- both VM implementations for any generated RXBIN used in the panel.

## Numbered execution plan

1. Freeze the exact start, accepted C0 authority, protected dirty scope,
   hypothesis, candidate routes, invariants, focused matrix and stop gates.
2. Replay the PERF3-01 recursive checksum authority and create an isolated
   detached worktree/build root for every diagnostic or PoC source change.
3. Attribute generic Richards and Towers copies to static RXAS/source sites,
   dynamic execution counts, top-level payload shapes, recursive operations/
   bytes and lifetime/TRACE context. Reconcile but do not merge CRI-13 evidence.
4. Quantify C1a, C1b, C2 and C3 eligibility before implementing a rewrite.
   Record explicit failure reasons for ineligible sites.
5. Build the exact C4 no-copy/direct-operation ceiling and any required C3
   known-shape ceiling in isolated artifacts. Validate outputs in both VMs.
6. Implement only the minimum isolated C1/C2/C3 PoCs needed to distinguish
   owners. Keep each diff and product hash separate; do not form an automatic
   optimization ladder.
7. Compare deterministic work removal first, then the smallest serial ordinary
   Release timing cells. Use retained valid C0 or a same-session drift control
   only when required; do not promote diagnostic/profile elapsed time.
8. Produce a compact checksum-closed evidence bundle, candidate matrix,
   rejected-route record and recommendation. Update this worklist/roadmap and
   stop for Adrian's production-candidate selection.

## Resumable stage ledger

### Stage A — control plane and exact start

- [x] PERF3-01 evidence/ranking accepted and locally committed as `e38e514bf`.
- [x] Root/performance instructions and live roadmap reread.
- [x] Branch/upstream, exact HEAD, host/toolchain and protected dirty scope
      verified.
- [x] Hypothesis, C0-C4 panel, semantic obligations, focused matrix and both
      mandatory stop points recorded.
- [x] Roadmap status changed to `in progress` and directory map linked.

### Stage B — C0 integrity and site attribution

- [x] Replay all PERF3-01 checksum rows before deriving new evidence.
- [x] Record exact product/build/image/library hashes used by diagnostics.
- [x] Capture/rank Richards generic copy static sites and dynamic counts.
- [x] Capture/rank Towers generic copy static sites and dynamic counts.
- [x] Attribute top-level site payload shape, recursive operations/bytes,
      lifetime, metadata and cleanup context without double-counting recursion.
- [x] Reconcile CRI-13's RXAS stop sites as a distinct retained projection.
- [x] Publish an owner/eligibility ledger with C1a/C1b/C2/C3/C4 columns.

### Stage C — C1 compiler proof/PoC

- [x] Freeze existing inline summary/binding/rewrite behavior and test coverage.
- [x] Quantify read-only/unused receiver copies eligible for C1a.
- [x] Quantify mutating/link-owning multi-return receiver sites for C1b; the
      additional ceiling is material but the proof is not bounded.
- [x] Quantify isolated object-formal receiver sites plausibly eligible for C1c.
- [x] Build isolated C1a proof/PoC and run the focused correctness subset.
- [x] Do not build C1b: the bounded-proof condition failed after C1a-R1 exposed
      receiver-owned link cleanup across exits.
- [x] Build C1c because the existing formal-binding proof keeps a private local
      value and the retained alias/generated-storage guards remain fail-closed.
- [x] Retain exact removed sites, operations/bytes, product hashes and failures.

### Stage D — C2 assembler proof/PoC

- [x] Freeze current generic-copy stop reason and typed-copy flow behavior.
- [x] Add and run an isolated eligibility-only analysis before any rewrite.
- [x] Quantify sites proved by C2 but not C1, and sites rejected for missing
      semantic ownership/TRACE/lifetime information.
- [x] Do not build a rewrite: eligibility is zero with current facts.
- [x] Run focused assembler/VM correctness and retain exact RXAS/RXBIN identity.

### Stage E — C3 residual and C4 ceilings

- [x] Subtract C1/C2 eligibility from C0 before sizing C3.
- [x] Partition residual top-level and recursive work into object, scalar,
      binary/string, reference, native and empty shapes.
- [x] Build exact C4 direct-operation/no-copy ceilings through C1a-R2/C1c-R1.
- [x] Do not build a C3 ceiling: the scalar residual is immaterial.
- [x] Prove success-path machine operations meet the declared ceiling.

### Stage F — isolated comparison

- [x] Validate exact output/correctness in Richards and Towers for both VMs.
- [x] Run Permute/Bounce/Sieve guards; Base64/RexxCPS are not applicable to
      these isolated compiler mechanisms.
- [x] Compare deterministic operations/bytes and separately attributable trim/
      teardown effects for C0-C4.
- [x] Run the first profiling-off Release timing cells serially; retain them as
      provisional after the remote-terminal confound was disclosed.
- [x] Record artifact effects; startup/load, RSS and late-load/plugin capture is
      not required for these compiler-only isolated image changes.
- [x] Keep every neutral, negative, noisy, invalidated and rejected row.

### Stage G — panel and stop

- [x] Produce candidate/owner, semantic-risk, maintenance, machine-ceiling and
      Release-effect tables.
- [ ] Re-run the same-session C0/C1a-R2/C1c-R1 speed matrix after Adrian
      confirms the remote terminal is off and the host is stable. Preserve
      every rejected/invalid option and do not time correctness-invalid C1a-R1.
- [ ] Confirm or revise the provisional C1c-R1 recommendation from that replay.
- [ ] Recursively checksum the compact final evidence bundle.
- [ ] Update roadmap/worklist, run `git diff --check`, verify protected RXBIN
      hashes and review the complete main-checkout diff.
- [ ] Present the panel to Adrian and stop before any production edit.

## Expected evidence root

Use:

`performance/evidence/2026-07-31-perf3-02-copy-ownership-panel/`

Keep it compact: provenance and PoC diffs/hashes, site/eligibility ledgers,
consolidated correctness/count/timing rows, candidate and decision summaries,
and one recursive checksum file. Reference PERF3-01/PERF2/CRI-13 evidence
rather than copying their raw artifacts.

## Resumption rule

Reread root/performance instructions and the live roadmap, verify HEAD,
upstream and dirty scope, replay the current evidence checksum, then resume at
the first unchecked item. Use a new isolated detached worktree/build root for
each source-changing PoC. Before timing, recheck AC/low-power/thermal/load state
and ensure no build, CTest or benchmark process overlaps. Record any divergence
from the frozen state before continuing.

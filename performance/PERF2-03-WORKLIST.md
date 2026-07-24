# PERF2-03 flow-aware inlining 2.0 worklist

Status: production in progress; H and slices 1-4 approved, pause before slice 5

Started: 2026-07-24

Purpose: resumable control plane for the inline-site census, semantic-fact
inventory, architecture comparison and bounded P0-P4 prototype panel required
before any production flow-aware inlining change. This worklist does not
authorize a production compiler edit, public RXAS/RXBIN/ABI change, language
change, staging, commit or push.

## Decision gate and mandatory stops

PERF2-03's first deliverable is a decision package. It must recommend exactly
one architecture and at most one smallest production slice, supported by an
exact current-site census, focused correctness, instruction/register/image
evidence and bounded profiling-off Release timing where decisive.

**Stop point 1:** after the architecture record and isolated P0-P4 evidence are
complete, report the recommendation to Adrian and stop. Do not install any PoC
in the production worktree.

**Stop point 2:** if Adrian later approves a production slice, run only the
minimum focused correctness checks needed for safe measurement, freeze the
implementation, build the ordinary profiling-off Release product, run the
smallest decisive retained-baseline comparison, report the first Release
verdict and stop again before broad closeout.

## Hard boundaries

- [x] Preserve writable by-value isolation, read-only by-value semantics,
      `.ref`, references, objects/arrays and repeated/overlapping actuals.
- [x] Preserve optional/default/omitted/status behavior and evaluation order.
- [x] Preserve cleanup, allocation, lifetime, signals/unwind, numeric context,
      TRACE/source identity and both VM paths.
- [x] Preserve public RXAS, canonical RXBIN 007, ABI and VM semantics; emit no
      compiler-only optimizer hint.
- [x] Reject only the affected site/fact when proof is unavailable; do not add
      blanket whole-procedure exclusions for calls, references, TRACE or
      handwritten RXAS.
- [x] Keep maintained performance orchestration in cREXX Level B. Native host
      tools may be diagnostic inputs only where already allowed.
- [x] Keep compiler PoCs in clean detached scratch worktrees/builds. Main-tree
      edits are limited to this worklist, the live roadmap and retained
      evidence/architecture records.
- [x] Use focused builds/tests during the prototype campaign. Any verbose
      compiler/debug output goes to a `mktemp` log and is read back narrowly.
- [x] Preserve the dated charter, accepted PERF2-01/02 evidence and closed
      predecessor worklists unchanged.

## Numbered execution plan

1. Freeze repository, upstream, host, power, toolchain and predecessor evidence.
2. Complete mandatory compiler/inliner/NR-26/fixture reading and record the
   exact missing semantic facts before the first compiler PoC edit.
3. Build P0's deterministic current-site census and ranked cost panel, covering
   accepted, rejected and cleanup-required sites without changing eligibility.
4. Compare pre-inline summary, clone-first post-inline flow, bounded cleanup
   fixed point and measured hybrid designs against explicit proof obligations.
5. Build P1-P4 as independent exact-HEAD patches in isolated worktrees: Q1
   replay, formal/default cleanup, result placement, and cleanup/profitability
   fallback.
6. Run the focused correctness matrix on both VMs and gather RXAS instruction,
   copy/branch/temp/register, RXAS/RXBIN/linked-image and bounded timing data.
7. Retain patch/checksum/build provenance, consolidate one evidence bundle,
   choose exactly one architecture and at most one production slice.
8. Set status to `decision required`, give Adrian a paste-ready decision report
   and stop without staging, committing or pushing.

## Stage 0 - exact baseline and isolation

### Repository state at start

- Repository: `/Users/adrian/CLionProjects/CREXX`
- Branch: `develop`
- HEAD: `086138f1e93da8e84d45f4cd3ba9b6620f792a14`
- Upstream: `origin/develop` at the same commit; ahead/behind `+0/-0`
- Commit subject: `perf: complete PERF2 attribution and reference fast path`
- Starting worktree: clean; empty tracked-diff SHA-256
  `e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855`
- Handover drift resolved: its earlier upstream/ahead orientation is stale;
  this work is bound to the live exact state above.

### Host and tool state at start

- Host: `Mac.lan`, model `Mac17,3`, arm64, 10 logical CPUs, 24 GiB RAM.
- OS: Darwin `25.5.0`; kernel build
  `Darwin Kernel Version 25.5.0: Tue Jun 9 22:28:17 PDT 2026`.
- Toolchain: CMake `4.3.2`, Ninja `1.13.2`.
- Power: AC attached, battery 80%, AC/Battery low-power mode `0`.
- Capture load: `1.65 3.10 5.07`; uptime 10 days 6:47.
- Storage: approximately 636 GiB free on the data volume and `/private/tmp`.

Formal timing blocks must recapture power, low-power, thermal and load before
and after, run serially and use ordinary profiling-off Release binaries.

### Prototype isolation contract

Each prototype starts from a clean detached worktree at the exact HEAD above.
It gets a distinct source tree, focused build tree, patch, patch SHA-256,
compiler/binary hashes, command log and result ledger. A prototype may combine
previous mechanisms only when the row is explicitly labelled as a combination;
otherwise patch identities remain independent.

| Prototype | Mechanism | Required comparison | Status |
| --- | --- | --- | --- |
| P0 | exact current compiler, diagnostics/census only | no-opt, current inline and accepted hand-equivalent controls | complete |
| P1 | exact PERF2-02 Q1 direct-receiver replay | P0 and retained Q1 evidence | complete; favorable |
| P2 | read-only scalar-formal fact and storage cleanup | independent P0 | complete; fact useful, unconditional rewrite rejected |
| P3 | inline-created block-result destination placement | independent P0 | complete; mechanism useful, unconditional rewrite rejected |
| P4 | P1+P2+P3 with detached 100-node fallback probe | P0 and combined candidate | complete; fallback works, size-only gate rejected |

- [x] Exact branch, HEAD/upstream and clean state captured.
- [x] Host, power, load, storage and tool versions captured.
- [x] Scratch isolation and independent patch contract defined.

## Stage 1 - mandatory reading and exact current gap

### Sources read before the first compiler PoC

- [x] Root and `performance/` `AGENTS.md` instructions.
- [x] Live `performance/ROADMAP.md` and performance governance.
- [x] Compiler architecture and debugging guidance.
- [x] Relevant dated-charter sections without modifying the charter.
- [x] NR-12/21 and NR-26 worklists and retained instruction evidence.
- [x] PERF2-02 worklist, architecture record and retained Q1/Q3b evidence.
- [x] Current `rxcp_opt.c`, full `rxcp_inline*`, `rxcp_flow.*` and
      `rxcpmain.c` implementation.
- [x] Complete inline source fixture set, reference-inline lifetime fixture,
      NR-26 flow fixture/contract and their CTest registrations.

### Current pipeline and missing semantic facts

Current optimized compilation runs destructive multi-pass inlining inside
`optimise()`, then constant folding/constant-symbol propagation, then SELECT
lowering, then one NR-26 build/apply before emission. No-opt skips inlining but
builds the same flow overlay without applying transformations.

The exact cross-stage gaps are:

1. `InlineCloneState` maps cloned nodes/scopes/symbols and ref/vararg/receiver
   mechanics, but carries no callee formal effect summary, actual-formal value
   equivalence, inline-site identity, result equivalence, cleanup ownership or
   pre/post-cleanup cost snapshot.
2. Non-reference formals are eagerly materialized as assignments, including
   optional/default paths, even where later proof could share or eliminate the
   storage. The existing `is_const_arg` fact concerns procedure entry emission;
   it is not an inline binding proof and is not exported in inline metadata.
3. Expression and multi-return expansion introduces `BLOCK_EXPR`, `LEAVE_WITH`,
   block-result and temporary scaffolding. The flow builder currently treats
   `BLOCK_EXPR`, SELECT/SWITCH and OPT_DISPATCH as an opaque single block and
   therefore cannot prove result identities or dead inline exits within them.
4. NR-26's CFG, values, definitions, reaching definitions and liveness are
   private to `rxcp_flow.c`; there is no query/summary interface the inliner can
   consume before cloning and no inline provenance that post-inline flow can
   use after cloning.
5. NR-26 models source/compiler symbols and selected expression temporaries,
   but not distinct inline formal, receiver, block-result, return-result and
   cleanup-owner identities. Existing copy propagation is limited to small
   scalar facts and cannot express aggregate/reference ownership equivalence.
6. Inlining has a structural 300-node ceiling but no site profitability model.
   Candidate blocks are built detached and installed only at the final replace,
   yet no untouched-call fallback is retained after installation and no cleaned
   candidate cost is computed before that irreversible step.
7. The optimizer loops only folding/constant-symbol propagation after all
   inline passes. NR-26 has an internal bounded marker fixed point, but the CFG
   is not rebuilt after general AST cleanup and there is no bounded
   inline-cleanup-profitability convergence contract.
8. Imported inline payloads serialize AST/scope/symbol/source dependencies, but
   carry no versioned semantic summary or cost summary. Cross-file decisions
   therefore cannot consume the same proof surface as local callees.

These are representation/proof gaps, not evidence for whole-procedure bans.

## Stage 2 - design-selection table

The final disposition remains evidence-driven. The initial hypothesis is that
the earliest reusable fact is a versioned callable semantic summary, but safe
installation and profitability also require an explicit per-site expansion
plan and a post-clone CFG capable of crossing inline boundaries.

| Design | Earliest facts | Cleanup power | Rollback/profitability | Imported parity | Main risk | Initial disposition |
| --- | --- | --- | --- | --- | --- | --- |
| A. Pre-inline callee summary plus current post-inline flow | formal effects, escape, returns and costs before clone | weak until NR-26 models inline/block identities | can reject before clone; post-cleanup estimate is indirect | natural with versioned payload | summary says what is true but not what became dead at this site | prototype component |
| B. Clone first, extend NR-26 across boundaries | exact site-expanded tree | strongest local/path cleanup | requires untouched original or speculative candidate ownership | imported bodies already clone, but lack summary | destructive install, opaque block expressions and source/cleanup identity | prototype component |
| C. Bounded cleanup/profitability fixed point | facts discovered after each accepted rewrite | can expose constants/dead paths monotonically | natural final gate if candidate remains detached | neutral | invalidation/rebuild discipline and convergence | prototype component |
| H. A+C with explicit expansion plan and boundary-aware post-clone CFG | summary before clone, site bindings during clone, full facts after clone | combines early safe placement with path-correct cleanup | detached candidate is installed only after final cleaned cost wins | versioned summary plus existing body payload | more interfaces; must keep summary, candidate and CFG ownership exact | leading hypothesis; not selected until P0-P4 |

### Selection criteria

The selected architecture must:

- express the full initial transformation panel without parsing emitted RXAS;
- make by-value, alias, ownership, cleanup, signal, TRACE/source and numeric
  proof obligations explicit and independently invalidatable;
- give local and imported callables the same versioned facts;
- retain or regenerate a non-inline fallback until the final cleaned candidate
  wins a measured instruction/register/image cost;
- converge monotonically under a documented bound; and
- add no public RXAS/RXBIN/ABI/VM contract.

## Stage 3 - P0 census and cost model

Complete. The retained `P0-CENSUS.md` records no-opt/current static product
shape, both-VM dynamic instruction counts, dynamic call removal and ranked
site dispositions. The decisive findings are:

- Richards removes 350,481 calls but adds 1,105 static instructions, 38 peak
  locals, 221 copies and 119 branches for only 182,965 fewer dynamic
  instructions: cleanup is required.
- Permute's non-recursive `swap` is a clear inline win; recursive `permute`
  remains a call.
- RexxCPS benefits dynamically from selected BIF inlining but retains numeric
  context rejections and large static scaffold: cleanup is required.
- List reference/object and JSON imported-body gaps require per-site facts, not
  blanket exclusions.

- [x] Deterministic current product and linked-library census retained.
- [x] No-opt/current dynamic procedure and instruction comparison retained.
- [x] Hot accepted, rejected and cleanup-required sites classified.
- [x] Every classification preserves site-local proof/fallback behavior.

## Stage 4 - isolated P1-P4 prototype panel

Complete. Exact patches, compiler/library hashes and isolation provenance are
retained in the evidence bundle.

| Prototype | Main positive result | Main negative/control result | Disposition |
| --- | --- | --- | --- |
| P1 | Richards -11 instructions/copies, peak locals unchanged; 33/33 exact-head focused checks | narrow receiver fact only | sole proposed first production mechanism, through the selected transaction scaffold |
| P2 | Richards -21 instructions/copies | Permute peak locals +2; exact-head Richards +0.052% `rxvm`, -1.152% `rxbvm` | fact retained in architecture; do not ship unconditional rewrite |
| P3 | Richards -11 instructions, -3 copies | RexxCPS +1 instruction | post-clone mechanism only behind final cost/ownership proof |
| P4 | detached fallback works; Richards 1,897 -> 1,385 instructions and locals 66 -> 59 | RexxCPS calls 27 -> 84 and locals 105 -> 107 | positive fallback probe; reject 100-node-only gate |

- [x] Exact patch/checksum/product provenance retained.
- [x] Independent P1-P3 and explicitly combined P4 identities preserved.
- [x] Both-VM runtime, instruction/copy/branch/local/image evidence retained.
- [x] Losing cases establish the need for the final fallback gate.

## Stage 5 - correctness matrix

- [x] Scalars, strings, binary, arrays and objects.
- [x] Read-only/writable by-value; direct/computed/repeated/overlapping actuals.
- [x] `.ref`, reference lifetime, snapshots, aliasing and cleanup ownership.
- [x] Optional/default/omitted/status and evaluation order.
- [x] Direct/computed method receivers and receiver copyback.
- [x] Local/imported/nested callees, recursion rejection and residual calls.
- [x] Single/multiple/fallthrough returns; assignment/expression/call arguments.
- [x] Live siblings, branches, joins, backedges and zero-trip loops.
- [x] Signals/unwind, inherited numeric context and TRACE/source identity.
- [x] Optimized/no-opt behavior and both `rxvm`/`rxbvm`.

P2, P3 and P4 each passed all 118 inline runtime tests. Their 22, 10 and 26
optimized-compiler golden differences respectively are expected emitted-shape
changes, not runtime failures. P4 additionally passed the five-workload
canonical portfolio on both VMs (10/10).

## Stage 6 - decision package

Complete and awaiting Adrian's decision. The selected architecture is H:
versioned pre-inline callable summary + explicit per-site expansion plan +
boundary-aware candidate-local flow + bounded cleanup/profitability fixed
point. The sole proposed first production slice is the detached
`InlineExpansionPlan` transaction/fallback scaffold used only for P1's already
proved direct-receiver equivalence.

Final pre-decision checklist:

- [x] Evidence bundle verifies independently from its checksum manifest.
- [x] No production compiler/VM/ABI/RXAS/RXBIN change is present in main tree.
- [x] Roadmap and worklist say `decision required`.
- [x] No files are staged; no commit or push was made.
- [x] Paste-ready Adrian report names the next stop explicitly.

Stop point 1 was released by Adrian's 2026-07-24 approval of H and production
slices 1-4. Each slice requires focused correctness, its ordinary profiling-off
Release verdict, requested QA and an independent commit. Stop before slice 5.

## Approved production ladder

| Slice | Scope | Status |
| --- | --- | --- |
| 1 | `InlineExpansionPlan` detached transaction plus direct receiver equivalence | complete; favorable verdict; 1,907/1,907 QA; `d51bdf30d` |
| 2 | conservative multi-metric candidate profitability/fallback gate | complete; byte-identical parity verdict; 1,907/1,907 QA; `6687d64d5` |
| 3 | gated boundary-aware local scalar/formal/result/control cleanup | complete; favorable verdict; 1,907/1,907 QA; commit pending in this slice |
| 4 | versioned local/imported callable summaries and binding parity | approved; pending |
| 5 | reference/object alias, lifetime and cleanup ownership | not authorized; mandatory pause |

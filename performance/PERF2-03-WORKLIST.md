# PERF2-03 flow-aware inlining 2.0 worklist

Status: complete and closed; all five approved production slices accepted,
committed and QA green

Started: 2026-07-24
Closed: 2026-07-25

Purpose: resumable control plane for the inline-site census, semantic-fact
inventory, architecture comparison and bounded P0-P4 prototype panel required
before any production flow-aware inlining change. It began as a decision-only
control plane; the production ladder was added and executed only under Adrian's
later slice-by-slice approvals recorded below. No approval changed the public
RXAS/RXBIN/ABI or language boundary.

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

The exact cross-stage gaps at the production starting point were:

1. `InlineCloneState` maps cloned nodes/scopes/symbols and ref/vararg/receiver
   mechanics, but carries no callee formal effect summary, actual-formal value
   equivalence, inline-site identity, result equivalence, cleanup ownership or
   pre/post-cleanup cost snapshot.
2. Non-reference formals were eagerly materialized as assignments, including
   optional/default paths, even where later proof could share or eliminate the
   storage. The original audit understated the transport: `is_const_arg` was
   present in the generic AST-node flags, but it was an emitter hint rather
   than an explicit, versioned inline binding proof. Slice 4 replaces that
   reliance with body-derived formal read/write/escape evidence.
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
slices 1-4. Adrian authorized slice 5 on 2026-07-24 after confirming the merged
tree rebuild and CTest were green and that the machine was on stable AC power.
Slice 5 first proved its ordinary profiling-off Release verdict and stopped.
Adrian accepted the decisive result on 2026-07-24 and authorized broad QA and
the independent closeout commit.

## Approved production ladder

| Slice | Scope | Status |
| --- | --- | --- |
| 1 | `InlineExpansionPlan` detached transaction plus direct receiver equivalence | complete; favorable verdict; 1,907/1,907 QA; `d51bdf30d` |
| 2 | conservative multi-metric candidate profitability/fallback gate | complete; byte-identical parity verdict; 1,907/1,907 QA; `6687d64d5` |
| 3 | gated boundary-aware local scalar/formal/result/control cleanup | complete; favorable verdict; 1,907/1,907 QA; `6b97c2ffd` |
| 4 | versioned local/imported callable summaries and binding parity | complete; runtime-neutral proof/metadata verdict; 1,910/1,910 QA; `26f4aeb6f` |
| 5 | reference/object alias, lifetime and cleanup ownership | complete; decisive List verdict; 1,915/1,915 QA; `d1c5245d4` |

Slice 4 retains the slice-3 Richards image exactly at 1,867 instructions, 62
peak locals, 79,094 bytes and SHA-256 `6aad1ca91ddb53089fe0b5040f47e1267cabf6bf13c1cc8527b8940d77b50f9a`.
Its I6 metadata adds 3,672 bytes (0.429%) to the shared library without adding
an executable instruction. The final same-session timing is neutral: no
slice-4 runtime improvement is claimed, and the accepted slice-3 Richards gain
remains the cumulative result. A non-reproducible incremental artifact and its
approximately 0.13%/0.146% timing were rejected rather than added to that gain.
Retained evidence is in `production/slice-4.md`.

### Evidence-open rule and fail-closed review ledger

A fail-closed gate is a temporary proof boundary, not a reason to forget an
optimization. Once the compiler has a complete mathematical proof for a
specific case, the production rule is to open that case narrowly, add a CTest
that distinguishes sufficient from insufficient evidence, and keep unknown
sites on the ordinary call path. Every deferred gate must remain in this ledger
until it is opened or explicitly rejected on semantic/profitability grounds.

| Gate | Current evidence | Disposition |
| --- | --- | --- |
| Imported read-only scalar formal binding without the legacy AST `is_const_arg` hint | I6 summary is derived from symbol reads/writes, exact scalar shape and escape facts; reader reconstructs it from the body | opened in slice 4; CTest proves I6 removes the call/formal copy while matching source register identity |
| Missing or I4/I5 callable summary | no current proof schema | remain fail closed; CTest requires the ordinary call and correct runtime output |
| I6 result shape that disagrees with the independently parsed callable declaration | contradictory evidence | remain fail closed; review-derived CTest requires the ordinary call and correct runtime output |
| I6 body-derived facts or costs that disagree with the payload summary | contradictory evidence | remain fail closed through exact summary comparison; review-derived CTest for the newly trusted formal effects forges a writable reference as read-only and requires the ordinary call/runtime result; add another focused CTest whenever a new field begins opening a transformation |
| Imported payload offered to an implicit `main` or generic class-factory procedure | declaration and body describe different callable semantics | fixed in slice 4: select the explicit procedure and defer factories to their real contract node; `address_inline_then_parse` guards the debug/attachment path |
| Reference/object alias, lifetime and cleanup ownership | slice 5 proves direct local receiver placement and the exact reference-attribute accessor family; broader ownership/last-use cases still lack proof | opened only for the proved slice-5 cases; all other reference/object cases remain fail closed |
| Dynamic vararg indexing, assembler alias effects and generated association transport | current summary cannot prove locator/liveness, per-operand alias effects or association reconstruction | remain fail closed and listed for later evidence-specific slices; do not treat as permanent blanket exclusions |

### Slice 5 proof and acceptance contract

The first slice-5 mechanism is efficient direct placement of ordinary method
receivers, including basic getters and setters. It is not a blanket object
exemption. A receiver binding may share the caller's object register only when
all of these facts are proved at the individual call site:

- the callable's versioned summary is present and exactly reconstructed from
  the validated body;
- the receiver is a direct object variable at the statically resolved method
  call site, with no computed evaluation, exposed/formal/generated storage,
  enclosing `§this`, prior inline alias or flow-substituted identity;
- the producer's validated declaration and body reconstruction prove that the
  callee instance is the method receiver; and
- any actual expression that reads receiver-owned state is captured before the
  body, while overlapping object/reference actuals retain isolation.

When one of those facts is missing, only receiver placement remains closed;
the existing materialized receiver/copyback inline path remains available.
Once a further case gains a complete mathematical proof, open it narrowly and
add a distinguishing CTest rather than leaving the conservative fence in
place. A real method call binds the receiver value pointer directly as `a1`, so
sharing a proved direct local uses the same storage even when a pre-existing
weak reference targets that object; the reference-alias regression proves that
the alias observes the post-call mutation. Positive local, source-import and
binary-import getter/setter tests must prove same-register receiver lowering
and correct output. Negative tests cover computed receiver expressions, nested
enclosing `§this`, unproved imported method identity and contradictory imported
summary facts.

Because receiver placement can affect the whole class-library surface, the
mandatory first Release verdict must retain broad five-workload artifact and
output guards even if the smallest decisive timing cell is narrower. Peak
locals, executable instructions, general/typed copies, RXAS/RXBIN/linked-image
bytes and both VM outputs are mandatory regression dimensions. After that
verdict, stop for Adrian before broad CTest, sanitizer, documentation closeout
or commit.

#### Hot reference-accessor extension

The first provisional verdict showed that direct receiver placement alone did
not reach the highest-frequency basic accessor: `ListElement.next()` remains a
3,820,600-call reference-attribute getter. Adrian therefore directed slice 5
to continue within its approved reference/object scope rather than accepting
the narrow receiver-only result.

| Approach | Proof and cost | Disposition |
| --- | --- | --- |
| Retain the whole-callable reference ban | Safe fallback, but ignores the validated body and leaves the hot basic getter/setter family unavailable | rejected as over-broad |
| Admit only exact reference-attribute accessors to the existing detached clone/bind/cleanup transaction | Reuses I6 body reconstruction, normal reference-formal capture, receiver evaluation/copyback, source/TRACE identity and the final profitability gate | selected |
| Replace calls with a new direct attribute-access rewrite | Could reduce clone scaffolding further, but would duplicate attribute lowering and require a new transported attribute-identity contract before the existing transaction has been measured | defer unless the selected form remains unprofitable |

An exact reference getter is a method with no formals and one final `RETURN`
whose expression is one receiver-owned reference attribute. An exact reference
setter is a void method with one required by-value reference formal, followed
only by assignment of that formal to one receiver-owned reference attribute
and a final bare `RETURN`. The target and formal reference shapes must match
exactly. Calls, `reference self`, dereference/snapshot/refvalid operations,
optional/by-reference/vararg formals, computed expressions and any extra read,
write or control effect remain closed.

This is mathematically narrower than general reference inlining. A reference
value is a weak alias descriptor: copying it neither changes its target nor
extends target lifetime. The exact getter copies that descriptor from the
attribute to the existing return path. The exact setter uses the established
reference-formal binding copy before the body and then stores that captured
descriptor through the established receiver/copyback path. The normal emitter
continues to own attribute link/copy/unlink operations, and the detached
candidate still has to win the existing profitability gate.

#### Slice 5 accepted result

The exact accessor extension removes all five static `ListElement.next()` call
sites and the one `setNext()` site. The measured 3,820,600 dynamic `next()`
calls disappear. List grows from 233 to 239 static instructions and from
16,186 to 16,650 RXBIN bytes while peak locals remain 34; general copies grow
7 to 14, typed copies remain 2 and call opcodes fall 21 to 15.

The ordinary profiling-off Release comparison at work=100 reduces median List
elapsed from 182.367 ms to 86.044 ms on `rxvm` and from 201.974 ms to 94.499 ms
on `rxbvm`: 52.818% and 53.212% respectively. Every recorded pair is
favorable. Permute, Richards, JSON, RexxCPS and the linked library remain
byte-identical to the immediate pre-slice-5 baseline, so the earlier Richards
and other gains are retained rather than replaced.

Closeout includes the complete Debug build, review and refresh of four
optimized goldens whose only changes are proved receiver-copy removal/register
renumbering, and final full Debug CTest 1,915/1,915. Local, source-import and
binary-import reference accessors run correctly on both VMs; unproved and
side-effecting cases remain calls. Retained evidence is in
`production/slice-5.md` and its adjacent `slice-5-*` files.

### PERF2-03 closure and future proof points

PERF2-03 is closed at production commit `d1c5245d4`. Its exit is satisfied by
the accepted Architecture H fallback/profitability transaction, the five
independently committed slices, the decisive profiling-off Release List result
and final Debug QA 1,915/1,915. The immediate baseline guards prove that the
earlier Richards gain and the other protected products were retained.

The entries below preserve worthwhile future questions without treating them
as unfinished PERF2-03 scope. At every site, missing proof rejects only the
candidate transformation; the ordinary call/materialized path remains valid.

| ID | Future point | Reopen gate | Successor route |
| --- | --- | --- | --- |
| PERF2-03-F01 | Remove residual link/copy/unlink scaffold or use direct attribute lowering for exact reference accessors | Current profile shows material cost after the accepted List win; exact semantic ceiling beats the existing inline transaction | PERF2-07 reference/value ownership; candidate-specific compiler work only if selected |
| PERF2-03-F02 | Open broader reference/object alias, lifetime, escape and last-use cases | Complete per-site alias/lifetime/cleanup/unwind proof plus distinguishing CTests | PERF2-07 or a later compiler-analysis activity |
| PERF2-03-F03 | Remove remaining formal, result, block-exit or compiler-temporary overhead | A current hot helper remains materially above its hand-equivalent instruction/register/image ceiling | Carry as a bounded companion in PERF2-04 or the selecting future activity |
| PERF2-03-F04 | Transport dynamic vararg, generated association and assembler alias/effect facts | Exact locator/liveness/effect reconstruction and multi-site evidence | Later compiler-analysis work and, where semantic instructions are selected, PERF2-05 |
| PERF2-03-F05 | Extend I6 with another trusted fact | Independent body/declaration reconstruction, exact summary comparison and a contradictory-evidence regression CTest | The activity consuming that new fact owns the proof and test |

The paste-ready successor brief is `performance/PERF2-04-HANDOVER-PROMPT.md`.
PERF2-04 begins with a current BIF census because the retained PERF2-01 profile
predates the accepted inlining work; old rankings are orientation, not a frozen
selection.

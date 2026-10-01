# Optimization maintainability, fusion ownership and quickening

Date: 2026-09-18. Status: source review and proposed next design; no RXAS
framework refactor, new opcode, VM fusion migration or adaptive quickener is
approved/implemented. The quickening review below incorporates Adrian's
follow-up about the original objective; investigation does not select a new
production performance programme.
The existing imported-inline and status-bit boundaries are accepted; see the
[audit decision record](optimization-boundary-audit-2026-09-18.md).

Subsequent planning on 18 September assigns Beta 6 (2027-03-31) as the
Performance Beta after January platform functional completion. The
[performance roadmap](../../../performance/ROADMAP.md) now carries adaptive
procedure-result memoization (VM-MEMO-01) and the other reviewed candidates.
That selects a delivery phase, not an individual quickener/cache/ISA design;
the review's mechanism-specific proof and measurement gates remain applicable.

## Intended outcome

Keep RXC's structured AST rewriting, including its Level C role, and give
machine optimizations a consistent, reviewable home. A maintainer should be
able to identify a transformation's matcher, required facts, semantic proof,
rewrite and regression coverage without reconstructing several independent
implementations. RXAS should own static machine-pattern selection. RXVM should
implement the documented instruction semantics, including necessary runtime
guards, representation-specific fast paths and caches of runtime-derived facts.
Adaptive specialization is a possible VM responsibility when those facts save
work that earlier stages cannot remove; it is not automatically justified by
the existence of an execution-image mechanism.

This review distinguishes that proposed ownership from the currently accepted
two private VM fusions. Removing or migrating them is a separate implementation
decision; current measurements are needed before claiming either is cost-free.

## RXAS: substantial infrastructure, incomplete uniformity

Current structure:

| Responsibility | Existing implementation |
| --- | --- |
| Cheap exact local rewrites | `assembler/rxas_opt.c`: declarative input/output rule table, operand capture, adjacency/hazard rules and shared matcher. Keep this cheap preprocessing stage. |
| Stable pass identity and required facts | `rxas_flow_pass.[ch]`: owner, capability mask and candidate census for each family. The census selects work; it does not prove correctness. |
| Shared analysis | `rxas_flow_graph`, `analysis`, `signal`, `ssa`, `use`: immutable graph epochs, lazy analyses, common storage/value/effect queries. |
| Semantic proofs | `rxas_flow_proof.[ch]`: typed query/plan interfaces such as `rxas_flow_prove_repetition`, `rxas_flow_prove_typed_copy_redirect`, and `rxas_flow_prove_compare_branch_fusion`; common rejection reasons, budgets, caches and diagnostics. |
| Application | `rxas_flow.c` contains consumer-specific application and shared record/operand helpers; `rxas_flow_rewrite.[ch]` owns branch-thread planning/application. |
| Transaction/lifetime checks | `rxas_flow_batch.[ch]`: sparse snapshots, epoch matching, commit validation and rollback. Common edits must preserve operand-vector/named-slot consistency and token ownership. |

This is mainly a typed C API framework, not a collection of optimization
macros. Macros are appropriate for the opcode inventories and small mechanical
declarations; they do not replace semantic interfaces or make a proof correct.

The concrete maintenance weaknesses are:

- `rxas_flow_proof.c` has about 8,200 lines and `rxas_flow.c` about 4,700 at the
  reviewed baseline. Many families share a file, making ownership and repeated
  preconditions hard to audit.
- Proof result/plan types already exist, but application remains substantially
  hand-written and family-specific. Structural edits, semantic transactions and
  cheap local rules have different necessary lifecycles; those differences
  need explicit extension guidance rather than accidental conventions.
- Transaction validation checks edit structure, ownership and consistency. It
  is not an independent semantic theorem checker. A faulty shared premise can
  still produce a well-formed but incorrect edit, as the entry-alias bug showed.
- The same underlying semantic question must not be reimplemented as a new
  scan or guard in each pass. The entry-alias repair therefore corrects common
  component queries and unifies fresh/cache write resolution; it does not
  patch only repeated-conversion elimination.

## Recommended RXAS response

Complete the existing separation, without replacing it with a new generic
optimizer language or moving cheap rules into SSA:

1. Make the extension path explicit: **candidate → shared facts → typed proof
   plan → validated application → epoch invalidation/rebuild**. Each family
   declares its required analyses and records why a rejected case is unsafe.
2. Factor repeated semantic questions into the shared fact/proof APIs: exact
   identity versus may-alias, effect stability, hidden cleanup, signal behavior,
   observations, dominance and call-window reachability. A family composes
   these predicates instead of recreating them.
3. Split proof families and their applications into bounded modules with one
   documented owner each. Keep shared primitive proofs together. Moving text
   into more files alone is not the objective.
4. Standardize the edit primitives used by those plans: delete a record,
   replace an instruction, redirect an operand, change a TRACE reference, and
   insert a local/label where supported. Require current epoch/record identity,
   consistent token ownership and atomic coupled edits. Preserve the existing
   dedicated structural-rewrite lifecycle where insertion changes indexing.
5. Add conformance coverage for the common contracts, plus per-family positive
   and negative examples. Include handwritten assembly, pre-existing incoming
   aliases, metadata observations, signal continuations, stale plans and
   rollback. Preserve known optimization successes while fixing unsound cases.

A small invariant-preserving extraction is preferable to a wholesale rewrite.
Representative emitted bytecode and diagnostics should remain unchanged after
each purely structural step; semantic repairs should be separate and identified.

## VM fusions: what is known statically and what is checked at runtime

The two existing load-time recognizers in `interpreter/rxvmintp.c` inspect only
opcodes, arities, adjacency and register numbers. **RXAS has those facts too.**
Neither recognizer discovers a source/compiler fact unavailable to RXAS.

| Current private identity | Statically visible pattern | Runtime-dependent condition |
| --- | --- | --- |
| R1 relink | `unlink d; linkref d,s`, distinct registers | Is the reference currently valid? |
| R2 attribute copy | `linkattr1 t,o,index; copy d,t; unlink t`, immediate index and distinct registers | Is the index in range, and does the attribute carry the supported reference descriptor? |

For R1, preparation chooses the private entry point from the pattern. On each
execution, the handler performs unlink, checks the actual reference target,
then either finishes the fast relink or resumes the ordinary `linkref` path.
Debug/breakpoint observation also selects the canonical path. R2 similarly
preserves range signaling, generic-value fallback, copying and temporary reset.

This is a conditional equivalence argument:

```text
if runtime preconditions and observation conditions hold:
    execute a proved equivalent fast implementation
else:
    execute the ordinary semantics
```

It is not permission to execute an unproved optimization. RXAS cannot generally
prove that an arbitrary future reference will be valid, but it can select an
instruction whose specified runtime behavior includes this test and fallback.
That is how ordinary guarded VM instructions can work as well.

The private approach's practical advantages are avoiding new public opcodes and
keeping the original instruction positions available, including entry into the
middle of a pattern and instruction-level debugging. These are representation
and tooling tradeoffs, not a unique pattern-discovery ability. An RXAS-selected
replacement must explicitly preserve or exclude intermediate entry points and
account for source/TRACE anchors and failure continuations.

## How extensible is the current private mechanism?

Handler bodies already use shared `RXVM_PRIVATE_HANDLER`/`RXVM_HANDLER` macros,
common value/reference helpers, and shared dispatch/PC machinery for switch and
threaded engines. This avoids maintaining separate semantic handler bodies for
each engine.

Registration is less systematic. Private numeric identities, recognizers,
preparation arguments/selection, label selection, sizing bounds, placement
policy and instrumentation mappings require coordinated edits in several
locations (`rxvmintp.c`, `rxvmhandlerpolicy.h` and the grouped handler files).
The detailed written inventory exists in
[`PERF3-FUSION-REGISTRY.md`](../../../performance/PERF3-FUSION-REGISTRY.md), but
that document is not a single machine-readable registration authority.

Adding more private fusions is possible. Doing so repeatedly would expand the
parallel optimization/registration surface that Adrian is questioning.

## The original quickening objective and what was actually delivered

The original [PERF2-02 brief](../../../performance/PERF2-02-HANDOVER-PROMPT.md)
explicitly required a reusable private quickener to be evaluated as a real
option. The question was whether persistent site knowledge beat the best
compiler, assembler or direct-handler implementation, including lifecycle and
memory costs. It did not require quickening to win. The bounded first clients
were Bounce reference ownership and Richards value copying; selector caches,
BIF specialization and a general opcode campaign were outside that experiment.

Terminology matters here:

| Mechanism | What it does | Current cREXX status |
| --- | --- | --- |
| Static fusion | Combines an instruction sequence into fewer dispatches, possibly with runtime guards. | Public RXAS-selected instructions and the two private R1/R2 load-time fusions. |
| Load-time specialization, sometimes called eager quickening | Resolves operands or chooses a handler from facts available during preparation. No execution history is required. | Callable-pointer/dispatch preparation and R1/R2 private handler selection. |
| Runtime cache | Remembers a runtime-derived result and validates its key before reuse; the instruction handler need not change. | Two-way method-site caches keyed by receiver descriptor and semantic generation; factory binding caches. |
| Adaptive quickening | Changes a site's execution strategy after observing executions, with guards, fallback and a policy for incompatible behavior. | Bounded Q7 prototype was evaluated and rejected; no general adaptive instruction-rewriting framework is installed. |
| Direct guarded fast path | Tests the current values and immediately performs the equivalent cheaper operation, without learned site state. | Canonical MKREF owner checks and other value/reference implementation paths. |

Fusion and quickening are independent choices: an adaptive implementation may
specialize a single instruction without fusing anything, and an eager fusion
may never learn anything from execution. A guard on each execution is not by
itself adaptive learning.

The [original architecture record](../../../performance/PERF2-02-ARCHITECTURE.md)
and [measurements](../../../performance/evidence/2026-07-23-perf2-02-quickening-poc/measurements.md)
retain the actual comparison:

- **Q3b direct handler:** rechecked exact live local/attribute ownership in
  canonical MKREF, with no persistent site state. In the final Bounce panel it
  was 7.584% faster than eager Q4 in the then-named `rxvm`, and tied in `rxbvm`.
  These are historical engine labels and workloads, not current product claims.
- **Q4 eager specialization:** preparation chose a private handler using the
  same ownership guards. Selecting it earlier did not beat the best direct
  implementation. This Q4 experiment is distinct from the later R1/R2 fusions.
- **Q7 reusable substrate:** module-owned records supported an eager reference
  client and a lazy scalar-COPY client. Its COPY state machine was candidate →
  specialized → disabled; mismatches resumed canonical behavior. Q7 tied Q4
  on Bounce, was neutral on Richards, and requested 56,264/62,536 persistent
  bytes respectively, excluding allocator overhead. Lifecycle proof was
  incomplete, including late-load replay, repeated/nested embedded execution,
  allocation failure and full observation-mode handling.
- **Compiler control:** Richards' expensive removable receiver copies belonged
  to compiler capture rewriting. Observing an object repeatedly did not make
  a semantically necessary object copy removable. Scalar candidates were a
  small minority and did not justify Q7.

The July work therefore delivered a useful production improvement rather than
an adaptive framework: the [accepted worklist](../../../performance/PERF2-02-WORKLIST.md)
records approval of Q3b inside MKREF and its subsequent favorable Release
verdict. That implementation remains visible at
`interpreter/rxvmhandlers_control.inc:2882`: it examines an adjacent
MINLINKATTR1 as a possible route, then checks the live base-local and physical
attribute identities, or falls back to general owner discovery. It does not
retain frame/attribute pointers as learned site facts or fuse the instructions.
This direct-handler optimization is additional to the two private fusions.

Two limits on the later August registry's compressed conclusion are important:
Q7 tied the eager Q4 reference option, not the final winning Q3b in both engines;
and absence of missed exact R1/R2 patterns says nothing about other adaptive
families. The historical registry remains a dated decision record. Its closure
of the then-proposed selector-cache activity also does not describe today's
separate, implemented interface-site caches.

## What runtime learning can know that RXAS cannot

For a call through an interface, assembly identifies the requested member but
may not determine which concrete receiver types this particular site will see,
their frequencies, or bindings supplied by later-loaded modules. A runtime
site could observe that almost every receiver has descriptor T, remember T's
target, and use it while both descriptor and binding generation still match.
RXAS could select an instruction supporting that behavior; it cannot generally
precompute the observed type distribution or the future loaded target.

This is already partly implemented. At
`interpreter/rxvmintp.c:4115`, `resolve_runtime_graph_method_cached()` keeps two
descriptor/target pairs, clears them on semantic-generation mismatch, and
resolves and inserts a target on a miss. The instruction still executes its
ordinary handler. `SRCFPROCSEL` similarly caches its bound provider range;
argument-dependent match procedures still have to run where required. Neither
cache licenses caching an arbitrary method result or skipping side effects.

An adaptive monomorphic handler could potentially shorten the existing cached
method path further, restoring the general path when types/bindings change.
That is an illustrative candidate, **not evidence of an outstanding bottleneck**:
the current dense dispatch rows, bound callable arrays and two-way cache may
already make selection too cheap for rewriting to pay. Measure incremental
cost over this implementation, not over an obsolete uncached lookup.

The same discipline applies to generic value operations. Observed stable
representation can sometimes justify a guarded specialized handler, but an
already typed integer opcode gains nothing merely by learning "integer" again.
Reference ownership also needs lifetime proof; a once-observed address is not
a durable fact. No additional compiler assertion or serialized proof channel
is required for runtime guards derived from current VM state.

## Feasibility and the smallest defensible next experiment

Adaptive quickening is technically feasible without JIT compilation or a new
RXBIN format: select among compiled C handlers in a worker-owned execution
image while canonical bytecode remains unchanged. The current execution image,
canonical instruction indexing, shared handler bodies and dynamic caches are
useful foundations. They are not a completed reusable quickener.

The concurrency context has changed since July. Today's sealed-generation
model shares immutable canonical images but keeps prepared images, bindings,
caches, globals and frames in separate worker overlays. A future quickener must
put mutable site state in that existing worker ownership model, not copy Q7's
historical process-global/confinement assumptions. See
`docs/ai-context/RXVM_INTERPRETER.md`, "Sealed Program Generations", and
`interpreter/rxvmintp.h:100` for the current cache/image structures.

A selected implementation would need explicit contracts for:

1. Site identity and bounded storage; no retained frame/value/reference pointers
   without a proved lifetime scheme. Prefer eligible/hot sites over records for
   every opcode, and account for per-worker multiplication of state.
2. Learning and publication; generic first execution, eligibility/hotness policy,
   complete state before use, guarded hit, correct miss, and bounded disable or
   respecialization behavior. Observed frequency selects a strategy, never
   proves semantics.
3. Binding invalidation, image refresh after late load, repeated calls and nested
   re-entry, worker attachment/teardown and failed specialization. Existing
   generation checks are reusable machinery, not proof for every new fact.
4. Canonical signal location, TRACE/debug stepping, interrupt/cancellation
   boundaries and profile/RXSEQ identity in both engines. Single-instruction
   specialization is an easier first contract than speculative fusion.
5. A common family registration path tying handler, guard, state/reset policy,
   diagnostics and tests together. A new family should not require another
   scattered set of private-enum/table edits.

Before choosing a family, establish a current hot site, runtime-dependent fact,
hit/miss distribution and repeated cost left after RXC/RXAS and existing caches.
Compare the same semantics in the ordinary current implementation, an improved
direct helper/cache if applicable, and the smallest private adaptive form.
Include cold, mixed-type and late-binding controls, startup/state cost, and
ordinary profiling-off Release timing. High hit rate alone is insufficient.
Only a demonstrated advantage merits a reusable substrate or second family.

## Recommendation and evidence boundary

**Retain runtime-derived specialization as a legitimate RXVM responsibility.
Do not start a general adaptive quickening framework on the current evidence.
Do not expand the independent static VM fusion recognizer by default.**

This qualifies the earlier remove-or-migrate recommendation: it concerns the
two static recognizers, not all VM fast paths, caches or future adaptive work.
The July prototype's negative result is a rejection of those clients/design,
not a proof that adaptive quickening can never help. The original work also
shows that improving ordinary instruction implementation can win decisively.

For the two existing mechanisms, compare these concrete outcomes:

1. Remove the private fusion and execute the existing ordinary instructions.
   Prefer this where current end-to-end evidence does not justify the extra
   mechanism.
2. Where benefit is material, migrate to an explicit RXAS-selected instruction
   with guarded VM semantics. Its complete effects, exceptional/observable
   behavior, disassembly, feature compatibility and tests become normal ISA
   assets. This requires Adrian's public-instruction/architecture approval.
3. Retain a narrowly documented private execution implementation if its current
   benefit is material and public-ISA/tooling migration would cost more than it
   removes. This remains a possible evidence-based exception, not approval to
   grow a parallel optimizer. Improve its common registration if retention is
   selected and the maintenance benefit warrants that work.

Keep the currently accepted implementation while deciding. Removing a static
recognizer would not remove the execution image used for callable resolution,
threaded dispatch and potential future adaptive handlers. Keep the existing
interface caches and direct MKREF improvement outside that removal question.

For Release 1, the present recommendation is **retain and contain the accepted
mechanisms**, rather than select a general quickener or an unmeasured removal.
The next bounded decision, if selected, is the R1/R2 on/off comparison below.
If their benefit has disappeared, remove them; if it remains, decide whether
explicit ISA migration actually reduces total maintenance cost. This is a
recommendation from the review, not a new user-approved architecture decision.

The retained August registry reports approximately 1–3% paired-median gains on
the specific List work-100 panels for the two mechanisms, with historical VM
labels. Those are narrow historical results, not current product-wide benefit.
The August portfolio census found 18 exact R1 and 16 exact R2 sites, with no
uncovered exact sites in those images. It does not prove there are no worthwhile
new shapes in today's Level C/other workloads.

There is consequently no evidence-backed "obvious next private fusion" to
implement now. A register-indexed variant of R2 is a plausible future census
candidate, but public assists such as `igetunlink` already cover parts of the
attribute/cleanup space. Count surviving final-bytecode shapes and execution
frequency before selecting anything; do not rediscover an existing public
instruction or infer benefit from static site counts.

## Proposed acceptance and sequence, pending selection

- [ ] **MAINT-AC-01:** a documented per-family extension contract maps matcher,
  fact requirements, typed plan, application and tests; maintained families use
  common primitives without independent duplicate semantic scans.
- [ ] **MAINT-AC-02:** structural RXAS refactoring preserves representative
  emitted bytecode, observations and existing positive/negative regressions;
  correctness changes are separately evidenced.
- [ ] **FUSION-AC-01:** retain a current matched baseline and isolated R1-off,
  R2-off and both-off controls using final identical bytecode. Measure material
  affected workloads, both concrete engines where relevant, and product
  runtime without profiling. Distinguish historical evidence from new results.
- [ ] **FUSION-AC-02:** Adrian selects removal, explicit instruction migration
  or bounded retention for each mechanism from the measured benefit,
  maintenance and compatibility tradeoff. This proposal's alternatives were
  expanded after the quickening follow-up; none is an accepted scope change.
- [ ] **FUSION-AC-03:** the selected implementation satisfies that disposition,
  preserves full error/lifetime/debug/TRACE behavior and valid intermediate
  entries, and passes ordinary toolchain/VM regressions.
- [x] **QUICKEN-AC-01:** distinguish the original objective, Q7 prototype,
  production direct-handler result, current static fusions and current runtime
  caches; identify feasibility requirements and limits of the historical evidence.
- [ ] **QUICKEN-AC-02:** before any new adaptive implementation is selected,
  identify a measured current runtime-dependent client and compare its proposed
  benefit against the best existing/direct/cache/static implementation. No such
  new client or experiment is selected by this review.

1. **MAINT-STEP-01** (MAINT-AC-01): inventory rule families and common operations;
   select one small extraction with unchanged output as the first structural
   slice. Preserve RXC's separate AST framework.
2. **MAINT-STEP-02** (MAINT-AC-01/02): apply approved extractions incrementally;
   reuse valid correctness evidence and retain negative controls.
3. **FUSION-STEP-01** (FUSION-AC-01): perform bounded isolated on/off measurements
   of the existing mechanisms; no new fusion or public instruction yet.
4. **FUSION-STEP-02** (FUSION-AC-02): choose the concrete retention, migration
   or removal contract from retained measurements; this is the architecture gate.
5. **FUSION-STEP-03** (FUSION-AC-03): implement the selected change with the
   mandatory first Release verdict before broad performance closeout, then
   finish required correctness/tooling qualification.
6. **QUICKEN-STEP-01 — review complete** (QUICKEN-AC-01): reconcile the original
   prototype and accepted production outcome with current source and ownership.
7. **QUICKEN-STEP-02 — unselected** (QUICKEN-AC-02): only if Adrian selects this
   further investigation, retain a current residual-cost census and propose one
   bounded client with explicit cold/miss/lifecycle and first-verdict gates.
   General substrate implementation remains a later architecture decision.

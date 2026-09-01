# PERF2-02 new-session handover prompt

Recommended setting: **Ultra**. This activity must distinguish a genuinely
runtime-stable semantic fact from one that belongs in the compiler, RXAS or a
direct value/reference helper, while preserving reference lifetime, process
image, re-entry, interrupt, TRACE and dual-VM contracts. Once Adrian has
selected and frozen the architecture, an individual production slice can
normally return to **Very High**.

Copy the prompt below into a new Ultra session.

---

You are working in `/Users/adrian/CLionProjects/CREXX` on **PERF2-02 —
stable-site semantic quickening architecture and bounded PoC panel**.

Complete the PERF2-02 design comparison and isolated prototypes. Do not merely
write a speculative design, but do **not** select or install a production
quickener without Adrian's explicit architecture decision. Your exit is a
measured, correctness-qualified placement recommendation and a clean stop for
that decision.

Exercise Ultra-level design judgment. The candidate families and Q0-Q7 panel
below are a required floor, not a closed solution menu. Derive specific
companion strategies that make quickening cheaper, safer or more applicable,
and compare the strongest bounded ideas when evidence supports them. A
companion may live in compiler proof/metadata, existing RXAS selection,
load-time preparation, private site-state layout, guard/state-machine design,
dequickening, or the value/reference helper boundary. Give each new strategy a
stable local ID, state its hypothesis and semantic proof obligation, and keep
it separable so its incremental benefit can be measured. Do not use this
freedom to bypass Adrian's approval for a public format or production
architecture decision.

Treat a reusable **core private quickener capability** as a real first-class
architecture option, not as forbidden generalization. The question is whether
cREXX should own a small extensible process-local substrate for site identity,
state transitions, guards, fallback, invalidation/dequickening, diagnostics and
dual-VM lifecycle, with semantic-family-specific fast paths layered on it.
Compare that option against purpose-built one-off specialization. Bounce and
Richards are the evidence-backed first clients and cost controls; they do not
limit the eventual capability if the core design wins.

The central question is deliberately broader than "how do we add
quickening?":

> For the exact hot reference/value sites selected by PERF2-01, which semantic
> fact is stable, what is the earliest safe owner of that fact, and does guarded
> runtime quickening beat the best static/direct form after startup, memory,
> fallback and lifecycle costs?

"Quickening is not the right owner for this mechanism" is a valid and useful
PERF2-02 result if the measured direct-helper, compiler or RXAS form wins.

## Accepted starting point

PERF2-01 Gate A was accepted by Adrian on 2026-07-23. Its selection baseline is:

`performance/evidence/2026-07-23-perf2-01-current-baseline/`

Accepted same-session orientation from that bundle:

- common-five cREXX/ooRexx geometric mean: **0.892218** for `rxvm` and
  **0.833885** for `rxbvm`;
- separately disclosed cREXX RexxCPS 2.2d versus canonical Classic ooRexx:
  **0.732569** and **0.677217**;
- Sieve and Permute already exceed the 1.50x per-cell target in both VMs;
- Base64 reaches 0.725045/0.774421 of ooRexx;
- Bounce reaches 0.331744/0.317745; and
- Richards reaches 0.153220/0.150768.

The bundle's measured ownership decision supersedes the pre-capture candidate
ordering in the general roadmap:

- Bounce spends about 263 ms in `MKREF_REG_REG`; reference-tree storage owns
  more than four fifths of its stable native samples.
- Richards executes 96.1 million general copy operations / 762.8 MB and spends
  about 0.84-0.85 s in `COPY_REG_REG`.
- All accepted optimized profiles execute **zero** `srcmethodsel` or
  `srcfprocsel` selector attempts.

Therefore the first quickening panel is the exact reference/attribute and
value-copy sites exposed by Bounce and Richards. Do not start with generic
selector caches, BIF quickening or a broad opcode/fusion campaign.

At handover preparation time, branch `develop`, HEAD and `origin/develop` were
all `d5b25a78fd6cd2b5b5962b45e508f3cb2bb782e6`. The main worktree contained
intentional uncommitted PERF2-00/01 documentation, evidence, schema-5
diagnostic instrumentation and tooling. Treat these as orientation, not live
facts. Verify the exact state and preserve every pre-existing change. Do not
reset, stash, overwrite, stage, commit or push unless Adrian explicitly asks.

## Mandatory reading and instruction order

Read completely before task actions:

1. repository `AGENTS.md`;
2. `performance/AGENTS.md`;
3. `performance/ROADMAP.md`, especially the North star, strategic ownership
   rule, execution order and PERF2-02, PERF2-06, PERF2-07 and PERF2-11 gates;
4. `performance/PERFORMANCE-GOVERNANCE.md`;
5. `performance/README.md`;
6. `performance/PERF2-01-WORKLIST.md`;
7. the accepted PERF2-01 bundle `README.md`, `10-dossiers/mechanism-census.md`,
   `10-dossiers/candidate-panels.md`, Bounce/Richards dossiers and their exact
   profile/site/native/heap evidence;
8. `docs/ai-context/RXVM_INTERPRETER.md`;
9. the reference/value, instruction-image, dispatch and instrumentation code
   relevant to `MKREF_REG_REG`, `COPY_REG_REG`, attribute linking, reference
   identity/lifetime, execution-image preparation, late load and both VMs; and
10. focused existing reference, value-copy, TRACE/signal, RXVML/re-entry,
    prepare-only, late-load, profiling and RXSEQ tests.

Use the live roadmap and accepted PERF2-01 evidence over historical roadmap
intuition. Preserve the dated charter and closed initial roadmap as history.
Do not assume cREXX syntax or VM semantics from memory when the repository
documents or handlers define them.

## Required work plan before execution

Before changing any production source or building a PoC:

1. give Adrian a numbered execution plan;
2. create `performance/PERF2-02-WORKLIST.md` with `apply_patch`;
3. update PERF2-02 from `queued` to `in progress` in the live roadmap in the
   same change that actually starts it;
4. record exact branch, HEAD/upstream, dirty scope, product/diagnostic source
   relationship, host state, toolchain and build configurations;
5. define the scratch/isolated PoC strategy that preserves the dirty main
   worktree and separately identifies every prototype patch;
6. write the semantic-invariant matrix and design-selection table before the
   first C implementation; and
7. state the final PERF2-02 stop point: present the panel and recommendation to
   Adrian before any production selection.

Use resumable stages, explicit commands/outputs/check boxes and named blockers.
Keep verbose build/profile output in `mktemp` logs and inspect focused slices.
Use target-only builds and focused tests during the PoC loop.

## Scope boundary

This activity may:

- inspect and, only where a demonstrated evidence gap exists, minimally extend
  diagnostic site-state observation;
- build isolated private prototypes of static/direct/eager/lazy/tiered forms;
- add focused correctness fixtures or microbenchmarks needed to compare the
  exact semantic unit; and
- measure ordinary profiling-off Release PoC binaries against the frozen
  accepted product baseline or a bounded same-session drift control.

It must not:

- install a production quickener or continue into production closeout before
  Adrian selects the architecture;
- add public RXAS syntax, assign a canonical opcode, change serialized RXBIN,
  public ABI or language semantics;
- begin generic `srcmethodsel`/`srcfprocsel` caches when accepted profiles have
  zero attempts;
- begin BIF/helper quickening, which waits for PERF2-03/04's cleaned static
  inlining ceiling;
- turn PERF2-02 into a general dispatcher, compact-stream, representation,
  pooling, frame, LTO/PGO or JIT rewrite;
- optimize a benchmark-specific constant or weaken benchmark work/correctness;
  or
- use instrumented elapsed time as product evidence.

Any proposed language, public ISA/RXBIN/ABI or irreversible architectural
change is `decision required`: document it and stop for Adrian. A temporary
private prototype is not approval to expose or retain its form.

## Stage 0 — freeze exact state and isolate prototypes

- Verify branch/HEAD/upstream and enumerate all existing modified/untracked
  paths, including PERF2-01 diagnostic code and evidence.
- Independently reverify the accepted bundle checksum and read its limitations.
- Audit whether current schema-5 code is committed, merely present in the main
  dirty tree, or differs from the clean accepted product source. Record exact
  hashes and never allow diagnostic changes to contaminate product timing.
- Use isolated scratch worktrees/builds for competing source prototypes. If a
  prototype needs uncommitted accepted diagnostic code, transfer only the
  necessary, hashed patch and record that provenance; do not silently use the
  dirty main worktree as a product baseline.
- Reuse accepted product evidence unless its exact runtime/input relationship
  is invalid for a PoC. If drift must be controlled, use the smallest bounded
  same-session control and say why.
- Record AC/low-power/thermal/load state around decisive measurements. Do not
  overlap benchmark campaigns or compare lifecycle-inclusive and steady-state
  cells.

## Stage 1 — prove the semantic unit and stability fact

For both candidate families, trace source -> optimized RXAS/RXBIN -> handler ->
value/reference helpers -> lifetime/teardown behavior. Produce a site table
containing exact module/procedure/instruction coordinate, dynamic count,
observed source and destination shape, variability, cost, and likely owner.

### Candidate A: Bounce reference construction/storage

Audit at least:

- `MKREF_REG_REG`, `LINKREF`, `SETREF`, `DEREF`, `LINKATTR*`, `ENDLIFE` and any
  compiler/RXAS scaffolding around the selected Bounce sites;
- `rxvm_reference_owner_kind_for_storage()`, reference identity lookup/cell
  creation, owner-frame marking, invalidation and recycled-frame cleanup;
- local, argument, global, attribute, caller-owned and nested-attribute
  storage; and
- whether the stable fact is the owner kind, register/storage class, attribute
  route, reference-cell identity, or something narrower.

Do not assume a register site makes its target pointer, owner frame or reference
cell permanently stable. Reused frames, linked locals, caller-owned receiver
storage and late lifetime invalidation must be proved explicitly.

### Candidate B: Richards general value copy

Audit the selected `COPY_REG_REG` sites and determine exactly why they enter
recursive `copy_value()`. Separate at least:

- scalar/status-only values;
- established reference payloads;
- strings, decimals and binaries;
- native payload hooks;
- objects and recursive attributes;
- destination-owned storage that must be released/reused; and
- alias/same-storage cases.

Do not replace `copy_value()` with a raw struct assignment. A safe ceiling must
retain destination cleanup, reference retain/release, native-copy hooks,
owned-buffer and recursive-attribute semantics wherever they can occur.

For each family write:

1. the smallest per-execution guard that proves the fast path;
2. facts provable statically by compiler flow/effects or RXAS;
3. facts knowable at load/preparation;
4. facts observable only at first/repeated execution;
5. mutation/invalidation events;
6. exact generic fallback; and
7. the evidence required to call the site stable.

If existing schema 5 cannot express those facts, first write a coverage-gap
record. Add only the minimum prototype telemetry, keep it diagnostic and prove
profiling-off compilation remains empty. Do not turn a two-site PoC into a new
general profiler programme.

## Stage 2 — architectural decision record

Before coding, compare at least these placements for each semantic unit:

| ID | Placement | Required comparison |
| --- | --- | --- |
| Q0 | current canonical path | Full semantic baseline and fallback. |
| Q1 | compiler-owned result-only/static lowering | Machine ceiling when typed flow/effects can prove the fact and intermediates are unobservable. |
| Q2 | existing authored RXAS/static rule | Best safe static control using existing public semantics; do not create public syntax. |
| Q3 | direct private value/reference helper or handler form | Shows whether removing general helper work is sufficient without persistent site state. |
| Q4 | eager process-image/side-table specialization | Uses only facts valid at load/preparation and measures cold-site cost. |
| Q5 | lazy guarded first-hit specialization | Learns an execution-only fact, retains a complete fallback and measures first-hit cost. |
| Q6 | threshold/tiered guarded specialization | Measures whether hotness/stability repays state and mutation overhead. |

Also include this architectural option:

| ID | Architecture | Required comparison |
| --- | --- | --- |
| Q7 | extensible core private quickener substrate | Reuse site/lifecycle/state/invalidation machinery across the two measured semantic families while keeping their guards and fast paths specialized; measure its incremental tax against the best one-off form. |

Q3 is an explicit addition required by PERF2-01's ownership result. It prevents
calling a direct value-path improvement "quickening" merely because it lives in
the VM.

After completing this minimum placement set, add any Ultra-derived companion
strategies that could materially improve the leading design. Examples of the
kind of question to explore—not prescribed answers—include whether an existing
compiler/effects proof can narrow the runtime guard, whether eager decoding can
remove work without learning a runtime fact, whether a compact mono/poly site
state avoids repeated setup, whether a purpose-built dequickening rule is
cheaper than a generation check, or whether one shared value/reference
primitive lets both VMs quicken without duplicating semantics. Prefer
orthogonal companions whose incremental effect can be turned on/off and
measured. Record plausible ideas even when time-boxing means only the strongest
one or two are prototyped.

For Q7, let the repository evidence drive the exact design. At minimum decide
and compare:

- process-local ownership: execution-image annotation, parallel side table or a
  justified hybrid;
- stable canonical site key and the mapping cost in both `rxvm` and `rxbvm`;
- compact shared state header versus semantic-family payload;
- how handler selection avoids adding an indirect-call or generic-branch tax
  to every hot hit;
- eager, lazy and threshold allocation/preparation policies;
- one shared lifecycle for publish, miss, replace, disable, invalidate,
  dequicken, late-link refresh and context teardown;
- concurrency/publication and failure atomicity;
- canonical profiling/RXSEQ/source identity plus quickener diagnostics; and
- an internal extension contract for later measured conversion, BIF, call or
  prepared-plan families, without implementing those unselected families now.

Prototype enough of Q7 to serve the two selected families or, if semantics
make one family unsuitable, one real family plus a second focused fixture that
proves extensibility. Measure Q7 against the best purpose-built version with
the same fast-path semantics. Reject decorative abstraction: a reusable core
must centralize difficult lifecycle/correctness machinery without imposing a
material steady-state, startup, RSS, image-size or code-layout penalty.

For every viable option record:

- exact stable fact and owner;
- state layout and byte cost per module/site;
- state machine (`cold`, candidate states, specialized, polymorphic/disabled or
  justified equivalents);
- guard, hit, miss, replacement, invalidation and failure behavior;
- eager/lazy/concurrency ownership and allocation-failure behavior;
- expected instruction/native-operation ceiling;
- startup/load/first-hit/steady-state/RSS/image/teardown costs;
- behavior in `rxvm` and `rxbvm`; and
- rejection or advancement reason.

Do not force all variants through implementation. Prune an impossible form only
after recording the semantic or architectural reason. At least two plausible
forms plus Q0 must be prototyped for the leading family.

## Stage 3 — non-negotiable VM invariants

Any persistent specialization must satisfy all of these by design and focused
test:

- Canonical RXBIN and canonical loaded instructions remain immutable.
  Quickened state belongs to the process-local execution image or an explicit
  process-owned side table; never serialize C handler pointers or learned
  process state.
- Direct-threaded `rxvm` keeps one stable computed-goto label owner in `run()`
  under `RXVM_LABEL_OWNER` (`noinline`/`noclone`) or proves an explicitly
  approved equivalent. Test prepared handler addresses in supported build
  modes.
- `rxbvm` preserves identical semantics. A different private layout is allowed
  only when measured and explained.
- `prepare_only` / `rxvm_prepare()` learns no execution-only fact. State is
  valid or reset for a later run, repeated runs on one context, `rxvm_call`,
  embedded/RXVML use and changed TRACE/debug/profiling mode.
- Late load/link, semantic/provider generation change and any relevant dynamic
  mutation rebuild, invalidate or safely guard learned facts.
- Every canonical instruction keeps its exception, retirement,
  interrupt-poll, TRACE/source-step, breakpoint, signal/unwind and profiler
  identity. If a prototype fuses work, it must dequick/de-fuse wherever an
  intermediate boundary is observable.
- RXSEQ and profiles retain canonical module/instruction/opcode identity while
  separately exposing prototype state, hits, misses, replacements,
  invalidations and disabled sites.
- Allocation or preparation failure leaves the canonical path executable and
  does not partially publish state.
- Prove the actual context/thread concurrency model. Do not assume one writer
  or one executing thread without evidence.

For reference/value candidates also cover:

- source/destination alias and identical register;
- local, argument, global, linked and attribute storage;
- nested attributes and recursive values;
- valid/invalid references and lifetime end;
- recycled frames, caller-owned receiver storage and repeated contexts;
- string, binary, decimal, object and native-payload variants;
- OOM/failure atomicity, catchable signals and teardown; and
- no time-for-retained-memory trade hidden in frame/value reuse.

## Stage 4 — bounded PoC panel

Time-box one semantic family at a time. Start with the smallest Bounce
reference-storage site that has an exact stable fact; then test the Richards
copy site if the first panel does not already answer the placement question.

For each implemented prototype:

1. build the exact machine-level inline/direct control first;
2. build the integrated static/direct or private quickened form in an isolated
   ordinary profiling-off Release tree;
3. run the minimum focused semantic tests before timing;
4. compare the same exact workload/image/library and both VMs;
5. collect site executions, hits/misses/invalidations only in a separate
   diagnostic run;
6. measure first hit, steady state, startup/load-first-result, RSS,
   process-image/side-table bytes and teardown where changed; and
7. run the named guard panel.

When a companion strategy is implemented, compare the leading quickened form
with and without that companion. Do not bundle several unmeasured mechanisms
into one apparently winning prototype. If a companion belongs to PERF2-03,
PERF2-05, PERF2-06 or PERF2-07 rather than PERF2-02, retain its interface and
expected benefit as a successor recommendation instead of silently absorbing
the other activity.

Required target and guards:

- target A: canonical Bounce under `rxvm` and `rxbvm`;
- target B when implemented: canonical Richards under both VMs;
- lifetime/allocation guards: Storage and Towers;
- existing-win guards: Sieve and Permute;
- unrelated string-path guard: Base64;
- focused reference/value and RXSEQ-boundary fixtures; and
- prepare-only, re-entry/RXVML, late-load, TRACE/source, signal/unwind and both
  dispatch-mode fixtures in proportion to the prototype surface.

Use accepted raw evidence for orientation and ordinary unprofiled Release wall
clock for the verdict. A diagnostic microbenchmark may prove the primitive's
ceiling but cannot select it without an end-to-end target result.

For a PoC panel, use the smallest governed sample sufficient to separate large
differences and retain all raw data. If variants are close/noisy or a design is
being recommended for production, use paired balanced/interleaved sampling in
accordance with `PERFORMANCE-GOVERNANCE.md`. Do not spend a full formal
portfolio campaign on a clearly failed prototype.

## Adoption test and required interpretation

A PERF2-02 candidate can be recommended only if:

1. its repeated semantic cost and site stability are demonstrated by the
   accepted evidence plus any bounded prototype counters;
2. it beats Q0 in ordinary profiling-off Release and beats the best safe
   static/direct form, or proves why only runtime can consume the winning fact;
3. the end-to-end target result is clearly favorable in both relevant VM modes
   or any mode difference is explicitly dispositioned;
4. correctness, fallback, invalidation, TRACE/source, signal, late-load,
   re-entry and lifetime fixtures pass;
5. startup, RSS, image/state bytes and lifecycle costs are explicit; and
6. Sieve, Permute, Storage, Towers and Base64 show no unexplained guard hit.

If Q7 is recommended, additionally show that its reusable machinery serves two
distinct site families (or one real family plus a convincing second semantic
fixture), that the shared substrate does not put generic lookup/callback work
on the monomorphic hit path, and that its measured incremental cost versus the
best one-off form is justified by the lifecycle/invalidation correctness it
centralizes and the credible extension path it creates.

Do not describe a lower instruction count, a faster instrumented profile or a
native-helper microbenchmark as a product win. Do not hide a neutral or
negative quickener if Q3's direct path wins; that is precisely the ownership
question this activity must answer.

## Required deliverables

Produce:

1. `performance/PERF2-02-WORKLIST.md`, complete through the decision gate;
2. an architectural decision record containing the semantic-invariant matrix,
   site table, Q0-Q7 comparison, Ultra-derived companion-strategy ledger and
   chosen/rejected reasoning;
3. isolated prototype patches/build provenance, clearly separated from any
   selected production source;
4. focused correctness results for references, value copying, execution-image
   lifecycle, TRACE/source, signal/unwind, late load, re-entry and both VMs;
5. compact raw/summary evidence for ceiling, target, guards, startup, RSS and
   state/image cost;
6. a recommendation choosing exactly one of:
   - an extensible core private quickener plus named first semantic client is
     the correct architecture;
   - purpose-built guarded private quickening is the correct owner and name the
     form;
   - static/compiler/RXAS placement is the correct owner;
   - direct value/reference helper work belongs first in PERF2-07/PERF2-06;
   - evidence is inconclusive and state the smallest next experiment; or
   - reject the candidate with retained reasons; and
7. a proposed first production slice only if the evidence supports one. The
   proposal is not authorization to implement it.

Update the live roadmap with the bounded PoC result, including negative or
owner-reassigned outcomes. Do not mark PERF2-02 `complete` unless its stated
design-and-PoC exit is met. Do not rewrite the accepted PERF2-01 bundle,
historical charter or closed initial roadmap.

## Mandatory stop

Present Adrian with:

- the exact stable fact at each selected site;
- the measured Q0-Q7 disposition and machine-level ceiling;
- target/guard results for both VMs;
- semantic, invalidation, lifecycle, RSS and image-state trade-offs;
- whether a reusable core quickener, a purpose-built quickener, static
  placement or direct value/reference work is the fastest correct owner; and
- the smallest proposed production slice, if any.

Then stop for Adrian's architecture selection. Do not turn the winning PoC into
production, do not begin broad closeout, and do not commit or push unless
explicitly asked.

If Adrian later approves a production slice, that separate implementation must
follow the mandatory first ordinary profiling-off Release verdict in
`performance/AGENTS.md`: minimum focused correctness, freeze implementation,
smallest decisive end-to-end comparison against retained valid baseline, report
the verdict and stop again before broad validation or cleanup.

---

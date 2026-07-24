# PERF2-02 semantic quickening architecture record

Status: bounded PoC complete; architecture decision required

Decision boundary: this record may recommend an architecture and a smallest
production slice, but it cannot select or install one without Adrian's explicit
decision.

## Question and accepted mechanism footprint

For each exact PERF2-01-selected site, identify the stable semantic fact, its
earliest safe owner, and whether persistent guarded runtime state beats the
best direct/static form after startup, memory, fallback and lifecycle costs.

Accepted aggregate orientation:

| Family | Accepted footprint | Initial owner question |
| --- | --- | --- |
| Bounce reference storage | about 263 ms in `MKREF_REG_REG`; reference-tree storage owns more than four fifths of stable native samples | Is any site fact narrower and more stable than the current owner-kind/reference-identity search, and can an earlier/direct form consume it? |
| Richards value copy | 96.1 million general copy operations, 762.8 MB, about 0.84-0.85 s in `COPY_REG_REG` | Which executed sites are safely scalar/reference-shaped, and does a direct guarded helper beat persistent site state? |

The exact module/procedure/instruction coordinates and shape distributions are
not inferred from these aggregates; they are filled only from disassembly,
schema-5 rows and handler/helper traces below.

## Site table

| ID | Family | Source -> procedure | Module / canonical instruction | Dynamic count | Observed shape/variability | Cost | Likely earliest owner | Status |
| --- | --- | --- | --- | ---: | --- | ---: | --- | --- |
| A-LOCAL | Bounce | `benchmark_awfy_bounce_opt` -> `awfy_bounce.bouncebenchmark.run()` line 92 | `mkref r10,r2`, canonical word `791` / `0x317` | 10,000 | `r2` is the current frame's base local `random`; the target/cell still changes with lifetime | part of 262.935/263.916 ms family total | direct current-frame-local guard | Q3 direct classification |
| A-ATTR | Bounce | same `run()` line 98 | `minlinkattr1 r14,r0,r6,0` at `839` / `0x347` then `mkref r7,r14` at `844` / `0x34c` | 500,000 | `balls` is a current-frame base local; the selected physical child cycles over 100 values/cells | dominant part of 262.935/263.916 ms family total | eager instruction-pair descriptor plus live unlinked-slot guard | leading Q3/Q4/Q5/Q7 panel |
| B-OBJECT | Richards | `benchmark_awfy_richards_opt`; chiefly `schedule`, `runTask`, `queuePacket` | 128 static `COPY_REG_REG` sites; dominant words `0x1ad9`, `0x1ae3`, `0x0bc6`, `0x10a1`, `0x0eb2`, `0x0ab8`, `0x0ac9`, `0x0b25`, `0x0b2f`, `0x0bb3` | 648,416 | compiler-generated inline receiver capture/bind/copyback; current receiver is a 27-attribute object tree | 84.21%/85.15% of selected profiled instruction time | compiler/inliner before materialisation | Q1 static alias/elision control; runtime quickening cannot remove a required copy |
| B-INT | Richards | same optimized module | remaining statically integer copy sites | 32,547 | plain integer result copies, only 4.8% of top-level copies | small minority of family total | compiler typed-copy selection | Q1/Q3 weak ceiling only |
| B-ARGV | Richards | implicit main entry | one general copy | 1 | `.string[]` command arguments | immaterial | canonical path | excluded from specialization |

## Semantic-invariant matrix

This is the required proof matrix before implementation. `Unknown` is an
explicit blocker, not permission to assume stability.

| Family / invariant | Compile-time fact | Load/preparation fact | Execution-only fact | Mutation/invalidation | Smallest legal guard | Exact fallback / proof obligation |
| --- | --- | --- | --- | --- | --- | --- |
| Bounce register/storage class | A-LOCAL's operand is an authored own local; A-ATTR's source is the immediately preceding `MINLINKATTR1` destination | A-ATTR descriptor is `{root=r0,index=r6,offset=0,target=r14}`; no runtime pointer is retained | Active `locals[]` mapping and linked-versus-physical child decide whether current-frame ownership is true | frame activation/reuse, `LINK*`/`UNLINK*`, attribute resize/delete/trim/move, late lifetime end | A-LOCAL: source register is a base own local. A-ATTR: root is a base own local and live plus unlinked child both equal target | execute canonical `MKREF_REG_REG`; never retain a target/cell/frame pointer |
| Bounce owner frame/kind | A-LOCAL proves `LOCAL`; A-ATTR pair proves only a candidate physical-child route | Procedure local count and canonical operands are stable | Only live base mapping plus unlinked-slot equality proves current-frame ownership | caller arguments, external links, nested attributes, return/unwind and recycled frames | exact guards below; on hit mark `current_frame`, on miss rediscover | canonical owner-kind scan and recursive parent/value-tree search; escaped reference invalidates at its true owner lifetime end |
| Bounce reference-cell identity | No persistent C cell identity may be serialized or assumed from a register site | A process-local site may reserve state but cannot learn execution identity | Existing cell for current storage identity is discoverable at runtime | `ENDLIFE`, frame cleanup, nested attribute invalidation, storage reuse | target storage identity plus valid cell/lifetime token, if such a token exists | canonical identity lookup/create and failure path; never keep a stale cell across recycled storage |
| Bounce attribute route | A-ATTR is the adjacent `MINLINKATTR1` destination consumed by `MKREF` | Root/index/offset and canonical pair identity are known | Receiver attributes may be linked, nested or caller-owned | link/unlink attribute, object/value replacement, lifetime end | root base-local plus live and unlinked child equality | canonical reference-owner/tree helper semantics including nested invalidation |
| Richards scalar/status-only copy | 32,547 top-level sites are source-typed `.int`; typed `ICOPY` is a compiler-owned weak ceiling | Operand indexes alone do not add proof | Current source/destination shape is needed for a generic runtime scalar guard | any value write/materialization/link, destination reuse | complete no-owned-payload/no-attributes/no-reference/native predicate plus alias safety | `copy_value(dest, src)`; direct path preserves identity, status, scalar fields and zero-length state |
| Richards established reference copy | Static proof may know reference-typed flow but runtime reference validity/identity remains dynamic | No execution-only reference cell may be learned during prepare | source reference payload/cell and destination ownership are current runtime facts | `ENDLIFE`, clear/overwrite, frame invalidation/recycle | reference-kind/validity plus destination-cleanup condition defined by helper semantics | canonical retain/release/copy behavior and `REFERENCE_INVALID` contract |
| Richards strings/decimals/binaries | Typed flow can sometimes choose existing typed instructions | Constants/operand types can be decoded | buffer length/capacity, decimal backend and native ops are runtime state | materialization, numeric context, buffer replacement, native finalization | only a family-specific helper predicate that preserves owned-copy semantics | canonical deep copy/provider hook; raw struct assignment is forbidden |
| Richards objects/attributes | Static concrete type does not prove empty attributes or non-recursion | Graph type binding may be known | current attribute tree, linked slots and nested payloads are runtime facts | attribute link/unlink/resize/replacement and lifetime end | exact no-attributes/non-owning predicate, or no fast path | recursive `copy_value`; preserve object type, initialization state and nested ownership |
| Richards destination cleanup/alias | Def-use/effects may prove dead/empty destination or identical source/destination at selected sites | register identity can prove same encoded register, not active storage alias | active pointers, linked attributes and owned destination payload are runtime facts | mapping/link changes and any prior destination value | pointer alias check plus complete destination-owning predicate | canonical same-storage handling and clear-before-copy ordering; failure must leave legal state |
| Both families: TRACE/signal/profile identity | Canonical instruction and source metadata are fixed | module/canonical coordinate is known | mode/pending signal and nested execution state are runtime facts | TRACE/debug/profile mode change, signal/unwind, late load/link generation | specialization must retire as the same canonical instruction; dequick only if its body removes an observable boundary | canonical instruction path with unchanged poll, exception address, source/RXSEQ/profile identity |
| Both families: context lifecycle/concurrency | No single-writer assumption is accepted without code proof | state allocation/publication can be prepared atomically | nested RXVML/re-entry and repeated runs may observe state | late load/link, prepare, repeated run, teardown and any parallel host use | process-context ownership plus proved publication protocol | absence/failure/invalid state always runs canonical path; teardown owns all private memory |

### Exact Bounce guards and completion order

A-LOCAL may bypass discovery only when `source_reg < procedure->locals` and
`locals[source_reg] == baselocals[source_reg] == source`. A-ATTR additionally
requires checked one-based index arithmetic, `root_reg < procedure->locals`,
`locals[root_reg] == baselocals[root_reg]`, an in-range live child, and both
`root->attributes[index-1] == source` and
`root->unlinked_attributes[index-1] == source`. The second equality prevents a
linked external slot from being misclassified as current-frame-owned.

After either guard succeeds, completion still resolves/reuses the canonical
reference identity, reports allocation failure before any destination or frame
mutation, marks `current_frame->has_reference_lifetimes`, clears destination
contents, and retains the payload. Any guard failure runs the unchanged generic
owner scan and recursive lifetime-owner search. Arguments, globals, caller-owned
or nested external storage and same-register cases therefore remain canonical.

### Richards static proof boundary

RXSEQ source kinds separate all 680,964 top-level general copies into 648,416
objects, 32,547 integers and one argument array. The recursive profile expands
these into 95,367,992 `copy_value` visits: 44,120,477 empty, 39,576,232 scalar,
11,671,282 object and one string. The object work is generated by inline method
receiver capture/bind/copyback, not runtime polymorphism. A Q1 alias/elision PoC
must prove a direct non-attribute receiver evaluated once, ordinary argument
capture order, no receiver rebind/destruction/escape mismatch, protected
register lifetime, nested-inline identity and correct TRACE/source provenance.
Computed/locator receivers, reference overlap or any uncertain exit behavior
fail closed to current capture/copyback.

## Evidence coverage and telemetry disposition

Schema 5 is sufficient for accepted aggregate timing, instruction counts,
recursive value shapes, native samples and RXSEQ site/source identity. It does
not provide per-site recursive `COPY` shape or reference owner/tree depth. RXSEQ
made a new product-visible schema extension unnecessary: Richards source kinds
prove its receiver-copy origin, while Bounce disassembly and handler semantics
prove the minimal guard. PoC hit/miss counters are retained from build-private,
profiling-off diagnostic runs and were not present in timed product binaries.

Existing dual-VM reference coverage includes local/global/caller/dead
lifetimes, attribute delete/shrink/trim, cell reuse, copy/move identity, signals,
profile/RXSEQ identity and six compiler reference fixtures. New PoC fixtures
must cover an owned unlinked-attribute hit, `LINKTOATTR` external miss, caller
argument/nested attribute miss, repeated recycled frames/cells and both modes.
Richards Q1 must retain the existing direct/computed receiver, mutating scalar
return, nested call, writable/ref argument and opt/noopt tests, adding RXAS
absence checks only for proved direct-receiver shapes.

## Q0-Q7 design-selection table

These are design candidates, not implementation decisions. A form is pruned
only with a recorded semantic or architectural reason.

| ID | Placement | Bounce comparison | Richards comparison | State/lifecycle cost | Pre-PoC disposition |
| --- | --- | --- | --- | --- | --- |
| Q0 | current canonical path | full owner search/reference identity/lifetime semantics | full recursive `copy_value` semantics | no new state | mandatory baseline/fallback |
| Q1 | compiler-owned result-only/static lowering | local/attribute route classification can be emitted, but A-ATTR still needs live mapping guards | leading Richards ceiling: direct proven receiver aliases caller storage; 32,547 integer copies can use typed copy | no runtime site state; compiler proof and register-lifetime cost | Richards control required; fail closed outside direct proved receivers |
| Q2 | existing authored RXAS/static rule | current public operations cannot express lifetime-owner proof without generic `MKREF` | temporary `LINK`/`UNLINK` aliasing would require restoration on every return/signal/unwind edge | no learned state but extra public instructions | pruned: Q1 direct symbol remap is earlier and safer; no new RXAS form allowed |
| Q3 | direct private helper/handler form | Q3a scans one owner level as a broad ceiling; Q3b decodes the immediate canonical predecessor on each hit and applies the exact local/physical-child guards with canonical completion/fallback | runtime scalar guard is a weak ceiling only; it cannot remove receiver copies | code only; no allocation/state invalidation | mandatory Bounce machine/direct controls |
| Q4 | eager process-image specialization | preparation recognizes the canonical pair and patches a private handler; operands remain canonical and every hit revalidates ownership | load-time metadata cannot prove object shape or make a semantically required receiver copy disappear | existing image opcode/handler overlay only; no new state | viable Bounce one-off comparator |
| Q5 | lazy guarded first-hit specialization | first generic success may patch the same private handler, but learns no fact beyond Q4's static pair | a generic COPY candidate can specialize only proven plain-scalar executions and disables on first incompatible shape | first-hit image mutation; no target/frame/cell retention | implement only as Q7 state-policy comparison; standalone Bounce Q5 is predicted redundant |
| Q6 | threshold/tiered specialization | no durable execution-only fact exists; a threshold merely delays the Q4 handler | object receiver copies stay required regardless of repetition | counter/branch with no semantic benefit | pruned before code: Q3/Q4 guards already distinguish every legal hit |
| Q7 | extensible core private quickener substrate | stable module-owned record plus operand overlay selects the same guarded reference primitive | candidate COPY record may specialize complete plain-scalar shape, otherwise restores canonical execution | shared record/header, prepare/replay/disable/teardown tax versus zero-state Q4 | first-class measured option; serve reference and scalar-copy families without a generic callback on hit |

## Companion-strategy ledger

Each companion remains independently switchable and measurable. These IDs are
stable within PERF2-02; recording an idea does not approve another roadmap
activity.

| ID | Hypothesis | Semantic proof obligation | Intended comparison | Status |
| --- | --- | --- | --- | --- |
| C1 `STATIC-NARROW` | Existing compiler/RXAS effects can emit or privately convey a narrower site class so runtime guards fewer facts. | The classification must be implied by existing semantics and require no public format/syntax change; false positives must be impossible. | Q3/Q5 with and without narrowed classification; otherwise route to PERF2-03/05. | inspect |
| C2 `LOAD-DECODE` | Eager private operand/procedure decoding can remove repeated indexing independent of learned semantics. | Decoded data is valid for current process binding and refreshes after late link without claiming runtime value shape. | Q0/Q3 versus Q4; otherwise route to PERF2-06. | inspect |
| C3 `MONO-DISABLE` | A compact cold -> monomorphic -> disabled state avoids polymorphic replacement machinery at Bounce/Richards sites. | Every mismatch executes canonical semantics; disabling cannot suppress later valid behavior; reset/invalidation is defined. | Q5 versus Q6 using identical fast helper. | inspect |
| C4 `LIFETIME-EPOCH` | A frame/storage lifetime epoch can validate cached reference ownership more cheaply than repeated identity-tree search. | Epoch identity cannot alias after wrap/reuse; caller-owned and nested attributes advance the correct owner; failure/dequick is atomic. | Bounce Q5 with guard versus epoch-assisted guard. | high-risk inspect; no implementation before code proof |
| C5 `SHARED-PRIMITIVE` | One family-specific inline primitive per semantic family, shared by both dispatch modes, avoids duplicated semantics without an indirect call on hits. | Macro/static-inline expansion must preserve each dispatch mode's interrupt/TRACE/profile boundary and exact helper semantics. | Purpose-built Q3/Q5 and Q7 using identical primitive. | inspect |
| C6 `SIDE-INDEX` | A canonical `(module, instruction index)` dense index can make Q7 lookup free after handler entry. | Mapping remains exact across prepared/canonical images, late refresh and both VMs; no hash/tree/callback appears on a hit. | Q7 dense side table versus purpose-built embedded pointer/index. | inspect |
| C7 `FAIL-CLOSED-PUBLISH` | Allocate/fill then atomically publish one immutable family payload; failure needs no rollback and leaves Q0 executable. | Actual context concurrency/re-entry model must justify publication primitive and teardown order; partial state is unreachable. | Q7 lifecycle/failure fixture and startup cost. | inspect |
| C8 `DEQUICK-ON-MODE` | Debug/TRACE/profiling mode changes can select canonical handlers/state without contaminating hot ordinary mode. | Mode transitions and nested runs cannot retain an observably fused or misidentified instruction; RXSEQ/profile keep canonical identity. | Q5/Q7 ordinary versus mode-transition fixtures. | inspect |

## Q7 bounded substrate design

The runtime key is `(module instance, canonical expanded instruction-word
index)`; persisted diagnostics would also need canonical module content
identity. Canonical RXBIN remains immutable. The bounded PoC's module-owned
record contains `family`, `state`, flags/counter and full-width canonical
operands; it does not implement an epoch or a general invalidation mechanism.
An eligible execution-image instruction is patched to a direct `run()` label
in `rxvm` or a private internal switch case in `rxbvm`. An operand cell points
at the record only while preserving the operands needed for canonical fallback.
No public opcode, format, callback or all-instruction quickening branch is
introduced.

The PoC state flow is `CANDIDATE -> SPECIALIZED_MONO -> DISABLED`; plan
construction is its cold phase. Bounce's static pair publishes eagerly. COPY
candidates run generic semantics on their first hit, publish plain-scalar
specialization only after successful completion, and restore the canonical
image form on incompatible shape. The successful path demonstrates
allocate-fill-publish/free and generic-before-disable ordering. A production
form would additionally have to prove that catchable failure and OOM leave Q0
reachable and failure-atomic; the PoC has no allocation-failure injection, so
it does not claim that proof. Exact record and module totals are measured.

Execution images persist across repeated runs while frames do not.
`prepare_only` may construct only static candidates and cannot learn execution
facts. A production substrate would have to replay borrowed operands after
late load and ensure newly loaded modules do not infer readiness from a non-null
image. The PoC frees Q7 records in ordinary teardown, but does not complete
overlay-replay, repeated embedded-run or callback re-entry fixtures. It retains
no frame, `value`, reference cell or handler address. If a future design adds a
semantic epoch, wrap must reset all records; this PoC has no epoch or wrap path.

Current supported execution is context-confined, including same-thread nested
RXVML entry. Process-global active-context, interrupt and signal state mean
atomics in Q7 would not make concurrent use of one context safe; the PoC records
that constraint but does not complete the embedded repeat/re-entry fixture.
Publication writes the complete stable record first and patches the image last.

Every specialized handler retains one canonical instruction-begin/retire,
interrupt poll, signal address and source/RXSEQ/profile identity. Profile
call-window code can decode execution-image operands, so borrowed operands make
canonical pointer use or dequick mandatory in profiling. Any future fusion must
dequick under TRACE/profiling; this PoC does not fuse instructions.

Material lifecycle coverage gaps are retained as blockers to production: no
overlay-replay late-load test, no `rxbvml` repeated/re-entry fixture, no nested
native-callback RXVML test, no allocation-failure injection, no epoch-wrap
fixture, incomplete dual-mode TRACE capture, and no documented concurrency
contract. The bounded PoC may measure startup/state and focused teardown, but
cannot claim these gaps solved.

## Analysis gate

The site coordinates/distributions, helper semantics, mutation/invalidation
paths, actual context confinement, focused tests and schema-5 disposition above
close the analysis gate. The first implementation is the exact Q3
machine/direct control. Q4 follows only if Q3 is correct; Q7 is compared with
identical reference completion/guards. Q6 is pruned. Q1 Richards work remains a
separate static-placement control and may not broaden language architecture.

## Measured Q0-Q7 disposition

All target medians below are ordinary profiling-off Release wall clock. The
final Q0/Q3b/Q4 block used the same accepted Bounce image/library, separate
VM-balanced groups, two warmups and 12 recorded rounds. Full raw evidence and
startup/RSS/image tables are in
[`measurements.md`](evidence/2026-07-23-perf2-02-quickening-poc/measurements.md).

| ID | Implemented comparison | Measured result | Final disposition |
| --- | --- | --- | --- |
| Q0 | canonical owner/tree discovery and recursive copy | Bounce 6.673949/7.004614 s; Richards 11.643875/11.778197 s (`rxvm`/`rxbvm`) | retained unchanged as fallback |
| Q1 | Richards direct receiver capture fold | 8.813834/8.956049 s, 24.305%/23.961% lower elapsed; focused inline/copyback suite 33/33 | compiler/inliner owns this removable Richards work; successor evidence, not a quickener client |
| Q2 | current public RXAS composition | cannot express reference owner/lifetime proof without extra observable restoration edges; direct compiler remap is earlier for Richards | pruned; no public form proposed |
| Q3a | zero-state broad one-level direct owner helper | 1.495130/1.727522 s; large machine ceiling, but not an identical-route Q4 control | superseded by exact Q3b |
| Q3b | zero-state canonical MKREF handler with exact Q4 guards | 1.317381/1.505788 s; Q0/Q3b 5.066075x/4.651793x; zero persistent state; Q0-identical RXSEQ | winner |
| Q4 | eager execution-image private handlers with the same target guards | 1.425490/1.502132 s; Q3b is 7.584% faster in `rxvm`; Q4's 0.243% `rxbvm` edge is inside noise | rejected: no benefit over the best direct form and larger code/layout sensitivity |
| Q5 | lazy first-hit policy represented by Q7 COPY client | Bounce learns no execution-only fact; Richards object candidates disable after canonical copy | pruned: first hit cannot make required object work disappear |
| Q6 | threshold/tiered policy | no durable learned fact; adds counter/branch before the same proof | pruned before code with recorded semantic reason |
| Q7 | module-owned core records with eager reference and lazy scalar-copy clients | Bounce tied Q4; Richards -0.120%/+0.199%; 56,264/62,536 requested state bytes; lifecycle gaps remain | rejected: abstraction/state/lifecycle tax is unjustified |

The Q3b/Q4 mode result is not averaged away. Q3b wins decisively in the
direct-threaded VM and is tied in the switch VM, while adding less text and no
execution-image lifecycle. It therefore satisfies the both-mode placement
test. Q4's ten diagnostic guard profiles show zero MKREF execution across
Sieve, Permute, Storage, Towers and Base64, but its unexecuted code shape still
produces a repeatable 4-5% `rxvm` Sieve penalty. Q3b reduces that to about 1%.

Separate counter builds make the target routing explicit. For both Q3b and Q4
and both VMs, short Bounce records 5,100/5,100 exact hits: 100 at A-LOCAL word
791 and 5,000 at A-ATTR word 844, with zero fallback. The guard fixture records
one direct-local hit and two canonical fallbacks, including the linked-external
case. Its owned-child case also falls back because an intervening `LOAD` breaks
the required adjacent pair; it is lifetime/fallback coverage, not the A-ATTR hit
proof. Q3b/Q4 invalidation is not applicable because neither form retains a
learned fact.

### State and lifecycle decision

Q3b and Q4 add zero persistent heap/module/site bytes and retain the existing
eight-byte-cell execution image. Q3b adds 16 transient stack bytes to `run()` in
each VM. Q7 allocates a 56-byte record for every COPY/MKREF site plus 24 bytes
for every loaded module: 943 records and 144 modules for Bounce, 1,055 and 144
for Richards. Its requested totals exclude allocator overhead and require 21
record-array allocations.

Q3b has no publication, invalidation or teardown state: its guard re-proves the
fact on every execution and every miss follows Q0. Dynamic-load, prepare/re-entry,
reference/lifetime, signal/unwind, TRACE/source, profile and Q0-identical RXSEQ
coverage passes. Q7 still lacks overlay-replay late-load, repeated embedded
run/re-entry, callback re-entry, OOM injection, epoch wrap, full TRACE-mode and
concurrency-contract proof. Those are retained rejection reasons, not deferred
assumptions.

Q7's separate transition diagnostic observes exactly one cold COPY
specialization, one specialized fallback/dequicken and one later disabled
canonical execution in both VMs. On one-repetition Richards it records 8 cold
specializations, 62 first-hit disables, 32,539 fast hits and 648,355 later
disabled/canonical executions per VM. Twelve fresh-process samples bound gross
first-specialize means at 116.319 ns/event (`rxvm`) and 18.663 ns/event
(`rxbvm`), but timer overhead makes the steady readings too perturbed for a
ratio or verdict. Q7 implements no general invalidation or reference-dequick
mechanism; that is N/A in the counter output and remains a rejection reason.

## Companion outcomes

| ID | Outcome |
| --- | --- |
| C1 `STATIC-NARROW` | Q1 proves compiler ownership for Richards and removes 11 static general copies; it does not help Bounce's live storage proof. |
| C2 `LOAD-DECODE` | Final controlled comparison: Q3b re-decodes the exact route and beats/ties eager Q4. Load-time handler selection is unnecessary. |
| C3 `MONO-DISABLE` | Q7's real COPY client disables at Richards object sites and is neutral; no reusable value is demonstrated. |
| C4 `LIFETIME-EPOCH` | Not implemented: per-execution equality guards already win without cached identity, while wrap/reuse proof would add risk. |
| C5 `SHARED-PRIMITIVE` | The selected canonical handler already supplies one source-level semantic body to both builds; a production refactor may factor completion, but it is not a separate performance mechanism. |
| C6 `SIDE-INDEX` | Q7's borrowed operand-cell pointer removes generic lookup on hit but eagerly records 943/1,055 sites; the saved lookup does not repay state. |
| C7 `FAIL-CLOSED-PUBLISH` | Basic allocate-fill-publish/free is demonstrated, but unresolved lifecycle/concurrency fixtures prevent advancement. |
| C8 `DEQUICK-ON-MODE` | No fusion is present. Fresh Q0/Q3b/Q4 RXSEQ files are byte-identical in both VMs; direct Q3b needs no dequick mode. |

## Placement recommendation and mandatory stop

The one permitted PERF2-02 disposition is:

**direct value/reference helper work belongs first in PERF2-07/PERF2-06**.

At A-LOCAL and A-ATTR, the fastest correct owner is the canonical
`MKREF_REG_REG` reference path itself: an exact current-frame-local guard and an
exact adjacent-`MINLINKATTR1` physical-child guard, followed by unchanged
reference identity, failure ordering, lifetime marking and generic fallback.
No process-private site memory is needed. At B-OBJECT, runtime quickening is the
wrong owner; compiler receiver capture is the earlier proven fact. B-INT is too
small to justify a runtime campaign here.

The evidence supports one smallest proposed production slice, subject to
Adrian's approval:

1. add only Q3b's exact A-LOCAL/A-ATTR guarded owner classification to the
   canonical MKREF implementation shared by both VM builds;
2. retain the current canonical instruction, public RXAS/RXBIN/ABI, owner/tree
   fallback, identity allocation, lifetime, signal, TRACE/profile/RXSEQ and
   teardown paths;
3. add the isolated guard fixture as a repository test and keep zero persistent
   state; and
4. after approval and implementation, run the mandatory smallest first
   profiling-off Release verdict before any broad closeout.

This proposal is not authorization to implement it. PERF2-02 stops here for
Adrian's architecture selection. No production source, stage, commit or push
follows this record.

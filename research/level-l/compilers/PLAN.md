# Complete Classic C compiler in B and Level L

**Complete C is open. Current: CLC-STEP05.ASSIGN3, qualified local closeout of
the existing C v3 assignment path. Next: LC-I-04 NOP review; no implementation
has started.**

Deliver one explicitly selected Classic C compiler and reusable compile service,
written in B using Level L parsing, IR, transformation and emission. Complete
the agreed instructions, all 95 BIF names, Unicode, diagnostics, conditions/TRACE,
host/lifecycle/platform contracts and, last, INTERPRET. Native B is the build
seed; new C requests never delegate to a native source compiler/backend.
Existing RXAS/RXLINK, VM and native host transport remain execution dependencies.

The [requirements ledger](doc/levelc-requirements.md) owns item-by-item status:
25 instructions, 95 BIFs, 75 reference capabilities and 36 syntax contracts,
with reuse, missing integration, owners and closure checks. Native passes and
existing B bodies are controls, not new-C passes. [Architecture](doc/architecture.md)
owns contracts; [work packages](doc/agent-work-packages.md) own assignments.
Astra reviews/coordinates; Sol 6.1 Extra High workers implement, build and integrate.

## Current action: assignment

Qualify [LC-I-03's existing one-write path](doc/LEVELC-INSTRUCTION-REVIEW.md#assignment-one-write-after-rhs-evaluation)
against LC-AC-54/63: evaluate the RHS once, then let the current shared pool
substitute/classify the raw target and store its exact value. Empty RHS assigns
empty text. Current C v3/schema, target exports and shared B algorithms stay
unchanged; no lowering defect has been reproduced. Add independent proof and
repair product code only for a demonstrated failure. Native C is read-only.

| ID / status | Checkable closure |
|---|---|
| **ASSIGN-AC01 — Qualified** | Source/AST/IR and independent execution cover scalar overwrite, keyword targets, empty RHS, stems/arbitrary compound targets, numeric/unset/empty tail components, substituted case, main/IF/group/private contexts and Unicode/NUL/high-ordinal values/tails. Stem assignment resets explicit/dropped tails; tail assignment clears its tombstone. |
| **ASSIGN-AC02 — Qualified** | A source marker call and old-value reads prove once-only RHS-before-write behavior; IR/RXAS places RHS call/read before the sole raw-name store, with no compiler tail pre-read. Complementary exported-IR/B-host mutation-before-store proves resolution uses current tail bindings; this is composed proof, not a source EXPOSE call. |
| **ASSIGN-AC03 — Qualified** | Prebound-context execution proves live scalar/stem/exact/nested aliases, private reset isolation and preservation of unrelated case-distinct tails, exact Unicode/NUL text and RexxValue caches through load/store without conversion. |
| **ASSIGN-AC04 — Qualified** | Invalid targets with explicit/empty RHS and malformed dead source refuse with logical located diagnostics and prior output intact. Fatal RHS failure cannot reach the store/later output. Actual caller/helper RXAS has one pool write, no unused explicit-RHS default/splitter/conversion, preserved owners/source; focused normal Debug passes both compiler/IR modes and VMs. |

**CLC-STEP05.ASSIGN1 — Reviewed (AC01–04):** freeze independent expectations and
inspect existing source/IR/shared-pool contracts; depends on committed DROP.
**CLC-STEP05.ASSIGN2 — Implemented (AC01–04):** fixtures, composed context proof and
structural guards; any reproduced repair needs coordinated source ownership.
**CLC-STEP05.ASSIGN3 — Qualified (AC01–04):** sole builder qualifies focused normal
Debug with measured serial scheduling; independent source/RXAS review precedes
local commit. Reuse unchanged grammar/tools/runtime evidence; a commit alone
does not require expensive grammar regeneration or a source-slice ASan run.

Whole LC-I-03 remains open for source EXPOSE/shared-label contexts (STEP06),
exact Classic diagnostics (STEP04), resumed conditions/NOVALUE and full
clause/TRACE observations (STEP07). Ordinary private calls have fresh pools;
the composed proof must not claim caller-pool mutation through those calls.

## Completed DROP increment

Local commit `fccb95b46dc4cfd75a453c0573fba27a78d5ff3b` qualified the
[approved C v3 DROP/name contract](doc/architecture.md#proposed-next-instruction-drop)
with independent source/RXAS review and [normal Debug proof](#current-drop-proof).
No remote publication or whole LC-I-02 closure is claimed; source EXPOSE,
conditions/full diagnostics and TRACE keep their later owners.

| Stable ID / status | Qualified closure |
|---|---|
| **DROP-AC01 — Qualified** | Raw Unicode symbols, ordered CDrop spans/items and exact v3/name admission; dead-source, reserved-dot-prefix and routine-profile refusals. |
| **DROP-AC02 — Qualified** | Direct/indirect order, one captured list, scalar/stem/compound states, defaults/tombstones and self-carrier cases. |
| **DROP-AC03 — Qualified** | Unicode/configured blanks, micro-sign/Greek identity, ARG names, aliases/private pools and invalid-word continuation. |
| **DROP-AC04 — Qualified** | Existing pool calls and caller/helper owners/source in actual RXAS; focused compiler/IR/assembler/VM Debug proof. |
| **CLC-STEP05.DROP1–3 — Complete** | Shared symbols/AST, ordered lowering and independent local qualification completed before ASSIGN1. |

<a id="active-first-implementation-increment"></a>
<a id="shared-b-unicode-interface-and-c-adapter-migration"></a>
<a id="active-c-contract-baseline-increment"></a>
<a id="active-borrowed-classic-context-increment"></a>

## Completed foundations and qualified C ABI

Bounded independent C expressions/control/private calls and LENGTH/ARG/DIGITS,
generic L services and shared B Unicode are foundations, not whole-instruction
closure. **CTX-AC01–06 / CTX-STEP01–04 are locally complete**, including borrowed
B owners and two reproduced B seed repairs; baseline commits are
`ff63527ae94b1d91acb7ba01025f2e197cf522bc` and
`eeca738af1c2f3b6dc826f087d398f6400b941ba`.

STEP03d now has the [reviewed clean C v2/context ABI](doc/architecture.md#approved-c-family-revision-and-generated-procedure-abi)
and normal Debug proof below. Adrian removed research-C backward compatibility:
one current C family/target, no old-image bridge or copied-context adapters.
Shared B/L contracts remain intact. Local commit
`62d6dd44df528d56f4ca191bdc28d73b960c9432` closed this slice before DROP began.

| Stable ID / status | Checkable closure and owning contract |
|---|---|
| **CABI-AC01 — Qualified** | Exact v2/40-op schema, required context reference, save/load, canonical payloads and entry-bound context slots; malformed/dead records refuse with immutable graph/empty output ([C admission](tests/c-context-contracts.crexx)). |
| **CABI-AC02 — Qualified** | One root config, stable borrowed root/child owners, live pool, call inheritance/presence/results; direct `target-context` plus STEP07 argv/ARG/call/recursion controls. |
| **CABI-AC03 — Qualified** | Same-frame Classic DIGITS/FUZZ/FORM install/restore; invalid NUMERIC has fatal diversion without installs, composed with direct adapter atomicity and numeric/fatal RXAS checks. |
| **CABI-AC04 — Qualified** | One 23-export `llctarget`; obsolete namespace/bodies/compatibility tests removed, BIF bodies/shared contracts/doc tags preserved ([target shape](tests/check-target-context.py)). |
| **CABI-AC05 — Qualified** | Exact runtime/class/receiver/optional signature admission, owner-hit/fresh-request metadata refusal and sentinel preservation; no native compiler fallback or new public L/VM API ([C matrix](tests/check-step07.py)). |
| **CABI-AC06 — Qualified** | Caller/helper source/owner/ref/numeric/RXAS guards, both compiler/IR modes and VMs, restricted C-only roots and measured serial normal Debug scheduling. Sanitizer boundary is stated once below. |
| **CLC-STEP03d.1–3 — Complete** | Approved contract freeze, implementation and independent fixtures/build/Debug measurement; architecture and maintained regression sources own the exact checks. |
| **CLC-STEP03d.4 — Complete** | Independent source/RXAS review, final normal Debug proof and coordinator-reviewed local commit completed before DROP started. |
| **META-AC01 — Complete** | Real linked constructor and independent merged definition/prototype fixture select only definition-linked metadata ([services](../../../langtools/tests/services.crexx)). |
| **META-AC02 — Complete** | Duplicate definitions remain ambiguous; private/missing/malformed links/envelopes refuse. Selected references stay in their pool/1,000,000-ID bound; ignored prototype signature fields stay opaque. |
| **META-AC03 / CLC-STEP03d.P — Complete** | Reader API/format/type ownership unchanged; separate reviewed six-path prerequisite commit `9c632093f6ab97e819ad9e48d3a6b379d88afba0`, independent services and real C consumer pass. |

<a id="remaining-actions"></a>

## Remaining delivery steps

The steps below describe remaining work, not a requirement to finish a universal
framework before its first consumer. Deliver the minimum shared service needed
by each reviewed instruction; complete its wider contract before closing that
step. Numeric, error and source foundations are needed early even though full
NUMERIC, SIGNAL and TRACE instruction reviews occur later.

| Step / current status | Remaining action and closure check | Dependencies / approval gate |
|---|---|---|
| **CLC-STEP01 — Partial** | Finish contract-to-test traceability, exact ABI/dependency/platform decisions and independent expectations. Every required ledger row has an explicit disposition and closure check. Serves CFC-AC01/06–08/12. | Accepted C direction; exact language/platform exceptions need Adrian's decision. |
| **CLC-STEP02 — Partial** | Supply registered exceptional/resumable edges, observations, facts/invalidation, liveness and checked rewrites as consumers require. Independent non-C/G tests prove correctness, limits and atomic failure. Serves AC07/10/11. | STEP01 contracts; approve new public L APIs/schemas. Keep block-local values/cross-block slots; no required SSA rewrite. |
| **CLC-STEP03 — Partial** | Complete remaining live activation/pool/context, direct operations/BIF ABI and one-body labels, recursion, EXPOSE, restoration and abrupt paths. Close remaining property/simple-map/traversal/budget contracts only where needed. Serves AC02/03/09/11/12. | Existing Unicode/CTX pieces are complete. Each later extension needs the relevant STEP01/02 contract and exact ABI approval; direct B and new-C proof are distinct. |
| **CLC-STEP04 — Partial** | Complete contextual source/Unicode names, AST/source maps, static errors even in unreachable text, all expressions/numeric semantics and BF01–05 bindings. Independent values, effects, diagnostics and RXAS checks close C-SRC/C-EXPR. Serves AC01–03/06/07/09/11. | Required L/runtime services from STEP02/03; no native frontend dependency. |
| **CLC-STEP05 — Partial; current assignment** | Review LC-I-01–10 individually: SAY through ITERATE. Close every form, error, order, pool/loop effect and source observation in the ledger. Serves AC01–03/06/07. | STEP04 plus each instruction's shared dependencies. Bounded SAY/DROP reviews retain shared dependencies; assignment is current. |
| **CLC-STEP06 — Partial** | Review LC-I-11–19 individually: ARG through PARSE. Complete one PARSE executor, shared labels/fall-through, presence/results, EXPOSE and static signed providers; integrate BF05/06/08 as needed. Serves AC01–04/06/09/11. | Needed STEP03–05 services. ARG may introduce shared PARSE infrastructure before full PARSE review. |
| **CLC-STEP07 — Partial** | Review LC-I-20–24 individually: ADDRESS, commands, NUMERIC, SIGNAL, TRACE. Complete conditions, clause clocks, source/error/TRACE observations and BF07/09. Serves AC01–04/06/09. | Required STEP06 control/activation contracts and host adapters; early foundations do not close these instructions. |
| **CLC-STEP08 — Not started** | Complete remaining BIF/host contracts: exact-length invocation/completion, callback-window pools, queues/streams, clocks/HALT and declared platform adapters. Qualify all static reference capabilities. Serves AC01–04/09/11/12. | Deliver needed adapters alongside earlier consumers; full closure follows STEP04–07. New host APIs and capability exclusions need approval. Reuse existing B bodies. |
| **CLC-STEP09 — Open throughout** | Inspect caller/helper RXAS every increment and fix direct construction/emission defects immediately. Close C-OPT-01–06, register/call-window/allocation obligations and named remaining costs. Serves AC06/07/10/11. | Uses STEP02 facts and explicit ABI/lifetime rules; final closure follows STEP08. New ISA/storage contracts need approval; production performance edits retain their verdict gate. |
| **CLC-STEP10 — Not started** | Close all static ledger rows, source/runtime/host tests, installed/cold/incremental/relocated delivery, declared platforms and independent walkthrough/review. Serves static AC01–04/06–12. | STEP01–09. Static acceptance is not complete C. |
| **CLC-STEP11 — Not started; last** | Implement INTERPRET through the same compiler, then close all CFC criteria and complete installed/platform proof. | STEP10 and exact continuation/code-owner mechanism approval. |

**CLC-STEP11a–d remain separate checkable parts:** 11a proves nested execution,
SYNTAX, CALL resumption and SIGNAL with live continuations and cleanup/limits;
11b implements approved ephemeral code ownership, pins/reclamation, stale handles,
all-pinned failure, rollback and both VMs; 11c connects group compilation,
assembly/loading, live pools/arguments/config, caller dispatch and spills;
11d closes LC-87-01–05, PROCEDURE/outer-loop rules, RETURN/EXIT/SIGNAL,
delayed handlers, source/TRACE, resource limits and installed/platform acceptance.
A file compiler or bounded lookup cache alone does not meet this contract.

## C-first delivery milestone

All **CFC-AC01–12 remain open for complete C**. An item closes only on its full
observable contract; bounded results below do not close whole instructions.

| ID | Observable pass condition and proof |
|---|---|
| **CFC-AC01** | Complete crosswalk of every requirements-ledger row: LC-I-01–25, all 95 BIF names, LC-REF-001–075, 36 syntax contracts, detailed native criteria/gap routing, extensions and approved exceptions. Each links new implementation, independent positive/error expectations and qualification; no omitted or inherited native passes. |
| **CFC-AC02** | Expressions, numeric/string/Unicode values, symbols/stems/pools/EXPOSE, PARSE/ARG, control and calls implement the agreed contract. Test state, order, errors and limits, including defects exposed by native comparisons; each instruction has its own disposition. |
| **CFC-AC03** | Activation/labels, SIGNAL/CALL conditions, RETURN/EXIT, clause clocks, source/TRACE and diagnostic origins survive normal/abrupt/nested paths, Unicode/CRLF/NUL and optimisation. Prior output survives failed compilation. Source identities are logical and stable. |
| **CFC-AC04** | Host invocation/completion, callback-window pool access, ADDRESS/commands, streams/queues, clocks/HALT and signed external calls have positive/error/ownership proof on their declared platforms. Configuration and missing host facilities stay open unless explicitly excluded. |
| **CFC-AC05** | INTERPRET satisfies LC-87-01–05 using the same B/L compiler, live caller state, real resumption/transfers, nested groups and bounded resident-code reclamation. Tests include trapped failures, all-pinned exhaustion, stale handles and teardown; no native/RexxScript fallback. |
| **CFC-AC06** | Representative full RXAS and helper paths have checked structural expectations for simple expressions, loops, calls, errors and dynamic execution. Compiler/RXAS modes are distinct. Direct defects are fixed; necessary cost and later opportunities have owners. Preserve ABI, source, TRACE and numeric observations; static counts are not speed claims. |
| **CFC-AC07** | Generic L services have independent non-C/G consumers, checked structure/effects/facts, bounded failure and documented algorithms. C/BIF/ISA policy stays in owning extensions; source/import/install audits prove no native or typed frontend in the C host closure. |
| **CFC-AC08** | Every required C row closes on qualified inputs, with CLI/service/installed/platform proof, documentation and runnable walkthroughs. Distinguish local, hosted, guest/SDK, sanitizer and release results. Complete C is separate from default replacement and B/G delivery. |
| **CFC-AC09** | B Classic runtime has one implementation of values, pool aliases, activation/context, BIFs and host adapters. C target closure uses the shared B Unicode foundation independently of G aggregation; native C, typed Classic mode and RexxScript retain their contracts. Public metadata/doc tags and consumer regressions prove compatibility. |
| **CFC-AC10** | Any RXAS/VM change has an approved operation/storage contract, exact error/alias/lifetime/source semantics, encoding/disassembly/loader tests and both-engine/platform proof. Alternatives are compared; production performance edits pass the first Release verdict gate. No native source-language implementation is introduced. |
| **CFC-AC11** | Compile/request/result and runtime activation/code lifetimes are isolated, reusable and bounded. Verify exact lengths, atomic failure, stale/foreign references, interruption, repeated/concurrent contexts, declared budgets and recovery without partially published output. |
| **CFC-AC12** | Delivery/build manifests separate native seed, L tools/SDK, C host, C runtime and optional dynamic-compile support. Cold/incremental/installed/relocated builds have one producer and correct freshness; static applications have no compiler dependency. Platform capability rows and source/user/API documentation match actual products. |


New B/G features, proper B self-hosting (LLCOMP-STEP07B) and combined replacement are outside this
delivery, with their unverified outcomes retained in the requirements ledger.
Existing consumers remain regressions. Default switching, native-C retirement,
release approval and resumed CI monitoring require separate direction. Native
Classic source is read-only; demonstrated B seed defects may be repaired with
focused regressions. No native Classic feature implementation is authorised.

<a id="step07a-validation"></a>
<a id="say-validation"></a>

## Current assignment proof

The assignment fixtures on `fccb95b46dc4cfd75a453c0573fba27a78d5ff3b` qualify
the unchanged C v3 store path; independent source and actual RXAS review pass.
Six direct `rxc`/`rxas`/`rxlink` commands prepared
the two B-mode contract images warning-free in 1.9524s, using the qualified
DROP import roots and generated grammar. No native, library, compiler-host or
grammar rebuild was needed. Unchanged product/runtime/name/schema proof is
reused; no fresh sanitizer, installed, hosted or whole-C claim follows.

| Normal Debug command / inspection | Observed result and scope |
|---|---|
| `check-assignment.py --build cmake-build-debug --work <temporary> --vm rxbvm --vm rxtvm` completed components | All 36 positive commands passed: 24 source/AST/IR/provider checks and 20 B-host alias/cache/live-binding assertions per B-mode/VM cell; the independent 23-line source oracle passed both B modes, C IR modes and VMs. All five source refusals passed, including dead malformed RHS in both IR modes and prior-output preservation. |
| Same checker with `--phase fatal` | Ten commands passed in 5.6418s: both IR modes and VMs produce only the prefix and `RXC-LC-42.3`; actual own-frame fatal diversion cannot enter the target store. Compiler host is B opt; linked target is B noopt; assembler optimisation is disabled. |
| Caller/helper inspection | Provider has three value reads and eight ordered raw-name writes without conversion/splitting/owner copies. Source marker/read precedes its sole target write; supplied-context mutation precedes compound resolution. Provider and same-IR source RXAS are byte-identical across B host modes. Shared pool/runtime bodies are unchanged. |
| CTest registration inspection | `llcompiler_c_assignment` defaults to all 51 commands, available VMs, 1800s hang protection, serial execution and the backend resource lock. Generated command/properties were inspected without rerunning the completed workload solely for registration. |

The completed positive/refusal components plus one fatal compile took at most
77s; the remaining fatal component took 5.6418s. A checker-only correction
distinguished executable handler calls from imported declarations. Valid
positive/refusal results were reused; no single successful full-workload timing
is claimed. Source-call ordering and exported-context mutation/storage remain
complementary proofs with the limits above; source EXPOSE is not implemented.

<a id="current-drop-proof"></a>

## Completed DROP proof

DROP commit `fccb95b46dc4cfd75a453c0573fba27a78d5ff3b` has a
warning-free focused Debug host/fixture build and independent source/actual RXAS
review. Positive execution passed 47 source/IR/name/schema assertions and nine
prebound alias/configuration assertions in each B-mode/VM cell. The independent
36-line source oracle passed both B modes, both C IR modes and both target VMs,
plus one assembler-on control. Unchanged native/runtime/core-product proof is
reused from the completed CABI baseline below; optional packaged/installed C
products, hosted, cross-platform and complete-C acceptance are not claimed.

| Normal Debug check / command | Observed result and scope |
|---|---|
| `check-drop.py --build cmake-build-debug --work <temporary> --vm rxbvm --vm rxtvm` positive component | All 40 positive commands passed; 47 contract assertions and nine B-root alias/config assertions per host-mode/VM cell, 36-line source oracle in both IR modes/VMs and assembler-on control. Observed elapsed upper bound: 125s, including one source refusal. |
| `check-drop.py --build cmake-build-debug --work <temporary> --refusals-only` | All 15 source refusals passed in 19.4944s: located syntax/profile diagnostics, dead malformed source, reserved-dot prefixes and previous-output preservation. Positive inputs/results were reused; no single successful full-workload timing is claimed. |
| Focused CTest selection below | Seven tests passed in 42.43s: C admission 54 checks in four cells; frontend syntax 205, grammar 82 and mapping 81. |
| Maintained views through existing frontend/CLI/service probes | Four commands refreshed seven C IR/RXAS assets plus the verified inline pipeline snippet. Only authored raw-name casing changed; instruction counts, frames, source records and byte sizes are unchanged. Prior walkthrough execution remains valid. |
| CTest registration inspection | Default `llcompiler_c_drop` composes the 55 commands, both available VMs; generated command and 1800s/serial/resource-lock properties verified without rerunning the positive workload solely for registration. |

```sh
ctest --test-dir cmake-build-debug -R '^(llcompiler_c_context_contracts_(opt|noopt)_(rxbvm|rxtvm)|llcompiler_frontend_(syntax|grammar|mapping)_opt_rxbvm)$' --parallel 4 --verbose --output-on-failure
```

**DROP-F01 — generation repaired (AC01/04):** C generation forwards the existing
`rxlang --limit dfa.states=16384`; global defaults remain 4096. The generated
Unicode 17 artifact has NFA 29,709 / DFA 5,609 / LR 1,785 and 7,550,816 RXAS bytes;
normal Debug generation plus assembly took 651.355s. The three-command
`check-generator.py ... --forwarding-only` control proves named budget refusal,
previous-output preservation/tmp cleanup and two-limit forwarding with a
byte-identical successful artifact. No language/default-policy change or formal
performance claim follows.

**L-CONSTRUCT-01 — later generic L construction cost (STEP02/09):** `llregular`
linearly searches prior subset keys and retains equivalent DFA states. The
generic L owner may add indexed subset lookup/equivalent-state reduction with
independent longest-match, priority, Unicode-membership/span and bounded-failure
proof. The observed cost above covers generation plus assembly, not target
execution. No such algorithm change is part of DROP.

## Completed CABI proof and limits

The following results qualify CABI commit
`62d6dd44df528d56f4ca191bdc28d73b960c9432`; later changed inputs need their own
focused qualification.
`cmake --build cmake-build-debug --target stage-product`
and focused hosts/fixtures passed warning-free; the optional `llctarget` product
copy byte-matches the qualified opt target. No other optional packaged/installed C product,
hosted, cross-platform, release or complete-C claim follows.

| Normal Debug command / check | Observed result |
|---|---|
| `check-step07.py --c-only`, four host-mode/VM cells, both IR modes | Representative opt/rxbvm: 110 commands, 96.4892s; other cells: 68 each, opt/rxtvm 60.29s, noopt/rxbvm 59.20s, noopt/rxtvm 60.34s; zero failures. Expanded aggregate uses 1800s/serial/resource-lock hang protection. |
| Focused CTest direct target, C admission, shape and independent IR selections | Target: 142 current checks in each of four cells; admission: 54 each/four cells; shapes: 168 opt / 171 noopt. Independent IR: four cells plus frontend mapping/calls controls passed. |
| Independent IR placement and existing optimiser/PoC consumers | Four standalone IR cells passed in 0.53s; fixture RXAS/RXBIN byte-identical across placement. Reuse optimiser 33 commands/24.69s and PoC 54/39.69s passes on unchanged inputs. |
| Maintained walkthroughs and direct backend control | Eight referenced C RXAS and eleven IR views regenerated; 37 commands assembled/linked/executed required views/errors. Scale RXAS byte-identical to the qualified sample; direct backend 58 checks passed. [Walkthrough counts](examples/step06/README.md) include wrappers/cold handlers and exclude helper bodies/metadata. |
| Same-source literal compile and code-shape review | Initialized catalog/owner admission cache: 8.113s to 1.174s, byte-identical RXAS. Explicit EXIT/NUMERIC and present args construct no unused defaults. Literal branch pruning retains effectful truth with derived provenance. |
| Shared metadata prerequisite services | Four Debug cells passed in 22.81s; prior maintained Apple ASan four cells in 34.35s, LSan unavailable. Real C numeric 12-output and IR54 consumer proof passed. |

Per Adrian's source-only policy, source slices claim no fresh sanitizer pass;
valid unchanged-native evidence is reused and any warranted new sanitizer work
is consolidated at final delivery under [AGENTS.md](../../../AGENTS.md).
Actual SAN findings remain blockers. Retained owner/packet/getter copies,
contextual truth work, numeric setup, conservative registers and seed cleanup
remain named C-OPT-01–03/05/06 work; static counts and the local timing comparison
are not speed or minimal-frame claims. No direct construction defect remains
open from this slice. All complete-C CFC criteria above remain open.

# Complete Classic C compiler in B and Level L

**Complete C is open. Next: approve the C-family metadata and generated-procedure
ABI, then migrate the existing C selector to the completed borrowed context.**

Deliver one explicitly selected Classic C compiler and reusable compile service,
written in B using Level L parsing, IR, transformation and emission. Complete
the agreed instructions, all 95 BIF names, Unicode, diagnostics, conditions/TRACE,
host/lifecycle/platform contracts and, last, INTERPRET. The native B compiler is
the build seed; new C requests never delegate to a native source compiler/backend.
Existing RXAS/RXLINK, VM and native host transport remain execution dependencies.

The [requirements ledger](doc/levelc-requirements.md) is the single item-by-item
status record: 25 instructions, 95 BIFs, 75 reference capabilities and 36 syntax
contracts. It records reuse, missing integration, the owning step and closure
check. Native passes and existing B runtime bodies are controls, not new-C passes.
[Architecture](doc/architecture.md) owns design; [work packages](doc/agent-work-packages.md)
own assignments. Astra reviews/coordinates; Sol 6.1 Extra High workers implement,
build, test and integrate.

## Completed

- Independent C compiler/service and bounded expressions, structured control,
  private calls and LENGTH/ARG/DIGITS routes; full instruction/BIF closure remains open.
- Isolated C-only build/runtime baseline, bounded generic L analysis and shared B
  Unicode core/public API. Local baseline: `a5fb618d8bf93f56461f88f77c492b6f88516421`.
- Borrowed B context and nine additive adapters, independent lifetime/numeric/RXAS
  proof, compatible old images and two reproduced B seed prerequisite fixes:
  **CTX-AC01–06 / CTX-STEP01–04 locally complete**. Generated C still uses legacy adapters.

<a id="active-first-implementation-increment"></a>
<a id="shared-b-unicode-interface-and-c-adapter-migration"></a>
<a id="active-c-contract-baseline-increment"></a>
<a id="active-borrowed-classic-context-increment"></a>
<a id="step07a-validation"></a>
<a id="say-validation"></a>

## Remaining actions

**Next action — CLC-STEP03:** review the exact C-family type/effect/call revision
and source-unit procedure ABI for the already admitted profile. Check stable
borrowed owners, one execution configuration, same-frame NUMERIC installation,
private-call restoration, precise incompatible-metadata refusal and preservation
of prior output. The B context/adapters are approved; this selector/schema change
still needs Adrian's approval before implementation.

The steps below describe remaining work, not a requirement to finish a universal
framework before its first consumer. Deliver the minimum shared service needed
by each reviewed instruction; complete its wider contract before closing that
step. Numeric, error and source foundations are needed early even though full
NUMERIC, SIGNAL and TRACE instruction reviews occur later.

| Step / current status | Remaining action and closure check | Dependencies / approval gate |
|---|---|---|
| **CLC-STEP01 — Partial** | Finish contract-to-test traceability, exact ABI/dependency/platform decisions and independent expectations. Every required ledger row has an explicit disposition and closure check. Serves CFC-AC01/06–08/12. | Accepted C direction; exact language/platform exceptions need Adrian's decision. |
| **CLC-STEP02 — Partial** | Supply registered exceptional/resumable edges, observations, facts/invalidation, liveness and checked rewrites as consumers require. Independent non-C/G tests prove correctness, limits and atomic failure. Serves AC07/10/11. | STEP01 contracts; approve new public L APIs/schemas. Keep block-local values/cross-block slots; no required SSA rewrite. |
| **CLC-STEP03 — Partial; next** | Migrate the admitted selector after the approval above; then complete live activation/pool/context, direct operations/BIF ABI and one-body labels, recursion, EXPOSE, restoration and abrupt paths. Close remaining property/simple-map/traversal/budget contracts only where needed. Serves AC02/03/09/11/12. | Existing Unicode/CTX pieces are complete. Each extension needs the relevant STEP01/02 contract and exact ABI approval; direct B and new-C proof are distinct. |
| **CLC-STEP04 — Partial** | Complete contextual source/Unicode names, AST/source maps, static errors even in unreachable text, all expressions/numeric semantics and BF01–05 bindings. Independent values, effects, diagnostics and RXAS checks close C-SRC/C-EXPR. Serves AC01–03/06/07/09/11. | Required L/runtime services from STEP02/03; no native frontend dependency. |
| **CLC-STEP05 — Partial** | Review LC-I-01–10 individually: SAY through ITERATE. Close every form, error, order, pool/loop effect and source observation in the ledger. Serves AC01–03/06/07. | STEP04 plus each instruction's shared dependencies. Whole SAY remains open; DROP is the next instruction review after its disposition. |
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

## Current qualification

Inputs: local baseline above plus frozen CTX runtime/adapters, B seed repairs
and maintained fixtures. Documentation changes do not invalidate these inputs.
This increment is locally qualified and unpushed; no hosted/platform/release claim follows.

| Command / check | Observed result and boundary |
|---|---|
| `cmake --build cmake-build-debug --target stage-product` and Classic/context/C compiler prerequisites; `cmake --build cmake-build-debug --parallel 4 --target llctarget` | Passed; final Debug product has zero warning/error diagnostics. Optional research target matches the qualified opt aggregate; core staging still excludes research. |
| Focused `ctest --test-dir cmake-build-debug --output-on-failure` selection | 58 final cases passed in 162.01s: context/adapters/shapes, Classic controls, reference/inline and four 55-command C-only cells. Ten unchanged native cases reuse prior passes: **68 distinct Debug tests**. |
| Native reference selection `^(reference_source_qualified_|reference_identity_)`; maintained Apple ASan | All 12 new cases passed Debug/ASan; ASan 26.22s. Revised inferred-array fixture passed two ASan reruns in 3.93s and final Debug. Imported noopt retains assembler optimisation; local qualified noopt disables both. |
| `cmake --build <temporary-fixture>/debug --target qualify --parallel 1`; `tools/asan-run.sh --build-dir <temporary-fixture>/asan --phase build --build-target qualify --build-jobs 1 --build-leaks off` | New class/adapters compiled opt/noopt; both concrete VMs with `--no-autoload`: Debug 13.32s / ASan 28.57s, 149 adapter checks per execution, 153 structural checks per RXAS mode. Unchanged runtime members reused. |
| Old emitted C image against both new target modes / both concrete VMs, `--no-autoload` | Four cells preserve exact `10/0/3`; frozen image and 24 public legacy signatures unchanged. |

No first-party sanitizer finding was observed; Apple LSan is unavailable and
focused ASan is not broad platform closure. The tool refresh's pre-existing SDK
`sprintf` deprecations are not part of the warning-free Debug claim. Retained
RXAS costs include ABI argument/result/config-getter copying, nonnumeric
`numsci` setup and seed post-`ret` cleanup text; C-OPT-02/03/05/06 remain open,
with no zero-copy, minimal-frame or speed claim. All 112 unrelated active files
were byte-identical at closeout and task-created diagnostics were removed.

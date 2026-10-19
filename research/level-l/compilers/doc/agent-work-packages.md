# Level C implementation work packages

**Approved execution instructions, 19 October 2026.** The
[plan](../PLAN.md#c-first-delivery-milestone) is authoritative; this document
specifies assignments, ownership and review. It does not start implementation,
automations or new branches. The [architecture](architecture.md) and
[requirements](levelc-requirements.md) are mandatory reading with AGENTS.md.

## Coordination model

Adrian requested an Astra Extra High coordinator for design and later
coordination, with GPT-6.1 Sol Extra High subagents. Configure future worker
assignments with `model=gpt-6.1-sol`, `reasoning_effort=xhigh`; a requested model
must not be silently substituted. The coordinator independently checks results,
resolves interfaces, updates acceptance, integrates and publishes only within
then-current authority. These are roles requested by Adrian, not a claim about
the model identity of a previous or current session.

Use the existing synchronized `develop` checkout. Do not create branches or
worktrees. All agents share files: the coordinator grants exclusive ownership
of named paths before writes. Record baseline HEAD and unrelated dirty paths;
keep their contents unchanged. Assign a single integrator for shared CMake,
public schemas and canonical plans. Workers report diffs; they do not commit,
push, stash, reset, clean, switch branches, change automation or build broadly.

With four available slots, use coordinator plus at most three workers. Useful
parallel work is a generic L implementation, a runtime/Unicode implementation
on disjoint files and an independent test/review task. The C frontend/selector
instruction stream has one writer at a time. Never use concurrent full builds
or build/test activity in the same output tree. Share prepared prerequisites
and schedule focused checks centrally; isolation is by build/test location,
not by extra source checkouts.

For each instruction, use a bounded implementation assignment followed by a
separate review assignment with independent expectations. The reviewer can
prepare test cases first without editing the implementation's paths. Read-only
review may overlap implementation only against a named stable checkpoint;
review the final diff again when changed. Agents do not approve their own
semantic exceptions or silently relax tests.

## Work package catalogue

Each package is split into reviewable slices. A package name is not authority
to implement every item at once. Dependencies below identify the earliest safe
start; CLC stage closure also requires the complete plan criteria.

| Package / owner | Allowed implementation area | Inputs and deliverables | Decisive acceptance / dependencies |
|---|---|---|---|
| **WP-01 Contract audit** | Read-only sources; coordinator writes requirements/plan | Reconcile LC-I, BIF, LC-REF and approved exceptions; inventory current test paths, APIs/platforms and C-only host/target closures. Propose minimal independent fixtures and exact changed interfaces. | CFC-AC01/07/12; CLC-STEP01. No native status copied into new completion. |
| **WP-02 L analyses** | `lib/rxfnsl/rexx/`, assigned `langtools/tests`, generic docs/examples | Registered edge/effect/observation schemas, finite facts/worklists, def/use/liveness, invalidation and checked rewrite APIs. Provide record diagrams, complexity/budgets and non-C examples. | CFC-AC07/11; STEP02. Public API/schema gate before implementation; no C names or RXAS opcode knowledge. |
| **WP-03 Classic runtime** | Assigned `lib/rxfnsc` classes/tests | Reconcile activation/pool/config/value/condition ownership, direct operations and BIF ABI; live pool binding, shared body/entry contracts, cleanup, presence and context inheritance. Keep old consumers compatible. | CFC-AC02/03/09/11; STEP03. Direct B tests plus affected native C/RexxScript/typed Classic regressions; public ABI approved. |
| **WP-04 Shared B Unicode** | Assigned `lib/rxfnsb/unicode` target, migration from `lib/rxfnsg`, `tools/unicode`, shared data consumers | Extract existing B executors and minimal procedural API, retain G facade/iterators and C adapters, single pinned data owner and acyclic L/bootstrap graph. Exact paths/API/product names reviewed before move. | CFC-AC07/09/12; STEP03. Direct B, unchanged G and C tests, UCD/codec negatives, linked table ownership, cold/freshness/installed closure. Coordinate CMake edits; no same-file overlap with WP-03. |
| **WP-05 C source/expressions** | Assigned `levelc` grammar/parser/frontend/IR modules and compiler tests | Complete C-SRC/C-EXPR, source/AST schemas, static validation, Unicode names, operators, numeric contracts and BF01–04 binding. Use L and runtime APIs, no duplicate algorithms. | CFC-AC01–03/06/07; STEP04 after needed WP-02/03/04 contracts. Native comparisons are controls. |
| **WP-06 One C instruction** | Named grammar/frontend/IR/selector paths and a named instruction fixture directory | One LC-I row, all forms/errors/edges, worked AST/IR/RXAS, runtime interface use and independent expectations. Required shared improvements are explicit linked dependencies. | CFC-AC01–04/06; STEP05–07. Serial instruction writer; separate review packet. Full closure waits for all listed shared proofs. |
| **WP-07 BIF/host family** | Assigned runtime family/adapter and tests; frontend binding edits coordinated | One BF integration group or C-HOST contract at a time. Reuse existing BIF bodies, fixtures and valid qualification; bind names/presence/context/errors through the new compiler. Complete only named missing host contracts. Library edits require a reproduced defect, identified open contract or approved Unicode extraction. | CFC-AC01/04/09/11; STEP06–08. New public host APIs require review; no alternate ADDRESS protocol or dynamic external-call policy. |
| **WP-08 C target quality** | Assigned C selector and `compilers/common` target utility paths | Target records, liveness-driven register reuse, call windows, ABI cleanup and C-OPT01–05. Explain legality and compare actual caller plus helper RXAS. | CFC-AC06/07/11; STEP09 incrementally. WP-02 generic facts; no target policy in SDK. No broad performance work for research. |
| **WP-09 RXAS/VM primitive** | Precisely assigned assembler/binutils/interpreter/disassembler/linker paths and direct tests | One reproduced cost or missing execution capability; compare existing sequence, library/selector fix and primitive. Propose exact operands/effects/alias/errors/source/encoding/platform contract before code. | CFC-AC10/11; STEP09 or final STEP11b. Explicit native/ABI gate; mandatory first Release verdict for production optimisation. No language parser/BIF dispatch in VM. |
| **WP-10 Delivery and independent QA** | Assigned build/test/packaging files; coordinator owns shared root integration | Cold/incremental/no-op/missing-input/installed manifests, platform capability matrix, static compiler-free runtime, independent reference/oracle and RXAS checks. Aggregate tests measured before registration. | CFC-AC08/11/12; STEP10 and STEP11d. Reuse existing qualification; no automatic overnight matrix. |
| **WP-11 INTERPRET compiler/runtime** | Assigned C mode/entry/continuation and B runtime dispatch files | Final-stage mechanism proof, then same-pipeline group compilation, live caller binding, cross-owner outcomes, handler resumption and source observations. Interfaces agreed with WP-09 image ownership. | CFC-AC05/11; STEP11 after static acceptance. No source compiler/evaluator fallback and no premature dynamic implementation. |
| **WP-12 Independent acceptance review** | Read-only implementation and temporary diagnostics; own test edits only if granted | Check contract-to-code-to-test traceability, inspect RXAS/helpers, challenge effect/lifetime assumptions, compare final diff to approved scope and verify preservation. Findings name mechanism, affected criterion and reproducer. | Every increment; full review at STEP10/11d. Coordinator independently validates; worker assertions alone never close a criterion. |

## Assignment template

The coordinator supplies these fields in each worker prompt:

1. **Objective:** one concrete behavior/failure and the intended end state.
2. **Baseline:** exact HEAD, approved architecture version and relevant existing
   local edits; no resetting or syncing by the worker.
3. **Scope:** CLC step, CFC criteria, LC-I/BF/LC-REF IDs and exact owning docs.
4. **Write ownership:** explicit files/directories, exclusions and integrator for
   shared files. Read access is broader; write ownership is not.
5. **Contracts:** accepted records/APIs/ABI/effects, decisions still open and
   producer/consumer interfaces expected from other packages.
6. **Tests:** independent expected values/state/errors/source and specific normal
   commands; maintained sanitizer work only where required/assigned. Declare
   shared build-tree access and any nested aggregate scheduling requirement.
7. **Quality:** representative complete RXAS cases, helper bodies, expected
   structural improvements and preserved observations; compiler and RXAS modes.
8. **Stop conditions:** unapproved language/architecture/ABI decision, unsafe
   dependency assumption, unexpected active-file edits, regression or actual
   sanitizer finding. Report promptly; do not weaken tests or invent a fallback.
9. **Handoff:** changed paths, mechanism, criterion status, exact tests/input
   identity and limitations, unresolved findings and proposed next step. Keep
   lasting conclusions in owning docs; delete temporary diagnostics. No raw-log
   bundles, dated receipts, screenshots or per-run hashes in the source tree.

For example, WP-06/LC-I-02 covers DROP direct/indirect forms, compound lookup
order, alias lifetime and invalid-word policy. It must state expected pool
states before/after each operation, specify `C-POOL` effects, verify native
comparisons only in matching dialects and inspect actual pool-drop RXAS calls.
It must not concurrently add PROCEDURE, change the global pool ABI or implement
VALUE merely to make a test pass; those dependencies belong to their owners.

## Integration and publication gates

The coordinator first checks changed paths against ownership and active-work
hashes, then reviews the implementation and the independent review findings.
Re-run only decisive checks whose evidence is missing or invalidated. Verify the
normal product build and appropriate functional/correctness suites before any
substantive publication, following AGENTS. An author and reviewer both reporting
success does not replace coordinator inspection.

For a production RXAS/VM performance edit, freeze after minimum focused
correctness, run the governed first ordinary Release comparison, report the
verdict and stop for Adrian's direction before broad closeout. This gate does
not apply to ordinary Level L research increments; actual sanitizer findings
retain SAN worklist/regression/platform closure requirements in either case.

Commit only assigned, qualified paths when publication is authorised. Verify
remote develop and discover actual exact-SHA normal workflows; do not rerun or
resume CI monitoring without the current monitoring instruction. A new unrelated
remote commit requires reconciliation while preserving active work. A green
individual job is not a green workflow or release.

Before a context handoff, read/update the current plan rather than creating
another plan or dated note. Name open criteria and decisions explicitly. After
approval, maintain one concise validation summary per active increment, replacing
superseded prose; preserve genuine regression fixtures and enduring decisions.

# RXPA native objects — C surface completion

Status: C implementation and normal local verification complete; STEP-06
platform/sanitizer qualification remains open. Selected by Adrian:
“We better complete the C RXPA surface”. This is
the generic dependency of [S4-D03](native-inference-typed-interface-proposal.md),
not a replacement for the full [native-inference plan](native-inference-backlog.md).
Baseline HEAD: `c2cf28a4f5c66430b4c2cc4d49b00ae720675ab8`, with existing
STEP-03/04 work retained at that capture. Adrian requested the baseline commit
on 15 September; the later [broad regression attempt](../qa/native-inference-baseline/README.md)
exposed NI-S4-QA01. The subsequent [QA01/QA02 repair](../qa/native-inference-qa01/README.md)
now accounts for all 2,347 ordinary Debug tests through the full run and affected
rechecks. It also removes the obsolete `rxstats.linearfit` Rexx construction
shim in favor of its C provider. The original focused results below remain
separate evidence; platform/sanitizer/release qualification is still open.

## Vision and intended outcomes

1. **NO-OUT-01:** A plugin author can implement a class entirely in C: declare
   its contracts, bind factories and methods, construct correctly identified
   objects, and use normal Rexx factories, methods, casts and interface dispatch.
   Applications need no Rexx construction shim or VM-private structure access.
2. **NO-OUT-02:** Preserve the existing value/receiver semantics, native payload
   ownership and synchronous nested-call protocol. Support dynamic providers,
   static/native delivery, declaration-only compiler imports and local workers.
3. **NO-OUT-03:** Use this generic facility for the subsequent typed llama
   interface. Preserve persistent/batched GPU-capable inference and all parent
   acceptance criteria. This task does not select new language syntax or the
   still-reviewable public llama names, nor reopen accepted inference timings.

## Numbered checkable acceptance criteria

1. **NO-AC-01:** An ordinary C-only fixture constructs objects through default
   and named factories and a typed procedure return. `<typeof>`, initialized,
   concrete methods, widening to `.object`, checked casts, interface methods
   and interface factory selection all observe the concrete class correctly.
   Retain a failing pre-implementation test and a working control.
2. **NO-AC-02:** The checked host operation rejects missing/invalid/unknown or
   interface targets and calls outside an active VM/native-call context without
   changing the destination. Successful type publication preserves the payload
   and attributes and clears only the uninitialized-object marker. C owns its
   class representation; stamping does not invoke Rexx factories.
3. **NO-AC-03:** Mutating methods preserve receiver and argument copyback;
   native-to-Rexx-to-native calls and propagated signals work. Owned native
   payloads retain/copy/finalize correctly, including replacement and teardown.
   Uninitialized receivers still fail through the existing language signal.
4. **NO-AC-04:** Dynamic and static providers, DECL_ONLY imports, optimized and
   unoptimized code, rxas/rxlink and both applicable VM engines pass focused
   normal checks. Include attached-worker/session isolation and a relocated
   native consumer; retain existing RXPA text-service/legacy compatibility proof.
5. **NO-AC-05:** Extend only the sized host-service tail; keep the legacy
   `rxpa_initctx` layout unchanged. Prefix-only, partial-tail, unknown-version
   and null-service hosts are handled safely. Plugin authors can bind members
   without embedding compiler-private symbol conventions in their own code.
6. **NO-AC-06:** Document C receiver/factory argument conventions, construction,
   failure/ownership rules, negotiation and delivery. Update S4-D03's selected
   implementation path without marking llama's typed API implemented.
7. **NO-AC-07:** Qualify Windows/Linux and maintained sanitizer/ownership paths
   under native-inference STEP-06. Until then this criterion remains open;
   SAN-009 remains release-blocking with Codex under Adrian's direction as
   release-QA owner. Do not run sanitizer builds/tests during this phase.

## Numbered implementation steps

1. **NO-STEP-01 (AC-01–06):** Trace existing metadata, factory/method ABI and
   graph-backed identity. Record this plan and ordinary failing native-class
   acceptance controls alongside positive controls before production edits.
2. **NO-STEP-02 (AC-01–03,05):** Complete checked native object type publication
   and member binding using existing class metadata, runtime descriptors and
   size-negotiated services. No new ISA/language syntax or legacy initializer
   extension. Keep ordinary receiver/value ownership semantics.
3. **NO-STEP-03 (AC-01–05):** Run focused Debug/Release normal compatibility,
   lifecycle and four-tool tests across delivery/VM/optimization modes, then
   attached-worker and native packaging proof. This is a capability addition,
   not a production performance optimization; do not start model benchmarks.
   If a hot-path optimization becomes necessary, the mandatory first Release
   performance verdict gate applies before broad closeout.
4. **NO-STEP-04 (AC-06–07):** Update SDK examples/reference and llama handoff;
   reconcile every criterion and retain evidence. Any new nested aggregate
   remains unregistered until isolated Debug and sanitizer scheduling evidence
   is available. STEP-06 owns later platform/sanitizer qualification.

## Current handoff

- NO-STEP-01–04 implementation, normal local verification and documentation
  are complete. [Retained evidence](../qa/rxpa-native-objects/README.md) records
  failures before each repair, source identity and passing delivery checks.
- Declaration-only macros remain metadata-only; new binding macros plus the
  checked host type service provide C construction. Before implementation,
  C payload publication alone did not establish class identity: see the retained
  [stats diagnostic](../qa/native-inference-native-surface-review/README.md).
- The chosen compatibility route is the existing sized host services. Native
  code publishes concrete identity after constructing its own representation;
  failure must leave the destination's identity/contents untouched.
- Existing sanitizer hold and accepted STEP-04 timing observation are unchanged.

## Acceptance disposition and next action

| Criterion | Disposition |
| --- | --- |
| NO-AC-01 | Pass locally: C-only default/named factories, typed procedure return, initialization/type tests, concrete/interface/default methods, casts and factory selection. The before control retains working concrete accessors and failing identity. |
| NO-AC-02 | Pass locally: active-context/type validation and nonmutating failures, preserved fields/payload/flags, explicit initialized publication. Native graphs belong to their VM module. |
| NO-AC-03 | Pass locally: receiver/expose copyback, C→Rexx→C and C→C callbacks, full-width results and signals, owned payload copy/replacement/reset, distinct worker sessions and teardown. Uninitialized receiver/child flags survive RXPA validation. |
| NO-AC-04 | Pass locally: 44 focused compatibility Debug tests, 24 registered native-object Debug tests, explicit 24-execution Debug and Release matrices, 2 Release units, external installed SDK and three relocated native consumers. Static compiler lanes exclude the dynamic provider import path. |
| NO-AC-05 | Pass locally: exact legacy `rxpa_initctx` layout unchanged; complete sized tail, allocated prefix-only host, every partial pointer tail, unknown version/null service, old dynamic/static manifests and C/C++ compilation controls. |
| NO-AC-06 | Complete: SDK binding/construction/ownership/callback documentation, runnable C fixture and S4-D03 native implementation handoff. Typed llama implementation now passes its own normal local acceptance/delivery checks; final public contract remains reviewable. |
| NO-AC-07 | Open: Windows/Linux, maintained sanitizers and full-product STEP-06 qualification; SAN-009 remains release-blocking with the named owner above. No sanitizer rerun in this phase. |

The [typed llama completion](../qa/native-inference-typed/README.md) exposed
and repaired two additional generic defects: missing native-provider dependencies
for interface-only factories, and a returned string's physical integer field
being mistaken for worker/callback execution failure. Both have minimal ordinary
failures and permanent C/REXX regressions. Current follow-up evidence adds **32**
individual native-object Debug CTests, **55** focused compatibility tests,
**eight** declaration-only/static-compiler executions and an installed C/C++ SDK
with **eight** VM/**four** relocated native runs. Earlier unchanged host-service,
legacy layout, ownership and delivery evidence remains retained; these additional
results supersede the older three-fixture counts for the expanded matrix.

NO-AC-01–06 remain passed locally. NO-AC-07 stays open under STEP-06 and now
explicitly includes the interface-dependency and typed-return repairs. The 32
individual regressions use prepared artifacts; the larger explicit matrix/package
aggregates remain unregistered pending STEP-06 scheduling proof. No commit,
publication or user-prefix installation was made.

**Next:** Adrian's review of the completed S4-D03/STEP-04 candidate. The C surface
is no longer an intermediate stopping point: llama-specific API, examples and
normal installed/native checks are complete. Preserve every parent outcome and
criterion, including persistent/batched GPU use and later qualification.

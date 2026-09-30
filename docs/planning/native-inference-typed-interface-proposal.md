# S4-D03 — Typed native llama interface

Status: implementation and normal local F-AC-01–04 verification complete,
14 September 2026; captured in the requested 15 September baseline. The
implemented public contract is retained in the 15 September STEP-04 closure
after the accepted factory-cleanup/regression report. The subsequent
[QA01/QA02 repair and factory cleanup](../qa/native-inference-qa01/README.md)
completes ordinary local regression coverage for all 2,347 tests using the full
run and affected rechecks. Platform/sanitizer qualification remains STEP-06.
The presented names/return contracts are unchanged.
The STEP-04 Release verdict is accepted. This records the delivered public
spelling selected through [STEP-01](native-inference-step-01.md#proposed-public-contract)
and the completed typed-interface review/implementation.
Parent outcomes and criteria remain in [STEP-04](native-inference-step-04.md)
and the complete native-inference plan. Adrian has selected completion of the
generic [C RXPA object surface](rxpa-native-objects.md) as its implementation
foundation; its implementation and normal local delivery checks now pass.
No alternative names or return contracts have been introduced; no new
language syntax is selected. Generation methods remain STEP-05.

## Numbered vision and outcomes

1. **F-OUT-01:** `import llama` gives applications typed configuration, runtime,
   model, embedding-session, request and result objects. The existing `rxllama`
   plugin remains independently usable; the brand stays **llama.rexx**.
2. **F-OUT-02:** Applications explicitly load and prepare once, then submit many
   single inputs or batches. Public handles are typed owners with explicit
   close; raw native binary handles remain implementation details.
3. **F-OUT-03:** Keep status, lifecycle state and durable diagnostics distinct.
   Preserve complete input bytes, packed results, hardware overrides, bounded
   admission and worker ownership. No model server, implicit per-call model
   loading, new vector format or application/RAG policy.

## Proposed public spelling

Names below are qualified by `llama`; signatures specify the candidate contract.
Public interfaces hide conventionally private implementation classes.
Methods that create child owners return the appropriate typed interface.

| Public type | Construction and capability-specific methods |
| --- | --- |
| `.configuration` | `.llama..configuration()`; `set_int(key, value)`, `set_float(key, value)`, `set_text(key, value)` return `.int` status. Preserve the existing validated option keys and defaults. |
| `.runtime` | `.llama..runtime(config)`; `device_count()` → `.int`; `device_info(index, key)` → `.string`; `model(path, sha256, profile, config)` → `.model`. Model opening remains asynchronous. |
| `.model` | Returned by `runtime.model(...)`; `state()` → `.string`; `cancel()` → `.int`; `embedding_session(config)` → `.embedding_session`. Generation-session construction is added in STEP-05. |
| `.embedding_session` | Returned by `model.embedding_session(...)`; `prepare(work_tokens)` → `.string` state; `request(config)` → `.embedding_request`. No request method reloads a model. |
| `.embedding_request` | Returned by `session.request(...)`; `add(text, role)` → one-based `.int` row, zero on failure; `add_all(texts, role)` → `.int` status; `submit()` → `.int`; `process(work_tokens)` → `.string` state; `state()` → `.string`; `cancel()` → `.int`; `result()` → `.embedding_result`. |
| `.embedding_result` | Returned by `request.result()`; `values()` → owned `.packedfloat`; `rows()` and `dimensions()` → `.int`. Results survive request closure. No reshape, row reordering or serialization. |
| `.diagnostic` | Returned as a value snapshot by `diagnostic()`; `code()` → `.int`, `operation()` and `message()` → `.string`. A later native call cannot overwrite an earlier snapshot. |

Every owner has `status()` → `.int`, `diagnostic()` → `.diagnostic`, and
`close()` → `.int`. Runtime/model/session/request also have `info_int(key)` and
`info_text(key)` over the native inspection keys. Result objects expose
`status()`/`diagnostic()` but need no native close: they own ordinary Rexx data.

Construction failures return an initialized typed object with nonzero status
and a captured diagnostic, following existing status-oriented library patterns.
It contains no usable native resource. Inspect status before invoking dependent
work; safe close on a failed or already-closed owner succeeds. No new signals or
signal names are introduced by this facade. Existing language failures retain
their ordinary behavior.

`add_all` fills one request in input order without submitting it. If any row
fails, capture that diagnostic, cancel the building request and return failure;
never leave a convenience-call failure looking like an accepted shorter batch.
Single-input use is `add` on the same persistent session. Processing one
noncausal embedding batch remains an indivisible bounded compute unit even when
the caller supplies a smaller positive work budget.

Objects stay in their creating VM. Ordinary value copying aliases the same
checked native resource; explicit close affects those aliases, and provider
generation/owner checks remain authoritative. Examples pass text, configuration
values and output bytes through existing workers and create native owners
inside each worker. They never transfer typed native owners or raw handles.
All native handles remain retained by the typed object's owned native payload;
there is no new finalizer syntax or second resource registry.

Publish the typed contracts and C bindings through the optional provider,
using RXPA class/interface metadata, checked `SETOBJECTTYPE` construction and
owned native payload hooks. The public `import llama` spelling resolves through the provider metadata;
F-03 and F-04 verify its installed dynamic/native package mapping. Dynamic and native consumers must resolve the declared provider and its
packaged inference dependencies from the installed product. This implementation
choice no longer requires a Rexx construction shim or a separate facade library.

## Numbered checkable acceptance criteria

1. [x] **F-AC-01 (S4-AC-05):** Public classes/methods above compile from the installed
   binary import, with ordinary optimized and `-n` consumers, through rxc,
   rxas, rxlink and both applicable VMs; native packaging also passes.
2. [x] **F-AC-02 (S4-AC-01/02/03):** Ordinary failing controls precede implementation
   for failure snapshots, complete strings, failed construction, partial
   `add_all` cancellation, child/parent close, copied-owner close, and result
   ownership. Existing native boundaries remain unchanged and pass.
3. [x] **F-AC-03 (S4-AC-03/04/05):** Persistent CPU/Metal examples submit repeated
   batches, retain one model load, consume packed output with rxvector and
   close explicitly. Real workers isolate mutable requests and share compatible
   model weights through the existing provider, with no new scheduler.
4. [x] **F-AC-04 (S4-AC-05):** Every exposed type/operation has RexxDoc contracts,
   parameters, returns and lifecycle notes. Documentation includes status checks,
   explicit preparation, bounded processing, shutdown and the accepted Metal
   observation. Generation/platform/sanitizer criteria remain visibly open.

## Numbered implementation steps

1. [x] **F-01:** Retain these presented names and return/failure contracts for
   Adrian's public-interface review. His subsequent STEP-04 continuation directs
   completion of the concrete candidate; do not substitute new contracts or
   confuse dependency completion with STEP-04 acceptance.
2. [x] **F-02:** Add focused ordinary acceptance consumers and retain their failures
   against the missing facade, plus working native positive controls.
3. [x] **F-03:** Implement the C typed contracts/bindings and optional provider packaging,
   preserving native request/layout/lifecycle semantics. Test the approved
   interface without changing the inference implementation or tuning its speed.
4. [x] **F-04:** Finish persistent examples and installed/native checks; reconcile
   S4-AC-01–06. Reuse accepted timing evidence. Sanitizer and other platforms
   remain the named STEP-06 gate; no inferred phase or release completion.

## Live task checklist — STEP-04 completion

- [x] **F-02 / S4-05c:** Add ordinary typed configuration/embedding acceptance
  consumers before implementation; retain missing-class failures in
  `docs/qa/native-inference-typed/`. Existing passing native controls remain in
  the STEP-04 closeout bundle.
- [x] **F-03 / S4-05c:** Complete and verify C bindings for all documented types,
  full strings, durable diagnostics, failed constructors, atomic convenience
  failure and owned packed results.
- [x] **F-04 / S4-05c:** Finish persistent public examples, real workers and
  optimized/unoptimized, both-VM, installed/native consumer evidence.
- [x] **F-AC-04 / S4-05d:** Reconcile source contracts, documentation and every
  S4-AC/F-AC; retain final acceptance review and named STEP-06 limitations.

This checklist supplements, and does not replace, F-OUT-01–03, F-AC-01–04,
S4-AC-01–06 or the parent OUT/NI/AC requirements.

### Completed findings and delivery proof

The C typed surface and both persistent examples now exist. The typed configuration
and CPU embedding consumers pass both VMs in focused checks. The latter exercises
failed construction, full NUL-bearing input, selector rejection, parent/child and
copied-owner closure, partial `add_all` cancellation, independent diagnostics,
twenty real batches, complete owned results and rxvector consumption.

Two ordinary toolchain findings were exposed by the complete public consumers:

1. **Interface-first native provider discovery:** a program using only interface
   factories emitted no concrete callable/provider dependency, so the provider
   was never loaded. A minimal `rxpa_objects_class_only.crexx` control now starts
   with unnamed/named interface factories and has no concrete/plain native call.
   The compiler retains known native candidates through existing provider metadata.
   The focused dynamic matrix passes both VMs and optimization modes.
2. **Typed external return status:** workers completed every batch and closed
   their owners, but a native string's nonzero physical integer field was treated
   as failed execution on return. The same cause affects C-to-Rexx `CALLMETHOD`.
   `rxpa_objects_text_worker.crexx` and the native fixture retain independent
   model-free failures in `text-worker-before.log` and `text-callback-before.log`.
   The repair uses captured signals and separates typed call success from the
   historical process return code. All 32 native-object Debug regressions and
   the four-worker CPU example pass on both VMs. F-04 now also passes the full
   CPU/Metal, installed and relocated native matrix.

Neither finding changes language syntax, inference algorithms, model defaults,
the accepted timing verdict or the STEP-06 sanitizer hold. All local F-04 and
S4-05d work is now evidenced in the [completion bundle](../qa/native-inference-typed/README.md).

| Criterion | Disposition |
| --- | --- |
| [x] F-AC-01 | Installed binary imports, both optimization modes, all four tools and both VMs pass; eight relocated native programs pass their CPU/Metal modes. 28 installed VM plus 14 native runs. |
| [x] F-AC-02 | Ordinary missing-API failures precede implementation. Typed configuration/embedding controls now pass failure snapshots/recovery, complete input, failed construction, partial add_all cancellation, close/alias semantics and result ownership. Existing native bounds remain unchanged. |
| [x] F-AC-03 | Both public examples pass CPU/Metal, Debug/installed Release, both optimizations/VMs and relocated native delivery. Four workers retain one shared model load, private results, twenty batches each and rxvector consumption. |
| [x] F-AC-04 | C RexxDoc and installed reference/examples cover all public operations, preparation, status/state, limits, ownership and shutdown. Parent/S4 criteria, accepted Metal observation and STEP-05/06 gaps remain explicit. |

F-01's concrete contract is retained for final review; F-02–04 are complete.
F-OUT-01–03 are delivered by the candidate. Public-contract and phase acceptance
remain Adrian's review decisions; they are separate from completed local proof.

## Alternatives considered

The existing status-returning native calls remain the implementation foundation
and low-level API, but alone do not satisfy the agreed typed public facade.
An exception-only convenience interface would change the agreed status/result
direction and is not proposed. A single `embed(model_path, text)` entry point
would obscure preparation and ownership, so it is not the primary surface.
This proposal selects ordinary typed owners and existing status semantics.

## Native exposure clarification — Adrian's follow-up

RXPA already supports class/interface/factory/method metadata, including dynamic
and static declarations. The proposed public surface is therefore not tied to
Rexx-written declarations. A [focused review](../qa/native-inference-native-surface-review/README.md)
established that the then-remaining gap was supported pure-C construction of
objects with correct runtime class identity; declaration and native payload
publication alone do not establish it. At that checkpoint the stats native
return had working concrete accessors but reported `.object`, whereas its Rexx
factory reported `.rxstats..linearfit`, in both optimization modes. The later
15 September shim cleanup moves that carrier to C RXPA and requires runtime
identity for native and factory results; see QA01-AC-05 in STEP-04.

Adrian subsequently selected “complete the C RXPA surface”. The generic
[RXPA completion plan](rxpa-native-objects.md) now owns checked native class
publication, member/factory binding, nested dispatch, ownership and delivery
proof. S4-D03 now uses that facility and its typed implementation is locally
verified; final public-name/return-contract acceptance remains a review decision. Retain all F-OUT/F-AC and parent
criteria through this dependency. The earlier diagnostic records the prior gap,
not the final state after completion.

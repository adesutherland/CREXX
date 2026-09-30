# STEP-05 — Persistent generation and concurrent processing

Status: STEP-05 complete locally, 15 September 2026; first Release verdict approved by Adrian. Baseline
`f9f87a8e8`; STEP-04 is closed. Parent authority:
[native-inference plan](native-inference-backlog.md), OUT-01–05,
CREXX-NI-01–07 and AC-01–14. Do not replace those whole-product requirements
with this phase's local results.

## Vision and intended outcomes

1. **S5-OUT-01:** A cREXX program loads the pinned SmolLM2 model once, explicitly
   prepares private generation state, and processes repeated independent prompts
   and bounded batches without a server or repeated weight loading.
2. **S5-OUT-02:** CPU and actual GPU execution share the same typed C RXPA API:
   bounded prefill/decode, ordered incremental UTF-8 output, token counts,
   request/row identity, explicit finish reasons, cancellation and safe recovery.
3. **S5-OUT-03:** Existing workers share compatible immutable weights with private
   contexts and request/output state. Embedding and generation can remain resident
   together within the existing admission envelope.
4. **S5-OUT-04:** Establish indicative integration cost using matched fixed
   direct-library controls. Adrian clarified that the earlier slowdown was Metal,
   not CUDA. If a comparable material slowdown recurs in generation, investigate
   and determine its root cause; do not accept it as another unexplained result.
5. **S5-OUT-05:** Complete native/dynamic consumers, runnable persistent/concurrent
   examples, source documentation and explicit acceptance evidence. STEP-06 keeps
   the previously assigned sanitizer and missing-platform gates.

## Numbered acceptance criteria

1. [x] **S5-AC-01 (parent AC-01/02/03/04/06):** Pinned SmolLM2 greedy generation
   works on CPU/Metal with explicit preparation, single/four-row batches, complete
   prompts, ordered row IDs and finish reasons. Repeated same-layout requests
   match a same-backend independent direct llama.cpp control. Do not require
   identical CPU/GPU wording.
2. [x] **S5-AC-02 (AC-06/08/09):** Enforce input bytes, rows, aggregate tokens,
   per-sequence context and output bounds without truncation. Positive work
   budgets limit prefill/decode units; no hidden one-shot loop. Reject wrong
   capabilities, empty batches, malformed input, invalid state/row and incompatible
   configuration. Incremental text buffers incomplete UTF-8; error, cancellation,
   EOS and output-limit finishes are distinguishable. Keep ordinary negative
   controls and owned result snapshots.
3. [x] **S5-AC-03 (AC-04/07/08/09):** Retain 100 singles/20 four-row batches,
   one model load, bounded retained memory, one/two/four-owner identity and
   isolated/concurrent output checks, simultaneous two-model residency,
   preparation/prefill/decode cancellation, recovery and teardown. Preserve
   STEP-02's 32 MiB retained growth, 4 GiB example admission and measured 250 ms
   M5 work-unit / 1 s load-drain diagnostics; do not lower workload to hide a miss.
4. [x] **S5-AC-04 (AC-12):** After minimum correctness, freeze the production
   slice and capture the first ordinary profiling-off Release verdict. Compare
   CPU/Metal single/four-row generation with identical model bytes, context,
   sampling, batching, CPU threads, work/output counts and completion boundaries.
   Separate cold load/preparation, first result and warm latency. Retain all raw
   samples and the existing 5% diagnostic tripwire. If a material Metal excess
   comparable to the accepted ~20% embedding result recurs, Adrian authorizes
   root-cause investigation: isolate bridge/VM dispatch, batching, synchronization,
   output conversion/copy and backend execution with causal controls. No model
   quality study, configuration sweep or speculative upstream fix. Report the
   concrete verdict/cause before broad closeout; new architecture still requires
   review. An unexplained recurrence cannot be ticked complete.
5. [x] **S5-AC-05 (AC-01/11/13/14):** Typed generation API, low-level compatibility,
   four-tool opt/noopt/both applicable VM consumers, persistent/shared-worker
   examples and installed/relocated native delivery pass their selected controls.
   Retain RexxDoc contracts and update human/agent guidance.
6. [x] **S5-AC-06 (all):** Reconcile all steps/criteria and the parent disposition.
   Required ordinary regressions pass without hidden skips/weakening. STEP-06
   explicitly retains maintained sanitizer, hardware/package matrix and exact-head
   hosted gates, including open release-blocking SAN-009 under Codex/Adrian.
   STEP-07 may overlap remaining hardware proof. Do not claim overall release
   readiness or generation completion while phase criteria remain open.

## Numbered implementation steps

1. [x] **S5-01 (AC-01/02/04):** Review the pinned contract/source, preserve ordinary
   failing generation consumers and direct positive controls, record fixed
   comparison shape and any concrete API decisions before production edits.
2. [x] **S5-02 (AC-01/02):** Extend the existing bridge request state and C RXPA
   interface for prompts, bounded prefill/decode, greedy selection, UTF-8 chunks,
   limits, finish reasons, cancellation and inspection. Keep shared-model and
   private-session ownership; no new worker, registry or public language syntax.
3. [x] **S5-03 (AC-01/02/04):** Run minimum normal CPU/Metal semantic/reference
   and four-tool controls needed for a valid timing comparison.
4. [x] **S5-04 (AC-04):** Freeze, build Release and report the first matched
   single/four-row verdict. Investigate a recurring material Metal slowdown under
   Adrian's new direction; retain the causal evidence and any repair verdict.
   Keep the existing interactive verdict gate before broad closeout.
5. [x] **S5-05 (AC-03/05):** After the Release verdict is accepted, complete
   sustained/concurrent/co-resident, installed/native and documentation checks.
6. [x] **S5-06 (AC-06):** Complete required ordinary regression coverage, retain
   evidence, tick each criterion, and reconcile parent outcomes and later gates.

## Implementation choices within the approved contract

At the STEP-05 baseline, the bridge prepared SmolLM2 contexts but rejected
generation request creation. The implementation reuses its request/owner graph
and model registry. A separate generation
engine/registry would duplicate ownership; a whole-request synchronous loop would
violate cancellation/incremental processing. Neither is selected.

Use the approved explicit SmolLM2 system/user template (default system text when
empty), greedy sampling and 512-token sequence / 32-output-token default. Prefill
processes at most the configured physical batch and caller work budget per call;
decode batches independent ready rows within that budget. Preserve the first
sample of every completed prompt before another decode replaces logits.

Extend the existing typed naming pattern with `model.generation_session(config)`,
`generation_session.request(config)`, `generation_request.add(system,prompt)`,
`add_all(prompts,system)`, `submit/process/state/cancel/close`, and
`read(row)` returning an owned `generation_chunk` with `text`, `tokens`, `finish`,
`row`, status and diagnostic methods. Low-level `addprompt`/`readtext` retain
STEP-01 signatures. Public spelling remains reviewable with the first verdict.

New aggregate QA scripts remain explicit/unregistered until STEP-06 has normal
and sanitizer scheduling evidence; no sanitizer execution is authorized now.

## Closure and takeover — 15 September 2026

All S5-AC-01–06 and S5-01–06 are complete locally. The
[ordinary qualification bundle](../qa/native-inference-step05/README.md) retains
commands, identities, the complete coverage ledger and failed controls with their
dispositions. The [parent disposition](native-inference-backlog.md#step-05-parent-acceptance-disposition--15-september-2026)
adds these results to the unchanged OUT-01–05, CREXX-NI-01–07 and AC-01–14.

Adrian accepted the [first Release verdict](../../performance/evidence/2026-09-15-ni-s5-first-release/README.md):
104 passing processes, identical completed work, CPU one/four-row mean paired
changes -0.00%/+2.66% and Metal -16.82%/-2.52%. No material positive Metal
recurrence triggered a root-cause investigation. The host was not quiescent;
these figures do not establish general speedup or explain the earlier embedding
observation. No timing panel was repeated or inference logic tuned during closeout.

[S5-D01](native-inference-output-boundary-proposal.md) is implemented under the
approved simple counted-output contract: C byte lengths, VM validation and
codepoint accounting, owned copies including U+0000 and aliases, optional host
size/version negotiation, no global padding or legacy initializer change.
Typed C RXPA owners/chunks and low-level addprompt/readtext share the same engine.
Source contracts and installed persistent/shared-worker examples are complete.

Native Debug/Release CPU/Metal controls pass exact boundaries, malformed/error
handling, output limits, incremental UTF-8, preparation/prefill/active-decode
cancellation and recovery. Each full qualifier retains 100 singles, 20 four-row
batches, 1/2/4 owners each processing 20 batches, private direct-reference output
parity, shared allocation identity and co-resident BGE/Smol processing. Retained
RSS growth is zero. Twelve separate direct/bridge/cREXX one/four-context processes
pass the unchanged wrapper-memory allowance.

The minimum 16 public executions are retained from the first verdict. The 24
additional Debug consumers pass through both VMs and optimization modes: nine
unchanged passes, one isolated replay and fourteen remaining runs. All 32
installed Release VM runs, 16 relocated native runs and eight native builds pass.
The fresh broad run passes all 2,347 selected ordinary Debug CTests; performance
measurement tests are outside this regression gate. No test was disabled or
semantic assertion weakened.

### Retained corrections and measurement limits

The first new Debug qualifier incorrectly applied the 250 ms Release timing
gate and stopped at 299.512 ms. It now follows STEP-02: Debug timing is diagnostic;
ordinary Release gates unchanged isolated work sizes, before concurrent work.
Release isolated maxima are 133.932 ms CPU and 87.1309 ms Metal. Concurrent
1/2/4-owner maxima are CPU 137.219/197.395/2,640.34 ms and Metal
83.2419/179.148/461.575 ms. Token work budgets are not wall-clock deadlines or
GPU preemption guarantees; application worker/native-thread choices matter.
No work, row, token or context limit was reduced to hide a miss.

The low-level consumer first assigned `state` inside a loop and read a different
binding afterward. The retained compiler warning is `#NOT_IN_SAME_SCOPE`.
Moving initialization before the loop fixed the fixture's cancellation/completion
assertions. Both assertions and all repeated workloads remain unchanged.

Overlapping Debug/package matrices reached the shared example's 120-second task
scope in optimized CPU `rxtvm`. Its exact image/arguments passed in isolation in
63.093 seconds. Remaining matrices ran sequentially and retained passing evidence
was reused. No product, workload, assertion or deadline changed.

### Next phase and unchanged approval boundaries

STEP-06 owns maintained sanitizers, the complete hardware/package matrix and
exact-head hosted publication gates. **SAN-009 remains open and release blocking,
owned by Codex under Adrian.** Its existing repair and the S3-D01/S4-D01/S5-D01,
native-object and typed-call changes still need that named platform proof. New
aggregate sanitizer memory accounting must follow S2-QA01's live-allocator
policy; sanitizer timings stay diagnostic and scheduling must be measured before
registration. No sanitizer build or run occurred in this closeout.

Windows/Linux/CUDA/Vulkan, remaining driver/low-memory/provisioning/provenance
cells and overall initial-delivery acceptance remain open. STEP-07 may start or
complete before unavailable hardware proof finishes, as already approved.
No user-prefix installation, commit, push or publication occurred in this phase;
the install was disposable QA only. Local STEP-05 closure is not a sanitizer-clean
or release-ready claim and does not shrink the parent scope.

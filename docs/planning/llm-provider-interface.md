# LLM-API-01: common drivers and native model compatibility

Status: D-01–04 approved by Adrian on 17 September 2026; implementation locally qualified
on `temp/llm-common-drivers`. Adrian requested review of the two-model restriction, native
llama as an ordinary LLM driver, and accurate, readable documentation.
Adrian clarified in this review that llama is optional: selecting it when the
plugin is unavailable must raise a catchable application-level exception.
That requirement is implemented using late-loaded RXPA factories and NOTREADY;
qualification remains tracked below. Adrian also authorized publication to
`origin/develop` when green, with local additional-model checks, on 17 September.
This is the implementation-plan record for the existing
[LLM-API-01 roadmap item](../ROADMAP.md). Architecture and compatibility decisions
D-01–04 below are approved. Any additional loader/ABI architectural extension
still needs the bounded review specified in D-04.

Reviewed checkout: `develop`, `d0feda283`. The existing
[native-inference plan](native-inference-backlog.md) remains authoritative for
OUT-01–05, NI-01–07, AC-01–14 and STEP-01–07. This follow-on neither closes nor
weakens its open platform, model-provenance or release criteria. The current
native delivery is not blocked by this new work.

## Vision and intended outcomes

An application selects its inference driver during setup and then uses the same
generation contract for Ollama, OpenAI, Anthropic, Gemini or local in-process
llama. Changing the transport does not require rewriting its processing code.
Native generation retains explicit preparation, loaded weights, private sessions,
bounded batches, incremental output and cancellation. The simple synchronous
call reuses that prepared owner. Applications can continue using the lower-level
typed llama owners when they need direct control.

Model selection must not be restricted to two compiled-in filenames or hashes.
Users can supply compatible local GGUFs with explicit identity and preprocessing
settings. The product reports what it can execute separately from which exact
model/backend combinations have been qualified. Existing BGE/Smol reference
profiles, preprocessing and retained evidence remain reproducible.

Embeddings are a separate capability with packed numeric results and an explicit
embedding-space identity. HTTP-only users retain an installation without llama
dependencies. A common application can start without the optional plugin and
catch a provider-unavailable exception if it selects llama, then make its own
fallback decision. CPU/GPU selection, Windows/macOS/Linux delivery, native consumers,
VM-local ownership and shared immutable weights remain part of the intended
outcome. Documentation starts with choosing a driver and running a useful example,
then explains configuration, lifecycle and limitations.

## Review findings

1. **R-01 — Model qualification is enforced as an allowlist.**
   `lib/plugins/llama/bridge.cpp:342–405` accepts only two profile strings and
   requires their two fixed SHA-256 values after checking the actual file hash.
   Another quantization of the same model is rejected. This is a first-party
   provider policy, not an upstream limit of two models or two live owners.
   The initial plan deliberately qualified a small reference set (NI-06 and
   STEP-01). That explains its origin but does not satisfy general model use.
2. **R-02 — The model-specific assumptions extend beyond the loader.**
   Capability selection uses the profile string; dimensions are fixed at
   384/960. Embedding allocation, normalization and result dimensions assume
   384; pooling is CLS, query preparation is BGE-specific, and input is capped
   at 512 tokens. `generation.h:4–9` embeds Smol's ChatML/default system prompt.
   Weight/context reservations are fixed profile estimates. Removing only the
   allowlist would give other models incorrect preparation or accounting.
3. **R-03 — The main interface is not yet interchangeable.**
   `lib/rxfnsg/rexx/llm.crexx:32–65` declares `.llm` and Ollama's implementation.
   OpenAI, Anthropic and Gemini at lines 189, 325 and 468 are independent classes.
   Their constructors also have different signatures from `.llm`'s factory.
   `typed.h` declares independent llama owners, with no `.llm` implementation.
   The ADDRESS demo selects concrete HTTP clients itself. Adding one native
   constructor would not by itself unify these consumers.
4. **R-04 — HTTP details are mandatory interface members.**
   `generateJson`, `buildBody`, `buildRequest`, `extractBody`, `extractText`,
   `lastJson` and `lastHttp` sit on `.llm`. A native driver has no HTTP exchange
   to report. Shared processing needs capabilities/results/errors independent
   of the transport, with a deliberate policy for the existing helper callers.
5. **R-05 — Current guides contradict retained status.**
   The llama README still called SAN-009 open, installation text called signing
   wholly pending, and the agent guide repeated those old phase boundaries.
   Other pages recorded closure and successful candidate installer checks.
   Calling the only accepted artifacts merely “examples” also hid the runtime
   restriction. The review corrects these current guides and links their status
   to retained evidence; historical QA records are preserved.

[Review evidence](../qa/llm-interface-review-20260917/README.md) contains the
bounded compiler/runtime checks and their limitations. Existing HTTP tests pass;
their coverage does not assert common-interface substitution. No hosted inference,
model download, new hardware qualification or broad suite was needed for review.

## Approved decisions

### D-01 — Separate model compatibility, identity and qualification

Retain the caller's expected SHA-256 and verify the actual bytes. Keep the two
existing named profiles as reproducible presets with their current strict
artifact identities. Add a general, configuration-driven model path that is not
restricted by a registry of approved model names or hashes.

Validate architecture/capability, output dimensions, context, pooling,
normalization, tokenizer and chat template using the pinned engine's metadata
and APIs. Require explicit task prefixes or template/pooling overrides where
metadata is insufficient. Unsupported or ambiguous combinations fail with an
actionable diagnostic; never silently apply BGE or Smol defaults. A trained
embedding profile needs an intentional preprocessing specification, not just
an architecture name. Qualified status is informational, not a prerequisite for
loading a compatible user-supplied artifact.

Dynamic dimensions must reach result allocation and packed-vector consumers.
Replace profile-sized memory estimates with model/configuration-dependent
admission accounting before broadening loading. Reject unsupported memory/state
layouts, allocation overflow, incompatible operations and oversized inputs.
Preserve bounded worker-private contexts and share compatible weights only.

Use the existing pinned engine first; no dependency upgrade is assumed. Its
`llama_chat_apply_template` explicitly supports a finite template set rather
than arbitrary Jinja. Reject unsupported templates or use an explicit supported
override; do not claim every GGUF or template works. Qualification tests retain
exact artifact/settings/backend identities separately from this compatibility
decision. No automatic downloads or RAG index migrations are introduced.

### D-02 — Make llama an explicit driver of the main LLM interface

Approved entry point: `.llm.open(config)`, with a typed configuration selecting
`ollama`, `openai`, `anthropic`, `gemini` or `llama`. These names and configuration/result contracts are implemented below.
Connection/model settings belong to setup. A local native configuration provides
path, hash and model settings; HTTP configurations provide their endpoint/model
and credentials. Unknown drivers fail instead of falling back to Ollama.
Selecting the known but unavailable `llama` driver raises a catchable application
exception with a stable provider-unavailable category and an actionable message.
It must not return an apparently usable client or require a later `status()`
check to discover plugin absence. Distinguish a missing plugin, an incompatible
provider/runtime package and a missing/invalid model in the diagnostic; never
silently substitute a server or another model. Use the existing signal/exception
facilities, with the precise condition/payload fixed in STEP-01.

Every selected driver returns the same `.llm` contract for `generate(prompt)`,
`status()`, `error()` and `close()`. Add consistent owned result/error values and
capability discovery, then common preparation/request contracts for supported
batch/incremental operations. Normalize finishes without calling an unknown
HTTP finish reason `eos`; retain provider codes/details separately. HTTP errors
and native negative codes must not collide in the normalized error category.

Capability discovery distinguishes generation, embeddings, batching, incremental
delivery and cancellation granularity. Current HTTP calls are blocking and
non-streaming; report that truthfully. Native `process(work_tokens)` keeps its
bounded-compute semantics. A work-token budget does not become an HTTP timeout
or a hard wall-clock deadline. Unsupported operations produce a consistent
error, rather than simulated streaming, invented results or ignored options.

Embedding operations use a separate typed capability selected through the same
driver configuration. Their result includes dimensions, packed values and the
complete model/preprocessing identity. Generation-only providers may reject that
capability. Do not force HTTP embedding APIs into existence in this change.

### D-03 — Preserve existing callers while separating transport details

Keep `.llm(model, host, port, timeout)` as the Ollama compatibility factory and
keep the existing direct hosted constructors and typed native API. Importing
the optional native driver must never change the old factory's selection.
Factory signatures must satisfy the existing language rules; merely adding
`implements .llm` to each hosted class is not sufficient.

Use driver adapters where the existing concrete constructor shapes require
them. HTTP adapters reuse the current HTTP owners and request/parsing code.
Implement native integration through C RXPA using the existing owners/bridge;
do not add a duplicate Rexx facade or separate model registry. The convenience
client owns and closes its children in order, preserves failure snapshots and
does not reload weights per prompt. It must not mask a failure during cleanup.

For source compatibility, retain the existing HTTP/JSON members on `.llm` as
legacy diagnostics. New common consumers do not depend on them. The native
driver reports an explicit unsupported-operation diagnostic for HTTP-only
methods; it never invents an HTTP exchange or a provider JSON body. Keep new
transport-specific inspection outside the shared processing contract. Removing
the legacy methods would be a separate breaking change, not implicit in this
proposal. `generateJson` does not mean constrained JSON generation.

Update ADDRESS driver selection to use the common client for generation while
preserving its existing aliases and HTTP diagnostic commands. A native model
path/hash/configuration is supplied explicitly, not guessed from an environment
name. Embedding calls remain separate from text generation.

### D-04 — Optional delivery and documentation remain first-class

Core contracts and common applications must compile and start without importing
or linking the optional provider. An explicit native-only import may retain its
existing requirements, but the common `.llm.open(config)` path must defer native
availability failure until selection, inside application exception handling.
In particular, requiring every switchable client to `import llama` would not
satisfy this requirement if compilation or VM startup failed before a handler ran.

STEP-01 must prove the optional-driver discovery/loading boundary against the
existing compiler metadata, package manifests and VM loader. Compile-time
interface factory selection alone is not assumed to provide runtime discovery.
Prefer existing RXPA/loader mechanisms and core-visible contracts; if a bounded
loader or ABI extension proves necessary, present it for architectural approval
before editing it. HTTP-only packages must contain no new llama engine dependency.
Check the plugin absent at build time, absent/removed at run time, present and
compatible, and present but incompatible. Every absence case on the common path
must reach the application's handler, after which another HTTP driver remains
usable. Installed dynamic and relocated native consumers both need that proof.
No language syntax, model server or worker pool is proposed.

Document one common-generation example that switches only setup, a separate
embedding example, and advanced persistent/batch/worker examples. Distinguish
accepted model configuration, tested reference artifacts and qualified hardware.
Preserve/update source RexxDoc blocks with the API and keep status in linked
qualification ledgers rather than duplicating stale phase narratives.

## Numbered acceptance criteria

### STEP-01 implementation contract, 17 September

The existing `loadmodule()` plus late-bound native interface factories passes
on both VMs with the plugin present and absent. The absent case reaches
`ON SIGNAL NOTREADY` and then constructs/uses the HTTP client successfully.
The consumer was compiled from interface-only declarations without importing
the native plugin. See `../qa/llm-interface-review-20260917/late-native-proof.txt`.
No new loader ABI or VM instruction is needed for that boundary.

Concrete library spelling:

- `.llmconfig(driver="ollama", model="")`: `set_text(key,value)`,
  `set_int(key,value)`, `text(key)`, `integer(key)`. Driver-specific secrets stay
  in configuration/HTTP owners and never enter diagnostics.
- `.llm.open(config)` returns `.llm`; legacy `.llm(...)` remains Ollama.
  Common additions: `driver()`, `supports(capability)`, `prepare(work_tokens)`,
  `request()`, `lastResult()`. Existing methods retain their signatures.
- `.llmrequest`: `add(system,prompt)`, `add_all(prompts,system)`, `submit()`,
  `process(work_tokens)`, `state()`, `read(row)`, `cancel()`, `status()`, `error()`,
  `close()`. HTTP supports one row and one blocking process call; native supports
  bounded batches and incremental processing. No pretend HTTP streaming.
- Owned `.llmchunk` exposes `text()`, `tokens()`, `finish()`, `row()`, `status()`,
  `error()`. Unknown HTTP token usage is `-1`; an unreported finish is `complete`,
  not `eos`. `.llmresult` adds driver/category to the simple-call snapshot.
- `.embedding.open(config)` is the separate embedding client with `prepare`,
  `embed(texts,role)`, `status`, `error`, `close`. Owned `.embeddingresult` contains
  packed values, rows, dimensions and a stable model/preprocessing specification.
- Core `.llmnative` is an internal late-bound interface implemented in C by
  rxllama using existing bridge resources. The core client supplies explicit
  trusted `provider_path` when configured, otherwise an exact plugin path beside
  the running VM/native executable. It does not sweep arbitrary directories.
  Missing/failed loading raises `NOTREADY` with `provider_unavailable: llama`;
  an incompatible interface/package is diagnosed distinctly. Native-only imports
  and static native packaging retain their existing paths.
- Generic low-level profiles are `generation` and `embedding`. Existing BGE/Smol
  names retain strict pins. Embedding preparation settings must be explicit;
  generation uses a supported GGUF template or an explicit template override.

These spellings implement approved D-01–04; unsupported model/layout/template
combinations remain diagnosed rather than silently approximated.

The acceptance scope below remains binding. See the live implementation ledger
for verified checks and outstanding criteria; review completion alone does not
close implementation acceptance.

| ID | Observable pass condition | Required evidence |
| --- | --- | --- |
| LLM-AC-01 | One typed consumer selects all five drivers at setup and uses unchanged common generation code. Unknown drivers fail explicitly. | Common conformance consumer; deterministic HTTP fixtures for each provider and real local native execution; no API keys required for regression QA. |
| LLM-AC-02 | Existing `.llm(...)`, direct hosted constructors, HTTP helper behavior, ADDRESS aliases and direct native owners remain compatible. | Current functional tests plus source/assignment/factory selection controls, including imports with and without llama. |
| LLM-AC-03 | Additional compatible GGUFs load without adding names/hashes to C code. Original presets retain their exact behavior. | A generated fixture plus at least one additional trained generation artifact and one additional trained embedding artifact with different dimensions; deliberate alternative quantization; altered-hash, unsupported capability and unsupported template controls. Freeze identities/licences/settings before running. |
| LLM-AC-04 | Preprocessing, context, dimensions, template and reservations derive from validated configuration/model properties. Embedding identity changes when its space changes. | Matched upstream controls, dimension/pooling/prefix/template/token-boundary checks, finite packed output, overflow and low-budget rejection. No silent truncation or default substitution. |
| LLM-AC-05 | Native common clients prepare once and preserve batching, incremental output, cancellation, private contexts and shared weights. | Load/allocation counters; repeated single and ordered batch requests; UTF-8/NUL output, independent contexts, cancellation and ownership/close controls. Reuse valid lower-level evidence where inputs are unchanged. |
| LLM-AC-06 | Results/errors/capabilities are consistent and truthful across drivers. Unsupported operations are explicit. Selecting an unavailable llama driver raises an application-catchable provider-unavailable exception. | Common error/finish/result-lifetime checks; application handler asserts category/driver/reason, then continues with another driver; distinguish missing plugin, incompatible package and bad model; HTTP transport/server/parse and native inference/cleanup controls. |
| LLM-AC-07 | Embeddings remain independent and use existing packed/vector facilities with inspectable model/preprocessing identity. | Embedding conformance consumer and index-identity mismatch checks; generation-only providers reject unsupported embedding calls. |
| LLM-AC-08 | Common clients compile/start without llama, and catch its absence when selecting that driver. Optional packaging and native/dynamic consumers work on maintained target OS/toolchains without imposing llama on HTTP-only users. | Provider absent at build time; removed/absent at run time; installed compatible and incompatible package controls; application handler and subsequent HTTP driver use; installed dynamic/relocated native checks on macOS, Windows and Linux. Real GPU claims remain device-specific and unavailable agreed cells stay open. |
| LLM-AC-09 | New behavior has focused regression coverage and appropriate ordinary correctness/build evidence. New native ownership/loading paths have focused sanitizer evidence. | Tests first for the reviewed gaps; rxc/rxas/rxlink/rxvm in applicable optimization/VM modes; focused normal/maintained sanitizer controls. Use ordinary publication policy, reuse unchanged evidence and do not dispatch extra overnight matrices without a concrete risk. Actual sanitizer findings follow SAN rules. |
| LLM-AC-10 | Human and agent guides agree with code and runnable examples, preserve API documentation and separate current behavior from proposals/evidence. | Guide/RexxDoc review; installed documentation/link checks; runnable examples; all remaining criteria and platform limits recorded. |

## Numbered implementation steps

1. **LLM-STEP-01 — Review and freeze the contract** (complete; D-01–04 approved): retain findings,
   model restrictions, compatibility examples and D-01–04. Obtain Adrian's
   architecture/API decision, prove how deferred optional-plugin selection can
   reach an application handler, then record exact method/type signatures and
   error/capability semantics before product edits. Any necessary loader/ABI
   extension needs its own explicit bounded approval. Serves all criteria.
2. **LLM-STEP-02 — Add decisive acceptance controls** (complete; depends on STEP-01):
   common typed consumer and deterministic HTTP fixtures; model generalization
   failures alongside original-profile controls. Pin the small additional model
   evidence and retain doc-tag coverage. Serves AC-01–04/06/09/10. Measure any
   materially expanded nested aggregate before registering it with CTest.
3. **LLM-STEP-03 — Generalize model handling** (complete; depends on STEP-02): replace
   profile-dependent shape/template/preparation/admission assumptions; keep
   original presets and checked artifact identity. Serves AC-03–05/07/09.
4. **LLM-STEP-04 — Implement common drivers** (complete; depends on STEP-02/03):
   contracts, compatibility factories, HTTP adapters, optional native C driver,
   common results/errors/capabilities and separate embeddings. Wire ADDRESS
   generation through the common client. Serves AC-01/02/05–09.
5. **LLM-STEP-05 — Verify delivery and refresh examples/guides** (local checks complete; publication checks govern delivery acceptance; depends
   on STEP-03/04): run the smallest decisive correctness/lifecycle/package checks,
   retain source/build identities and update current documentation together.
   Serves AC-08–10 and closes only criteria actually demonstrated. No upstream
   speed tuning is in scope; if performance-programme work is proposed, apply
   its separate first-Release-verdict gate before broad closeout.

## Review handoff (historical)

The initial review confirmed both weaknesses. Adrian approved D-01–04 on
17 September, including the unavailable-plugin application exception. The
late-load proof and exact signatures above subsequently fixed the implementation
contract. Use the live ledger below for current acceptance and next actions.

### LLM-STEP-02a — Unavailable-factory signal restoration

A minimal cREXX reproducer on 17 September shows that `srcfprocsel` raising
FUNCTION_NOT_FOUND leaves its call arguments swapped out of caller variables.
The existing cold-path restoration recognizes CALL/DCALL but omits factory
selection, whose argument-count operand uses the same position. This blocks
AC-06/08: an application must retain usable configuration after catching absence.
Evidence: `/tmp/llm-minimal.WLWioS` and `/tmp/llm-missing-factory.rxas` (to be
retained with the permanent regression). This is a correctness repair within
existing signal semantics, not a syntax/ABI or performance programme change.

1. **STEP-02a-01:** retain a self-contained absent-interface-factory regression
   asserting scalar/object arguments after the caught signal (AC-06/09).
2. **STEP-02a-02:** recognize the existing factory-selector count operand in
   the VM's interrupted-call mapping restoration (AC-06/08). No opcode changes.
3. **STEP-02a-03:** run both VMs and optimization modes, existing native-signal
   restoration tests and the new common-interface test; retain results (AC-09).

## Live implementation ledger

17 September: common HTTP/native contracts, generic GGUF metadata/preprocessing,
owned results, separate packed embeddings and ADDRESS integration are implemented
locally. STEP-01–04 and all local STEP-05 checks are complete. Delivery acceptance
is governed by the normal automatic checks on the published revision, as set out
in the linked qualification record; a missing or failed platform check leaves
that delivery criterion open.
Evidence is retained in [the qualification record](../qa/llm-interface-review-20260917/qualification.md)
and its linked command/source manifests.

| Criterion | Current verification |
| --- | --- |
| LLM-AC-01 | Passed locally: one common contract, four deterministic HTTP fixtures and native execution, both VMs and optimization modes; unknown drivers rejected. |
| LLM-AC-02 | Passed locally: legacy Ollama factory, direct constructors, HTTP diagnostics, ADDRESS aliases and original native preset controls. |
| LLM-AC-03 | Passed locally: independent generated fixture, Stories generation, BGE-base F16/Q4_K_M at 768 dimensions, plus Gemma 4 E4B raw-template CPU generation. Identities and provenance limits are retained. |
| LLM-AC-04 | Passed locally: separately loaded upstream embedding controls (CLS/mean), dynamic dimensions, prefixes/specification, template/tokenizer/context/budget/hash negative controls. |
| LLM-AC-05 | Passed locally: repeated/batched native requests, cancellation, child closure, retained output, original preset parity/ownership controls and shared-weight counters. Existing unchanged lower-level controls remain part of the correctness suite. |
| LLM-AC-06 | Passed locally: failure snapshots, truthful capabilities/finish/token counts, HTTP errors and unsupported operations; missing/incompatible plugin reaches NOTREADY and HTTP recovery. |
| LLM-AC-07 | Passed locally: independent packed embeddings retain values after closure; changed preprocessing changes identity; HTTP embedding selection fails explicitly. |
| LLM-AC-08 | macOS Debug/ASan installed dynamic and relocated native cases pass, including absent/incompatible/present/removed plugin. Linux/Windows delivery acceptance is recorded by the published revision's normal Build package matrix; missing or failed checks leave this criterion open. No new real-GPU claim. |
| LLM-AC-09 | Passed locally: product build, 2,333 passing comprehensive cases plus all 16 repaired static-harness cases, and focused normal/Apple ASan controls. Apple LSan is unsupported; no first-party sanitizer finding was observed. |
| LLM-AC-10 | Guides/examples, installed documentation, 88 local Markdown targets and preserved RexxDoc coverage pass. Publication status is the associated revision's automatic check record, as linked below. |

The sanitizer runner's generic `focused-lsan` preparation target omits the new
standalone HTTP fixture binaries. Its first run passed 15 tests but could not
open those two bytecode files. Building `ts_llm_http_fixture` explicitly and
rerunning those two tests passed; the common aggregate had already passed the
same HTTP scenarios. This was missing test preparation, not a product failure.

The comprehensive run exposed a further integration gap: the RXPA object-test
VMs statically register their object fixture but omit the bundled filesystem
provider now referenced by the common loader. All 16 static object/worker cases
failed before entry with unresolved `rxfs.loadpath`. Their CMake provider closure
now uses the existing static-link helper. All 16 cases passed again in Debug
(11.46s) and the maintained ASan build (17.24s). The common library's core filesystem
dependency is already installed/packaged by ordinary product builds; this repair
does not add a llama dependency or alter runtime loader semantics. The other
2,333 comprehensive cases passed in the 769.35s run; their inputs are unchanged
and that evidence is reused. See the retained normal log and focused rerun.
All local AC-09 checks are satisfied.

Publication is authorized after relevant local checks are green. Then monitor
normal automatic publication gates to terminal completion. Do not dispatch
extra overnight matrices or imply real-GPU/platform/model qualification beyond
retained evidence. Preserve existing parent-plan outstanding acceptance.


Final-review corrections before qualification: removed the obsolete Ollama
interface matcher from the now-concrete class (direct `.ollama` import now
compiles); explicit options for another driver are rejected instead of ignored;
counted model path/hash/profile text reaches native validation without NUL
truncation. The text fixture was replaced with a standalone generated model and
byte vocabulary, with no trained tokenizer input, to give distributed test data
an unambiguous first-party origin. Final evidence must use the hash recorded in
`tests/native-inference/fixtures/text-manifest.json`; earlier fixture runs are
historical first-pass evidence. Existing trained Stories checks remain separate.

Final configuration review also found that the newly configurable hosted origin
was not reflected by legacy `buildRequest` diagnostics. Hosted clients now copy
their validated HTTP owner's host/port during construction; the common HTTP
fixture asserts that diagnostic requests use its configured local endpoint.
This source/test change invalidates earlier library-input qualification until
the focused HTTP checks and affected build outputs are refreshed.

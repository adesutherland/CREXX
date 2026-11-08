# UI architecture and capability expansion

Status: design and implementation plan; no browser or 2D driver has been
selected or delivered. Browser-local RAG, database and GPU compute are option
gates, not selected delivery scope. This is the authoritative UI expansion plan. The
[portfolio roadmap](../../docs/ROADMAP.md) owns Release 1 ordering and the
January platform freeze; this plan does not add these capabilities to an
unapproved Release 1 baseline.

## Vision and boundaries

Keep application state, transitions, commands, and effect decisions in Level G
components. Develop the GTK desktop, ANSI full-screen terminal and browser
hosts as parallel projections of the same application contract, stage by stage.
Let the same application present forms and interactive 2D scenes where supported,
and choose semantic alternatives or refuse unavailable operations elsewhere.
An RXPP declaration may shorten view, layout, and command wiring, but generated
code must use the ordinary UI contract and leave application behavior with its
feature owner. A browser version should execute that owner in a matching cREXX
WebAssembly runtime where
feasible. Browser API access still needs a small host bridge.

The intended examples are an editable UML-like graph and a pan/zoom strategy
map with selectable units and terrain. They are acceptance workloads, not a
request to put UML or game rules into the UI library. ANSI may provide a
documented list or cell viewport fallback; pixel-equivalent graphics across
every host are not required. The existing line TUI remains a compatibility host,
not a second full-screen graphics target. Existing UI contract v0 and all three
reference hosts must keep working. Public scene, input, ABI, packaging, and
target-platform choices remain explicit design gates before product implementation.

Sequence the core contract and handwritten consumers first. Extend Peter's
RXPP façade only after the capabilities it describes have stable semantics and
at least one direct consumer. Peter may own that later authoring increment; its
ownership and timing do not block core UI development. Keep the existing
experimental macro available, with its limits documented, while the core work
proceeds.

## Current baseline and review findings

- `ui_contract.crexx` supplies session routing, bounded queues, capabilities,
  correlated effects and close lifecycle. `ui.crexx` supplies a flat logical
  grid. Text Inspector runs through line, ANSI and GTK drivers; none is a
  browser or general 2D driver. `catalogue.json` contains planned pointer,
  viewport and web vocabulary, not implemented scene support.
- Peter's `DEFINE-GUI` uses the generic RXPP `DALIAS ... HANDLER` path. The
  current macro generates nodes, binding metadata, a controller, command
  registration and static callback dispatch. This is a useful authoring layer,
  and RXPP needs no built-in GUI parser. `rxBuild` expresses its build graph;
  CMake still prepares GTK/native prerequisites.
- The generated controller stores `node|value` display bindings separately from
  the feature model. Setters can change rendered values without changing the
  feature's state or its next `view()`. The macro has Text Inspector-specific
  callback names and a `ui.resource.selected` special case. It silently skips
  unsupported declarations, including `MENU` and `ITEM` in the authored
  sample; `ON` branches and rendering support are not one validated grammar.
  These are experimental limits, not a general application architecture.
- The example graph names `text_inspector_gui_macro_test.crexx` and
  `text_inspector_gui_macro_gtk.rxpp`, which are absent from the checkout.
  Its macro run/probe targets therefore need source/dependency repair and
  focused replay before being cited as executable evidence.
- CREXX's single-threaded Emscripten profile and THE's separate browser
  preview demonstrate a matching Worker/runtime cohort. They do not implement
  `uidriver` or qualify this UI library in a browser. THE owns its editor UI;
  this plan can reuse host lessons without making THE an application shell.
- Existing host capability sets describe service families and are fixed when a
  v0 session is constructed. Absent effect capabilities already produce a
  correlated `ui.effect.failed` result. An unsupported widget currently fails
  its host's render path; this is not yet an application-visible choice of
  alternative presentation. The 2D contract must close that gap deliberately.

## Parallel host coverage and refusal contract

GTK, ANSI and browser are delivery tracks for each selected application stage.
The same Level G feature owns transitions and effects; host-specific views may
differ. A stage is complete only when each track has its stated projection or
fallback working and explicitly refuses any unavailable optional operation
with a reason visible to the application and user. The line TUI retains its
existing widget checks;
additional line-mode coverage is selected only when a feature needs it.

| Stage | GTK desktop | ANSI full-screen terminal | Browser | Shared check |
| --- | --- | --- | --- | --- |
| Forms and dynamic widgets | Native controls and current session routing | Cell controls, keyboard and supported pointer input | Matching cREXX feature through a selected local Wasm or hosted session, with semantic HTML controls | Same command/effect/close transitions; host-specific views and failure paths. |
| Editable graph | Native 2D projection with keyboard and pointer editing | Semantic node/edge list or bounded cell viewport; edit/connect by stable ID where usable | SVG projection with keyboard and pointer editing | Same graph model, selection and undo history; unsupported gestures refuse explicitly. |
| Strategy map | Native 2D terrain/unit projection with pan/zoom and selection | Bounded cell viewport and unit/terrain detail/actions where usable | Canvas 2D projection with pan/zoom and selection | Same rules, turns and stale-input handling; unavailable precision or interaction is declared. |
| Application services | Correlated native or hosted effects | Correlated native or hosted effects | Correlated effects through the selected browser session; hosted RAG first, local database/RAG/inference separate options | Same result and cancellation semantics, with provider and storage limits stated per host. |

The approved support direction combines capability discovery with render
preflight. A host advertises precise widget, scene, input and limit support at
session composition, not only broad families such as `ui.widgets` or
`ui.pointer`. The feature or application composition chooses a named semantic
fallback and which commands to enable. Before projecting each complete view or
scene, the driver validates its required operations and limits. An unexpected
unsupported request returns a typed, correlated refusal to the application
owner, leaving the last valid frame intact. The owner may render an alternative
or explain why the action is unavailable; retries must be bounded. This does
not move application state or fallback policy into the driver. Specify the
exact manifest, refusal API, retry count and state/close behavior at the public
contract gate; current render errors do not yet supply this behavior.

Capabilities remain fixed for the life of a session in the first extension.
A host may recover a lost device or switch renderers internally only while it
still provides the advertised semantic operations. Otherwise it reports a
typed host-loss failure and ends that session; a new session is composed from
application-owned saved model state. Define the exact failure, in-flight work,
state snapshot and restart protocol before claiming automatic recovery. The
catalogue's `ui.capabilities.changed` remains future vocabulary, not a live
v0 or first-extension promise.
Terminal fallbacks need meaningful keyboard commands and readable labels, not
a promise that freehand dragging or dense graphical maps work in cells. For
operations with no useful fallback, the application disables or rejects the
command with a specific reason. Test the successful alternative and its
refusal path against the same domain transition journal on every host.

## Options and recommended sequence

| Choice | Shape | Benefit | Cost or limit | Decision |
| --- | --- | --- | --- | --- |
| Local browser Wasm | One Worker owns cREXX VM, session and feature; a small main-thread browser adapter projects semantic views and returns normalized input. | Application behavior stays in cREXX; local use need not run a server session. | Wasm cannot directly use DOM; JS/TS glue, async effects, asset packaging and Worker lifecycle remain. | First browser proof candidate; select after a matching UI image and callback/lifetime spike. |
| Hosted browser | Native cREXX session on a server; browser renders versioned views over an authenticated transport. | Server resources, collaboration and larger workloads are easier to own centrally. | Authentication, authorization, reconnect/revision rules and latency are part of the product. | Later or application-specific path, sharing the same logical view and events. |
| Wasm graphics renderer | Wasm computes layout/geometry and possibly draws through Canvas/WebGL bindings. | Keeps expensive scene calculations in cREXX/native code. | DOM controls, accessibility, input and browser APIs still need host glue. | Optional optimization after the semantic browser driver works. |

| Surface | Initial fit | Upgrade condition |
| --- | --- | --- |
| SVG plus ordinary HTML controls | Editable diagrams with modest object counts, stable element IDs, labels and focus/selection overlays. | Move a heavy layer to Canvas when measured DOM cost or scene size warrants it. |
| Canvas 2D | Bounded map/tile prototype and shared primitive renderer. | Add culling/batching first; use WebGL2 if measured redraw cost remains excessive. |
| WebGL2 | Large maps, dense layers or animation requiring GPU batching. | Gate on a representative map and fallback/restore behavior. |
| WebGPU | Optional renderer and separate compute facility for embeddings or small-model inference. | Select each workload independently after browser coverage, device limits and failure policy are qualified; compute may be useful before a WebGPU renderer. |

The application owns graph nodes/edges, map cells/units, selection, edit
transactions, undo/redo and game rules. A proposed reusable scene contract
would own stable element IDs, bounded geometry, z-order, world coordinates,
viewport transform, style and semantic labels. The host owns painting, physical
input, device-pixel conversion and browser/native resources. Define whether
shared scene hit testing lives in Level G or a single reusable lower-level
module; drivers must not each invent application picking rules. Stable IDs and
scene revision must make stale pointer events rejectable. Start with bounded
full snapshots and profile a real graph/map before choosing deltas or binary
transfer. No scene API shape is approved by this planning record.

## Browser data, retrieval and compute options

The web host must leave domain data and inference with their existing owners.
[`crexxrag`](https://github.com/adesutherland/crexx-rag/blob/main/docs/architecture.md)
is a separate Level G application: SQLite is authoritative for its
corpus, graph, embedding records and work history; generation-specific `.rxvec`
sidecars accelerate retrieval. Its retrieval owner checks vector membership
against the current SQLite snapshot, collapses embedding windows to cited
parent passages and retains model/input compatibility. CREXX's generic
`rxsqlite` and `rxvector` providers supply database and exact vector mechanisms;
the optional `llama` provider owns native CPU/Metal/CUDA/Vulkan inference. None
of these providers is currently a browser UI driver or a proved browser
deployment of `crexxrag`.

| Candidate | Boundary and first useful proof | Main unresolved work |
| --- | --- | --- |
| Hosted RAG query | A proposed authenticated adapter calls the existing native `crexxrag` query path and returns bounded cited results to a browser feature through an application-owned effect. This is the smallest end-to-end RAG integration candidate. | Define the service adapter, authorization, corpus/session identity, cancellation, result size and failure contract. The present CLI/JSON/MCP adapters are not an HTTP service by themselves. |
| Local read-only RAG | A matching Wasm Worker opens an imported, validated SQLite snapshot plus compatible immutable vector sidecars and performs lexical/vector/graph queries locally. Start with a bounded transient snapshot; persistent OPFS is a separate gate. | Cross-compile the needed Level G modules, `rxsqlite` and `rxvector`; prove bundle identity, sidecar verification, query-embedding provider, memory limits and reload, then VFS/persistence if selected. A native provider's existence is not proof of a Wasm build. |
| Full local RAG | Browser-local ingestion, embeddings, index publication, review and maintenance. | RAG currently uses independent VM processes, WAL connections, leases and transactional publication; a one-Worker browser profile needs a separate ownership and recovery design. Defer until read-only query is proved and offline authoring is selected. |
| Other application databases | A graph editor or map game stores its own domain model behind an application repository/effect. | Select schema, transactions, import/export and sync policy per application; do not couple those domains to RAG's SQLite schema or put SQL in the UI driver. |

An illustrative `rag.query` effect would carry a bounded query, corpus/version
identity and optional retrieval mode. Its completion would carry ranked passage
IDs, excerpts, citations, scores and the identity of the evidence snapshot;
the application feature decides what to show. A browser host must retain
request/session correlation, cancellation and stale-result rejection. DB
handles, paths and provider secrets must not cross the view or Worker message
boundary. A query embedding must use a compatible model, dimensions and input
representation; an unrelated small LLM cannot silently replace it. Lexical
retrieval remains a possible capability when no matching embedding provider is
available. UI persistence for editable graphs/maps uses the same effect pattern
but has separate domain transactions and undo rules.

WebGPU is relevant beyond drawing. It exposes compute pipelines, so a browser
embedding model or small generative model could be an application service
behind the existing embedding/generation concepts. This requires a browser
provider or a separately selected inference engine; the native `llama` GPU
backends do not automatically run through WebGPU. Keep heavy compute in a
compute Worker and return bounded results to the UI session owner; never make
`update()` wait for a model. Evaluate model format/identity, download and cache,
device memory, request limits, cancellation, device loss, privacy, numerical
compatibility and fallback to CPU or a hosted provider. GPU vector scoring is
another optional workload, selected only against the existing deterministic
CPU `rxvector` result/order contract and measured corpus sizes. A WebGPU map
renderer, vector scorer and LLM provider may share device discovery policy,
but each needs its own capability and qualification.

| Browser inference route | What it would reuse | Proof before selection |
| --- | --- | --- |
| Native/hosted `.llm` and `.embedding` drivers | Current cREXX provider concepts and server/native execution. | Async browser effect adapter, credentials/privacy route, bounded streaming and cancellation. |
| Browser WebGPU engine behind a new provider | cREXX application request/result policy, with an engine such as WebLLM as a comparison candidate. | Exact model and embedding compatibility, JS/Wasm bridge ownership, Worker lifecycle, device loss, local asset/cache policy and supported browsers. |
| Port of the existing native inference engine | Potentially its model formats and selected CPU code. | A real Wasm/WebGPU backend and package/ABI proof; Metal/CUDA/Vulkan paths alone do not provide it. |

[WebLLM's project](https://github.com/mlc-ai/web-llm) shows that browser-local
WebGPU generation is feasible, but is not a chosen cREXX provider or proof of
compatibility with the current RAG embedding corpus.

SQLite's browser OPFS VFS is a feasibility input, not proof that the bundled
`rxsqlite` provider can persist unchanged in the current Emscripten profile.
Browser storage is origin-scoped and subject to quota, eviction and user data
clearing, so any local authoritative library needs explicit export/backup and
recovery behavior. These choices are outside the UI contract and remain
unselected until a specific offline product outcome is agreed.

| Decision ID | Evidence required before selection | Status |
| --- | --- | --- |
| **UIX-D01** | Choose hosted, local read-only or full local RAG scope using a representative cited query, data size, privacy/offline requirement and lifecycle owner. | open |
| **UIX-D02** | For a local query option, prove matching `rxsqlite`/`rxvector` Wasm builds and sidecar identity. Before promising persistence, also prove SQLite VFS, multi-tab/storage behavior, export and recovery. | open; conditional on D01 |
| **UIX-D03** | Choose browser embedding/generation provider and whether WebGPU is required or optional using a real small model, compatible embedding identity, device/memory inventory, CPU/hosted control and failure policy. | open |
| **UIX-D04** | Select GPU rendering, vector search and inference independently from representative correctness and cost; do not infer one from another. | open |
| **UIX-D05** | Approved direction: precise host support at composition plus complete-view/scene preflight, application-owned fallback and command availability, and typed correlated refusal that preserves the last valid frame with bounded retry. Specify the exact API and prove refusal leaves no partial new projection or additional feature state mutation. | direction approved; API/proof open |
| **UIX-D06** | Approved direction: support is fixed for a session. Hosts may recover internally while preserving advertised semantics; otherwise report typed host loss and recompose a new session from application-owned saved model state. Specify failure, in-flight work, snapshot and restart APIs and prove state continuity before promising automatic recovery. | direction approved; API/proof open |
| **UIX-D07** | Select the first browser UI execution route: local cREXX Wasm Worker, hosted native session, or parallel implementation. Prove matching runtime packaging and owner-turn behavior for local execution, or authentication/reconnect/revision ownership for hosted execution. | open; browser host gate |

## Acceptance criteria

| ID | Observable pass condition and verification | Status |
| --- | --- | --- |
| **UIX-AC01** | A handwritten Level G feature/session handles equivalent commands, events, effects, dynamic values and close outcomes on two existing hosts, with one authoritative model. Compare independent expected transition journals and views. | open |
| **UIX-AC02** | Once the core contracts are stable, RXPP generates declarations that reproduce the direct consumer's current dynamic values and routing without a second authoritative model. It rejects malformed/unknown controls, duplicate IDs, inconsistent command definitions and invalid event/binding references with source locations; multiple controls may share one command. Generated code has no named Text Inspector special cases. Inspect representative cREXX/RXAS and run positive/negative and regeneration checks. | open; later authoring increment |
| **UIX-AC03** | A browser host runs a matching cREXX feature/session in the selected local Wasm Worker or hosted native route. It serializes owner turns, projects current controls, returns normalized input and correlated async outcomes, and closes or fails without leaked authority. Check browser interaction, one bounded synthetic application-service result, cancellation, stale/duplicate messages and Worker restart or hosted reconnect as applicable; no DB/model handle enters a view. | open; target and packaging choice unselected |
| **UIX-AC04** | A versioned 2D scene contract edits a representative graph: create/move/connect/delete, zoom/pan, selection, keyboard operation and undo/redo. GTK and browser project the 2D scene; ANSI offers a tested semantic node/edge alternative or refuses a specific unavailable operation. Compare shared model state and check chosen coordinate precision, stable picking under resize/scale and keyboard access. | open; scene API decision required |
| **UIX-AC05** | A representative strategy map renders terrain and units, supports pan/zoom, selection and turn actions, and keeps rules/state in the feature. GTK and browser provide 2D projections; ANSI supplies a bounded cell viewport and detail/actions where usable, with explicit refusal for unavailable interactions. Compare shared turn/state results; check bounded tiles, clipping/culling, pointer coordinate conversion and stale input on graphical hosts. Measure before selecting an accelerated renderer. | open |
| **UIX-AC06** | Unsupported view/scene operations are declared at application composition or returned as structured refusals before partial projection, and the feature selects a fallback or a specific rejection. Absent effect capabilities retain correlated failures. Malformed assets/messages, context loss, resize, cancellation and setup/teardown failures have explicit behavior; browser resource grants cannot be forged from display text. Run focused host and scene lifecycle/negative checks. | open |
| **UIX-AC07** | GTK, ANSI and browser tracks have documented build cohorts, platform/browser coverage, accessibility limits, package contents and reproducible consumer routes for their selected support levels. Complete agreed integration/milestone QA separately from each development increment. | open; milestone scope unselected |
| **UIX-AC08** | At each forms, graph and map stage, one handwritten Level G feature drives GTK, ANSI and browser. Common actions have matching expected domain transitions and close policy; host-specific capability and effect outcomes are explicit. Record each host's native, semantic fallback or unsupported operations; verify supported paths and one application-visible refusal per stage without claiming visual equivalence. | open; parallel host gate |

## Stable implementation steps and gates

| ID | Work, dependency and criteria served | Status |
| --- | --- | --- |
| **UIX-STEP01** | Trace the direct Text Inspector feature through session, view, commands and effect outcomes on GTK and ANSI, retaining the line TUI regression. Use handwritten graph/map sketches to identify missing dynamic layout, scene, input and fallback contracts; AC01/04–06/08. | open |
| **UIX-STEP02** | Agree the core view, scene, normalized input and per-operation support/refusal contracts using handwritten consumers: dynamic values, IDs, geometry, transforms, hit testing, accessibility, coordinate precision and capacity. Apply D05–06, preserve v0 consumers and test a rejected whole snapshot and alternative view; AC01/04–06/08. | open; public core contract gate |
| **UIX-STEP03** | Implement the shared scene owner and bounded graph/map model-first fixtures. Build the initial GTK 2D and ANSI semantic graph projections against the same feature, including keyboard paths and explicit refusals; AC04/06/08. Keep editing, undo and game rules in the feature. Depends on STEP02; may proceed alongside STEP04. | open |
| **UIX-STEP04** | Decide D07 and select browser packaging, runtime cohort, owner-turn API and bounded bridge or transport protocol. For local Wasm, choose a matching UI VM image and owner-turn/step or qualified suspend/resume route for `uidriver.run`; for hosted use, define authentication and reconnect/revision ownership. Implement a handwritten Text Inspector browser consumer with the same forms-stage transitions and focused lifecycle/async/refusal tests; AC03/06/08. Keep a separate asynchronous application-service route for later RAG/database/inference effects. Depends on STEP02; may proceed alongside STEP03. | open; browser host/API gate |
| **UIX-STEP05** | Complete the graph stage across GTK 2D, ANSI semantic/cell and browser SVG projections, then the map stage across GTK 2D, ANSI bounded viewport and browser Canvas 2D. At each stage compare the same domain journal, keyboard/input paths and fallback/refusal behavior before moving on. Evaluate culling and measured thresholds before WebGL2; AC04–06/08. Depends on STEP03–04. | open |
| **UIX-STEP06** | After direct consumers prove the core contract, reconcile the experimental `DEFINE-GUI` macro and its missing graph inputs. Decide its grammar/dynamic-value mapping, generate only layout, typed bindings, command/schema wiring and static routing, and run focused source-map/negative checks; AC01–02. Depends on STEP02–05; Peter may own this increment. | open; later RXPP design gate |
| **UIX-STEP07** | Qualify selected application/host and generated-authoring combinations across GTK, ANSI and browser, including fallback/refusal, failure paths, package closure and supported browsers/platforms at an agreed milestone. Reconcile Release 1 selection and January freeze separately; AC01–08. Depends on selected preceding steps. | open; milestone/scope gate |

If a browser RAG or local inference outcome is selected, add its own numbered
delivery criteria in the owning RAG/LLM plan and reference them here. The UI
milestone does not become an implicit database, embedding or GPU-inference
qualification gate.

For each development step, use the smallest changed-component build and
behavior/contract checks, inspect representative RXAS for changed cREXX paths
and RXPP output when STEP06 begins, then freeze and report local evidence.
Full browser/platform, sanitizer and delivery gates occur only when selected
under the repository lifecycle. No publication or platform scope expansion is
implied here.

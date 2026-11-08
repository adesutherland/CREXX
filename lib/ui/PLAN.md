# UI architecture and capability expansion

Status: design and implementation plan; both browser execution routes are
selected, but no browser or 2D driver has been delivered. RAG query integration
targets hosted and browser-local read-only routes, with hosted-only fallback
conditional on implementation evidence and an agreed reasonable-effort judgment.
Small-LLM answer generation likewise targets browser-local and native/hosted
providers behind one application service, with a conditional native/hosted-only
fallback after an effort discussion informed by implementation. Full local RAG,
other application databases and each GPU acceleration workload remain separate
option gates. A persistent browser cache for the read-only RAG bundle is a
second-stage conditional target after transient import works. Browser-local
corpus writes receive a later feasibility assessment; native/hosted RAG keeps
ingestion ownership until a separate implementation decision.
This is the authoritative UI expansion plan. The
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
feature owner. The browser track runs the same feature in both a matching cREXX
WebAssembly Worker and a hosted native session, using one browser view/input
contract. Browser API access still needs a small host bridge.
The UI library must support multiple independently authored applications over
time: common session/effect lifecycle and host mechanisms are shared, while
each application owns its model, service contracts, repository and evolution.
Each team packages its application with a versioned declaration of its entry
composition, UI contract compatibility, required/optional operations and
dependencies. Native and browser routes may assemble different artifacts from
that declaration. Loading a new application into a running host remains a
separate proof, not a package requirement.

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
| Forms and dynamic widgets | Native controls and current session routing | Cell controls, keyboard and supported pointer input | Matching cREXX feature through both local Wasm and hosted sessions, with the same semantic HTML controls | Same command/effect/close transitions; host-specific views and failure paths. |
| Editable graph | Native 2D projection with keyboard and pointer editing | Semantic node/edge list or bounded cell viewport; edit/connect by stable ID where usable | SVG projection with keyboard and pointer editing | Same graph model, selection and undo history; unsupported gestures refuse explicitly. |
| Strategy map | Native 2D terrain/unit projection with pan/zoom and selection | Bounded cell viewport and unit/terrain detail/actions where usable | Canvas 2D projection with pan/zoom and selection | Same rules, turns and stale-input handling; unavailable precision or interaction is declared. |
| Application services | Correlated native or hosted effects | Correlated native or hosted effects | Correlated effects through both browser session routes; hosted and local read-only RAG query routes targeted, with conditional hosted-only fallback | Same result and cancellation semantics, with provider and storage limits stated per host. |

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
| Local browser Wasm | One Worker owns cREXX VM, session and feature; a small main-thread browser adapter projects semantic views and returns normalized input. | Application behavior stays in cREXX; local use need not run a server session. | Wasm cannot directly use DOM; JS/TS glue, async effects, asset packaging and Worker lifecycle remain. | Selected as one of two initial browser routes; matching UI image and owner-turn proof required. |
| Hosted browser | Native cREXX session on a server; the same browser adapter renders versioned views over an authenticated transport. | Server resources, collaboration and larger workloads are easier to own centrally. | Authentication, authorization, reconnect/revision rules and latency are part of the product. | Selected as the second initial browser route; shared protocol and hosted lifecycle proof required. |
| Wasm graphics renderer | Wasm computes layout/geometry and possibly draws through Canvas/WebGL bindings. | Keeps expensive scene calculations in cREXX/native code. | DOM controls, accessibility, input and browser APIs still need host glue. | Optional optimization after the semantic browser driver works. |

The two browser routes share one versioned view/input/effect envelope and one
main-thread projection. Local Worker messaging and hosted transport are
adapters to that envelope; they must not create separate browser feature or
rendering semantics. Each forms, graph and map stage qualifies both routes,
including local Worker restart and hosted reconnect/revision behavior. This
selection of browser UI routes is independent of the separate RAG query decision.

| Surface | Selected role or later gate | Upgrade condition |
| --- | --- | --- |
| GTK 3 drawing area/Cairo | Selected native 2D projection for graph and map; the Level G owner retains scene geometry and picking. | Measure redraw, input and memory on representative scenes before changing the native renderer. |
| SVG plus ordinary HTML controls | Selected browser graph projection with stable element IDs, labels and focus/selection overlays. | Move a heavy layer to Canvas when measured DOM cost or scene size warrants it. |
| Canvas 2D | Selected browser map projection for bounded terrain/unit scenes. | Add culling/batching first; use WebGL2 if measured redraw cost remains excessive. |
| ANSI semantic/cell | Selected graph list and bounded map viewport; unsupported gestures get application-visible refusal. | Improve supported keyboard/cell operations where useful; no pixel-equivalent promise. |
| WebGL2 | Large maps, dense layers or animation requiring GPU batching. | Gate on a representative map and fallback/restore behavior. |
| WebGPU | Optional renderer and separate compute facility for embeddings or small-model inference. | Select each workload independently after browser coverage, device limits and failure policy are qualified; compute may be useful before a WebGPU renderer. |

The application owns graph nodes/edges, map cells/units, selection, edit
transactions, undo/redo and game rules. A proposed reusable scene contract
would own stable element IDs, bounded geometry, z-order, world coordinates,
viewport transform, style and semantic labels. The host owns painting, physical
input, device-pixel conversion and browser/native resources. One reusable
Level G scene module owns world geometry, transforms and scene-object hit
testing across GTK and both browser routes. Hosts convert physical input into
declared logical viewport units; ANSI retains its own displayed cell/control
hit areas and can address semantic objects by stable ID. This does not move
graph editing or game rules out of the application. World geometry uses bounded
fixed-point integer coordinates. The public contract must state the ticks per
world unit, valid range, checked arithmetic and viewport rounding rules; JSON
numeric fields must stay within the browser's exact-integer range. Map tile
identities remain separate integers, and hosts convert to pixels only for
painting. Scene updates use a
versioned delta protocol. An initial full snapshot establishes the host replica;
each bounded delta names its scene/session epoch, base revision and resulting
revision and changes stable object IDs atomically. A host accepts a delta only
against the expected base, validates the whole change before projection and
rejects stale scene input. A revision gap, conflicting duplicate, invalid delta
or transport restart is a synchronization error: keep the last valid frame,
pause scene interaction, request a full authoritative snapshot, then replace
the replica atomically and resume. Identical duplicate delivery may be ignored
idempotently. Bound resync attempts and report a typed failure if recovery
cannot complete. The exact message shapes, limits and state handoff remain a
public contract gate; profile real graph/map traffic before choosing a binary
encoding or delta coalescing.

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

| Candidate | Boundary and first useful proof | Main unresolved work | Status |
| --- | --- | --- | --- |
| Hosted RAG query | An authenticated adapter calls the existing native `crexxrag` query path and returns bounded cited results to a browser feature through an application-owned effect. | Define the service adapter, authorization, corpus/session identity, cancellation, result size and failure contract. The present CLI/JSON/MCP adapters are not an HTTP service by themselves. | selected; required fallback route |
| Local read-only RAG | A matching Wasm Worker opens an imported, validated SQLite snapshot plus compatible immutable vector sidecars and performs lexical/vector/graph queries locally. Prove a bounded transient import first, then target a validated OPFS snapshot cache as a separate stage. | Cross-compile the needed Level G modules, `rxsqlite` and `rxvector`; prove bundle identity, sidecar verification, query-embedding provider, memory limits and reload, then qualify the selected persistence stage. A native provider's existence is not proof of a Wasm build. | selected query target; staged cache target conditional on feasibility |
| Full local RAG | Potential browser-local ingestion, embeddings, index publication, review and maintenance after the read-only and persistence stages provide evidence. Native/hosted RAG continues to own corpus writes meanwhile. | RAG currently uses independent VM processes, WAL connections, leases and transactional publication; a browser profile needs a separate ownership and recovery design. | later feasibility stage selected; implementation unselected |
| Application repositories | Graph, map and future applications register their own namespaced service effects and repository executors through composition, using the shared session lifecycle. | Select schema, transactions, migration, import/export, storage route and sync policy per application; do not couple those domains to RAG's SQLite schema or put SQL in the UI driver. | shared extension direction selected; application policies remain their owners' decisions |

The two selected query routes share one application-owned `rag.query` effect and
bounded cited-result contract, with route and evidence-snapshot identity. Local
read-only retrieval remains in scope while its feasibility is investigated.
If implementation experience reveals a concrete blocker, discuss the remaining
work and reasonable effort with Adrian before using the authorized hosted-only
fallback. Do not set an arbitrary effort limit now or silently narrow the local
route. The first local read-only proof uses a transient validated import.
Persistent OPFS caching follows as a separate conditional stage under D13; it
does not gate that first query proof. Both a compatible browser-local
query embedding model and a hosted query embedding provider are targeted behind
the same request/result contract. The local model's CPU/Wasm or WebGPU backend
remains to be selected and proved. If the local embedding route encounters a
concrete blocker, discuss reasonable effort with Adrian after implementation
experience before using the authorized hosted-only embedding fallback. This
embedding decision does not select browser-local answer generation.

An illustrative `rag.query` effect would carry a bounded query, corpus/version
identity and optional retrieval mode. Its completion would carry ranked passage
IDs, excerpts, citations, scores and the identity of the evidence snapshot;
the application feature decides what to show. A browser host must retain
request/session correlation, cancellation and stale-result rejection. DB
handles, paths and provider secrets must not cross the view or Worker message
boundary. A query embedding must use a compatible model, dimensions and input
representation; an unrelated small LLM cannot silently replace it. Lexical
retrieval remains a possible capability when no matching embedding provider is
available. Persistence for editable graphs, maps and later applications uses
the same correlated effect lifecycle. The UI layer validates registered
message shapes, routes results and reports capability, cancellation and failure
outcomes. Each application owns its effect vocabulary, data format, transaction
and undo rules, compatibility/migration, storage choice and conflict policy.
Host renderers do not interpret domain records, and application-only services
need no global UI catalogue entry.

WebGPU is relevant beyond drawing. It exposes compute pipelines, so a browser
embedding model or small generative model could serve the selected local
provider targets behind application-owned embedding/generation contracts. This
requires a browser provider or a separately selected inference engine; the
native `llama` GPU backends do not automatically run through WebGPU. Keep heavy
compute in a compute Worker and return bounded results to the UI session owner;
never make `update()` wait for a model. Evaluate model format/identity, download and cache,
device memory, request limits, cancellation, device loss, privacy, numerical
compatibility and provider fallback. A local generation engine and its CPU/Wasm
or WebGPU backend are implementation choices, not established by this direction.
GPU vector scoring is another optional workload, selected only against the
existing deterministic CPU `rxvector` result/order contract and measured
corpus sizes. Canvas 2D is the initial bounded map renderer. A WebGPU map
renderer, vector scorer and local model provider may share device discovery
policy, but each needs its own capability, correctness and resource proof.
Browser-local embedding and generation remain targets independent of a GPU
requirement; choose each model backend during implementation.

| Browser inference route | What it would reuse | Proof before provider selection |
| --- | --- | --- |
| Native/hosted `.llm` and `.embedding` drivers | Current cREXX provider concepts and server/native execution; selected generation and embedding service route. | Async browser effect adapter, credentials/privacy route, bounded streaming and cancellation. |
| Browser WebGPU engine behind a new provider | cREXX application request/result policy; candidate for the selected local generation target, with an engine such as WebLLM for comparison. | Model and output contract, JS/Wasm bridge ownership, Worker lifecycle, device loss, local asset/cache policy and supported browsers; embedding compatibility separately for D03. |
| Port of the existing native inference engine | Potentially its model formats and selected CPU code; another candidate for local generation. | A real Wasm/backend and package/ABI proof; Metal/CUDA/Vulkan paths alone do not provide it. |

[WebLLM's project](https://github.com/mlc-ai/web-llm) shows that browser-local
WebGPU generation is feasible, but is not a chosen cREXX provider or proof of
compatibility with the current RAG embedding corpus.

SQLite's browser OPFS VFS is a feasibility input, not proof that the bundled
`rxsqlite` provider can persist unchanged in the current Emscripten profile.
Browser storage is origin-scoped and subject to quota, eviction and user data
clearing. The selected read-only cache must preserve bundle validation and
provide export/re-import recovery; it is not a browser-local ingestion or
authoritative publication store. Assess browser-local writes only after the
read-only query and persistence results are available; a specific offline
product outcome and implementation scope remain to be agreed.

| Decision ID | Evidence required before selection | Status |
| --- | --- | --- |
| **UIX-D01** | Approved conditional target: hosted and browser-local read-only cited query routes share one `rag.query` effect; hosted-only is authorized if the local route proves infeasible within reasonable effort. Discuss that effort when implementation experience exposes concrete work, then record the evidence before changing scope. Full local ingestion remains unselected. | direction approved; local feasibility open |
| **UIX-D02** | Prove matching `rxsqlite`/`rxvector` Wasm builds, validated SQLite/sidecar identity and a representative cited lexical/vector/graph query using the D03 embedding contract. Discuss reasonable effort only after implementation reveals concrete blockers. The selected D13 persistence stage has its own VFS, multi-tab/storage, export and recovery proof. | open; implementation feasibility gate |
| **UIX-D03** | Approved conditional target: a compatible browser-local query embedding model and a hosted embedding provider share one query-embedding request/result contract. If local model support proves infeasible within reasonable effort discussed after concrete implementation experience, use hosted embedding only and record the evidence. Select and prove the local CPU/Wasm or WebGPU backend separately; no answer-generation route is selected by this decision. | direction approved; local backend/proof open |
| **UIX-D04** | Approved: assess acceleration independently for map rendering, vector scoring, query embedding and small-LLM generation. Begin with Canvas 2D for bounded maps and exact CPU `rxvector` scoring; select each local model backend during implementation. Add a GPU path only after representative correctness, latency and resource evidence against agreed targets, then qualify its device support, failure and fallback without making other workloads depend on it. A browser-local model target does not itself require WebGPU. | direction approved; per-workload evidence gates open |
| **UIX-D05** | Approved direction: precise host support at composition plus complete-view/scene preflight, application-owned fallback and command availability, and typed correlated refusal that preserves the last valid frame with bounded retry. Specify the exact API and prove refusal leaves no partial new projection or additional feature state mutation. | direction approved; API/proof open |
| **UIX-D06** | Approved direction: support is fixed for a session. Hosts may recover internally while preserving advertised semantics; otherwise report typed host loss and recompose a new session from application-owned saved model state. Specify failure, in-flight work, snapshot and restart APIs and prove state continuity before promising automatic recovery. | direction approved; API/proof open |
| **UIX-D07** | Approved: implement local cREXX Wasm Worker and hosted native browser sessions in parallel, sharing one browser projection and versioned view/input/effect envelope. Prove matching runtime packaging and owner-turn behavior locally, plus authentication, reconnect and revision ownership for hosted execution. | direction approved; both route proofs open |
| **UIX-D08** | Approved: one reusable Level G scene module owns world geometry, transforms and scene-object hit testing. Hosts map physical input to logical viewport units and paint; ANSI retains displayed cell/control hit areas and uses stable semantic object IDs. Preserve one coordinate/identity contract across GTK, ANSI and both browser routes. | direction approved; scene API/proof open |
| **UIX-D09** | Approved: an initial full snapshot establishes the replica; bounded versioned deltas apply atomically from an expected base to a resulting revision. On synchronization error or route restart, pause scene input and replace the replica from a full authoritative snapshot, with bounded retries and typed failure. Define exact message shapes, stable IDs, limits and verification for GTK, ANSI and both browser routes. | direction approved; API/proof open |
| **UIX-D10** | Approved: use bounded fixed-point integer world coordinates shared by GTK and both browser routes; keep map tile identities separate and convert to pixels for painting. Define ticks per world unit, exact wire range, viewport rounding, checked arithmetic and terminal mapping; prove identical picking and explicit overflow rejection before freezing the API. | direction approved; scale/API proof open |
| **UIX-D11** | Approved: GTK 3 drawing area/Cairo for graph and map, browser SVG/HTML for editable graph and Canvas 2D for bounded map through both browser routes, and ANSI semantic/cell alternatives. Add culling/batching first; select accelerated rendering only after representative scene size, interaction latency and memory evidence against agreed targets. GPU compute is a separate decision. | direction approved; host/proof open |
| **UIX-D12** | Approved conditional target: browser-local and native/hosted small-LLM answer generation share one application-service request/result contract. If local generation proves infeasible within reasonable effort, discuss the concrete implementation evidence and effort then use native/hosted generation only. Select the local engine/backend and define model identity, evidence/citation, privacy, device and failure policy during implementation; D03 query embeddings and D04 GPU workloads remain separate decisions. | direction approved; local provider/proof open |
| **UIX-D13** | Approved staged persistence target for browser-local read-only RAG: first prove bounded transient import of a validated SQLite/vector bundle, then add an OPFS snapshot cache with bundle identity, export/re-import recovery and multi-tab behavior. If persistent caching proves infeasible within reasonable effort, discuss concrete implementation evidence and effort before retaining transient import as the local route. This does not select browser-local ingestion or make persistence a prerequisite for the first query proof. | direction approved; cache VFS/proof open |
| **UIX-D14** | Approved: retain native/hosted corpus writes while the browser route proves read-only queries and evaluates persistence. After those stages provide evidence, assess browser-local ingestion, embedding, index publication, review and recovery as a separate capability. Record the feasible scope, product need, ownership and effort before a new implementation decision; this approval does not select full local writes. | staged assessment approved; implementation decision open |
| **UIX-D15** | Approved: use the common registered-effect lifecycle as an extension point for many independently authored applications. The platform supplies namespaced registration, schema validation, capability/refusal, correlation, cancellation and terminal outcomes; application teams own persistence/service vocabulary, data format, repository, migrations, transactions, conflicts and local/hosted policy. Do not impose a shared graph/map/RAG schema or require UI-core edits for an application-only service. Define exact compatibility/versioning rules with direct consumers. | direction approved; extension contract proof open |
| **UIX-D16** | Approved: independently authored applications use versioned packages declaring entry composition, UI contract compatibility, required/optional operation capabilities, dependencies and assets. Build/deployment may assemble native and browser variants differently while preserving application behavior and declared support. Check compatibility and required capability closure before session activation; the application handles optional fallbacks. Runtime hot loading across hosts is not selected and needs separate proof. | direction approved; manifest/package proof open |

## Acceptance criteria

| ID | Observable pass condition and verification | Status |
| --- | --- | --- |
| **UIX-AC01** | A handwritten Level G feature/session handles equivalent commands, events, effects, dynamic values and close outcomes on two existing hosts, with one authoritative model. Compare independent expected transition journals and views. | open |
| **UIX-AC02** | Once the core contracts are stable, RXPP generates declarations that reproduce the direct consumer's current dynamic values and routing without a second authoritative model. It rejects malformed/unknown controls, duplicate IDs, inconsistent command definitions and invalid event/binding references with source locations; multiple controls may share one command. Generated code has no named Text Inspector special cases. Inspect representative cREXX/RXAS and run positive/negative and regeneration checks. | open; later authoring increment |
| **UIX-AC03** | Both a matching local Wasm Worker and hosted native session run the same cREXX feature behind one browser projection and versioned view/input/effect envelope. Each serializes owner turns, projects current controls, returns normalized input and correlated async outcomes, and closes or fails without leaked authority. Check both routes against the same browser interactions and one bounded synthetic application-service result; check cancellation and stale/duplicate messages, Worker restart, and hosted authentication/reconnect/revision handling. No DB/model handle enters a view. | open; route implementation and packaging unproved |
| **UIX-AC04** | A versioned 2D scene contract edits a representative graph: create/move/connect/delete, zoom/pan, selection, keyboard operation and undo/redo. GTK 3 drawing area and browser SVG through both routes project initial snapshot and atomic deltas; ANSI offers a tested semantic node/edge alternative or refuses a specific unavailable operation. Compare shared model state and check fixed-point precision, overflow refusal, stable picking under resize/scale, resync and keyboard access. | open; scene API decision required |
| **UIX-AC05** | A representative strategy map renders terrain and units, supports pan/zoom, selection and turn actions, and keeps rules/state in the feature. GTK 3 drawing area and browser Canvas 2D through both routes project bounded deltas; ANSI supplies a bounded cell viewport and detail/actions where usable, with explicit refusal for unavailable interactions. Compare shared turn/state results; check bounded tiles, clipping/culling, pointer coordinate conversion, stale input and full resync on graphical hosts. Measure before selecting an accelerated renderer. | open |
| **UIX-AC06** | Unsupported view/scene operations are declared at application composition or returned as structured refusals before partial projection, and the feature selects a fallback or a specific rejection. Scene deltas validate and commit atomically; gaps, conflicting duplicates, invalid changes and route restart trigger full resync, while stale scene input is refused and retries are bounded. Absent effect capabilities retain correlated failures. Malformed assets/messages, context loss, resize, cancellation and setup/teardown failures have explicit behavior; browser resource grants cannot be forged from display text. Run focused host and scene lifecycle/negative checks. | open |
| **UIX-AC07** | GTK, ANSI and both browser routes have documented build cohorts, platform/browser coverage, accessibility limits, package contents and reproducible consumer routes for their selected support levels. Complete agreed integration/milestone QA separately from each development increment. | open; milestone scope unselected |
| **UIX-AC08** | At each forms, graph and map stage, one handwritten Level G feature drives GTK, ANSI and both browser routes. Common actions have matching expected domain transitions and close policy; host-specific capability and effect outcomes are explicit. Record each host's native, semantic fallback or unsupported operations; verify supported paths and a representative application-visible refusal on each route without claiming visual equivalence. | open; parallel host gate |
| **UIX-AC09** | A browser feature issues the same bounded `rag.query` effect through hosted and targeted local read-only executors and receives cited results with corpus/evidence identity, compatible query embedding, cancellation and stale-result handling. Compare representative lexical/vector/graph evidence with native RAG. If local feasibility is later declined under D01–02, document the concrete blocker and agreed effort judgment; hosted cited-query acceptance remains required. | open; conditional local route |
| **UIX-AC10** | The same query-embedding request/result contract supports hosted and targeted browser-local compatible models. For a representative corpus profile, verify model/input identity, dimensions, vector compatibility, cancellation, provider failure and privacy route; compare resulting cited retrieval to the native control. If local model support is later declined under D03, record the blocker and agreed effort judgment; hosted embedding remains required. | open; conditional local embedding route |
| **UIX-AC11** | The same application-service generation request/result contract supports native/hosted and targeted browser-local small-LLM providers, independently of query embedding and UI rendering. Verify bounded input/output or streaming, model identity, privacy route, cancellation, stale completion, provider/device failure and application-owned result policy. For cited RAG answers, preserve the owning RAG evidence and citation validation. If local generation is later declined under D12, record the concrete blocker and agreed effort judgment; native/hosted generation remains required. | open; conditional local generation route |
| **UIX-AC12** | After a validated transient bundle supports local read-only cited queries, the targeted OPFS cache reloads that same corpus/evidence version across browser restarts without weakening SQLite/sidecar validation. Verify quota or eviction failure, clearing/re-import, export, multi-tab access/locking and bounded cleanup. If persistence is later declined under D13, record the concrete blocker and agreed effort judgment; the transient local query route remains the fallback. | open; conditional persistent cache |
| **UIX-AC13** | After the read-only query and persistence stages yield implementation evidence, an owning RAG assessment maps source ingestion, provider/model identity, worker/lease behavior, review, transactional SQLite and `.rxvec` publication, multi-tab ownership and crash recovery to a browser profile. Identify the smallest representative proof and remaining risks, then record a separate implementation decision with scope and effort; native/hosted ingestion remains available throughout. | open; later feasibility decision gate |
| **UIX-AC14** | Two independently structured Level G example applications register distinct namespaced repository/service effects and compatible payload schemas through composition without editing the UI core or global catalogue. Each receives correlated success, cancellation, failure and unsupported-capability outcomes; stale/conflicting domain writes remain application-owned. Use graph save/load and map turn persistence as different verification operations, preserving their respective model/undo or rule state without prescribing product storage policies; hosts do not inspect domain records. | open; multi-application extension gate |
| **UIX-AC15** | Two independently authored example applications produce versioned package declarations and selected native/browser artifacts without UI-core source edits. For each selected host route, verify entry composition, UI contract compatibility, dependencies/assets, required operation preflight and application-owned optional fallback; reject an incompatible package or missing required asset/capability before starting its session. Document package contents and route-specific build/install steps. Runtime hot loading is not required. | open; independent application packaging gate |

## Stable implementation steps and gates

| ID | Work, dependency and criteria served | Status |
| --- | --- | --- |
| **UIX-STEP01** | Trace the direct Text Inspector feature through session, view, commands and effect outcomes on GTK and ANSI, retaining the line TUI regression. Use handwritten graph/map sketches to identify missing dynamic layout, scene, input and fallback contracts; AC01/04–06/08. | open |
| **UIX-STEP02** | Agree the core view, scene, normalized input and per-operation support/refusal contracts using handwritten consumers: dynamic values, IDs, geometry, transforms, hit testing, accessibility, fixed-point scale/rounding and capacity. Apply D05–06 and D08–10, preserve v0 consumers and test rejected view changes, delta synchronization errors, full resync and alternative views; AC01/04–06/08. | open; public core contract gate |
| **UIX-STEP03** | Apply D11 before host projection. Implement the shared Level G scene owner and bounded graph/map model-first fixtures. Build the initial GTK 3 drawing-area and ANSI semantic graph projections against the same feature, including snapshot/delta application, resync, keyboard paths and explicit refusals; AC04–06/08. Keep editing, undo and game rules in the feature. Depends on STEP02; may proceed alongside STEP04. | open |
| **UIX-STEP04** | Apply D07: build one browser projection and versioned envelope with local Worker and hosted transport adapters. Select matching runtime packaging and an owner-turn/step or qualified suspend/resume route for local `uidriver.run`; define hosted authentication and reconnect/revision ownership. Implement the handwritten Text Inspector feature through both browser routes with the same forms-stage transitions and focused lifecycle/async/refusal tests; AC03/06/08. Keep a separate asynchronous application-service route for later RAG/database/inference effects. Depends on STEP02; may proceed alongside STEP03. | open; two browser host/API gates |
| **UIX-STEP05** | Complete the graph stage across GTK 2D, ANSI semantic/cell and browser SVG projections through both browser routes, then the map stage across GTK 2D, ANSI bounded viewport and browser Canvas 2D through both routes. At each stage compare the same domain journal, keyboard/input paths, bounded delta application, full resync and fallback/refusal behavior before moving on. Apply D04: evaluate culling and representative latency/resource evidence before selecting any accelerated map path; AC04–06/08. Depends on STEP03–04. | open |
| **UIX-STEP06** | After direct consumers prove the core contract, reconcile the experimental `DEFINE-GUI` macro and its missing graph inputs. Decide its grammar/dynamic-value mapping, generate only layout, typed bindings, command/schema wiring and static routing, and run focused source-map/negative checks; AC01–02. Depends on STEP02–05; Peter may own this increment. | open; later RXPP design gate |
| **UIX-STEP07** | Qualify selected application/host and generated-authoring combinations across GTK, ANSI and both browser routes, including fallback/refusal, failure paths, package closure and supported browsers/platforms at an agreed milestone. Reconcile Release 1 selection and January freeze separately; AC01–08. Depends on selected preceding steps. | open; milestone/scope gate |
| **UIX-STEP08** | After the core application-service boundary exists, define one `rag.query` effect and implement the hosted adapter plus a browser-local read-only feasibility/consumer path against one validated corpus bundle. Keep RAG retrieval and evidence policy with its product owner; resolve D02 and the D03 local embedding backend during implementation. Use exact CPU `rxvector` scoring as the baseline and apply D04 separately before any GPU scorer. Qualify query and embedding routes against AC09–10 or record the D01/D03 conditional hosted-only fallbacks after their agreed effort discussions. This application integration does not block STEP01–07. | open; separate RAG integration gate |
| **UIX-STEP09** | After the core application-service boundary exists, define the D12 generation request/result contract and implement native/hosted and browser-local small-LLM consumer paths. Keep answer/evidence policy with its application or RAG owner; choose and prove the local engine/backend during implementation under D04 without requiring a GPU path. Qualify both routes against AC11 or record the conditional native/hosted-only fallback after discussing concrete local effort. This application integration does not block STEP01–07 and need not wait for full local RAG. | open; separate generation integration gate |
| **UIX-STEP10** | After STEP08 proves a transient validated local corpus, implement the D13 OPFS cache through a qualified SQLite VFS and bundle-sidecar storage boundary. Prove versioned reload, export/re-import recovery, quota/eviction, multi-tab locking and cleanup against AC12; if the cache is infeasible within reasonable effort, discuss concrete evidence and retain transient import. This storage stage does not block STEP01–09 or the initial local query proof. | open; separate persistence gate |
| **UIX-STEP11** | After STEP08 and STEP10 produce read-only and storage evidence, assess D14 with the RAG owner against AC13. Use a bounded critical-path proof where architecture evidence alone cannot settle a blocker, then decide separately whether to implement browser-local corpus writes and on what scope. Do not fold writable ingestion into STEP08/10 or the core UI milestone. | open; later ingestion feasibility gate |
| **UIX-STEP12** | Apply D15 at the application-service boundary: specify namespacing, schema compatibility and common outcome/capability behavior using the existing effect registry, then exercise independent graph and map repository consumers against AC14. Keep each repository's storage route, domain schema and conflict policy with its application owner; add only missing generic lifecycle mechanisms to UI core. Depends on STEP02–05 and may run alongside application-service integrations. | open; multi-application extension gate |
| **UIX-STEP13** | Define and implement the D16 application package declaration and host-specific assembly after the core composition and browser route contracts are stable. Package two independently authored example consumers, check exact contract/dependency/capability preflight and malformed/missing-asset rejection against AC15, then document native and browser installation paths. Depends on STEP04/05/12; this is a delivery gate, not a requirement for runtime hot loading. | open; package compatibility gate |

At RAG implementation start, add retrieval/provider delivery criteria to the
owning RAG plan and reference them from STEP08; AC09–10 own only the UI effect,
embedding-provider and cross-route consumer boundaries. At generation
implementation start, place provider and answer-policy criteria with the owning
application or RAG plan and reference them from STEP09; AC11 owns the shared
service and cross-route consumer boundary. At persistence implementation start,
place cache/VFS, export and recovery criteria with the owning RAG plan and
reference them from STEP10; AC12 owns the browser consumer boundary. The core
UI milestone does not become an implicit database, embedding, generation or
persistence qualification gate. STEP11 is a later RAG-owned assessment; any
selected write implementation needs its own owning-plan delivery criteria.

For each development step, use the smallest changed-component build and
behavior/contract checks, inspect representative RXAS for changed cREXX paths
and RXPP output when STEP06 begins, then freeze and report local evidence.
Full browser/platform, sanitizer and delivery gates occur only when selected
under the repository lifecycle. No publication or platform scope expansion is
implied here.

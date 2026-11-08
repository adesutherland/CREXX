# UI architecture and capability expansion

Status: design and implementation plan. Line, ANSI and GTK forms exist; no
browser or general 2D UI driver has been delivered. All acceptance criteria
remain open. This is the authoritative UI expansion plan. The
[portfolio roadmap](../../docs/ROADMAP.md) owns Release 1 ordering and the
January platform freeze; this plan does not change that baseline.

## Outcome and boundaries

Level G applications own state, transitions, commands, effects, graph edits,
game rules and persistence policy. The shared UI layer owns session routing,
view/scene contracts, normalized input, capability/refusal and host lifecycle.
It must support independently authored applications without embedding their
domain schemas or behavior in drivers.

Develop GTK desktop, ANSI full-screen terminal and browser projections in
parallel for forms, editable graphs and strategy maps. The browser has both a
local cREXX WebAssembly Worker and a hosted native session behind one
projection. Hosts may use semantic fallbacks or report a specific unavailable
operation; visual equivalence is not required. The line TUI remains a
compatibility host. Preserve UI contract v0 and the three existing reference
hosts while extending them.

The graph and map are representative consumers, not platform data models.
Versioned application packages support separately developed applications;
their initial deployment uses trusted packages. Peter's RXPP `DEFINE-GUI`
façade follows stable handwritten consumers and generates wiring, leaving
application behavior with its owner. RAG, embedding and generation use the
application-service boundary and have separate implementation gates after the
core UI contract. Public scene, input, ABI, packaging and target-platform
choices still need explicit contract and delivery gates.

## Delivery plan

### Execution steps

Step IDs are stable; the dependencies below set execution order. The core UI
path is **STEP01 → STEP02 → STEP03 and STEP04 in parallel → STEP05 → STEP06 →
STEP07**. Application-service work can proceed after the browser service
boundary exists; it does not redefine the core UI milestone. `ACnn` and `Dnn`
below abbreviate the stable `UIX-ACnn` and `UIX-Dnn` IDs.

| Step | Depends on | Deliverable | Criteria | Status |
| --- | --- | --- | --- | --- |
| **UIX-STEP01** Baseline | — | Trace handwritten Text Inspector through GTK/ANSI, retain the line regression, and sketch graph/map gaps with independent transition expectations. | AC01, AC04–06, AC08 | todo |
| **UIX-STEP02** Core contracts | STEP01 | Freeze direct-consumer view, scene, input and support contracts (D05–06, D08–10); preserve v0 and prove refusal, delta/resync and representative RXAS. | AC01, AC04–06, AC08 | todo |
| **UIX-STEP03** GTK/ANSI scene | STEP02; parallel with STEP04 | Build the Level G scene owner and bounded graph/map fixtures, then GTK/ANSI graph projections with keyboard, delta/resync and refusal checks. | AC04–06, AC08 | todo |
| **UIX-STEP04** Browser forms | STEP02; parallel with STEP03 | Build one browser projection and local/hosted adapters (D07); prove handwritten Text Inspector forms, owner turns, one async effect, restart and reconnect. | AC03, AC06, AC08 | todo |
| **UIX-STEP05** Graph then map | STEP03–04 | Complete graph then map on GTK, ANSI and both browser routes; compare domain journals and scene lifecycle at each stage, measuring before acceleration. | AC04–06, AC08 | todo |
| **UIX-STEP06** RXPP authoring | STEP02–05 | Reconcile `DEFINE-GUI` after direct consumers; generate wiring, repair missing graph inputs and prove dynamic values, source maps and invalid-declaration errors. | AC01–02 | todo |
| **UIX-STEP07** Core milestone QA | STEP03–06; agreed milestone | Qualify selected host/generated combinations, fallback/refusal, lifecycle, host packages and supported browsers/platforms; reconcile Release 1 separately. | AC01–08 | todo |
| **UIX-STEP08** RAG query/embedding | STEP04 service boundary; independent of STEP05–07 | Add `rag.query`, the hosted adapter and local transient read-only proof; compare cited evidence and compatible embeddings under D01–03. | AC09–10 | todo |
| **UIX-STEP09** Generation | STEP04 service boundary; independent of STEP05–08 | Add D12 native/hosted and targeted local generation consumers; choose local backend under D04 and prove bounded, cancelled, failed and cited results. | AC11 | todo |
| **UIX-STEP10** RAG cache | STEP08 transient local query proof | Prove D13 OPFS cache identity, reload, export/re-import, quota/eviction, multi-tab and cleanup, or record the conditional transient fallback. | AC12 | todo |
| **UIX-STEP11** Local-write assessment | STEP08 and STEP10 evidence | Assess D14 browser corpus writes with the RAG owner and make a separate implementation decision from concrete evidence. | AC13 | todo |
| **UIX-STEP12** Application effects | STEP02–05; may run beside STEP08–10 | Prove D15 independent graph/map application effects on the shared lifecycle; keep repository and conflict policy application-owned. | AC14 | todo |
| **UIX-STEP13** Application packages | STEP04–05 and STEP12 | Define D16 package declarations and host assembly; preflight two applications for contract, capabilities and assets, and document installation. | AC15 | todo |

At the start of STEP08–10, place retrieval, provider and cache delivery
criteria in the owning RAG/application plan; AC09–12 cover only UI service and
cross-route consumer boundaries. STEP11 is an assessment under AC13; any later
writable RAG implementation needs its own decision and owning-plan criteria.
STEP08–13 are separate gates and do not expand STEP07 or the Release 1 baseline.

For each development increment, use the smallest changed-component build and
behavior/contract checks, inspect representative RXAS for changed cREXX and
RXPP output at STEP06, then freeze and report evidence. Full platform,
sanitizer and delivery QA belong to named milestones under repository policy.

### Acceptance criteria

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

### Decisions and open gates

The direction in each row is approved except D02, which is an implementation
feasibility gate. D01, D03, D12 and D13 are **conditional local targets**. A
fallback requires concrete implementation evidence and a discussion with Adrian
of reasonable remaining effort at that time; no effort limit is set now and no
local target may be silently dropped. The owning acceptance criterion records
which route must still pass after any agreed fallback.

| ID | Settled direction | Remaining gate |
| --- | --- | --- |
| **UIX-D01** | Hosted and browser-local read-only cited RAG query share `rag.query`; hosted-only is the conditional fallback. Local ingestion stays separate. | D02 local proof. |
| **UIX-D02** | Evaluate matching `rxsqlite`/`rxvector` Wasm builds and a validated cited lexical/vector/graph query using D03. | Implementation feasibility; D13 owns persistence proof. |
| **UIX-D03** | Hosted and compatible browser-local query embeddings share one contract; hosted-only is the conditional fallback. | Model/input/dimension compatibility and local CPU/Wasm or WebGPU backend. |
| **UIX-D04** | Evaluate GPU acceleration independently per workload. Canvas 2D maps and exact CPU `rxvector` are baselines; browser-local models do not require WebGPU. | Representative correctness, latency, resource, device-loss and fallback proof for each selected accelerator. |
| **UIX-D05** | Discover precise per-operation support at composition, preflight complete views/scenes, and return typed correlated refusal with last frame intact. Applications own fallbacks and command availability. | Exact manifest/API, bounded retry and no partial state/projection proof. |
| **UIX-D06** | Capabilities stay fixed per session. Internal recovery preserves advertised semantics; otherwise typed host loss starts a new session from application-owned saved state. | Failure, in-flight work, snapshot and restart API/proof. |
| **UIX-D07** | Both local Wasm Worker and hosted native browser sessions use one browser projection and versioned view/input/effect envelope. | Local owner-turn/runtime image and hosted auth/reconnect/revision proof. |
| **UIX-D08** | One Level G scene owner handles world geometry, transforms and object picking; hosts map physical input, while ANSI owns cell/control hit areas. | Shared coordinate, identity and input API/proof. |
| **UIX-D09** | Full snapshot followed by bounded atomic revision deltas; synchronization error or route restart pauses scene input and replaces the replica from a full authoritative snapshot. | Message limits, stable IDs, bounded resync and typed failure proof. |
| **UIX-D10** | Bounded fixed-point integer world coordinates; map tile IDs are separate. | Tick scale, wire range, rounding, checked arithmetic and overflow/picking proof. |
| **UIX-D11** | GTK 3 drawing area/Cairo; browser SVG/HTML graph and Canvas 2D map; ANSI semantic/cell alternatives. | Host behavior, accessibility and measured acceleration gate. |
| **UIX-D12** | Native/hosted and browser-local small-LLM answer generation share an application service; native/hosted-only is the conditional fallback. | Local provider/backend, model, evidence/citation, privacy and device/failure policy. |
| **UIX-D13** | Prove transient local RAG import first, then target an OPFS read-only snapshot cache; transient import is the conditional persistence fallback. | Qualified VFS, bundle identity, export/re-import, quota/eviction, multi-tab and cleanup proof. |
| **UIX-D14** | Native/hosted RAG keeps corpus writes; assess browser-local writes after read-only and storage evidence. | Separate product need, scope, ownership and implementation decision. |
| **UIX-D15** | Registered effects form the extension point for independently authored applications; teams own repositories and policies without a shared graph/map/RAG schema. | Namespacing, schema compatibility/versioning and independent-consumer proof. |
| **UIX-D16** | Versioned application packages declare entry, UI contract, required/optional capabilities, dependencies and assets. Native/browser assembly may differ; initial packages are deployer-approved and trusted. | Manifest/package preflight and install proof; runtime hot loading and untrusted-package isolation remain outside this UI delivery. |

## Architecture details

### Existing UI and RXPP baseline

- [`ui_contract.crexx`](ui_contract.crexx) owns session routing, bounded queues,
  capabilities, correlated effects and close lifecycle. [`ui.crexx`](ui.crexx)
  provides a flat logical grid. Text Inspector runs on line, ANSI and GTK;
  browser and general 2D drivers do not yet exist. The
  [catalogue](contracts/catalogue.json) contains planned pointer, viewport and
  web vocabulary, not implemented scene behavior.
- Peter's experimental [DEFINE-GUI example](../../examples/ui/text-inspector/text_inspector_gui_macro.rxpp)
  uses RXPP `DALIAS ... HANDLER`, with no
  built-in GUI parser. It generates nodes, binding metadata, a controller,
  command registration and static callback dispatch. Its `node|value` display
  binding store can diverge from the feature's next `view()`. It also has Text
  Inspector callback and `ui.resource.selected` special cases and silently
  skips authored `MENU`/`ITEM` declarations. `ON` parsing and render support
  are not a validated grammar.
  The example graph refers to absent
  `text_inspector_gui_macro_test.crexx` and
  `text_inspector_gui_macro_gtk.rxpp` inputs. Repair that graph and validate
  source locations during STEP06. `rxBuild` describes its macro graph; CMake
  still prepares native GTK prerequisites. Keep the experimental macro
  available while handwritten core consumers are developed.
- CREXX's single-threaded Emscripten profile and THE's separate browser
  preview establish useful Worker/runtime precedent. THE owns its editor UI;
  neither it nor the current cREXX profile supplies this library's browser
  `uidriver`. Existing host capabilities are broad service families fixed at
  session construction. An absent effect yields correlated `ui.effect.failed`,
  while an unsupported widget currently fails rendering. STEP02 must make the
  latter application-visible before general scenes use it.

### Ownership, host coverage and failure

| Owner | Responsibility |
| --- | --- |
| Level G application | Model, transitions, commands, graph edits, game rules, effect decisions, semantic fallback, persistence and undo. |
| UI session and shared scene | Validate and route messages; serialize owner turns; track correlated work, semantic view/scene identity, geometry and object picking. |
| Host adapter | Project GTK, terminal or browser controls; map physical input to normalized facts; own focus, painting, device resources and teardown. |
| Application executor | Perform a declared external service and return bounded, correlated outcomes; keep domain handles and secrets outside views. |

The same handwritten feature drives the GTK, ANSI and two browser execution
routes at each selected stage. Their presentation may differ:

| Stage | GTK | ANSI | Browser projection |
| --- | --- | --- | --- |
| Forms | Native controls. | Cell controls and supported keyboard/pointer input. | Semantic HTML controls on both routes. |
| Editable graph | Drawing area/Cairo with keyboard and pointer editing. | Node/edge list or bounded cell viewport with stable-ID actions. | SVG plus HTML text/focus controls. |
| Strategy map | Drawing area/Cairo terrain and units. | Bounded cell viewport, detail and turn actions. | Canvas 2D terrain/units with pan, zoom and selection. |

D05–06 govern host support. Composition sees precise widget, scene, input and
limit support beyond broad `ui.widgets` or `ui.pointer` families; the feature
enables commands and selects alternatives. Before projecting a complete view
or scene, the host validates required operations.
Unexpected refusal leaves the last valid frame and returns a typed outcome to
the feature. Support stays fixed during a session. A host may recover internally
while preserving its advertised semantics; otherwise it reports host loss and
a new session starts from application-owned saved state. Exact APIs, bounds and
snapshot handoff remain STEP02 gates. ANSI fallbacks need usable keyboard
commands and labels; unavailable gestures receive a specific application
refusal. The line TUI retains its existing checks and gains coverage only when
an application needs a line-mode operation. The catalogue's
`ui.capabilities.changed` remains future vocabulary; the first extension keeps
capabilities fixed for each session.

### Browser routes and 2D scene

D07 uses one versioned browser view/input/effect envelope and one main-thread
projection. The local route keeps the cREXX VM, session and feature in a Wasm
Worker; a small browser bridge handles DOM and asynchronous Web APIs. The hosted
route keeps the native session on a server and uses an authenticated transport.
Both routes serialize owner turns and share browser semantics. Worker restart
and hosted reconnect have distinct transport recovery checks. Wasm geometry or
painting through Canvas/WebGL remains an optional optimization after the
semantic projection works. STEP04 must select an owner-turn/step or qualified
suspend/resume path for local `uidriver.run`.

D08–11 define the scene boundary. A reusable Level G scene module owns world
geometry, z-order, styles, semantic labels, viewport transforms and object hit
testing; application features own nodes, edges, tiles, units, selection, undo
and game turns. Hosts convert
physical input to declared logical viewport units and paint. ANSI owns its
visible cell/control hit areas and addresses semantic objects by stable ID.
World positions use bounded fixed-point integers; the public contract must
settle tick scale, range, checked arithmetic and viewport rounding. Browser
numeric transport must keep accepted values exact. Map tile IDs remain separate
from world positions.

An initial full versioned snapshot establishes each host replica. Later bounded
deltas name the scene/session epoch, expected base and resulting revision and
change stable IDs atomically. The host validates a whole delta before applying
it and rejects stale scene input. On a gap, conflicting duplicate, invalid
change or route restart, it keeps the last valid frame, pauses scene input and
replaces the replica from a full
authoritative snapshot. Identical duplicates may be ignored; failed bounded
resync returns a typed error. STEP02 fixes the wire shape, limits and state
handoff. Profile representative graph/map traffic before selecting binary
encoding or delta coalescing.

D04 keeps accelerators independent. Use culling and batching before considering
WebGL2 for larger maps, Canvas layers for dense graphs, or WebGPU rendering.
Measure representative redraw, interaction and memory on each selected host;
GPU compute for vectors or local models has its own gate and needs
representative corpus/model sizes.

### Application services, RAG and compute

The UI session carries application-owned service effects; it does not own a
corpus, database schema or model provider. Separately authored applications
register namespaced effect schemas through composition and provide their own
repositories/executors. The shared lifecycle supplies correlation, capability
refusal, cancellation and terminal outcomes. Each application owns data format,
transactions, migration, storage route and conflicts. D16's versioned package
declaration lets a host check contract, capabilities, dependencies and assets
before starting a trusted application session; host-specific artifacts and
runtime loading remain implementation/delivery questions.

[`crexxrag`](https://github.com/adesutherland/crexx-rag/blob/main/docs/architecture.md)
is a separate Level G application. SQLite owns its corpus, graph, embeddings
and work history; immutable generation-specific `.rxvec` sidecars accelerate
retrieval. Its owner validates sidecar membership against the SQLite snapshot,
collapses embedding windows to cited parent passages and preserves model/input
identity. CREXX's `rxsqlite` and `rxvector` provide generic mechanisms, while
native `llama` supplies a separate inference provider. Their native existence
does not prove a browser build or make them UI drivers.

D01–03 route a bounded `rag.query` effect to an authenticated native adapter or
a targeted local read-only Worker. A request carries corpus/version identity and
retrieval mode; a result carries ranked passage IDs, excerpts, citations,
scores and evidence-snapshot identity. The application chooses presentation.
Correlation, cancellation and stale-result rejection stay at the service
boundary; database handles, paths and secrets do not cross a view or Worker
message. The hosted adapter still needs an actual service surface: current
CLI/JSON/MCP entry points do not constitute one. The local route must prove
matching `rxsqlite`/`rxvector` Wasm builds, validated SQLite/sidecar import,
query-embedding compatibility, memory limits and reload. Exact CPU `rxvector`
result ordering is its scoring baseline. Lexical retrieval may be offered
separately if a compatible embedding provider is unavailable.

D13 adds an OPFS read-only cache only after transient import works. SQLite's
[browser VFS guidance](https://www.sqlite.org/wasm/doc/trunk/persistence.md)
is a feasibility input, not proof for this `rxsqlite` build. Browser storage is
origin-scoped and subject to
[quota, eviction and clearing](https://developer.mozilla.org/en-US/docs/Web/API/Storage_API/Storage_quotas_and_eviction_criteria);
the cache needs export/re-import recovery and keeps the validated bundle as its
evidence source. Native/hosted RAG continues to own ingestion. Its independent
VM workers, WAL connections, leases, review and transactional index publication
make browser-local writes a later D14 assessment, not an implicit cache feature.

Query embedding and small-LLM answer generation have separate application
contracts (D03 and D12). A query embedding must match the corpus model,
dimensions and input representation; a generative model cannot silently replace
it. Hosted/native providers need asynchronous browser adapters with bounded
results, cancellation and privacy routing for the existing `.embedding` and
`.llm` concepts. Local inference stays off the UI owner turn in a compute
Worker, never waiting inside `update()`, and needs model/asset identity, memory
admission, device loss and provider fallback. A browser WebGPU provider such as
[WebLLM](https://github.com/mlc-ai/web-llm) is a candidate for generation, not
a selected cREXX provider or proof of RAG embedding compatibility. A port of
the native inference engine needs a real Wasm backend; its Metal/CUDA/Vulkan
backends do not become WebGPU automatically. D04 selects map rendering, vector
scoring, embedding and generation acceleration separately after evidence.

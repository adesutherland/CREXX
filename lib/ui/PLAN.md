# UI architecture and capability expansion

Status: design and implementation plan; no browser or 2D driver has been
selected or delivered. This is the authoritative UI expansion plan. The
[portfolio roadmap](../../docs/ROADMAP.md) owns Release 1 ordering and the
January platform freeze; this plan does not add these capabilities to an
unapproved Release 1 baseline.

## Vision and boundaries

Keep application state, transitions, commands, and effect decisions in Level G
components. Let the same application present forms and interactive 2D scenes
through supported desktop and browser hosts. An RXPP declaration may shorten
view, layout, and command wiring, but generated code must use the ordinary UI
contract and leave application behavior with its feature owner. A browser
version should execute that owner in a matching cREXX WebAssembly runtime where
feasible. Browser API access still needs a small host bridge.

The intended examples are an editable UML-like graph and a pan/zoom strategy
map with selectable units and terrain. They are acceptance workloads, not a
request to put UML or game rules into the UI library. Terminal hosts may provide
a documented list or viewport fallback; pixel-equivalent graphics across every
host are not required. Existing UI contract v0 and the three reference hosts
must keep working. Public scene, input, ABI, packaging, and target-platform
choices remain explicit design gates before product implementation.

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
| WebGPU | Future optional renderer. | Select only after browser coverage and device-failure policy are qualified. |

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

## Acceptance criteria

| ID | Observable pass condition and verification | Status |
| --- | --- | --- |
| **UIX-AC01** | One ordinary Level G feature/session handles equivalent commands, events, effects and close outcomes on two existing hosts; generated declarations reproduce its current dynamic values without a second authoritative model. Compare independent expected transition journals and views. | open |
| **UIX-AC02** | RXPP rejects malformed/unknown controls, duplicate IDs, inconsistent command definitions and invalid event/binding references with source locations; multiple controls may intentionally share one command. Generated controller contains no named Text Inspector special cases. Inspect representative generated cREXX/RXAS and run positive/negative and regeneration checks. | open |
| **UIX-AC03** | A browser host runs a matching cREXX UI image in one Worker, serializes owner turns, projects current controls, returns normalized input and correlated async outcomes, and closes or fails without leaked authority. Check browser interaction, cancellation, stale/duplicate messages and Worker restart. | open; target and packaging choice unselected |
| **UIX-AC04** | A versioned 2D scene contract renders and edits a representative graph: create/move/connect/delete, zoom/pan, selection, keyboard operation, undo/redo and readable semantic alternatives. Check model state, chosen coordinate precision and stable picking under resize and scale. | open; scene API decision required |
| **UIX-AC05** | A representative strategy map renders terrain and units, supports pan/zoom, selection and turn actions, and keeps rules/state in the feature. Check bounded tiles, clipping/culling, pointer coordinate conversion and stale input; measure before selecting an accelerated renderer. | open |
| **UIX-AC06** | Unsupported capabilities, malformed assets/messages, context loss, resize, cancellation and setup/teardown failures have explicit behavior; browser resource grants cannot be forged from display text. Run focused host and scene lifecycle/negative checks. | open |
| **UIX-AC07** | Selected host/renderer has documented build cohort, browser/platform coverage, accessibility limits, package contents and a reproducible consumer route. Complete the agreed integration/milestone QA separately from each development increment. | open; milestone scope unselected |

## Stable implementation steps and gates

| ID | Work, dependency and criteria served | Status |
| --- | --- | --- |
| **UIX-STEP01** | Reconcile the current macro example with its actual graph inputs; trace one generated command, one dynamic label and one effect outcome through the single session. Record the owning application/model boundary; AC01–02. | open |
| **UIX-STEP02** | Decide the declarative grammar and dynamic-value contract. Generate only layout, typed view bindings, command/schema wiring and static routing; keep edit buffers transient and feature values authoritative. Repair missing graph sources and add focused negative/source-map checks; AC01–02. | open; syntax/ownership design gate |
| **UIX-STEP03** | Select browser packaging, matching UI VM image, an owner-turn API or qualified suspend/resume path for the synchronous `uidriver.run` boundary, and a bounded Worker/main-thread protocol. Implement the smallest Text Inspector browser host and focused lifecycle/async tests; AC03/06. | open; browser host/API gate |
| **UIX-STEP04** | Agree a versioned scene and input contract using the graph and map fixtures, including IDs, geometry, transforms, hit testing, accessibility and capacity. Implement shared scene owner and one simple renderer, with model-first tests; AC04–06. | open; public scene contract gate |
| **UIX-STEP05** | Add SVG graph projection and a Canvas 2D map projection. Reuse the same semantic input and scene contracts on one desktop host where feasible; evaluate culling and measured renderer thresholds before WebGL2; AC04–06. | open |
| **UIX-STEP06** | Qualify the selected application/host combinations, resource and failure paths, package closure and supported browsers/platforms at an agreed milestone. Reconcile Release 1 selection and January freeze separately; AC01–07. | open; milestone/scope gate |

For each development step, use the smallest changed-component build and
behavior/contract checks, inspect a representative generated source/RXAS early,
then freeze and report local evidence. Full browser/platform, sanitizer and
delivery gates occur only when selected under the repository lifecycle. No
publication or platform scope expansion is implied here.

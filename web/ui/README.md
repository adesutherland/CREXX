# Browser UI routes

One Level G ui_web.crexx owner loop runs Text Inspector, the editable graph,
Tile Map Demo, the process designer or a separately packaged graph/map/RAG
application. The local route runs its RXBIN in a
single-threaded Wasm Worker. The hosted route runs the same RXBIN in a native
VM behind a loopback authenticated relay. Both use projection.mjs: HTML form
controls, SVG for the graph and Canvas 2D for the map. Browser code owns file
grants and physical input; cREXX owns application transitions. The scene frame
carries its delta and a full authoritative snapshot for reconnect and resync.

## Build and serve

~~~sh
cmake --build cmake-build-ui --target rxvm ui_web text_inspector_web_artifact web_service_probe_artifact example_ui_scene_editor_artifacts example_ui_process_artifacts example_ui_application_packages example_ui_rag_query_browser --parallel 6
source /path/to/emsdk/emsdk_env.sh
emcmake cmake -S ports/ui-web-wasm -B cmake-build-ui-wasm -G Ninja \
  -DCMAKE_BUILD_TYPE=MinSizeRel -DCREXX_GENERATED_DIR="$PWD/cmake-build-ui/generated"
cmake --build cmake-build-ui-wasm --target crexx_ui_vm --parallel 6
node web/ui/package-site.mjs cmake-build-ui cmake-build-ui-wasm cmake-build-ui-web-site \
  examples/ui/app-effects/graph.package.json examples/ui/app-effects/map.package.json \
  examples/ui/rag-query/rag-query.package.json
CREXX_UI_TOKEN=your-private-token-at-least-16-characters \
  CREXX_UI_BUILD="$PWD/cmake-build-ui" \
  node cmake-build-ui-web-site/hosted-server.mjs
~~~

Open / for local Wasm; append ?hosted=1 for the native relay and
?app=graph, ?app=map or ?app=process for the scene consumers. The relay binds
loopback HTTP and requires a bearer token on every API request. Deployment needs TLS
termination and user authorization outside this development relay. The
package manifest hashes every copied asset; the hosted process still uses the
native RXBIN build path.

The independent application routes are `?app=graph_app` and `?app=map_app`,
with `&hosted=1` for hosted native execution. The graph package saves a
revisioned snapshot including undo/redo history; the map package appends turn
records. Their browser executors use origin-scoped `localStorage` as an example
repository. GTK and ANSI expose the same editing actions and report the optional
repository capability as unavailable. A browser with blocked origin storage
reports the same application-visible failure and retains the editing controls.
The models and schemas remain in the application modules; the browser routes
deliver effects to the declared executor or hosted service.

The `?app=rag_demo&hosted=1` route is a STEP08 three-route comparison
consumer. Its package declares an application-owned `rag.query` service; the
generic relay handles correlation and cancellation while the service runs
`crexxrag query compare` with read access. A compatible local embedding model
and vector sidecar are required. For a pre-ingested RAG library, set:

~~~sh
CREXX_UI_RAG_CORPUS=heart-failure \
CREXX_UI_RAG_BINARY=/absolute/path/to/crexxrag \
CREXX_UI_RAG_LIBRARY=/absolute/path/to/serving-snapshot \
CREXX_UI_RAG_CONFIG=/absolute/path/to/compatible/crexxrag.conf \
CREXX_UI_TOKEN=your-private-token-at-least-16-characters \
CREXX_UI_BUILD="$PWD/cmake-build-ui" \
  node cmake-build-ui-web-site/hosted-server.mjs
~~~

The `?app=rag_demo` route runs the query locally: a browser compute Worker
creates a BGE vector with CPU Wasm, and a separate cREXX Wasm VM Worker runs
RAG's read-only comparison RXBIN against an immutable SQLite/vector snapshot.
No query server or installed client is needed. The generic `package-site.mjs`
output contains the UI route only; assemble a corpus-bearing bundle separately.
First run RAG's `library serving-export --output PATH` with admin access and
include its compatible BGE GGUF model. With the UI site and opt-in RAG Wasm VM
built, use a **new, empty output directory**:

~~~sh
(cd examples/ui/rag-query && npm ci)
emcmake cmake -S examples/ui/rag-query/wasm -B cmake-build-rag-query-wasm -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug -DCREXX_GENERATED_DIR="$PWD/cmake-build-ui/generated"
cmake --build cmake-build-rag-query-wasm --target crexx_rag_query_vm --parallel 4
node examples/ui/rag-query/package-static.mjs \
  cmake-build-ui-web-site cmake-build-rag-query-wasm \
  /absolute/path/to/crexxrag-wasm-reader.rxbin \
  /absolute/path/to/serving-snapshot \
  /absolute/path/to/compatible/profile.tsv \
  /absolute/path/to/compatible/crexxrag.conf \
  cmake-build-rag-query-static
python3 -m http.server 8788 --bind 127.0.0.1 --directory cmake-build-rag-query-static
~~~

Open `http://127.0.0.1:8788/?app=rag_demo`. The browser returns independently
ranked keyword, embedding and graph excerpts with library/generation/model
identity, source URL, citation ID and captured UTF-8 span. The graph control
accepts 0–4 generations. The package checks snapshot, model and asset hashes
before retrieval; it makes no provider call or query-gap write. The query VM
opens SQLite read-only and primes FTS5 on a separate read-only connection to
avoid a cold `fts5vocab` re-entry observed in the selected Wasm build. The
default question demonstrates route differences, not answer quality or
clinical use. This is a local development proof. Public static release still
requires source-by-source distribution-rights review and HTTPS/browser/resource
qualification under STEP08.

The older same-origin Node query API remains a loopback integration fixture;
it is not used by this local route.
A user of an internal deployment needs only a browser. The question input,
graph generation field, Search button, cited excerpts and source positions are ordinary HTML
controls and text, so an agent with browser control can use the same page.
This does not automatically grant Codex, Claude or Gemini browser access: each
agent host must have its own browser tool and permission to reach the URL.
For direct machine access, cREXX-RAG also exposes the `query.compare` CLI/MCP
operation. Its separate Level G HTTP reader now serves this package and the
same query contract through native `rxvm`; the Google-hosted pilot uses IAP at
the service origin. The Node adapter remains a loopback proof. The
browser-local reader has a selected Chrome/macOS proof. Source-rights release,
Stephen's sign-in, cross-browser/resource checks and complete STEP08
qualification remain open.

## Application packages

Each trusted version-1 declaration in `examples/ui/*/*.package.json`
names an application ID, UI contract version, dependency RXBINs and its supported
routes. Every route declares
required operations/capabilities and optional operations. Browser routes also
declare their application executor, which supplies its optional repository
capability. Executor paths resolve beside their declaration; RXBIN paths
resolve under the named build or install root. A package may include one
validated browser stylesheet for presentation without adding application node
rules to the shared projection. Preflight rejects an incompatible contract, missing required
operation/capability or missing RXBIN/executor asset before session creation.
Optional scene rendering falls back to semantic ANSI controls; the absent
native repository returns a correlated application-visible refusal. The site
manifest lists each declaration and hashes its selected browser assets.
Hosted packages can declare a server-only application service module; its
source is packaged but cannot be fetched over the browser HTTP route. The relay
loads it after package preflight. Application-specific module names come from
the declarations; the host code contains no graph/map/RAG domain schemas or
storage rules.

The build command above creates the six graph/map route entry RXBINs and two
RAG browser entries. Check or run a
native package from the build tree with:

~~~sh
node web/ui/run-package.mjs examples/ui/app-effects/graph.package.json cmake-build-ui gtk --check
node web/ui/run-package.mjs examples/ui/app-effects/graph.package.json cmake-build-ui ansi
~~~

`cmake --install cmake-build-ui --prefix PREFIX` installs the application
RXBINs, declarations, repository executors and native runner under
`PREFIX/examples/ui/app-effects/`, plus the VM and UI dependencies under
`PREFIX/bin/`. After installation, run
`node PREFIX/examples/ui/app-effects/run-package.mjs
PREFIX/examples/ui/app-effects/map.package.json PREFIX gtk` (or `ansi`).
The GTK route requires the selected GTK 3 host build; ANSI needs a terminal.
For a browser site, build the Wasm VM and run the package-site command above,
then serve the resulting directory with the hosted relay. A second team can
pass its own version-1 declaration as another `package-site.mjs` argument;
its RXBINs and executor must already be built at the declared paths.

The selected core development support profile is macOS ARM64 with Chrome 156,
GTK 3.24 and an ANSI `xterm-256color` terminal. The same site package runs
local Wasm and hosted native sessions in Chrome; the hosted build can be a
staged `cmake --install` tree. Linux, Windows, Firefox and Safari are separate
platform/browser qualification gates. This development relay is loopback only;
it is not a public deployment profile.

The site package contains its manifest, HTML/JavaScript projection and route
bridges, the Wasm VM, core UI RXBIN modules and the Text Inspector, graph, map,
process-designer and service-probe RXBIN consumers. `test-package.mjs` verifies
the required asset closure and SHA-256 hashes. The native install tree supplies the VM, GTK
provider, UI modules and GTK/ANSI/Web example RXBINs. The service probe
exercises a registered application effect and a missing-capability result
through both browser routes. Hosted sessions are released on close/stop and
expire after 30 seconds without polling.

The browser profile supports labels, lines, buttons, single-line text inputs
and one scene per session. An input commits on change/blur with its originating
view revision; the owner validates its control ID and bound before posting the
standard value event. Text Inspector admits text files up to 128 KiB. A Worker
restart creates a new session epoch; a hosted reconnect replays the last full view. Unsupported
controls return a typed refusal. Scene input carries its epoch and revision;
stale input cannot mutate a newer scene. The local Worker has no dynamic
plugins or ambient filesystem access.

Forms use native GTK widgets, ANSI cell controls or semantic HTML buttons,
labels and text fields. Browser graph/map/process scenes are focusable with
keyboard actions and a scene-level accessible name; individual objects and map cells do not yet have
screen-reader navigation or editable accessibility semantics. GTK scenes have
the same item-level limitation. ANSI graph lists and the bounded map cell
viewport are semantic alternatives, with unavailable pointer operations
reported explicitly. These are the accessibility limits of this profile.

## Focused checks

~~~sh
node web/ui/test-protocol.mjs cmake-build-ui
node web/ui/test-scene.mjs
CREXX_PLAYWRIGHT=/path/to/playwright/index.mjs \
  CREXX_CHROME=/path/to/chrome \
  CREXX_UI_TOKEN=your-private-token-at-least-16-characters \
  node web/ui/test-process.mjs
node web/ui/test-hosted.mjs
node web/ui/test-package.mjs cmake-build-ui-web-site
node web/ui/test-application-packages.mjs
node examples/ui/rag-query/test-rag-query.mjs cmake-build-ui
node examples/ui/rag-query/test-hosted-service.mjs
node examples/ui/rag-query/test-query-http.mjs
node examples/ui/app-effects/test-repositories.mjs
python3 lib/plugins/console/tests/pty_checks.py --build cmake-build-ui --source . --mode application-packages
CREXX_PLAYWRIGHT=/path/to/playwright/index.mjs \
  CREXX_CHROME=/path/to/chrome \
  CREXX_UI_TOKEN=your-private-token-at-least-16-characters \
  node web/ui/test-browser.mjs
CREXX_PLAYWRIGHT=/path/to/playwright/index.mjs \
  CREXX_CHROME=/path/to/chrome \
  CREXX_UI_TOKEN=your-private-token-at-least-16-characters \
  node web/ui/test-application-browser.mjs
~~~

The browser check covers both routes' forms, graph and map interactions,
application-service results and capability refusal, file selection, close,
Worker restart, hosted reconnect and authentication. The selected support
profile and later platform gates are in the [UI plan](../../lib/ui/PLAN.md).

## Semantic browser forms and asynchronous input

The browser supports explicit `form.group`, `form.textarea`, `form.document`,
`form.link`, `form.properties` and `form.statusbar` operations in addition to the original forms.
These are direct-consumer additions. They do not imply native-host or RXPP
facade coverage. A group has a stable ID and accessible heading; a child names
an earlier group in its `parent` property. The projection validates every
parent, property, document and link before replacing the last valid view.

`uiview_impl.set_properties(id, json)` stores at most 4096 characters of semantic
metadata. The browser accepts only these fields:

| Property | Meaning |
| --- | --- |
| `parent` | Earlier group ID; omission preserves the original flat grid. |
| `label`, `description` | Accessible name and description. |
| `disabled` | Boolean input/button availability; features still validate intent. |
| `expanded`, `controls` | Boolean disclosure state and controlled group ID. |
| `role` | `region`, `complementary`, `log` or `status`. |
| `level` | Group heading level, 2–6. |
| `maxLength` | Text editor limit, 1–1024. |
| `focus` | Stable node ID or command action to focus when this directive changes; a separate command button is a fallback when no inline document action exists. |
| `cancel` | Effect name cancelled by this button, logically and physically. |

Any nonempty properties object requires `form.properties` at whole-view
preflight. A host that only supports the original kind cannot silently ignore
disabled, grouping or accessibility semantics.

A `document` node's value is JSON `{blocks:[{kind,runs:[{text,action?,href?}]}]}`.
It is bounded to 49152 characters, 64 blocks and 512 total runs; a block contains
at most 128 runs and a run's text at most 16384 characters. `paragraph`,
`heading` and `list-item` blocks are accepted. An action invokes an application
command; a link opens a validated HTTP(S) URL with `noopener noreferrer`.
Root-relative same-origin locators are supported; protocol-relative URLs,
credentials, control characters and non-HTTP schemes are refused. Text always
uses DOM text nodes. This is not Markdown/HTML interpretation or a RAG schema.

The projection preserves local drafts, focused node/selection and inner scroll
positions across owner views. Text changes and commands are queued in order.
An optional bounded `input_id` is echoed by an owner view with numeric
`accepted` 0/1. Only an explicitly unadmitted stale input may be resubmitted
against the new view; accepted commands are never automatically repeated.
Detached text-control changes retain their originating view revision.
Asynchronous completions use request/session/target ownership independently of
an unrelated intervening view revision. Stale/duplicate completions are still
rejected by the session.

Application extension schemas can opt into scalar text bounds up to 49152
characters for bounded structured completions. The ordinary scalar default and
individual `strings` array-item limit remain 16384; the complete payload/wire
limits remain 262144. A larger explicit schema is rejected at registration.

Browser executors receive an optional third `{signal}` argument. Stop/session
teardown aborts physical browser work; `cancel` input also cancels the session's
active operation. Hosted services receive `{signal, progress}`; `progress`
admits only the existing correlated `ui.effect.progress` outcome. The executor
must report completed work truthfully and preserve the terminal lifecycle.

A trusted executor or service declaration can optionally list a bounded,
duplicate-free `capabilities` array containing its original `capability` value.
Package preflight checks all required effect capabilities against that list;
legacy declarations retain their single-capability behavior. Declaring support
does not implement an executor or replace the entry composition's capability
registration.

The [RAG question example](../../examples/ui/rag-query/README.md) uses these
contracts without application-specific node IDs or retrieval rules in the
projection. Its server-backed and browser-local corpus assemblies remain
separate deployment profiles.

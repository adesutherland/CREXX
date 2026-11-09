# Browser UI routes

One Level G ui_web.crexx owner loop runs Text Inspector, the editable graph,
Tile Map Demo, the process designer or a separately packaged graph/map
application. The local route runs its RXBIN in a
single-threaded Wasm Worker. The hosted route runs the same RXBIN in a native
VM behind a loopback authenticated relay. Both use projection.mjs: HTML form
controls, SVG for the graph and Canvas 2D for the map. Browser code owns file
grants and physical input; cREXX owns application transitions. The scene frame
carries its delta and a full authoritative snapshot for reconnect and resync.

## Build and serve

~~~sh
cmake --build cmake-build-ui --target rxvm ui_web text_inspector_web_artifact web_service_probe_artifact example_ui_scene_editor_artifacts example_ui_process_artifacts example_ui_application_packages --parallel 6
source /path/to/emsdk/emsdk_env.sh
emcmake cmake -S ports/ui-web-wasm -B cmake-build-ui-wasm -G Ninja \
  -DCMAKE_BUILD_TYPE=MinSizeRel -DCREXX_GENERATED_DIR="$PWD/cmake-build-ui/generated"
cmake --build cmake-build-ui-wasm --target crexx_ui_vm --parallel 6
node web/ui/package-site.mjs cmake-build-ui cmake-build-ui-wasm cmake-build-ui-web-site \
  examples/ui/app-effects/graph.package.json examples/ui/app-effects/map.package.json
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
The models and schemas remain in the
application modules; the shared browser projection passes effect envelopes.

## Application packages

Each trusted version-1 declaration in `examples/ui/app-effects/*.package.json`
names an application ID, UI contract version, dependency RXBINs and a route
entry for GTK, ANSI, local Wasm and hosted native browser. Every route declares
required operations/capabilities and optional operations. Browser routes also
declare their application executor, which supplies its optional repository
capability. Executor paths resolve beside their declaration; RXBIN paths
resolve under the named build or install root. Preflight rejects an incompatible contract, missing required
operation/capability or missing RXBIN/executor asset before session creation.
Optional scene rendering falls back to semantic ANSI controls; the absent
native repository returns a correlated application-visible refusal. The site
manifest lists each declaration and hashes its selected browser assets.
Application-specific module names come from the declarations; the host code
contains no graph/map repository schemas or storage rules.

The build command above creates all six route entry RXBINs. Check or run a
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

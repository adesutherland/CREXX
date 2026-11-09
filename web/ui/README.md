# Browser UI routes

One Level G ui_web.crexx owner loop runs Text Inspector, the editable graph,
Tile Map Demo or the process designer. The local route runs its RXBIN in a
single-threaded Wasm Worker. The hosted route runs the same RXBIN in a native
VM behind a loopback authenticated relay. Both use projection.mjs: HTML form
controls, SVG for the graph and Canvas 2D for the map. Browser code owns file
grants and physical input; cREXX owns application transitions. The scene frame
carries its delta and a full authoritative snapshot for reconnect and resync.

## Build and serve

~~~sh
cmake --build cmake-build-ui --target rxvm ui_web text_inspector_web_artifact web_service_probe_artifact example_ui_scene_editor_artifacts example_ui_process_artifacts --parallel 6
source /path/to/emsdk/emsdk_env.sh
emcmake cmake -S ports/ui-web-wasm -B cmake-build-ui-wasm -G Ninja \
  -DCMAKE_BUILD_TYPE=MinSizeRel -DCREXX_GENERATED_DIR="$PWD/cmake-build-ui/generated"
cmake --build cmake-build-ui-wasm --target crexx_ui_vm --parallel 6
node web/ui/package-site.mjs cmake-build-ui cmake-build-ui-wasm cmake-build-ui-web-site
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

The selected core development support profile is macOS ARM64 with Chrome 156,
GTK 3.24 and an ANSI `xterm-256color` terminal. The same site package runs
local Wasm and hosted native sessions in Chrome; the hosted build can be a
staged `cmake --install` tree. Linux, Windows, Firefox and Safari are separate
platform/browser qualification gates. This development relay is loopback only;
it is not a public deployment profile.

The site package contains its manifest, HTML/JavaScript projection and route
bridges, the Wasm VM, core UI RXBIN modules and the Text Inspector, graph, map,
process-designer and service-probe RXBIN consumers. `test-package.mjs` verifies
the complete asset list and SHA-256 hashes. The native install tree supplies the VM, GTK
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
CREXX_PLAYWRIGHT=/path/to/playwright/index.mjs \
  CREXX_CHROME=/path/to/chrome \
  CREXX_UI_TOKEN=your-private-token-at-least-16-characters \
  node web/ui/test-browser.mjs
~~~

The browser check covers both routes' forms, graph and map interactions,
application-service results and capability refusal, file selection, close,
Worker restart, hosted reconnect and authentication. The selected support
profile and later platform gates are in the [UI plan](../../lib/ui/PLAN.md).

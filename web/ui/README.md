# Browser UI routes

One Level G ui_web.crexx owner loop runs Text Inspector, the editable graph
or the strategy map. The local route runs its RXBIN in a single-threaded Wasm
Worker. The hosted route runs the same RXBIN in a native VM behind a loopback
authenticated relay. Both use projection.mjs: HTML form controls, SVG for
the graph and Canvas 2D for the map. Browser code owns file grants and physical
input; cREXX owns application transitions. The scene frame carries its delta
and a full authoritative snapshot for reconnect and resync.

## Build and serve

~~~sh
cmake --build cmake-build-ui --target rxvm ui_web text_inspector_web_artifact example_ui_scene_editor_artifacts --parallel 6
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
?app=graph or ?app=map for the scene consumers. The relay binds loopback
HTTP and requires a bearer token on every API request. Deployment needs TLS
termination and user authorization outside this development relay. The
package manifest hashes every copied asset; the hosted process still uses the
native RXBIN build path.

The browser profile supports labels, lines, buttons and one scene per session.
Text Inspector admits text files up to 128 KiB. A Worker restart creates a new
session epoch; a hosted reconnect replays the last full view. Unsupported
controls return a typed refusal. Scene input carries its epoch and revision;
stale input cannot mutate a newer scene. The local Worker has no dynamic
plugins or ambient filesystem access.

## Focused checks

~~~sh
node web/ui/test-protocol.mjs cmake-build-ui
node web/ui/test-scene.mjs
CREXX_PLAYWRIGHT=/path/to/playwright/index.mjs \
  CREXX_CHROME=/path/to/chrome \
  CREXX_UI_TOKEN=your-private-token-at-least-16-characters \
  node web/ui/test-browser.mjs
~~~

The browser check covers both routes' forms, graph and map interactions,
file selection, close, Worker restart and hosted reconnect. Supported browser
and platform qualification belongs to STEP07 of the [UI plan](../../lib/ui/PLAN.md).

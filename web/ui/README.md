# Browser UI form route

`ui_web.crexx` owns one cREXX session per process. Both routes run the same
Text Inspector RXBIN and send version-1 view/input/effect frames over JSON
lines. `projection.mjs` owns DOM controls and browser file grants; it contains
no Text Inspector state transitions or counting. Browser-local Wasm suspends
only at stdin read on the VM owner. The hosted relay keeps the native VM on the
server, authenticates every API request and resynchronizes from its latest full
view after reconnect. A Worker restart makes a new session epoch; application
saved state must seed that new session when persistence is added.

## Build and serve

The root native build supplies `rxvm`, `ui_web.rxbin` and the Text Inspector
RXBIN. Configure the independent Emscripten VM against its generated headers:

```sh
cmake --build cmake-build-ui --target rxvm ui_web text_inspector_web_artifact --parallel 10
source /path/to/emsdk/emsdk_env.sh
emcmake cmake -S ports/ui-web-wasm -B cmake-build-ui-wasm -G Ninja \
  -DCMAKE_BUILD_TYPE=MinSizeRel -DCREXX_GENERATED_DIR="$PWD/cmake-build-ui/generated"
cmake --build cmake-build-ui-wasm --target crexx_ui_vm --parallel 10
node web/ui/package-site.mjs cmake-build-ui cmake-build-ui-wasm cmake-build-ui-web-site
CREXX_UI_TOKEN=your-private-token-at-least-16-characters \
  CREXX_UI_BUILD="$PWD/cmake-build-ui" \
  node cmake-build-ui-web-site/hosted-server.mjs
```

Open `http://127.0.0.1:8787/` for local Wasm or `?hosted=1` for the hosted
route; the latter prompts for the relay token. The sample server binds only
loopback HTTP. A deployment must terminate TLS and enforce its own user/session
authorization outside this development relay. The package manifest hashes
every copied asset. The hosted route still needs the native build path supplied
above; it does not execute RXBIN in Node.

The browser form profile supports labels, lines and buttons. It admits text
files up to 128 KiB for this example. Other controls receive a typed refusal
and may be handled by an application fallback. Graph/map projections, scene
input and service/database adapters belong to later plan steps. The local
Worker uses the shared single-threaded VM profile with no dynamic plugins or
ambient filesystem access.

## Focused checks

```sh
node web/ui/test-protocol.mjs cmake-build-ui
CREXX_PLAYWRIGHT=/path/to/playwright/index.mjs \
  CREXX_CHROME=/path/to/chrome \
  CREXX_UI_TOKEN=your-private-token-at-least-16-characters \
  node web/ui/test-browser.mjs
```

The browser check uses the packaged local Wasm and hosted native routes with
the same file-selection interaction. It also checks Worker restart, hosted
reconnect and clean close. The native protocol check covers cancellation,
stale input, duplicate completion and the cREXX owner-turn journal.

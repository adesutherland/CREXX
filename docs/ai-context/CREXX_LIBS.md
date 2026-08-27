# cREXX Standard Libraries and Built-In Functions (BIFs)

The `crexx` toolchain implements its Standard Libraries and Built-In Functions (BIFs) using a hybrid approach. Some core functions are implemented natively in C via the **cREXX Plugin Architecture (RXPA)**, while many standard library functions (like those in Classic REXX) are actually implemented in cREXX itself.

Libraries are housed in the `lib/` directory, which is divided into domains like:

- `lib/rxfnsb/` (Classic REXX Built-In Functions for Level B)

- `lib/rxfnsg/` (Level G class-shaped general-purpose interfaces)

- `lib/rxfnsl/` (Level L language-engineering examples and generated-output
  proving slices)

- `lib/rxfnsc/` (shared Level C/RexxScript runtime foundation, housing the
  Rexx value, stem, and variable-pool classes plus directly callable Classic
  BIF implementations; `rexxclassicbif_call` is a deprecated compatibility
  path for current compiler output, while name-based intrinsic dispatch belongs
  to RexxScript)

- `lib/plugins/float/` (native scalar binary-float mathematics)

- `lib/plugins/stats/` (packed-float native statistics)

- `lib/plugins/vector/` (exact packed-vector computation and explicit portable
  float32 conversion)

- `lib/plugins/hash/`, `id/`, `fs/`, and `platform/` (narrow declarative
  native providers replacing the historical mixed `rxmath` and broad `system`
  bundles)

- `lib/rxfnsg/rexx/integer.crexx` and `decimal.crexx` (Level-G standard
  integer and decimal mathematics authored in Level B)

- `lib/plugins/` (General-purpose extensions like `fileio`, `regex`, `strings`, `socket`, etc.)

Same-named Level B and Level C functions are separate APIs. For example,
`lib/rxfnsb/rexx/value.crexx` is a read-only, immediate-caller metadata helper,
whereas `lib/rxfnsc/RexxClassicBifValue.crexx` implements the Classic
old-value/optional-assignment contract over `RexxValue` and
`RexxVariablePool`. Direct Level C harnesses call selector functions without
`rexxclassicbif_call`; compiler lowering to those entries is deferred to the
later bulk lowering change.

The CMake build treats each `rxfnsc` member as an isolated source compilation.
Its own source and only its declared transitive `rxfnsc` source dependencies
are copied into a member-private, dependency-keyed source root under
`lib/rxfnsc/members/`. External `library.rxbin`, `classlib.rxbin`, and
`rxcexits.rxbin` imports are copied into the separate curated
`lib/rxfnsc/imports/` root, and `rxc` runs with `--no-exe-import`. This gives
each namespace one intended source or RXBIN provider while the library is
being built; neither stale member files nor an older public `rxfnsc.rxbin` are
eligible through the compiler executable directory. All members may build in
parallel from the declared source dependency graph. `rxlink` writes the
consolidated image under `lib/rxfnsc/linked/`, after which one publication
action replaces `bin/rxfnsc.rxbin` through a temporary-file-and-rename step.
Member actions never delete or rewrite another member's output or the public
image.

The coherent source route is deliberate. The previous shared work directory
could resolve some internal dependencies from source and others from generated
RXBIN metadata depending on what was already present. Source/RXAS/RXBIN route
parity is a separate compiler and metadata qualification obligation; do not
restore mixed candidate visibility merely to reproduce an old linked-image
hash.

`lib/rxfnsb/rexx/binary.crexx` provides the Level B binary helper surface. It
contains the older Classic-style, 1-based, copy-returning helpers such as
`binlength`, `binbyte`, `binsubstr`, `binoverlay`, `bininsert`, `bindelstr`,
`binpos`, `bincompare`, `bin2x`, and `x2bin`. It also contains the Release 1
packed-memory helper surface, which is zero-based and mutates the first binary
argument through `arg expose`: `binresize`, `binclear`, `binfill`,
`binfillat`, `bincopy`, `binmemmove`, `binappend`, `binupdate`, `binmakegap`,
and `bindrop`. These helpers are public fallbacks; compiler or inliner
recognition for direct RXAS lowering should be added only for measured hot
paths.

RexxScript is a first-class runtime product under `rexxscript/`, not a Level B
BIF hidden inside `lib/rxfnsb`. It builds `bin/rexxscript.rxbin`, exposes the
`rexxscript` namespace (`rexxscript_evaluate`,
`rexxscript_evaluate_exposed`, `rexxscript_output`, `rexxscript_value`, and
`.rexxscriptevaluator`), and carries the compatibility `rxfnsb.evaluate`
facade for callers that still use the original prototype API. The `crexx`
driver includes `rexxscript.rxbin` in its default runtime set; direct VM runs
using the `REXXSCRIPT` compiler exit or compatibility `evaluate()` surface must
load `rexxscript.rxbin` at runtime in addition to `library.rxbin`.

The same source directory also builds the standalone `bin/rexxscript`
executable, which packages a file runner around an isolated
`.rexxscriptevaluator()` instance.

The CMake build gives each RexxScript source member a private work directory
under `rexxscript/members/`. Compiler imports are copied into the curated
`rexxscript/imports/` staging directory, and `rxc` is invoked with
`--no-exe-import` so an older executable-directory `rexxscript.rxbin` cannot
be selected while its replacement is being built. The members are linked to
the private `rexxscript/linked/rexxscript.rxbin` image. One publication action
then replaces `bin/rexxscript.rxbin`; member actions do not delete or rewrite
that shared product. The standalone runner likewise imports only from `bin/`,
not from member work directories. This separation is a build-graph ownership
rule: generated RXAS/RXBIN metadata remains owned by the action that generated
it until the consolidated image is published.

The product master documentation is in `rexxscript/doc/`.

`lib/rxfnsb/rexx/rxjson.crexx` contains the first JSON foundation library module
for Level B web-service and transport work. It is implemented in Rexx, ships in
`library.rxbin`, and intentionally exposes string-oriented helpers first:

- `jsonvalid(json)`
- `jsontype(json, path)`
- `jsonget(json, path)`
- `jsoncount(json, path)`
- `jsonmembers(json, path, names[])`
- `jsonquote(text)`
- `jsonunquote(json)`
- `jsonarray(values[])`
- `jsonobject(keys[], values[])`

Paths are Rexx-friendly and one-based for arrays, for example
`choices.1.message.content`. This is enough to build LLM-style request JSON and
extract common response fields without introducing a full object mapper yet.
The parser internals use the binary-memory surface for the JSON source scan:
the input is converted to `.binary` once per public helper call, structural
bytes are read with direct `<at..u8>` lowering, and unescaped object keys are
matched with binary compare before any string materialization. Repeated
extraction from the same large document still reparses per helper call; a parsed
or indexed JSON handle remains the next product-level performance question.

For the full API contract, path syntax, examples, limits, and test location, see
`lib/rxfnsb/rexx/rxjson.md`.

`lib/rxfnsb/rexx/rxsocket.crexx` provides the Level B wrapper for the VM's core
TCP socket instructions. It ships in `library.rxbin` and exposes a small raw
socket API for loopback clients, servers, and future web-service work:

- `socketcreate()`, `socketclose(sock)`
- `socketconnect(sock, host, port)`
- `socketconnecttls(sock, host, port)`
- `socketbind(sock, host, port)`, `socketlisten(sock, backlog)`,
  `socketaccept(sock)`
- `socketsend(sock, text)`, `socketrecv(sock, maxbytes)`
- `socketsendb(sock, data)`, `socketrecvb(sock, maxbytes)`
- `sockettimeout(sock, milliseconds)`, `socketblocking(sock, enabled)`,
  `socketnodelay(sock, enabled)`, `socketkeepalive(sock, enabled)`
- `socketpending(sock)`, `socketshutdown(sock, how)`
- `socketpeer(sock)`, `socketlocal(sock)`, `socketstatus(sock)`,
  `socketerror(sock)`

The default library path is raw TCP and does not attempt HTTP parsing. Its core
value is deployment stability: VM opcodes use platform socket APIs directly,
avoiding dynamic `.rxplugin` discovery. Optional client TLS is exposed through
the same VM-managed handle model as `socketconnecttls(sock, host, port)`, which
connects and starts TLS before application bytes are exchanged. The instruction
exists in all builds; without a TLS backend it returns a negative socket status
rather than signalling. True STARTTLS remains a lower-level RXAS/VM instruction
for future protocol-specific libraries and is not exposed by the public Level B
`rxsocket` wrapper. Fresh CMake configurations select a TLS backend by platform:
`NETWORK` on Apple platforms, `OPENSSL` on non-Windows Unix-like platforms, and
`SCHANNEL` on Windows. `NETWORK` uses macOS Network.framework,
Security.framework, CoreFoundation.framework, and the system trust store,
`SCHANNEL` uses Windows SChannel/SSPI and the Windows trust store, and
`OPENSSL` uses OpenSSL with default verification paths and hostname checks.
`CREXX_ENABLE_TLS=OFF` can be used for dependency-minimal builds.
`CREXX_TLS_STATIC_OPENSSL=ON` asks CMake to prefer static OpenSSL libraries when
the OpenSSL backend is selected. For API details, status codes, and examples,
see
`lib/rxfnsb/rexx/rxsocket.md`.

The older OpenSSL-backed dynamic socket plugin is deprecated and no longer
builds by default. Developers who still need it can configure with
`CREXX_BUILD_LEGACY_SOCKET_PLUGIN=ON`; otherwise source builds avoid that
plugin's OpenSSL discovery and distribution burden.

`lib/rxfnsg/rexx/httpcore.crexx` is the private Level B `_rxhttpcore` backend
for binary HTTP framing, parsing and codecs. It is shared by the Level G client,
server and LLM providers and is not a public convenience client. The public
pre-release HTTP surface is intentionally Level G so that policy, bounded task
lifecycle and typed request/response values are not duplicated at two language
levels.

`lib/rxfnsb/rexx/trace.crexx` provides the Level B trace/debugger internals used
by `rxdb` and by the `TRACE` certified compiler exit:

- `.tracecontroller`: breakpoint enable/disable, module/procedure helpers,
  source/ASM lookup, default runtime/debugger filtering, and shared
  structured trace-event metadata lookup
- `.tracecontext`: immutable per-event module/address/source/ASM/procedure
  snapshot
- `.trace_interrupt_raw`: internal register-mapped view of the VM interrupt
  object used by breakpoint handlers
- `_trace_set(mode)`, `_trace_set_from_env(allow_output)`,
  `_trace_set_format(format)`, `_trace_set_output(target)`,
  `_trace_current_mode()`, `_trace_mode_from_option(option)`,
  `_trace_needs_breakpoints(mode)`,
  `_trace_context_from_raw(raw)`, `_trace_should_trace_context(event)`,
  `_trace_should_emit_key(key)`, `_trace_capture_result_target(module, addr)`,
  `_trace_pending_parent_register(module, type, value)`,
  `_trace_supply_parent_value(value)`, `_trace_pending_result()`,
  `_trace_pending_prefix()`, and `_trace_clear_pending_result()`: the
  compiler-exit-facing runtime surface for setting trace mode/format/output,
  normalizing dynamic `TRACE VALUE` options, applying explicit `TRACE ENV`
  `CREXX_TRACE` / `CREXX_TRACE_TO` environment settings, coordinating simple
  `TRACE R`/`TRACE I` parent-frame value reads, and servicing `BREAKPOINT`
  events. `.tracecontroller` owns the trace-event metadata lookup and pending
  value state so other trace/debug users can share the same interpretation. The
  exit
  still emits caller-frame assembler to enable/disable breakpoints, install the
  handler, and perform the actual `metalinkpreg` read for a pending register,
  because VM signal tables and `metalinkpreg` are frame-sensitive. Register
  reads are driven only by `.traceevent` metadata; `.meta_reg` remains
  scope/register-placement metadata, not a value-change stream.
- `_trace_command_before(environment, command)` and
  `_trace_command_after(environment, command, rc, condition)`: ADDRESS dispatch
  hooks used by `TRACE C`, `TRACE E`, `TRACE F`, and quiet/default `TRACE N`.

Classic text and LLM trace formatting belong to the generated TRACE exit
handler. The shared runtime provides controller/filter helpers and output
plumbing; RXDB and other debugger UIs should make their own presentation and
stepping choices from the structured metadata.
`_trace_set_output` accepts `stdout`, `stderr`, or a file path; file targets are
opened in append mode per trace record.

The helpers rely on VM metadata instructions such as `metaloaddata`,
`metaloadinst`, `metadecodeinst`, and `metaloadedmodules`, so deployable linked
images that strip source/TRACE debug metadata may still provide ASM/module data
while source-line and trace-event lookup return empty. Debugger UI text and
menu rendering belong to `debugger/rxdb_gui.crexx`, not the library trace
internals.

Trace contexts cache their metadata. Breakpoint events resolve procedure data
for namespace filtering first, then load source/instruction content only after
the event is accepted. Namespace matching operates on already-normalized
stored components, and trace escaping scans/appends code points directly.
Invalid direct `_trace_set` modes raise `INVALID_ARGUMENTS`; failure to open a
configured output target raises `NOTREADY`. VM metadata/module queries retain
their zero/empty status protocols.

`lib/rxfnsb/rexx/_address.crexx` owns the Rexx-side ADDRESS protocol. In
addition to command dispatch, redirects, sandboxes, and function calls,
`addressrequest.get_binding_value(name)` gives Rexx providers a simple way to
read scalar bindings auto-exposed from ADDRESS host-variable anchors such as
`:name` and `${name}`. Anchor interpretation remains provider-specific; the VM
only carries binding values and write-back updates.

`lib/rxfnsg/rexx/llm.crexx` contains the Level G LLM integration surface. The
module is `options levelg`, builds into `rxfnsg.rxbin`, uses `rxjson`, and sends
through the public Level G `.httpclient` plus the shared private `_rxhttpcore`.
It exposes a class-shaped interface in the `rxfnsg` namespace:

- `llm`: provider-selecting interface for the local Ollama default
- `ollama`: concrete local Ollama implementation over plain HTTP
- `openai`: concrete OpenAI Responses API implementation over HTTPS
- `anthropic`: concrete Anthropic Messages API implementation over HTTPS
- `gemini`: concrete Gemini `generateContent` implementation over HTTPS

The first provider posts JSON to a local Ollama `/api/generate` endpoint with
`stream:false`. It keeps reconstructed HTTP diagnostics plus the decoded JSON
body available. Hosted providers are also Rexx implementations: they use
environment-variable API keys by default, build provider-specific JSON request
bodies and authentication headers, and send through the same pooled client with
TLS enabled. Each provider closes its owned HTTP pool through `close()`.
`demos/llm/llm_address_environment.crexx` layers a Rexx ADDRESS
provider over these clients so scripts can use model-shaped environments such
as `ADDRESS LLM_GPT_4_1`, `ADDRESS CLAUDE_SONNET_4_5`, and
`ADDRESS GEMMA4_LATEST`; the provider caches by environment instance and routes
through a small driver registry of exact aliases and prefixes. See
`lib/rxfnsg/rexx/llm.md` and `demos/llm/`.

`lib/rxfnsg/rexx/http.crexx` and `httpserver.crexx` are the initial Level
G HTTP surface. The transferable
`.httpclient.pooled(origin, connections, admission, maximum_response, ?policy)`
supports buffered `request`, `get` and `post` task methods plus endpoint-backed
`post_stream`. It exposes `.httpheaders`, `.httppolicy`, `.httpbody` and
independent typed `.httpresponse` values. Each long-lived `.taskwork` connection
owner holds at most one reusable socket. Fixed-size admission descriptors and
type-4 byte endpoints carry canonical references and bytes, never socket
integers or live VM values. The client provides safe headers, bounded budgets,
verified TLS, same-origin 307/308 and idempotency-key replay policy, ambiguity
history, fixed/chunked request streams, identity-encoded response streams and
bounded gzip/zlib/raw-DEFLATE decoding for buffered responses.

The clear-text buffered `.httpserver` owns every accepted socket on its
controller and sends only complete `.httprequest` records to sealed
`.httpservice .taskwork` targets. Handlers return `.httpresponse.text(...)` or
`.httpresponse.binary(...)`; the controller revalidates the result against its
limits. This does not use or implement `.taskscope.ask()` or `.serviceref`.
Server TLS, HTTP/2, WebSockets, background lifecycle and streaming handlers are
outside the current contract. See the
[concurrent HTTP client/server guide](../books/crexx_library_reference/concurrent_http.md)
and the both-VM `lib/rxfnsg/tests_functional/ts_http_*` fixtures.

`lib/rxfnsl/rexx/tinyexpr.crexx` contains the first Level L
language-engineering proving slice. It is deliberately not a lexer generator or
parser generator yet. Instead, it is hand-written in the shape that a future
generator might emit: a packed binary character-class table, fixed-width binary
token records, an exposed declaration procedure for generated token/layout
constants, direct binary-memory reads/writes, a zero-copy lexeme compare helper,
and a tiny precedence parser over the packed token stream. The purpose is to
learn whether the Rexx/RXAS binary-memory surface is usable for generated
language tooling before porting a tool such as re2c or changing a generator
backend to emit this style directly. See `lib/rxfnsl/rexx/tinyexpr.md`.

## 1. BIFs Implemented in cREXX (`lib/rxfnsb/rexx/`)

A significant portion of the Classic REXX Built-In Functions (such as `abs()`, `date()`, `length()`, `substr()`) are written entirely in cREXX. These are located in `lib/rxfnsb/rexx/`. 

This approach minimizes the VM footprint and demonstrates the capability of the cREXX compiler to handle system-level logic.

For repo-native Level B authoring patterns, argument signature examples, and
wayfinding to real `.crexx` examples, see `docs/ai-context/CREXX_LEVELB_AUTHORING.md`.

`lib/rxfnsb/rexx/fileio.crexx` exposes the sequential Level B text file BIFs
`linein`, `lineout`, `charin`, `charout`, and `lines`. These are UTF text
functions over `.string`: `linein` reads one line without its line terminator,
`lineout` writes text plus a newline, `charin` reads UTF-8 codepoints, and
`charout` writes text without adding a newline. They are not the binary byte
I/O surface. On `stdin`, `linein` returns as soon as the line terminator is
read; it does not probe for a following byte. A trailing line terminator at
physical EOF does not create a synthetic empty `linein` record; physical blank
lines are still preserved.
`lines(name)` returns `-1` when the named stream cannot be opened so callers can
distinguish missing/unreadable input from an empty file. Future binary file
BIFs should take and return `.binary` and use
the VM byte instructions (`freadb`, `fwriteb`, `freadbyte`, or `fwritebyte`)
rather than weakening the Level B `.string` UTF contract.

### Build And Debugging Rules

The Rexx BIF library build is a bootstrap build. `lib/rxfnsb/rexx/CMakeLists.txt`
compiles most Rexx BIF modules with `rxc -x --import-rxas`, which disables
certified compiler exits while building the library used by those exits. An
explicit `TRACE`, `PARSE`, `ADDRESS`, or other certified-exit statement added
directly to a BIF source file will fail with `#CERTIFIED_EXIT_DISABLED`.

Do not debug BIFs by adding `TRACE RESULTS` inside `lib/rxfnsb/rexx/abs.crexx`
or another library source file. Use one of these instead:

- a normal scratch program or functional test that imports `rxfnsb` and calls
  the BIF with compiler exits enabled;
- `TRACE UNSUPPRESS NAMESPACE rxfnsb` (or `rxfnsg`, `rxfnsl`, `rxfnsc`) when
  library frames should be visible despite the default system-namespace filter;
- `TRACE ASM` or `TRACE LLM` from the caller for lower-level metadata checks;
- `crexx -native --link-keep-source` or an unstripped `rxlink` image when
  debugging native/linked output, because stripped linked images drop
  `META_SOURCE_STEP` and `META_TRACE_EVENT`.

Useful focused checks for this area:

```sh
cmake --build cmake-build-debug --target testbifs
ctest --test-dir cmake-build-debug -R '^ts_.*_(noopt|opt)$' --output-on-failure
ctest --test-dir cmake-build-debug -R '^test_system$' --output-on-failure
ctest --test-dir cmake-build-debug \
  -R '^(trace_event_metadata|test_trace_|ts_trace_|rxlink_format_check|rxlink_rxdas_strip_smoke)' \
  --output-on-failure
```

The standard Level B array helpers live in `lib/rxfnsb/rexx` and are preferred
over the legacy `lib/plugins/arrays` RXPA plugin. Current array BIFs include
`arrayfind`, `arrayinsert`, `arraydelete`, `arraysort`, `arraycopy`,
`arraydrop`, `arrayhi`, `arraymove`, `arrayappend`, `arrayprepend`, `arraypop`,
`arrayshift`, `arrayget`,
`arrayset`, `arraycontains`, `arrayindexof`, `arrayreverse`, and `arrayjoin`.
The former diagnostic `arraydump` and `arrayformat` procedures are deployed as
the `arrayformatdemo` module under `examples/functions/array-formatting`; they
are demonstration support rather than Level B or Level G library contracts.
These `array*` helpers are currently the `.string[]` helper family, not generic
or numeric typed-array helpers. For `.object[]`, use the separate
`objectarray*` family below. For raw one-dimensional dynamic typed arrays,
including numeric arrays, Level B now has core `rxc` statement forms:
`append array with value`, `insert array with value at index`,
`remove array at index [for count]`, `remove array at first to last`, and
`clear array`. These are compiler syntax, not exits or public helper BIFs, and
lower directly to VM array attribute opcodes.
Object-shaped mutating helpers are exposed separately as `objectarrayinsert`,
`objectarraydelete`, `objectarrayappend`, `objectarrayprepend`, and
`objectarraydrop`, plus `objectarraymove` for block moves. Insertion, deletion,
movement, append, prepend, pop, shift, gap-growing set, and the object-array
insert/delete/append/prepend/drop/move helpers
use VM bulk attribute instructions so the logical array pointer list can be
shifted without a Rexx-level per-element copy loop. Mutating array BIFs must
declare the array with `arg expose`. To clear an existing array object through
the helper surface, use `arraydrop` for `.string[]` and `objectarraydrop` for
`.object[]`; `clear array` is the source-level raw dynamic-array clear
statement.

Container naming is intentionally split by shape:

- Arrays/lists are ordered, one-based, duplicate-preserving sequences using
  `array[0]` as the high water mark. Classic code should use the `array*` BIFs;
  OO code should expose the same semantics through `StringArrayList`/`StringLinkedList`
  methods such as `add`/`append`, `insert`, `remove`, `get`, `set`, `size`,
  `clear`, `contains`, and `indexOf`.
  The Release 1 Rexx-first iterator direction is explicit: `iterator()` returns
  an unsynchronized live iterator over the current collection, while
  `snapshotIterator()` returns an iterator over a factory-time snapshot.
- Maps/stems are keyed containers. Do not overload `array*` for keyed lookup;
  use `stem*` or `map*` BIF names and class methods such as `put`, `get`,
  `containsKey`, `remove`, `keys`, and `values`. The current pure-Rexx
  classlib map iterators are factory-time snapshots built from key/value
  arrays. Do not infer `StringArrayList.iterator()` live-reference semantics
  for maps.
- Sets are uniqueness containers. Use `add`, `contains`, `remove`, `size`,
  `clear`, and `toArray`/`fromArray` semantics consistently across classic and
  class-shaped APIs. The current pure-Rexx classlib set iterators are also
  snapshot iterators.

`lib/rxfnsb/rexx/stem.crexx` is the Level B keyed-container implementation.
Its public hash method retains the conventional Rexx-facing polynomial hash,
but factory/get/set/default-reset/size/key/value methods use the NR-15 native
stem instructions. The private D2-hybrid runtime layout keeps hash metadata in
the receiver binary and strings in ordinary VM values; callers must not inspect
or persist those bytes. The compiler may lower exact calls on a proved simple
concrete `rxfnsb.stem` receiver directly. Class attributes, computed receivers,
and other shapes keep normal call/copyback behavior unless their storage proof
is equally strong. Multi-tail source expressions retain canonical construction
where conversion, evaluation order, or TRACE observation has not proved the
two-segment form equivalent.

`lib/classlib/Concurrency.crexx` is the explicit Level B concurrency surface.
It ships in `classlib.rxbin` and implements the
pool, scope, task, target/work/context, completion, channel/request,
`ChannelValue`/codec, byte-endpoint, service-reference and transfer-buffer
interfaces. Local and isolated-process pool paths reach RXVM only through
authored `chanopen`, `chanstart`, `chanwait`, `chancancel` and `chanclose`
instructions; there is no RXPA task-start path or procedure-name-string
dispatch. Level G task procedures/methods, task expressions and `DO PARALLEL`
lower through these classes, including receiver-side `.taskwork` factories.
The syntax is gated by `OPTIONS LEVELG`; Level B programs may use the explicit
classes directly. Service `ask` and pool statistics deliberately signal
unsupported status `19`. `.taskcontext.endpoint()` reconstructs a worker-local
byte endpoint from a transferable type-4 provider reference and has a direct
public-contract test through the full toolchain and both VM variants.
Functional tests are in `lib/classlib/tests_functional/testConcurrency.crexx`;
`testTaskContextEndpoint.crexx` and imported task method/`.taskwork` tests
exercise `rxc`, `rxas`, `rxlink` and both VMs. The enduring architecture and
status boundary is
[`CREXX_CONCURRENCY.md`](CREXX_CONCURRENCY.md).

Level B classlib collection names carry their value contract because the
language does not yet have generics. Current public classlib containers and
iterator interfaces are therefore explicitly tagged:

- `String...` classes store string values, for example `StringIterator`,
  `StringIterable`, `StringArrayList`, `StringHashMap`, `StringTreeMap`,
  `StringStack`, and related iterator/set/list classes.
- `Object...` classes store object values where no key contract is involved,
  for example `ObjectIterator`, `ObjectIterable`, `ObjectArrayList`,
  `ObjectLinkedList`, and `ObjectStack`. Callers explicitly upcast concrete
  class instances with `as .object` when storing them.
- `StringObject...` map classes use string keys and object values, for example
  `StringObjectHashMap` and `StringObjectTreeMap`. They store keys in Rexx
  string arrays and values in Rexx object arrays.

The current public classlib collection surfaces are Rexx-only. `StringHashMap`,
`StringTreeMap`, `StringHashSet`, `StringTreeSet`, `StringLinkedList`, and the
string-key/object-value map variants no longer require the historical native
`treemap` or `llist` plugins. `StringTreeMap` is backed by an AVL node pool in
parallel arrays; `StringOldTreeMap` retains the previous array-backed map only
for comparative benchmarks. Temporary `StringTreeMapV2`/`V2A`/`V3`/`V4`
experiments were used to measure binary-memory and source-shape alternatives,
then removed because none improved on the production `StringTreeMap` overall.
String-key lookup uses strict equality so empty string keys and blank string
keys remain distinct.

The core `classlib.rxbin` CMake build gives each of its 53 members a private
work directory under `lib/classlib/members/`. External `library.rxbin` and
`rxcexits.rxbin` inputs are copied into a curated import root, and every `rxc`
invocation uses `--no-exe-import`. Internal providers have an explicit route:
`CLASS_DEPS_*` entries are private generated RXBIN inputs, while
`CLASS_SOURCE_DEPS_*` entries are copied into the member's dependency-keyed
source root. The split is required because some current class cycles and
interface signatures remain source/RXBIN-route-sensitive. A member sees only
its declared providers; an unrelated member that happens to finish first and
an older public `classlib.rxbin` are never candidates.

After all member actions complete, `rxlink` writes the core image under
`lib/classlib/linked/main/`. One publication action then replaces
`bin/classlib.rxbin` through a temporary-file-and-rename step. Member actions
do not delete or rewrite the public image or another member's output. Keep the
source and RXBIN dependency tables separate until permanent route-parity tests
prove that a single route is semantically interchangeable.

`Id`, `KeyDB`, and `Os` are intentionally kept out of the core
`classlib.rxbin` image so products such as RexxScript can use the class
library without pulling in unrelated native plugins. They are built and tested
as the opt-in `classlib_native.rxbin` adapter image and depend on the `id`,
`keyaccess`, and `system` plugins respectively. Build, test, and package
changes that expose these classes must include their native plugin runtime
modules. Do not rewrite these wrappers in Rexx merely to avoid native code;
classify or remove the underlying capability explicitly if it is not part of
the intended release surface.

The native adapter build uses the same ownership discipline as the core
classlib. `library.rxbin` and `rxcexits.rxbin` are staged in a curated base
import root, while every adapter receives a dependency-keyed private source and
plugin root. `Id` sees only `rxid.rxplugin`, `KeyDB` sees only
`rx_keyaccess.rxplugin`, and `Os` sees only `rxfs.rxplugin` and
`rxplatform.rxplugin`. Each member runs `rxc --no-exe-import` and `rxas` in its
own directory. `rxlink` writes the complete image under
`lib/classlib/linked/native/`, and one temporary-file-and-rename action
publishes `bin/classlib_native.rxbin`. Do not add a plugin to a broad shared
search directory; declare it against the adapter that imports it.

Bare collection names such as `Iterator`, `Iterable`, `ArrayList`, `HashMap`,
`TreeMap`, and `Stack` are intentionally left free for future Level G generic
or generic-like surfaces. Object-key maps and object sets are also deferred
until the language has a clear object equality/hash/ordering contract.

Imported Rexx BIF calls inline only when the imported artifact carries
`META_INLINE`. `rxlink` strips `META_INLINE` by default, so the standard-library
link explicitly uses `PRESERVE INLINE`. Release linked libraries also use
`STRIP SOURCE`, keeping inline bodies available to downstream optimisation while
dropping source-level debug metadata: `META_SOURCE_STEP` file/source-line
records and `META_TRACE_EVENT` semantic TRACE value records. Debug linked
libraries keep both inline bodies and source/TRACE metadata. Class-library hot
paths that rely on this should inspect generated `.rxas` when changing import
paths or link-strip policy.

RXAS review matters for performance-sensitive Level B classlib code. The
`StringTreeMap` AVL rewrite showed that an inlined private lookup helper still
left block-expression scaffolding in generated RXAS; writing `get()` and
`containsKey()` as direct loops cut Release lookup time for 2,500 entries from
about 200 ms to about 2.3 ms on the local arm64 development machine. Keep hot
lookup/update loops direct unless RXAS inspection proves the helper shape is
equivalent.

The binary-backed treemap trials showed that packed metadata can be expressed
cleanly with `.binary` and direct binary-memory lowering. Source-shape caching
helps both the array/register and binary versions; a packed `.u32`/`.u8` binary
layout improves the binary variant versus the first 64-bit-field cut. The
current optimized `.int[]` AVL metadata remains the best overall shape for this
workload. The measurements are retained in
`docs/planning/beta-3/notes/string-avl-treemap-trial.md` as evidence for
binary-surface ergonomics and missing intrinsics, not as live classlib APIs.

The focused `binary_fastpath_compare` benchmark isolates scalar binary-memory
instructions from collection algorithms. In Release builds, direct `.u8`,
`.u32`, and `.int` binary reads/writes are substantially faster than indexed
`.int[]` attribute-array access. A VM fast-path change keeps strict bounds
checks and canonical little-endian semantics, but replaces byte-by-byte
fixed-width load/store loops with `memcpy`-based 1/2/4/8-byte helpers plus
byte-swap only on known big-endian hosts. A temporary unsafe no-upper-bound
experiment showed only modest gains, so Release 1 should keep strict checked
binary access. See
`docs/planning/beta-3/notes/binary-fastpath-research.md` for timings and user
guidance.

`lib/plugins/arrays` is deprecated and retained only as a legacy plugin smoke
test. New Level B code should import `rxfnsb` and use the standard BIFs.

### Anatomy of a cREXX BIF
Functions written in cREXX follow standard language rules, utilizing namespaces and type enforcement:

```rexx
/* lib/rxfnsb/rexx/abs.crexx */
options levelb

namespace rxfnsb expose abs

abs: procedure = .string
  arg number = .string
  if left(number, 1) = '-' then number = substr(number, 2)
  return number
```

**Key Features:**

1. **Namespaces:** Functions must declare `namespace rxfnsb expose <function_name>` so they correctly bind into the Standard Library space that user scripts import.

2. **Inline Assembly (`assembler`)**: When low-level access is required (such as fetching the current system time in `date.crexx`), cREXX BIFs can drop down into inline bytecode using the `assembler` keyword.

3. **Compilation:** These `.crexx` files are compiled into `.rxbin` bytecodes during the build process and are packaged or shipped exactly like user-compiled binaries.

4. **Explicit Late Load:** `loadmodule(path) -> .int` wraps the VM's
   `METALOADMODULE` instruction. Use it when Rexx code deliberately loads a
   `.rxbin` or `.rxplugin` provider before calling imports supplied by that
   file. The VM relinks immediately after a successful load.

5. **ADDRESS Public Helpers:** `addressenv(name) -> .addressenvironment`
   returns the cached environment object, including `environment_name()` and
   `environment_id()` for the traditional `SYSTEM`/`PATH` environments and for
   Rexx/native providers. `addresscall(env, name, ...) -> .string` wraps the
   lower-level `_address_function(env, name, args[])` request object path.
   `_address_call(...)` and `_address_call_response(...)` remain internal
   compatibility spellings for code that needs the raw response object.

## 2. RXPA (cREXX Plugin Architecture)

For functions requiring native performance or access to C-level system libraries (like cryptography or sockets), `crexx` provides the RXPA framework. This macro-driven C API (defined in `rxpa/crexxpa.h` and `rxpa/rxpa.h`) allows developers to write REXX-callable functions without interacting directly with the VM's internal `stack_frame` or `value` structures.

Plugins can be compiled in two ways:

1. **Dynamic Plugins (`.rxplugin`)**: Recommended for user extensions. The
   compiler discovers declarations on binary import roots. At runtime an
   explicitly loaded legacy plugin still uses module search paths, while a
   compiled native dependency is resolved declaratively through a trusted
   `<provider-id>.rxplugin` artifact.

2. **Static Plugins**: Built directly into the `crexx` binaries. These are typically reserved for core Standard Libraries to guarantee they are always available.

### RXPA concurrency contract

An unmodified RXPA plugin remains valid in a multi-VM process. The host treats
it as a **legacy process-shared plugin**. While the process has only one VM that
has loaded a legacy plugin, its procedures use the same direct adapter as the
single-threaded product. Loading a legacy plugin into a second VM starts one
cold, sticky transition: the host waits for existing direct legacy execution
to leave its VM execution boundary, rebinds every live legacy procedure to one
process-wide recursive compatibility lock, and publishes the new load only
after rebinding. A VM that loads only process-reentrant plugins does not trigger
the transition. Once concurrent legacy mode has been entered, later legacy
loads remain locked for the rest of the process lifetime.

This is conservative because the existing initializer ABI has no way to prove
that the plugin's C statics, its dependencies, or its error paths tolerate
concurrent entry. Recursive locking allows a legacy plugin to make a nested
call that reaches another legacy RXPA procedure without deadlocking. The cold
transition can wait for a long-running legacy-capable VM invocation to return;
plugin load must not assume that publication is instantaneous.

An audited plugin that is safe for concurrent entry can opt in by adding one
file-scope declaration after including `crexxpa.h`:

```c
#include "crexxpa.h"

RXPA_PLUGIN_PROCESS_REENTRANT
```

The declaration is plugin-wide and must appear exactly once in a dynamic
plugin. It exports an optional versioned manifest; it does not change
`_initfuncs(rxpa_initctxptr)`, `rxpa_libfunc`, `ADDPROC`, the language-level
option string, RXAS, or RXBIN. Older hosts ignore the extra symbol. New hosts
treat an absent, malformed, unsupported, or unknown manifest as legacy mode.
Static plugins use the same source declaration and are associated with their
rebuild-together `PLUGIN_ID`.

`PROCESS_REENTRANT` means that all procedures published by that plugin may be
entered concurrently and still have defined behavior. It does **not** mean
side-effect-free. Synchronized logging or I/O, atomics, and calls into
documented thread-safe services are fine. Before adding the macro, audit:

- every writable file-scope/static variable;
- lazy initialization, caches, random-number state, locale and error buffers;
- all libraries and OS APIs called by every published procedure;
- cleanup and failure paths, including calls made during nested execution; and
- any assumption that one call completes before the next starts.

A function that uses only stack locals, immutable tables and RXPA helpers is a
typical candidate. A plugin that increments an ordinary static counter, reuses
one static work buffer, changes process locale, or relies on an undocumented
non-thread-safe library is not; leave it unmarked or add its own synchronization
first. Plugin maintainers should make the assertion wherever this audit passes:
the binding then remains permanently direct, regardless of how many other VMs
or OS threads the process starts.

The P1 capability applies to procedure calls. Native-payload `copy` and
`finalize` callbacks remain on the recursive compatibility lane because they
can run outside the originating procedure and need a separate lifetime claim.

Plugins with mixed policy can instead export the V2 procedure query:

```c
static uint32_t procedure_capabilities(const char *name) {
    if (strcmp(name, "example.stateless") == 0) {
        return RXPA_PROCEDURE_CAP_PROCESS_REENTRANT;
    }
    return 0u; /* conservative legacy lane */
}

RXPA_PLUGIN_PROCEDURE_CAPABILITIES(procedure_capabilities)
```

The query runs while the plugin is loaded and returns exactly one of `0`,
`RXPA_PROCEDURE_CAP_PROCESS_REENTRANT`, or
`RXPA_PROCEDURE_CAP_SESSION_AFFINE`. Unknown bits, both known bits together,
or a session-affine result without a complete session hook set fail closed to
legacy behavior. The host stores the selected invoker in the procedure at load;
ordinary calls do not repeat the query or branch on the capability.

A plugin whose mutable native resources belong to one VM uses
`RXPA_PLUGIN_SESSION_AWARE(create, destroy, enter, leave, query)`. The host
creates one session per VM/plugin load, rejects the load if creation fails,
enters that session around each session-affine procedure call, and destroys it
after VM values/modules are released but before the plugin DSO closes. `enter`
must store the previous thread-local session in its output cookie and `leave`
must restore it, so nested native calls are safe. Session callbacks themselves
must not retain VM-owned `value *` or register handles after the call.

The optional V2 symbol does not change `_initfuncs`, `rxpa_libfunc`, `ADDPROC`,
RXAS, or RXBIN. Older hosts ignore it and call the unchanged procedures as a
legacy plugin. A session-aware plugin that promises old-host compatibility must
therefore provide its own process-default session when no V2 `enter` callback
has selected a per-VM session; ODBC is the reference implementation.

Current bundled classification is deliberately conservative:

| Classification | Bundled examples | Rule |
| --- | --- | --- |
| Plugin-wide process-reentrant | `cipher`, `rx_hash`, `rxfloat`, `rxstats`, `rxvector`, `rxid`, `rxfs`, `rxplatform`, `stack`, `strings`, `getpi` | Audited/repaired and marked with `RXPA_PLUGIN_PROCESS_REENTRANT`. `rxfloat` also publishes direct `rxmath` scalar compatibility names; the historical `inlinec`, statistics, hash and UUID mixture and the broad `system` provider are removed. |
| Per-VM session | `odbc` | Database procedures are session-affine; `odbc.show_message` is process-reentrant; old hosts use the plugin's default session. |
| Unqualified | All other bundled plugins | Remain legacy and serialized until their complete state, dependencies, failure paths and teardown have been audited. |

ODBC testing has two layers. The test-only mock driver is always built when
`BUILD_TESTING` is enabled and deterministically covers failure injection,
retained parameter pointers, concurrent per-VM sessions, teardown and the
old-host default session. When `ENABLE_ODBC=ON` and a `sqlite3odbc` library is
discoverable, CMake also generates a private SQLite `:memory:` DSN and adds
`odbc_sqlite_prepared_rxbvm` and `odbc_sqlite_prepared_rxtvm`. Those tests load
the real unixODBC plugin/driver and cover prepared binds, independent active
statements, reset/re-execute, fetch, transactions, metadata and diagnostics.
Set the `CREXX_SQLITE_ODBC_DRIVER` CMake cache path explicitly when the driver
is installed outside the standard Unix, `/usr/local`, or Homebrew locations.

## 3. Writing a Native Function

A native C function meant to be exposed to REXX is defined using the `PROCEDURE` macro. 

### Argument Access and Returns

The VM passes arguments as opaque handles mapped to internal VM registers. The RXPA headers provide macros to extract native C types from these registers and to write results back:

- `NUM_ARGS`: The count of arguments passed from REXX.

- `ARG(n)`: Retrieves the opaque handle for the *n*th argument.

- `GETINT()`, `GETFLOAT()`, `GETSTRING()`: Extracts the native C value from a register handle.

- `SETINT()`, `SETFLOAT()`, `SETSTRING()`: Writes a native C value into a target register.

  `SETSTRING()` copies the supplied NUL-terminated bytes into VM-owned value
  storage. The plugin retains ownership of the source buffer and must release
  it after `SETSTRING()` when that buffer was allocated by the plugin.

- `SETNATIVEPAYLOAD()` / `GETNATIVEPAYLOAD()`: Attach or read binary payload
  storage. This is the RXPA path for ordinary `.binary` arguments and results,
  as used by `rxhash.sha256()`. Object-shaped native payloads additionally
  require a clear copy/finalizer contract; Rexx code never sees a C pointer.

- `ISINITIALIZED()`: Non-raising test of the language-level typed-object
  initialization flag. Payload-consuming functions use this before treating a
  zero-length object payload as an initialized empty value. RXPA does not reject
  bare typed objects automatically because ordinary procedures may legitimately
  accept one in order to inspect its initialization state.

- `RETURN`: The specific target register designated for the function's return value.

### Error Handling

Errors are thrown using the `RETURNSIGNAL` macro, which halts execution and raises a specific `RXSIGNAL_*` exception within the VM. Successful execution must conclude with `RESETSIGNAL`.

### Example Native Function

```c
#include "crexxpa.h"

// Example: Add two integers together
PROCEDURE(add_integers)
{
    int result;

    // 1. Validate argument count
    if (NUM_ARGS != 2) {
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "2 arguments expected")
    }

    // 2. Extract values and perform logic
    result = GETINT(ARG(0)) + GETINT(ARG(1));

    // 3. Set the return value
    SETINT(RETURN, result);

    // 4. Clear the signal state and exit
    RESETSIGNAL
}
```

## 4. Registering Functions with the VM

Once the C functions are written, they must be registered so the REXX compiler (`rxc`) and interpreter (`rxvm`) can map REXX namespace calls to the native C function pointers. 

This is accomplished using the `LOADFUNCS` mapping block, which binds the C function pointer to a REXX namespace, declares its return type, and defines its expected argument signature.

```c
// Publish functions to the cREXX compiler and VM
LOADFUNCS
//       C Function       REXX Namespace & Name      Opt  Return Type   Argument Signature
ADDPROC( add_integers,    "rxexample.add_integers",  "b", ".int",       "i1 = .int, i2 = .int" );
ADDPROC( string_concat,   "rxstr.string_concat",     "b", ".string",    "s1 = .string, s2 = .string" );
ENDLOADFUNCS
```

### Registration Breakdown:

1. **C Function**: The literal name of the C function defined by `PROCEDURE(...)`.

2. **REXX Namespace & Name**: How the function will be called from REXX code (e.g., `import rxexample; x = add_integers(1, 2)`).

3. **Option**: The target cREXX language level (`"b"` for Level B, `"c"` for Level C).

4. **Return Type**: A string literal dictating the exact cREXX type returned (`".int"`, `".string"`, `".void"`).

5. **Arguments**: The exact type signature expected by the compiler. It supports standard types, array syntax (`.int[]`), and reference passing (`expose`). 

During compilation, `rxc` parses this block to strictly enforce type safety when
the REXX code invokes the native plugin. It also retains the manifest's stable
provider ID and emits `.provider` metadata for used native declarations. During
execution, `rxvm` resolves that ID to an already linked static provider or a
trusted `<provider-id>.rxplugin`, verifies the binary's provider identity and
signature, and only then links the callable into the execution space. No Rexx
wrapper is required.

For a provider with both delivery forms, use the build helper after declaring
the targets:

```cmake
add_dynamic_plugin_target(_example example.c)
add_static_plugin_target(_example example.c)
add_rxpa_provider_package(_example)
```

It publishes `bin/providers/rx_example.rxplugin`, the canonical native archive
`rx_example.a` (or `rx_example.lib`), and the compatibility archive
`rx_example_static.a` (or `.lib`). `crexx -native` reads the same RXBIN
requirements via `rxlink -p`, prefers the canonical archive, falls back to the
compatibility name, and retains its static registration automatically. Tests
and application-local package builds may pass
`OUTPUT_DIRECTORY`; omitting it deliberately selects the standard
build/install provider directory.

A deliberate alternate implementation, such as a mock provider, may retain a
distinct CMake target while publishing the same stable manifest identity:

```cmake
add_dynamic_plugin_target(_example_mock PROVIDER_ID rx_example mock.c)
```

Its delivered artifact must still use the canonical `rx_example.rxplugin`
stem. `PROVIDER_ID` is not an aliasing mechanism: the runtime requires the
requested artifact stem and embedded manifest identity to agree.

### Standard `rx_hash` provider

`rx_hash` is a B+G standard/default provider, not compiler or VM core. It is
built and installed in dynamic and static forms and publishes one-shot,
incremental, hexadecimal, and bounded-memory file SHA-256 plus four named
32-bit hash/checksum procedures:

```rexx
import rxhash

digest = rxhash..sha256(data)
hex_digest = rxhash..sha256hex(data)
state = rxhash..sha256init()
state = rxhash..sha256update(state, data)
stream_digest = rxhash..sha256final(state)
file_digest = rxhash..sha256file(path)
table_hash = rxhash..djb2(data)
seeded_hash = rxhash..murmur3(data, seed)
fnv_hash = rxhash..fnv1a(data)
checksum = rxhash..crc32(data)
```

The exact SHA-256 family is `sha256(data = .binary) = .binary`,
`sha256hex(data = .binary) = .string`, `sha256init() = .binary`,
`sha256update(state = .binary, data = .binary) = .binary`,
`sha256final(state = .binary) = .binary`,
`sha256finalhex(state = .binary) = .string`,
`sha256file(path = .string) = .binary`, and
`sha256filehex(path = .string) = .string`, all in namespace `rxhash`. Raw
results are exactly 32 bytes and direct hex results are exactly 64 canonical
lowercase characters. Embedded zero and invalid UTF-8 bytes are data.

Incremental state is a 152-byte versioned, big-endian, pointer-free binary
value containing a canonical header, byte count, chaining words, pending-byte
prefix/zero padding, and an integrity digest. Updates and finalization decode a
validated copy; updates return a new state and finalization is repeatable.
Malformed states raise `INVALID_ARGUMENTS`. File calls use the same engine with
fixed 32 KiB binary reads; open/read/close failures raise `NOTREADY`, while
filesystem selection, cancellation, and application size ceilings remain
caller policy. See the maintained
[rxhash reference](../books/crexx_library_reference/rxhash.md) for the exact
layout, signal categories, and examples.

`djb2`, `fnv1a`, and `crc32` accept one `.binary`; `murmur3` also accepts an
`.int` seed. Each returns the algorithm's unsigned 32-bit bit pattern
represented in `.int`. Inputs are not mutated. No historical `rxmath` aliases
are retained.

`rxc` records provider ID `rx_hash` in RXBIN metadata for a retained call.
`rxvm` and `rxbvm` then find the trusted `rx_hash.rxplugin` automatically,
while `crexx -native` selects `rx_hash.a` (or `.lib`) and retains its
registration anchor. No Rexx declaration wrapper or explicit runtime provider
argument is required in a standard build or installation.

### RCC-5D through RCC-5F providers

The remaining RCC-5 providers use the same declarative dynamic/static
delivery route:

| Provider ID | Public namespace and procedures | Contract note |
|---|---|---|
| `rxstats` | `rxstats.mean`, `stddev`, `covariance`, `correlation`, `regression` | Level G statistics over borrowed read-only `.packedfloat` payloads. Compensated shifted-origin accumulation plus a compensated second pass protects ill-conditioned central moments; regression returns immutable `.linearfit`. Boxed arrays, `.packedint`, and raw `.binary` are not production overloads. |
| `rxvector` | `rxvector.decodef32le`, `encodef32le`, `cosine`, `topkcosine` | Level G exact vector computation over borrowed `.packedfloat`/`.packedint` payloads, with explicit canonical little-endian float32 conversion. The provider is stateless and process-reentrant; prepared/ANN indexes are not part of this contract. |
| `rxid` | `rxid.uuid4`, `uuid7`, `ulid`, `nanoid`, `snowflake`, `base58` | Bundled optional Level G identifier strings, callable from B when installed; random forms use platform cryptographic randomness and generation failures signal. |
| `rxfs` | `rxfs.cwd`, `loadpath`, `chdir`, `isdir`, `mkdir`, `rmdir`, `delete`, `rename`, `isfile`, `listdir`, `append` | Narrow filesystem and directory operations. Return/status contracts are documented in the library reference. |
| `rxplatform` | `rxplatform.uptime`, `user`, `host`, `osname`, `sleep` | Bundled optional Level G host/platform information and millisecond sleep, callable from B when installed. Clipboard, beep, process-global and developer functions from the old draft `system` surface were retired. |

The source CMake target for `rxplatform` is named `_platform` to avoid a target
collision, but `PROVIDER_ID rxplatform` makes its manifest, artifact stem,
RXBIN dependency, runtime lookup, and native archive identity consistently
`rxplatform`.

### Level G packed numeric owners

The `rxfnsg` Rexx library publishes `.packedfloat` and `.packedint` as the
comfortable Release 1 Level G owners of the Level B host-native packed numeric
surface:

```rexx
import rxfnsg

values = .packedfloat(3)
call values.set(2, 100.0)
say values.get(2)

indexes = .packedint(16)
call indexes.fill(-1)
```

Both classes provide `*(size)`, `fromBinary(data)`, `size()`, `get(index)`,
`set(index, value)`, `resize(size)`, `fill(value)`, and `binary()`. Counts and
indexes are item-based and zero-based. Construction and growth zero-fill;
`fromBinary` is an explicit inbound copy boundary. `binary()` returns a weak
mutable `reference .binary`: `dereference` creates a scoped live alias without
copying, while `snapshot` is the explicit outbound copy boundary. The packed
owner must outlive the reference.

The owned storage is the class object's `register.0.binary` component rather
than a separate Rexx binary attribute. Native providers may therefore receive
a declared `.packedfloat` or `.packedint` argument and consume its payload
directly; the caller does not invoke `binary()` and no intermediate Rexx byte
copy is required. The payload remains host-local and must use the raw encoded
binary route for files, persistence, wire formats, or incompatible hosts.

RXPA's `ISINITIALIZED(value)` helper lets such a provider preserve the ordinary
typed-object `OBJECT_NOT_INITIALIZED` boundary before borrowing the payload.
It is appended to `rxpa_initctx`, so earlier field offsets remain stable, but
the initializer context has no negotiated size. This pre-release extension is
therefore a rebuild-together boundary: do not mix a plugin compiled against the
current `crexxpa.h` with an older host binary. The compiler scan stub and both
dynamic and static VM initializer contexts must populate every appended helper.

The classes deliberately wrap the existing `<packed..float>` and
`<packed..int>` Level B instructions. They do not change ordinary `.float[]`
or `.int[]` arrays and do not yet provide `x[index]` syntax; both are
post-release language work.

## 5. Declaring Native Classes and Interfaces

RXPA can also publish class/interface contract metadata to the compiler and VM.
Use this when a native or hybrid provider needs to expose the same class-shaped
contract that Rexx source would normally declare.

```c
LOADFUNCS
ADDINTERFACE("demo.environment");
ADDFACTORY("demo.environment", "*", ".environment", "name=.string");
ADDMETHOD("demo.environment", "describe", ".string", "");

ADDCLASS("demo.nativeenvironment");
ADDIMPLEMENTS("demo.nativeenvironment", "demo.environment");
ADDFACTORY("demo.nativeenvironment", "*", ".nativeenvironment", "name=.string");
ADDMETHOD("demo.nativeenvironment", "describe", ".string", "");

ADDPROC(make_env, "demo.make", "b", ".environment", "");
ENDLOADFUNCS
```

Rexx source factories omit return types (`*: factory`). RXPA declaration
macros still carry a return-type string because they emit low-level metadata
directly; use the owner contract type for that metadata until RXPA grows an
owner-derived factory helper.

Available declaration macros:

- `ADDCLASS(name)` and `ADDCLASSX(name, option, type)`

- `ADDINTERFACE(name)` and `ADDINTERFACEX(name, option, type)`

- `ADDIMPLEMENTS(class_name, interface_name)`

- `ADDFACTORY(owner, member, return_type, args)`

- `ADDMETHOD(owner, member, return_type, args)`

- `ADDDEFAULTMETHOD(owner, member, return_type, args)`

- `ADDMEMBER(owner, kind, member, return_type, args)` for the generic form

These declarations are consumed from both dynamic `.rxplugin` modules and
static `DECL_ONLY` declaration libraries. They make contracts visible for type
checking and runtime metadata discovery.

Declaration is not construction. `ADDCLASS`, `ADDINTERFACE`,
`ADDIMPLEMENTS`, and the member macros tell the compiler and VM that a contract
exists, but they do not by themselves run a factory or stamp class identity on
a return value. Existing RXPA return helpers such as `SETSTRING`, `SETINT`, and
array attribute helpers fill a return value slot; factory/class construction is
still a separate operation.

That means a native procedure can advertise a typed signature, for example:

```c
ADDPROC(make_env, "demo.make", "b", ".environment", "");
```

but the C body must still create or receive an object value that has the right
shape and class identity. For complete object creation today, use a small Rexx
factory/class shim to create the typed Rexx object, and let that object
delegate selected work to native C functions. A future RXPA helper should cover
the pure-C operation of constructing/stamping a typed object directly.

Ordering matters when a native procedure signature references a class or
interface type. Put the relevant `ADDCLASS`/`ADDINTERFACE` metadata before the
dependent `ADDPROC` so the compiler knows the type before it validates the
procedure signature:

```c
LOADFUNCS
ADDINTERFACE("demo.environment");
ADDMETHOD("demo.environment", "describe", ".string", "");

ADDPROC(make_env, "demo.make", "b", ".environment", "");
ENDLOADFUNCS
```

### Native payload ownership

When a native implementation needs to hide a C-side handle inside a Rexx object,
the preferred physical storage is the value's binary payload. Attach shared
static payload operations with `SETNATIVEPAYLOAD()` if the payload owns native
resources:

```c
static const rxpa_native_payload_ops env_payload_ops;

static void env_finalize(rxpa_attribute_value value) {
    EnvHandle **slot = (EnvHandle **)GETNATIVEPAYLOAD(value, NULL, NULL, NULL);
    if (slot && *slot) env_release(*slot);
}

static void env_copy(rxpa_attribute_value dest, rxpa_attribute_value source) {
    EnvHandle **slot = (EnvHandle **)GETNATIVEPAYLOAD(source, NULL, NULL, NULL);
    EnvHandle *copy = slot && *slot ? env_retain(*slot) : NULL;
    SETNATIVEPAYLOAD(dest, &copy, sizeof(copy), &env_payload_ops, 0);
}

static const rxpa_native_payload_ops env_payload_ops = {
    "demo.EnvHandle",
    env_copy,
    env_finalize
};
```

The ops object is provided by the native module and shared across instances;
the value stores only a pointer to it. The VM owns the per-value binary payload
buffer. `SETNATIVEPAYLOAD()` mallocs VM-owned storage, copies the supplied
payload bytes into it, and records the shared ops pointer and flags. Copy hooks
must install the destination payload through `SETNATIVEPAYLOAD()` rather than
storing externally allocated memory directly in the destination value. On
`clear_value()`, the finalizer runs before the VM frees the binary buffer; the
finalizer releases nested native resources but must not free the payload buffer
itself. On `move_value()`, the buffer and ops pointer move together. On
`copy_value()`, the VM calls the copy hook if set; if it is not set, the VM
byte-copies the payload and copies the ops pointer. That fallback is only safe
when the payload was deliberately designed to tolerate duplicate finalization,
such as a registry handle or refcounted pointer. Use
`RXVM_NATIVE_PAYLOAD_FLAG_BITCOPY_SAFE` to document that intent on such
payloads.

# UI contract v0

This is the shared, experimental cREXX UI vocabulary and executable contract.
It is **not** a promise that every named widget or service is implemented.
The [first full-screen terminal slice](../TERMINAL.md) now exercises this contract;
a browser UI remains the next surface priority.
Peter's desktop work can use the same contract. Mobile contributes lifecycle,
touch, permission and resource requirements without forcing a mobile toolkit
into cREXX application code.

## How the framework fits together

A **component** is a cohesive feature: it owns a model, `update(event)` and
`view()`. A **command** is a named user intent shared by buttons, menus and
shortcuts. The session resolves an enabled command to a registered event and
target component. An **event** records something that happened; an **effect**
requests work outside the component. Effects complete by sending another event.
The component returns an owning `uieffects` collection, including an empty
collection when it wants no work. It never calls the desktop, console or browser
directly.

The session owns component values and serializes their updates. The driver owns
the actual OS/browser event loop, input and rendering. An executor owns each
physical operation (file grant, timer, dialog, worker, etc.). In a small driver,
the driver and executor can be the same object; they are responsibilities, not
a mandatory class hierarchy.

1. Create a session with an identity and **implemented** capabilities. Register
   feature components, application message schemas and command descriptors.
2. Start the surface, then `post(ui.ready)` to the relevant component.
3. Input uses `invoke(command_id)` or `post(event)`. Admission validates the
   schema and routing. It queues work; it does not call the feature immediately.
4. The owner calls `pump(budget)`. Each event updates just its target component;
   its zero-or-more effect requests are validated and queued as a batch.
5. The executor calls `take_effect()`, performs work and calls `complete()` with
   the matching result. A delayed result uses exactly the same route as an
   immediate one. Nothing re-enters `update` while it is already running.
6. Ask for the component view and render it. Views are descriptions, not live
   widget objects. Repeat when input, completion, or another wake-up arrives.
7. `ui.close.requested` is an intent the feature may decline or precede with
   save/confirmation. `ui.close` asks the host to close; `ui.closed` is its
   completion. Finally release physical operations/resources and call the
   explicit session `close()` teardown method.

The session's `pump` is not a busy polling loop. GTK may call it from its own
loop, the terminal calls it after console input/wake-ups, and a
browser host can call it after receiving a message. Foreign threads must marshal
owning data to the owner; neither `uisession` nor borrowed RXPA handles are
thread-safe mailboxes. No new VM callback or thread mechanism is introduced here.

## One source of names and shapes

[catalogue.json](catalogue.json) is authoritative. [Generate.cmake](Generate.cmake)
generates, under `BUILD/lib/ui/contracts/`:

- `ui_catalog.crexx`: Level G catalogue/profile lookup functions;
- `ui-contract.ts`: typed JavaScript-facing payload/envelope types and builders;
- `ui-contract.schema.json`: JSON Schema for standard message envelopes;
- `CATALOGUE.md`: the full readable list of names, fields and meanings.

These artifacts are installed with the source catalogue. Building them requires
CMake, not Node or Python. The TypeScript builders are authoring helpers, not
runtime validation or a browser driver. A transport must validate incoming data.

The catalogue distinguishes a small required profile from optional capability
families. A name's presence does not require every host to implement it.

| Family | Standard surface and purpose |
| --- | --- |
| Required core | `ui.ready`, `ui.close.requested`, `ui.closed`, `ui.command.invoked`; `ui.close`; generic effect completion, failure, cancellation and progress |
| Input and focus | semantic keys plus modifiers, text input, composition, paste, focus changes and focus requests |
| Widgets and collections | activation, values, checked state, selection, item activation, expansion, form submission and validation |
| Layout and navigation | viewport with explicit units, scrolling, reveal-item and navigation requests/results |
| Resources and services | scoped resource choice, text read/write, confirmation, clipboard and timers |
| Rich surfaces | pointer, context menu, gestures, drop, text selection and accessibility announcements |
| Lifecycle and preferences | visibility, suspension/resumption, theme/locale/reduced-motion/high-contrast preferences |
| Remote/mobile seams | session connection/disconnection/resumption, capability changes, permission requests, sharing and stream subscription/items |

`core` and `tracer.compat` are usable profiles. `terminal.planned` and
`web.planned` are coverage checklists, **not capability claims**. Only advertise
services actually supplied by your host. Capability changes are vocabulary for
future adapters; the v0 session's advertised set is fixed at construction.

Standard names are reserved under `ui.*`, `widget.*` and `io.*`. Applications
extend them under their own namespace, for example `document.open.requested` or
`debugger.continue.requested`. Register the schema before use; an unregistered
name is rejected. An application effect additionally declares a capability and
a registered completion event. Register that event with role `completion` so it
cannot bypass request correlation through ordinary input admission. Registering
an effect does not create its executor.

Command IDs are a separate registry, with label, target, intent event, shortcut
hint and enabled state. In v0 a command invokes an **empty-payload input event**;
parameterized operations use an explicitly constructed event. Command IDs need
not be the same as event names. The tracer retains two old action spellings to
avoid breaking existing scripted input. An incoming `ui.command.invoked` is
resolved by the session, not broadcast to all features. Disabled commands are
rejected centrally regardless of which control invoked them.

There is no implicit event bubbling, global broadcast or feature-to-feature bus
in v0. The application owner explicitly coordinates cross-feature workflows.
Likewise `cancel()` is currently an owner API: this step does not yet define a
component-emitted cancellation effect or a request-start notification. Applications
needing that control from inside `update` should settle that small follow-on
contract before adding driver-specific conventions. Native drivers must not
acquire application policy to fill the gap.

## Envelope and payload rules

All eight envelope fields are required:

| Field | Meaning |
| --- | --- |
| `version` | Contract version, currently integer `0` |
| `direction` | `event` or `effect`, matching its catalogue entry |
| `name` | Registered, case-sensitive dotted semantic name |
| `target` | Component identity, not a native widget handle |
| `payload` | Named, typed object fields, with no undeclared fields |
| `request_id` | Effect correlation token; empty for independent input |
| `session_id` | Owning session lifetime identity |
| `sequence` | Session admission order; zero before admission |

Local callers normally construct `uimessage(name, body, target)` for an event
and add `"effect"` for an effect. `uipayload` offers `put_text`, `put_integer`,
`put_flag`, `put_texts` and `put_resource`; application code need not assemble
JSON. The session stamps admitted events and requested effects. Completion
copies the request's target, request ID and session ID, and supplies sequence
zero; it receives a new sequence when accepted.

Order is per session, not global. Effects can finish out of request order.
Session IDs must not be reused across owner lifetimes where stale results could
arrive. Transport retries, acknowledgements, authentication, replay protection
for ordinary input and reconnection resynchronization are **not implemented** by
these local queues. Schema validation is not authorization.

Payload fields support `string`, `integer`, `boolean`, `strings` (text array)
and `resource`. Missing optional fields are allowed; `null` is not absence.
String-to-number/boolean coercion is forbidden. Integers are exact and bounded
by JavaScript's safe range, with narrower per-field bounds where declared.
Integral JSON values such as `24.0` or `8e1` are accepted. Text lengths and text
selection offsets count Unicode scalar values, not UTF-8 bytes, JavaScript UTF-16
units, graphemes or terminal cells. Text selection uses zero-based, end-exclusive
offsets. Terminal cell width/grapheme movement remain terminal-driver policy.

The default text-field limit is 16,384 scalars, array limit 4,096 items, and
serialized payload limit 262,144 scalars; field definitions can tighten them.
The schema expresses per-field bounds; the aggregate serialized-size limit is
an additional host admission rule, not expressible as JSON Schema object length.
A remote host must also bound bytes and nesting **before parsing**, authenticate
its session and authorize resource/command access. This is not yet a secure
network ingress implementation or a bulk file-transfer protocol.

A resource has `id`, `scope` (`local`, `session`, `remote`), `display_name`,
`readable` and `writable`. The ID is an opaque provider grant. Display text must
never be used to resolve a path. Providers enforce scope and access; a plausible
JSON descriptor does not itself grant permission. This lets native choosers,
browser uploads and remote objects use the same application contract.

## Effects, cancellation and ownership

Finite work follows `pending -> running -> terminal`. `take_effect` marks a
request running; `complete` accepts only its declared success, common failure,
common cancellation or progress outcome. Only the first valid terminal outcome
is admitted. Unknown/stale IDs, wrong targets/sessions/outcomes and duplicate
terminal results are rejected without calling a component. Generic failures
carry a stable `code` and display `message`; cancellation carries a `reason`.
Progress is nonterminal. `ui.stream.subscribe` is a subscription: repeated
`ui.stream.item` events leave it active until completion/failure/cancellation.

`cancel(request_id)` is **logical cancellation**: it queues one cancelled event
and makes late results stale. The host must also stop the physical operation
and release its resources. A cancelled timer/worker does not magically stop in
the OS. `close()` is teardown, not the `ui.close` effect: it drops queued work
and owning component/command values and rejects later activity. It does not
promise to deliver cancellation notifications after the owner has gone away.

Default bounds are 128 queued events and 64 concurrent requests. Work slots are
reused, so this is not a lifetime limit. Full-queue input/completion admission
returns `-2`; a rejected completion stays active and can be retried. Success is
`0`; invalid/stale input returns `-1` (other registration APIs return negative
diagnostics). Hosts must handle return values rather than silently drop work.
`pump` returns handled count or `-1`. A component emitting an invalid/oversized
batch creates a fatal `fault()` and publishes no effects from that batch: its
already-mutated model is not rolled back. The host should report the fault and
tear down. This is a programming error, not user input backpressure.

## How to trace the worked example

Read [Text Inspector](../../../examples/ui/text-inspector/text_inspector.rxpp),
then [ui_contract.crexx](../ui_contract.crexx), then
[ui_compat.crexx](../ui_compat.crexx), and finally the chosen legacy driver.

The feature implements `uicomponent`. Its tiny `uiapp` shell registers its
`document.*` events and commands. `UI_COMMAND` and `UI_NODE` come from
`##LOADMACRO ui`; RXPP generates wiring and views, not hidden application policy.
The shell's bridge supplies the existing `uiruntime`/`uidriver` entry point.

| Legacy adapter protocol | Standard feature contract |
| --- | --- |
| `app.ready` | `ui.ready` |
| `app.quit.requested` | `ui.close.requested` |
| `choose_file` / `file.selected` | `ui.resource.choose` / `ui.resource.selected` |
| `file.selection.cancelled` | `ui.effect.cancelled` |
| `read_text` / `file.loaded` | `io.text.read` / `io.text.read.completed` |
| `file.load.failed` | `ui.effect.failed` |
| `quit` | `ui.close` request |

The bridge owns the actual path and one current local resource grant. It is
synchronous, supports one root feature, and only advertises the tracer profile.
Reads exceeding the message/record limits produce `ui.effect.failed` with code
`resource_too_large`, not a partial word count. Its legacy `readlines` executor
still reads the file before this check; streaming/bounded-memory I/O is later work.
It does **not** turn the legacy callbacks into asynchronous callbacks. Legacy
drivers lack a post-destruction callback, so the bridge does not fabricate
`ui.closed` before their window closes: it abandons that final request on
teardown and returns `quit`. A new v0 driver must implement the complete close
handshake. Multi-component routing and deferred effects are exercised separately
by the session conformance tests.

## Testing and the next driver

`ui_functional_tests` and `example_text_inspector_artifacts` prepare focused tests.
Run `ctest --test-dir BUILD -R '^ui_|^text_inspector_' --output-on-failure`
after building those targets. The new `ui_contract_v0_noopt`/`_opt` tests exercise
the compiler, assembler, linker (optimized mode) and VM, every catalogue entry,
malformed fields, routing, commands, correlation, cancellation and backpressure.
[conformance.jsonl](conformance.jsonl) is a shared language-neutral wire corpus
for future JavaScript and desktop adapter tests as well as the Level G validator.
JSON validity is deliberately separate from session-state admissibility.

The optional client check uses an independent JSON Schema implementation and
the TypeScript compiler, including expected-error calls for invalid payloads.
Keep its development dependencies outside the product build:

```sh
ui_js_deps=$(mktemp -d)
npm install --prefix "$ui_js_deps" --no-audit --no-fund --no-package-lock typescript@5.9.3 ajv@8.17.1
node lib/ui/contracts/CheckClient.cjs BUILD/lib/ui/contracts "$ui_js_deps/node_modules"
```

These dependencies are for this explicit check only, not installed UI runtime
requirements. The CMake generator and Level G conformance tests remain mandatory
normal QA and do not depend on JavaScript tooling.

Use `qa-comprehensive` for broader prepared normal QA. Sanitizer qualification
remains the later agreed tracer-scope gate; this step makes no sanitizer claim.
See the [local qualification record](QUALIFICATION-2026-09-06.md) for retained
commands, results and the tested input fingerprint.
The first native console provider and full-screen terminal dialogs are described
in [TERMINAL.md](../TERMINAL.md). `terminal.planned` remains a wider checklist,
not a claim that every terminal service exists. Web hosting/transport, responsive
layout and accessibility implementation remain subsequent work.

The builder seam is now explicit: stable view/node IDs and relative placement;
command descriptors; registered message schemas; feature-owned handlers; host
capabilities. A builder should generate those declarations and preserve the
handwritten feature classes. v0 still uses the tracer's flat logical view (now
including a terminal `input` node):
containers, rich properties, validation metadata and state composition across
views are future additions, not secretly implemented by the event catalogue.

# Extending the reference model

Start with [the lifecycle](README.md#the-framework-in-a-minute), then follow
[Text Inspector](../../examples/ui/text-inspector/text_inspector.rxpp).
Keep policy in cohesive features and platform mechanism in drivers. The
composition function wires owners, schemas and commands; it is not a dispatcher
containing every application's behavior. Use existing catalogue names before
inventing an extension. All cREXX UI layers and applications are Level G.

## Add a feature

1. Implement `uicomponent`: typed model attributes, `update(.uimessage)` returning
   `.uieffects`, and `view()` returning `.uiview`. Keep a workflow's state and
   transitions together rather than creating a class per button.
2. In composition, register the component under a stable target and register
   its application event schemas. Add commands whose target is that owner.
3. Put command IDs in view buttons. Multiple buttons/menu entries/keys can
   invoke the same command. Payload-bearing input uses explicit `post(event)`;
   v0 commands carry no parameters.
4. Unit-test model transitions without a driver, then test them through a
   session to exercise schema, target, capability and correlation rules.

For example, `document.open` is the command; `document.open.requested` is its
application event; `ui.resource.choose` is standard requested work. The feature
does not learn which driver implements it. Several targets are supported by the
session; the reference hosts currently project the root view. Composition of
multiple visible component views is a next design step, not implicit bubbling.
Cross-feature orchestration belongs in an explicit application owner, never in
a native driver or a global catch-all event handler.

## Add a widget

1. Define its driver-neutral value and interaction semantics. Reuse standard
   `widget.*` events and typed payloads where appropriate. A logical node ID
   identifies a control; the message target identifies the owning component.
2. Extend `uiview`/`uiview_impl` and their RexxDoc tags together. Update
   `UI_NODE`/`add_spec` if the declarative authoring surface changes.
3. Implement projection and input translation in each supporting driver:
   layout, focus, clipping/hit-testing, activation and state updates. Unsupported
   rendering must be explicit, not silently converted to a different widget.
4. Add view tests plus real input tests in each supported host. The existing
   `line` is a small projection example; ANSI's `input` illustrates editing.

The current flat view is intentionally limited. A new container, rich property
model or accessibility contract needs an agreed cREXX design before native
code drives the shape of the API.

## Add an effect

1. Prefer a standard effect from [catalogue.json](contracts/catalogue.json).
   A standard extension changes that one source, generated bindings and the
   shared conformance corpus. An application-only effect registers its own
   namespace, capability, payload shape and completion event in composition.
2. Return an unstamped request from `update`; the session stamps target/request/
   session ownership and queues it. Never call an executor inside `update`.
3. The executor takes the request once, retains it while pending and reports
   progress or one terminal success/failure/cancellation using `complete`.
   Preserve all three correlation fields. Display resource names are not grants.
4. Check admission results. On a full completion queue the request remains
   active: retain/retry after an owner turn, or fail and cleanly terminate the
   host. Do not silently drop outcomes or report success. Stop physical work on
   cancellation/teardown and reject late callbacks.
5. Test success, cancellation, failure, wrong/stale/duplicate outcomes,
   overlapping work, capacity and close with work outstanding. Add the capability
   only when an executor exists; registration alone does not implement it.

`ui_local_resources.read` demonstrates a shared executor. Dialog execution in
each host demonstrates deferred completion. Subscriptions/progress are exercised
by session tests, not yet by a production worker or streaming-file executor.

## Add a driver: Peter's checklist

- Implement `uidriver.run(session)` and a namespace-level `capabilities()`.
  Use `UI_LAUNCHER` with the existing session factory; do not create an app wrapper.
- Acquire the surface, post `ui.ready`, admit input through `invoke`/`post`,
  service bounded `pump`/`take_effect` turns, and render logical views.
- Native callbacks capture owning facts and wake the owner. GTK shows an idle
  mailbox; ANSI shows timed polling. No native signal calls a feature directly,
  no borrowed RXPA handle escapes its outer call, and no callback re-enters update.
- Keep active modal requests correlated and return to the owning loop. Handle
  unsupported purpose/effect, simultaneous dialogs and capacity explicitly.
- Separate close intent from destruction: support veto, cancel physical work,
  destroy/restore the surface, complete and pump `ui.closed`, then tear down the
  session. Clean up on setup/callback/input failure too; return a diagnostic.
- Reuse the [driver observer](tests_functional/driver_probe.crexx) and the
  [example test composition](../../examples/ui/text-inspector/test_host.crexx).
  Match the line/GTK event journal and add native input, modal-close, error and
  resource-cleanup tests. ANSI's foreground PTY tests cover terminal restoration.
- Add explicit CMake producer and file dependencies, opt/noopt linked runtime
  tests, and install/provider closure checks. Do not depend on stale build files.

A builder should generate declarations—IDs, layout, commands, schemas and
composition—while leaving feature methods handwritten. It can emit RXPP or
equivalent cREXX; the runtime contract must not depend on the builder. A web
driver additionally needs transport authentication, bounded parsing, session
identity/reconnection and resource authority; the generated TypeScript schema
does not provide those services by itself.

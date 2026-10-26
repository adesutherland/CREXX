# Optional Level L IR execution

`rxfnsl-execute.rxbin` supplies the neutral `llirexecute` runner. It follows
admitted IR control and owns execution frames; registered handlers supply
operation and value semantics. It contains no source grammar, language condition
names, variable pools, BIF dispatch or RXAS instructions. The core SDK, grammar
bootstrap and static applications remain independent of this package.

Build target `rxfnsl_execute`. Consumers import `llirexecute` and link the execute
image with `rxfnsl-analysis.rxbin`, `rxfnsl.rxbin` and `library.rxbin`.
`stage-l-libraries` prepares these separately named SDK images. The independent
[ordinary execution fixture](../tests/library/ts_llirexecute.crexx) and
[manifest/lifecycle fixture](../tests/library/ts_llirexecute_dynamic.crexx) use
non-cREXX operation families. Language-specific adapters live outside this
package.

## Trusted services and whole-module admission

Construct `registry(contract)` from the trusted frozen schema. Register pure,
immutable type services with `type(name,service)`, then named mutable handlers
with `handler(name,service)`. `operation(opname,handlerName,summary,bytes)` binds
an opcode to a sealed `llireffects.summary` and an output-byte bound. Several
opcodes may share one authoritative handler; dispatch writes its changed state
back to that entry. `call(opname,functionId,summary)` instead binds a local IR
call, whose stack and return are owned by the runner. Runtime handlers are
separate from constant-fold evaluators. Saved IR cannot register host code.

`plan(reference graph,registry,work=1000000,bytes=67108864,?execution)` preflights
the complete frozen module, including unreachable operations. It checks exact
schema/family versions, structured effects, types, calls, definite assignment,
metadata and resource bounds before dispatch. Check `valid()`, `status()` and
`message()`. A valid plan owns its graph snapshot; execution does not depend on
the original source graph remaining alive. Revision/matching queries still refer
to that original owner.

Core constants, arguments, optional arguments, presence, slots, jumps, branches,
returns and explicitly admitted invocation protection are supported. Other core
control, unregistered operations, opaque effects and escaping operands fail
admission. A borrowed operand requires a registered binding-lease type.

## Executable manifest

`manifest(reference graph)` belongs to that exact live graph revision. Equal
saved bytes from another owner do not substitute for its identity. Supply the
manifest as the plan's optional final argument; the plan checks and seals its
copy. The declarations are:

| API | Checked meaning |
|---|---|
| `protect(operationId)` | Admit that exact `core.protect` declaration. Its normal/exceptional successors already belong to the graph. |
| `port(operationId,kind)` | Admit `invoke`, `transfer` or `invoke transfer` at that site. `registry.port(opname,kind)` supplies the corresponding trusted capability; neither declaration chooses a program counter. |
| `gate(valueId,guardValueId,branchOperation,presentWhen=1)` | Bind one conditional extension result to a same-block Boolean branch and its present edge. |
| `callmap(operationId,formal,inputOrdinal,presenceInputOrdinal=0)` | Map every formal of a local IR call explicitly. Input ordinal zero means omitted; a supplied presence operand is an exact Boolean. |

A `transfer` or `diverges` schema operation requires a checked transfer port and
cannot complete normally. Structured summaries must declare the corresponding
transfer/resume possibilities. New explicitly registered invocation ports require
the matching graph declaration. Legacy suspension-only bindings retain their
previous resume-summary contract.

Conditional extension results may be transported only through typed slots.
Admission follows the absent edge and refuses a load until the slot is replaced;
shared joins are allowed when their incoming absent paths have replaced it. At
execution, cells distinguish never initialized, present and conditionally absent.
Storing absence never reads its payload. Loads reject both unavailable states,
and the branch checks that actual presence agrees with the declared Boolean.
A manifest does not make a false runtime result or guard acceptable.

Without an explicit call map, local calls retain the present-prefix/omitted-tail
profile. Complete maps support middle omissions and independently supplied later
arguments. Argument validation precedes child-frame entry.

## Values, leases and frame lifetime

`value(type,bits,?payload)` carries exact binary encoding and an optional strongly
owned immutable payload. Type services materialize/check values and report their
logical size. A payload may wrap an existing language-owned value object; the
runner adds no second value, numeric or pool algorithm.

`registry.lease(typeName)` marks an opaque binding type. On the original ready
session, `bind(typeName,bindingObject)` mints a value tied to its private storage
anchor and invocation identity. Constants, handler results, control carriers and
function returns cannot manufacture or export leases. `request.resolve(ordinal)`
returns the exact retained binding only during the matching synchronous dispatch
epoch. Foreign, revoked, copied-owner and retained-request resolution fails.
The language adapter owns validation and access to its binding's referenced
resources.

A handler may also implement `llirexecute.lifecycle` and be registered with
`registry.lifecycle(handlerName)`. It receives `frameenter(functionId,depth)` and
`frameleave(functionId,depth,reason)`. Leaving a frame is marked before callback
entry, so a cleanup fault cannot notify it twice. Every entered invocation owns
its frame-local resources until return or unwind; a language's longer-lived
invocation resources remain with its separate owner.

`core.protect` protects the remainder of its declaring invocation. A handler
failure unwinds crossed child frames and enters that graph's exceptional
successor. `request.hasfault()`/`fault()` expose the retained owned outcome there.
An omitted outcome origin is filled from the failing dispatch before exposure.
Protection is disabled before entry, so a failure in its exceptional path cannot
catch itself. Handler inputs still obey the setup-based definite-assignment
contract; later normal-path stores do not establish exceptional-entry values.

## Boundary results, events and resumption

`session(plan,fuel=100000,frames=64,values=100000,bytes=67108864)` owns its plan,
handler state and retained frame values. `start(functionId,values,present)` accepts
explicit argument presence, then runs to a boundary. `run()` returns the retained
boundary result; it does not service a suspended invocation.

Results distinguish `done`, `failure`, `transferred`, `suspended`, `limit`,
`cancelled`, `discarded` and `rejected`. Ordinary result cells retain presence
independently of their bytes. Failure/control packets retain code, message,
exact binary payload and supplied logical origin. Runtime suspension/transfer
boundaries also expose `operation()` for the dispatched site. A terminal failure
publishes no partial successful result. Generic failure/limit diagnostics retain
the currently attempted operation and origin when known. A fuel stop identifies
the next operation before dispatch; initial admission failures retain unknown zero.

`outcome.setevent(eventObject,bytes=64)` retains an opaque owned capsule;
`hasevent()` and `event()` preserve its presence separately from the object field.
The registry's default event service admits nominal `rxfnsb.runtime_signal`
capsules and charges 64 logical handle bytes. It preserves the original native
payload/module/address rather than reducing the capsule to strings.
`registry.events(service)` may install another pure `eventhandler` with
`check(event)` and `bytes(event)`; its charge must match the outcome. The capsule's
external payload owner retains its separate resource contract.

`outcome.addcontrol(value,present=1)` supplies ordinary typed boundary carriers,
separate from operation results. Outcomes and results expose `controls()`,
`controlat(i)` and `controlpresent(i)`. The current control profile requires
present, type-checked non-lease values; it charges their cells and payloads.
A normal outcome contains no control code/message/payload, event or control
carriers. Transfer and suspension contain no ordinary operation results.

Suspension returns a token for the original session storage and generation.
Keep that session alive: tokens validate authority without extending its
lifetime. `resume(token,outcome)` validates completion before consuming the token
and advances once. Malformed completion leaves the valid suspension available
for correction. `discard(token)` releases it; `cancel()` also works while
suspended. Stale, foreign and consumed tokens are rejected without changing the
valid session. Copying an active session does not copy execution authority; a
ready unbound session may be copied before independent starts.

`abort(failureOutcome)` accepts an owned primary failure from an interrupted
running or suspended dispatch. It clears busy/dispatch state and revokes
continuations before cleanup, including when a native event skipped a callback's
normal epilogue. Terminal abort is idempotent. Each retained handler receives
one terminal `cleanup(reason)` notification; cleanup continues after another
callback raises and preserves an existing primary failure. Successful completion
is published only after cleanup. Handler inspection returns a value copy.

Native capture explicitly covers FAILURE, ERROR, OVERFLOW_UNDERFLOW,
DIVISION_BY_ZERO, CONVERSION_ERROR, INVALID_ARGUMENTS, OUT_OF_RANGE, UNICODE_ERROR,
REFERENCE_INVALID, OBJECT_NOT_INITIALIZED, RXBIN_CORRUPTION, UNKNOWN_INSTRUCTION,
FUNCTION_NOT_FOUND, NOT_IMPLEMENTED, INVALID_SIGNAL_CODE, NOTREADY, CANCEL, QUIT,
TERM, POSIX_INT, CHANNEL_ERROR, TASK_FAILURE and OTHER. It does not install
handlers for POSIX_HUP, POSIX_USR1, POSIX_USR2 or POSIX_CHLD, preserving their VM
ignore defaults.
Adapters must convert their additional registered VM events to owned failure
outcomes before leaving dispatch. Lifecycle callbacks must stay within the
captured fault set; type services remain pure. A host abort supplies an already
owned outcome rather than raising a language-specific event through the runner.
The C adapter catches its Classic events in dispatch; its current frame hooks
only manage storage and perform no Classic semantic or host-callback work.

## Bounds and integration limits

Preflight bounds each analysis stage's work and logical resident/temporary
storage. Execution bounds fuel, call depth, cells and logical bytes. Frame and
handler-output reservations precede effects. Completion checks types, arity,
presence, origin and bytes before advancing or publishing. The same output
reservation covers control text, payload, owned event handles and control values;
exceeding it is a protocol failure. A supplied operation origin is zero (use the
dispatched origin) or that operation's exact origin.

Frames conservatively retain storage. A session still copies its prepared plan
and B value state; preparation reuse does not remove that cost. Bounds describe
logical ownership, not process RSS. Fuel/cancellation operate between dispatches
and cannot interrupt an uncooperative callback. Public calls are synchronous;
ordinary same-session reentry is rejected. Handler-owned external resources and
aggregate ownership across several sessions need their host's separate ledger.

The independent neutral fixtures qualify this component profile. They do not
establish the complete Classic instruction inventory, a live compiled caller's
pool/labels/conditions, checked VM call windows, RexxScript migration or installed,
platform or production acceptance. Classic integration belongs to the
[dynamic execution architecture](../../research/level-l/compilers/doc/architecture.md#dynamic-execution-strategy)
and its [conformance and known limitations](../../research/level-l/compilers/doc/levelc-conformance.md).
Source frontends retain lexical/prepared-binding definition snapshots and reuse
their own calling-contract owner; these records are neither generic L leases nor
permission to load host code from saved IR.

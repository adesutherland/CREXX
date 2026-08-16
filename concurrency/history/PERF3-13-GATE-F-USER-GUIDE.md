# Gate F concurrency in cREXX: approved user model and Rexx surface

Date: 2026-08-15

Status: **user model approved; F0-S through F1g-D and Mac closeout QA are
complete; initial publication remains gated by portable conformance**

This document is the approved user-oriented source of truth for Gate F
concurrency. It explains the terms, the conceptual machine, the Rexx source
surface and the relationship between the simple syntax and the Level B class
library. It is deliberately written for a Rexx programmer who wants to get
useful parallel work done without first learning VM or operating-system thread
internals.

The approved ownership, transfer and lifecycle constraints remain in
[`PERF3-13-GATE-F-DESIGN.md`](PERF3-13-GATE-F-DESIGN.md). This document records
the approved F0 source additions discussed after that record was written: task
declarations, transparently scheduled task expressions, `DO PARALLEL`,
`.taskwork` classes and controller-side projection of failed completions.
Adrian approved this user model and authorized staged implementation on
2026-08-14. The exact compile-checked declarations and machine contracts are
now locked in
[`PERF3-13-GATE-F-AI-SPEC.md`](PERF3-13-GATE-F-AI-SPEC.md).

This document is the authority for user-visible Gate F semantics. The AI
reference specification defines exact grammar, validation, lowering, Level B
declarations, state machines, diagnostics and conformance tests without
changing this user model. Neither the specification nor implementation may
silently reopen an approved language decision. The staged execution and
first-verdict stops are recorded in
[`PERF3-13-GATE-F-IMPLEMENTATION-PLAN.md`](PERF3-13-GATE-F-IMPLEMENTATION-PLAN.md).

Examples using `task` declarations or `DO PARALLEL` compile when the source
selects `OPTIONS LEVELG`. The five low-level RXAS instructions, complete local
and isolated-process providers, explicit Level B classes and Level G lowering
exist in both concrete VMs. F1g-A adds typed object results; F1g-B exercises
that surface through a bounded transferable HTTP proxy and reusable
single-owner connections; F1g-C adds safe headers, policy, verified TLS and
explicit redirect/retry/ambiguity rules; F1g-D adds bounded request/response
streams, gzip/deflate decoding and the concurrent `crexx-rag` fixture.

## The idea in one page

cREXX has two kinds of callable work:

- an ordinary procedure or method runs synchronously in the current execution;
- a task is independent work that cREXX may run in another execution.

A task is declared much like a procedure:

```rexx
square: task = .int
  arg number = .int
  return number * number
```

It is called like an ordinary Rexx function:

```rexx
answer = square(9)
```

The call is submitted to a task pool. Because this statement needs the result
immediately, cREXX waits for it before assigning `answer`.

Independent task calls in the same expression may overlap:

```rexx
answer = square(9) + cube(4)
```

cREXX starts `square(9)` and `cube(4)` when their arguments are ready, waits
for both results, and then performs the ordinary addition. The source values
remain `.int`; the user does not need future or promise types.

`DO PARALLEL` extends the structured task lifetime across several statements:

```rexx
do parallel
  left = square(9)
  say "the controller can continue"
  right = cube(4)
end

answer = left + right
```

Inside this block:

- task calls start child work;
- ordinary code runs sequentially in the controlling execution;
- reading a task result waits for that result if necessary; and
- `END` waits for every remaining child task.

Tasks do not detach. The program cannot leave the expression or parallel block
while its ordinary child tasks are still unaccounted for.

The simple syntax is a typed layer over the same explicit class model used by
systems programmers:

```text
task call in Rexx source
        -> TaskTarget plus ChannelValue arguments
        -> TaskScope submission
        -> bounded TaskPool/provider
        -> receiver-owned task execution
        -> one terminal Completion
        -> typed Rexx result or controller-side task failure
```

## Terms

The public terms describe logical behavior. They do not expose OS thread IDs or
worker numbers.

| Term | Meaning for a Rexx programmer |
| --- | --- |
| concurrency | more than one operation is in progress during the same period |
| parallelism | two or more operations actually execute at the same time when capacity is available |
| controlling execution | the execution currently evaluating the ordinary Rexx program and coordinating its child tasks; this is often the top-level VM thread, but the contract does not depend on one OS thread |
| execution or isolate | one independently mutable cREXX execution owning its globals, frames, registers, references and runtime state |
| task | one independently schedulable invocation with transferable arguments and exactly one terminal completion |
| task body | the code executed for a task invocation |
| task work | the fixed Level B receiver-side class contract used to implement advanced or generated task bodies |
| task target | a sealed descriptor identifying statically resolved receiver-side task work without a procedure-name string or raw pointer |
| task pool | bounded execution capacity and admission policy for independent work |
| task scope | the lifetime, failure, cancellation and deadline owner for a group of child tasks |
| child task | a task submitted through a particular scope and therefore unable to outlive it |
| ready task | a task whose argument values and task dependencies are available |
| task result | the normal typed value produced by a successful task |
| completion | the explicit Level B record of one terminal outcome, including success, failure, cancellation, deadline or rejection |
| channel | a bounded provider-neutral request/completion relationship used below tasks, services and I/O libraries |
| channel value | a portable value tree copied, moved or materialized into receiver-owned storage |
| byte endpoint | a bounded byte source, sink or duplex pair used for redirects, process I/O, streaming and provider adapters |
| service | long-lived, single-owner mutable state addressed by logical identity |
| service reference | a transferable handle to a service identity, not a shared pointer to its state |
| transfer buffer | a binary value with explicit mutable ownership, move and immutable seal states |
| provider | the implementation that supplies local threads, processes, remote hosts or an I/O facility while preserving the same logical contract |

### Pool and scope are different

A pool answers **where and within what capacity can work run?** A scope answers
**which work belongs together, when must it finish, and what happens when one
child fails?**

Several short-lived scopes may use one pool. Closing a scope waits for its
children; closing a pool shuts down its execution capacity.

A pool is an object created by a factory. It is not itself a factory. The
approved Level B factory model makes provider selection visible:

```rexx
pool = .taskpool.local(4, 64)
scope = .taskscope.failfast(pool, 5000)
```

Here `4` is requested parallel capacity, `64` is bounded admission capacity,
and `5000` is the scope timeout in milliseconds. Exact argument types and
the `-1`/`0` timeout forms are fixed in the F0-S AI specification.

## The conceptual machine

### Executions own mutable VM state

Each cREXX execution owns its module globals, frames, registers, references,
ordinary objects and mutable runtime overlays. A task may run in another
execution, so it cannot see or modify the caller's live variables by alias.

Task arguments cross the boundary as `ChannelValue` data. The receiving
execution creates its own values and objects from that data. Results return by
the same rule.

This is why task code is safer than sharing ordinary Rexx objects between OS
threads: the ownership boundary is part of the language contract.

### Task calls create dependencies, not visible futures

The compiler treats task calls as nodes in a structured expression plan. A
task can start when all its inputs are ready. An operation that consumes the
task result waits until that result is terminal.

For example:

```rexx
answer = third(first()) + second()
```

when `first`, `second` and `third` are tasks, means:

1. submit `first()` and `second()` when their arguments are ready;
2. wait for `first()` before submitting `third(first-result)`;
3. allow `second()` to continue independently;
4. wait for `third()` and `second()`; and
5. add their ordinary typed results.

This is a dependency graph internally, but it is still a normal Rexx
expression to its author.

### What remains left-to-right

cREXX preserves the following caller-side order:

- ordinary argument expressions are evaluated in normal left-to-right order;
- task submissions occur in source evaluation order when their inputs become
  ready;
- ordinary procedures, methods, operators and instructions are evaluated in
  their normal language order; and
- the final operator is applied to materialized results using the normal type
  and signal rules.

Task bodies are different: declaring work as a task is the programmer's
explicit statement that independent invocations may overlap and may complete
in either order. Observable effects performed through services, files, HTTP or
other providers can therefore interleave unless those facilities provide a
stronger ordering guarantee.

Short-circuit behavior is preserved. cREXX must not start a task call from a
right-hand branch that the ordinary expression would not evaluate.

### The controller continues ordinary work

`DO PARALLEL` does not send every clause to the pool. The controlling execution
walks the block normally. Only declared task calls and explicit Level B
submissions create child work.

```rexx
do parallel
  call writer1                /* task: submit child work */
  local = prepareIndex()      /* ordinary function: run here and wait */
  say "preparation complete" /* ordinary instruction: run here */
  call writer2                /* task: submit child work */
end                           /* join writer1 and writer2 */
```

“Main thread” is a useful intuition for top-level code. “Controlling
execution” is the precise term because the semantics must also work with a
process or remote provider and must not promise one particular OS thread.

### Structured lifetime

Every task belongs to a scope:

- a task expression outside `DO PARALLEL` uses an implicit expression scope;
- `DO PARALLEL` opens a scope covering its block; and
- advanced Level B code creates and closes scopes explicitly.

A scope cannot finish until every child is terminal. On an early `RETURN`,
`LEAVE`, `EXIT`, signal or ordinary controller-side failure, cREXX requests
cancellation of unfinished children and performs the mandatory join before
control escapes.

Ordinary tasks cannot be detached. Work intended to live independently of one
request or block is a service, not an abandoned task.

### Capacity affects speed, not correctness

A correct program cannot depend on a particular worker number or on two tasks
actually overlapping. With capacity one, the same program remains correct but
runs its ready tasks one at a time. Greater capacity may allow real parallel
execution.

All queues are bounded. Saturation follows the selected scope deadline and
provider policy; it never silently creates an unbounded queue.

## Surface layering

Gate F has one semantic model presented at different levels:

| Layer | Authored surface | Purpose |
| --- | --- | --- |
| Level G | `name: task`, task methods, normal task calls and `DO PARALLEL` | approachable application concurrency with typed results and structured automatic joins |
| Level B | `.taskpool`, `.taskscope`, `.task`, `.tasktarget`, `.taskwork`, `.taskcontext`, `.completion`, `.channel`, `.channelrequest`, `.channelvalue`, `.channelcodec`, `.byteendpoint`, `.serviceref` and `.transferbuffer`, plus a statically resolved task-target construct | complete explicit control and the implementation foundation for Level G |
| RXAS/RXBIN | mandatory transport-neutral channel open/start/wait/cancel/close operations | the only Level B-to-RXVM bridge for core multithreading |
| RXVM provider backend | internal channel state, bounded admission, wait/wakeup, materialization, cancellation and terminal publication | implements the RXAS contract over local threads, processes or later host providers without exposing C pointers or RXPA payloads to Rexx |

`task` declarations and `DO PARALLEL` are approved Level G syntax. Level B
programs use the explicit classes and handles; their library implementations
reach core RXVM multithreading through authored Level B `ASSEMBLER` statements.
The statically resolved task-target construct is available to the Level B
foundation because explicit submission must not use procedure-name strings.

Adrian corrected and selected this RXAS-only core bridge on 2026-08-14. It
supersedes the earlier RXPA-first staging proposal. F0-S fixes the exact
instruction signatures and the compile-checked Level B declaration contract.
Adrian separately approved the staged implementation recorded in the
implementation plan.

## Approved Level G Rexx source surface

The constructs in this section are enabled only by `OPTIONS LEVELG` (or an
equivalent Level G compiler selection): task declarations, explicit
`task target` expressions, and statement or expression `DO PARALLEL`.
`task` and `parallel` remain contextual words, so their ordinary classic REXX
uses are not reserved at other language levels.

### Declaring a task procedure

At module level, `task` is a procedure-like callable kind:

```rexx
checksum: task = .string
  arg document = .binary
  return calculateChecksum(document)
```

The declaration supplies both:

- a typed callable surface for normal Rexx source; and
- a statically resolved task target for explicit Level B submission.

A task body may use local variables and construct local objects normally. It
must not accept exposed/reference arguments, capture caller locals, transfer a
live object graph or depend on persistent worker-local module globals.

Direct recursion remains ordinary Rexx recursion: a task body that calls its
own name continues synchronously in the same worker. Calling a different task
from a task body is not silently converted into an ordinary call; it remains a
forbidden nested task wait for Gate F. Mutual recursion between separately
declared tasks is therefore also rejected in this first surface.

### Task methods on transferable proxies

Inside a class, the same callable spelling denotes a task method:

```rexx
httpclient: class
  from_channel: factory
    arg encoded = .channelvalue
  to_channel: method = .channelvalue
  get: task = .httpresponse
    arg path = .string
```

A task method receiver must be explicitly transferable. It is normally an
immutable typed proxy containing a logical provider or service reference, not an
ordinary mutable object whose attributes are copied behind the user's back.
The exact transfer contract uses a `to_channel()` method plus a statically
resolved `from_channel` factory. Standard generated service/HTTP proxies supply
that pair. The compiler rejects a task method call when the receiver has no
such exact contract.

The same contract makes a typed object safe as a task argument or result. For
an argument, the controller calls `to_channel()` and the worker calls the
formal class's `from_channel` factory before entering the task body. The
special `.channelvalue` type is already the canonical transfer value and does
not need another wrapper. For a result, the worker calls `to_channel()` on the
returned object and publishes only the canonical
`.channelvalue`. The controller then calls the result class's statically
resolved `from_channel` factory and receives a new independent object. Object
identity, references and mutable VM storage do not cross the worker boundary.

For example, the approved HTTP task method may return `.httpresponse` because
that class supplies the transfer pair. The response seen by the caller is a
controller-owned reconstruction, not the worker's original object:

```rexx
response = client.post("/v1/generate", body)
say response.status()
```

If the declared object result class omits either exact member, compilation
fails with `#TASK_NONTRANSFERABLE_TYPE`. This check also applies when the task
class is imported from a separately compiled library. Importing or calling a
task method does not weaken the `OPTIONS LEVELG` gate.

The explicit `.taskscope.submit(target, request)` form likewise passes the
given `.channelvalue` as the taskwork request itself. A `.taskwork.run` method
therefore sees the same canonical value the controller submitted, not a
library-added record around it.

An ordinary method remains synchronous:

```rexx
status = response.status()   /* ordinary method, evaluated by controller */
```

### Calling tasks in expressions

No special call operator is needed:

```rexx
total = task1() + task2()
```

Both calls use the active task scope. The compiler keeps their hidden task
handles until their values are required. `total` receives the ordinary result
type of the addition.

A single task call still uses the task machinery:

```rexx
result = task1()
```

Outside a larger parallel scope, the implicit expression scope joins before
the statement finishes.

Ordinary routines are not automatically moved to the pool:

```rexx
result = task1() + ordinaryFunction()
```

`task1()` may execute while the controller evaluates `ordinaryFunction()`, but
`ordinaryFunction()` itself runs synchronously in the controlling execution.

### `CALL` and result-free work

`CALL` still means invoke a callable and discard its returned value:

```rexx
call writer1 output
```

If `writer1` is a task, the invocation is scheduled. Outside `DO PARALLEL`, the
implicit statement scope joins before the next statement. Consequently these
calls remain sequential at statement level:

```rexx
call writer1
call writer2
```

To overlap result-free work, put the calls in one parallel scope:

```rexx
do parallel
  call writer1
  call writer2
end
```

Both tasks are joined at `END`; neither is detached.

### `DO PARALLEL`

The simplest form uses an execution-local default pool and a fail-fast child
scope:

```rexx
do parallel
  left = task1()
  right = task2()
end
```

Assignment of a task result inside the block does not force an immediate wait.
The compiler retains a hidden completion and the variable keeps its declared
result type. Reading the variable materializes it and waits if necessary:

```rexx
do parallel
  left = task1()
  right = task2()
  say left                  /* waits for left; right may continue */
end                         /* waits for right if still running */
```

The first surface should reject reference-taking, exposed-argument passing,
indexed mutation or reassignment of a result binding while that binding is
still pending. Reading it first materializes the ordinary value, after which
normal operations are permitted.

An explicitly configured scope is supplied with `USING`:

```rexx
pool = .taskpool.local(8, 128)
scope = .taskscope.collectall(pool, 10000)

do parallel using scope
  first = task1()
  second = task2()
end

call pool.close()
```

`DO PARALLEL USING scope` owns and closes the supplied scope at `END`; the
scope must not be reused. It does not close the pool, which may serve later
scopes.

### Parallel block expressions

cREXX already has expression-form `DO ... END` with `LEAVE WITH`. The approved
parallel modifier composes with that model:

```rexx
answer = do parallel using scope
           left = task1()
           right = task2()
           leave with left + right
         end
```

`LEAVE WITH` materializes the values used by its expression. The block still
joins every other child before returning its result.

### Obtaining a task target

The approved unary target form is:

```rexx
target = task checksum
handle = scope.submit(target, request)
```

`task checksum` is compile-time checked and produces a sealed `.tasktarget`.
It is not a string containing `"checksum"`, a native procedure pointer or a
request for a numbered worker.

### Advanced task-work classes

`.taskwork` is the receiver-side runnable contract. A separate `.runnable`
abstraction is unnecessary.

Conceptually its fixed Level B contract is:

```rexx
taskwork: interface
  run: method = .channelvalue
    arg request = .channelvalue, context = .taskcontext
```

An implementation is instantiated by its selected factory in the receiving
execution:

```rexx
imagework: class implements .taskwork
  _mode = .string

  *: factory
    arg mode = .string
    _mode = mode
    return

  run: method = .channelvalue
    arg request = .channelvalue, context = .taskcontext
    /* Decode request, perform work, encode result. */
```

The approved statically resolved target form is:

```rexx
target = task .imagework("thumbnail")
```

Within the `task` target operator, this describes receiver-side factory
construction. It does not construct an `.imagework` object in the caller and
copy that live object to a worker. Factory arguments must themselves be
transferable.

A typed `name: task = type` declaration is conceptually lowered to a generated
`.taskwork` adapter with the same target, transfer and completion rules. The
generated class is an implementation detail rather than a second task model.

## Level B control surface

The approved working vocabulary is:

| Interface/class | Responsibility |
| --- | --- |
| `.taskpool` | bounded provider capacity, admission, diagnostics and shutdown |
| `.taskscope` | child ownership, policy, deadline, submit/ask, cancel, join and close |
| `.task` | explicit handle for one submitted child |
| `.tasktarget` | sealed statically resolved receiver-side work descriptor |
| `.taskwork` | fixed receiver-side runnable class contract |
| `.taskcontext` | receiver-side deadline, cancellation and tracing context without caller storage |
| `.completion` | one terminal state, result/error and diagnostics |
| `.channel` | provider-neutral bounded request/completion transport |
| `.channelrequest` | explicit advanced wrapper around one provider ticket; task scopes expose `.task` instead |
| `.byteendpoint` | reusable bounded byte source/sink for redirects, streaming and child I/O |
| `.channelvalue` | portable value-tree construction and inspection |
| `.channelcodec` | versioned domain-value encode/decode contract |
| `.serviceref` | transferable logical identity for single-owner mutable state |
| `.transferbuffer` | explicit binary copy, move, seal and immutable-view lifecycle |

### Level B operation roles

The class contract must expose the complete low-level lifecycle even though
ordinary task syntax hides it:

| Owner | Required operation roles |
| --- | --- |
| `.taskpool` | create/select provider, inspect bounded capacity and saturation, stop admission, close |
| `.taskscope` | `submit`, `ask`, observe next completion, deterministic `join`, `cancel`, `close` |
| `.task` | identify one child, request cancellation, wait subject to the parent-only rule, inspect completion |
| `.completion` | inspect terminal state, typed error/diagnostics and optional `ChannelValue` result |
| `.channel` | nonblocking/deadline-aware `start`, `wait`, `cancel`, `close` over a provider-neutral ticket |
| `.channelrequest` | wrap one local ticket for advanced channel users without exposing the integer directly |
| `.byteendpoint` | open/reopen a logical byte reference and perform bounded read/write/half-close operations |
| `.taskcontext` | inspect deadline budget, cancellation request and trace identity without blocking or reaching caller storage |

`TaskScope.submit` is the structured task operation: it creates a child owned
by the scope and returns a `.task` handle. `Channel.start` is the lower provider
operation used by task, service, HTTP and other libraries and returns a
`.channelrequest`. Keeping those roles distinct prevents a raw integer ticket
from bypassing scope ownership.

Named factories should be preferred where they express a genuine semantic
choice and avoid unsupported overloads:

```rexx
localPool = .taskpool.local(4, 64)
processPool = .taskpool.process(4, 64)

fastScope = .taskscope.failfast(localPool, 5000)
allScope = .taskscope.collectall(localPool, 5000)
```

`processPool` changes isolation, not the task model. The first number is the
bounded count of warm worker processes and the second is the total admitted
running-plus-queued work. Every submitted task still receives a fresh cREXX
execution even when its worker process is reused. Cancellation and deadlines
are cooperative first; if work cannot stop within the provider grace period,
only its isolated worker process may be terminated and replaced. A crash known
to occur before task execution is reported as transport loss; a crash after
execution starts is reported as an unknown outcome rather than pretending the
task did not run.

The explicit lifecycle remains available:

```rexx
pool = .taskpool.local(4, 64)
scope = .taskscope.failfast(pool, 5000)

left = scope.submit(task scanner, .channelvalue.string_value("north"))
right = scope.submit(task scanner, .channelvalue.string_value("south"))

completions = scope.join()
call scope.close()
call pool.close()
```

An individual wait remains possible when the controller needs one child before
the rest:

```rexx
handle = scope.submit(task scanner, request)
completion = handle.wait(5000)

if completion.succeeded() then
  value = completion.value()
```

`scope.next(timeout)` observes whichever child becomes terminal next, while
`scope.join()` always returns the complete result set in stable submission
order. Observing a child through `next()` or `wait()` does not remove it from
the final join set. A finite observation timeout returns a `.completion` with
`available() = 0`; it is not a terminal child and never enters the join set.

The compiling declarations are fixed by the F0-S specification and exercised
by the `gate_f_levelb_contract` compiler/assembler/disassembler test.

### How the Level B classes reach the runtime

Core multithreading is an RXVM capability. Its Level B classes therefore reach
the runtime only through transport-neutral RXAS instructions:

```text
Level G task call or DO PARALLEL
        -> Level B TaskScope/Task/Channel contract
        -> Level B ASSEMBLER channel operations
        -> RXVM Gate E executor/provider backend
```

There is no RXPA task-start/wait path and no hidden RXPA native-payload handle
inside `.taskpool`, `.taskscope`, `.task` or `.channel`. The VM necessarily
owns internal C channel/provider structures, but Rexx sees only a validated,
typed VM channel capability created and consumed by the RXAS contract. No C
pointer or plugin-native payload crosses that boundary.

F0 does not add angle-bracket task intrinsics. A public family such as
`<taskstart>`, `<taskwait>` and `<taskcancel>` would duplicate the class API.
Authored `ASSEMBLER` is already Level B-only, while authored Level G assembler
is rejected, so the class library is the deliberate systems boundary.

The mandatory conceptual instruction roles are:

```text
chanopen    status,channel,providerType,requiredCapabilities,configuration
chanstart   status,ticket,channel,envelope,waitMicroseconds
chanwait    status,completion,channel,waitMicroseconds
chancancel  status,channel,ticket,reason
chanclose   status,channel,mode
```

These are the exact F0-S semantic roles and operand order. The maintainer/AI
specification assigns opcodes `650..654`, runtime register types, effects,
signals, ownership, cleanup, feature/version gates and malformed-image
behavior.

`providerType` selects one implementation; `requiredCapabilities` is a
separate flags word describing behavior the caller requires. Core types cover
local tasks, isolated worker processes, open-host transport, bounded byte
endpoints and child processes. A registered extension range allows future
RXVM plugins to supply other channel technology without adding opcodes.
Unknown types or required flags fail open rather than silently falling back.

`.taskpool` factories use `chanopen`; `.taskscope.submit` and service/HTTP
adapters use `chanstart`; `.task`, `.taskscope` and `.channel` use `chanwait`,
`chancancel` and `chanclose` as appropriate. At RXAS, `-1` waits indefinitely,
`0` is nonblocking and a positive value is a relative wait in microseconds.
Level B presents milliseconds and converts them with overflow checks. Scope
deadlines are anchored once by the provider's monotonic clock when the scope
channel opens; `MTIME` is not a Gate F deadline clock.

Factory selection, task-target construction, envelope and codec work, scope
policy, HTTP connection-pool policy and typed diagnostics remain Level B/G
responsibilities around those instructions. They do not require RXPA and must
not become provider-specific opcode families such as `httpstart` or `dbquery`.

## Values that may cross

The simple typed syntax does not weaken the transfer rules.

| May cross | Does not cross |
| --- | --- |
| scalar boolean, integer, float, decimal, string and binary values | live RXVM `value` storage |
| ordered arrays and schema-tagged records of transferable values | references, exposed cells or aliases |
| explicitly encoded domain values with versioned codecs | arbitrary object identity or cyclic object graphs |
| service references and validated logical provider references | closures, stack frames or suspended continuations |
| copied, moved or sealed immutable transfer buffers | raw procedure/runtime/native pointers |

Repeated references in an encoded value tree become independent
receiver-owned values. Identity is preserved only by an explicitly
transferable identity such as `.serviceref`.

## Tasks are stateless; services own durable mutable state

A task target describes independent work. The runtime may schedule two
invocations on different executions, and task code must not infer affinity from
module globals.

When a program needs one durable mutable history, it uses a service:

- one logical service identity owns the state;
- calls to that identity are serialized in accepted order in the first
  surface;
- callers transfer a `.serviceref`, not the service object; and
- the provider may choose physical placement without exposing worker numbers.

Examples include a mutable account, job coordinator, cache catalogue or
session whose operations must observe one authoritative order.

A task or service handler must not synchronously join another task in the first
surface. The orchestrating controller owns blocking joins. The compiler should
reject a statically visible nested task wait; dynamic misuse must fail
immediately rather than block a bounded worker pool.

The one exception is direct self-recursion, which is an ordinary same-worker
procedure call and creates no child task or join.

## Completion, failure and cancellation

The explicit Level B terminal states remain:

- `SUCCEEDED` with an optional value;
- `FAILED` with stable error information;
- `CANCELLED`;
- `DEADLINE_EXCEEDED`;
- `REJECTED`, including bounded backpressure;
- `ENDPOINT_CLOSED`;
- `TRANSPORT_LOST`; and
- `UNKNOWN_OUTCOME` for ambiguous remote execution.

Explicit Level B users inspect these states through `.completion`. The typed
task-call sugar needs an equally explicit projection because a failed task has
no value of its declared result type.

The approved F0 rule is:

- successful completion materializes the declared result;
- a non-success terminal outcome raises controller-side `TASK_FAILURE` when
  the result is demanded or the containing scope closes;
- the signal object carries the task identity, terminal state, stable error,
  diagnostics and the complete stable completion set when a group is being
  reported; and
- fail-fast requests cancellation of siblings but still joins all of them
  before the signal escapes.

This is a controller-side projection of an already terminal completion. It
does not expose the private cancellation interrupt to the task body and does
not add `async`/`await` or suspended frames.

Applications that need to treat cancellation, rejection or partial group
failure as ordinary data use the explicit `.task`/`.completion` Level B API
rather than the typed must-succeed sugar.

Cancellation is cooperative. Hard termination is process-provider-only. An
in-process provider quarantines work that cannot return safely rather than
terminating an OS thread that may own VM, plugin or host locks.

## Default pool and explicit control

For normal users, a task call must work without boilerplate. Each top-level
cREXX execution therefore has a lazily created, bounded local default pool and
a default fail-fast scope policy. Its exact capacity is a documented runtime
selection, not a semantic guarantee, and is visible through diagnostics.

Programs requiring fixed capacity, queue size, deadline, failure policy,
process isolation or remote execution create a pool and scope explicitly and
use `DO PARALLEL USING scope` or the Level B submission API.

The default pool is execution-local. It is closed during normal program
teardown only after all structured scopes have closed. It is not a mutable
process-global singleton shared invisibly by unrelated cREXX executions.

## Spawn and reusable redirects

The behavior previously implemented by RXAS `spawn`, `redir2str`, `redir2arr`,
`str2redir`, `arr2redir` and `nullredir` is now folded into Gate F rather than
left as spawn-only plumbing. F1d retires those authored mnemonics in favor of
the consistent five-operation channel family. Their old numeric opcode slots
remain reserved, and stale RXBIN halts with `UNKNOWN_INSTRUCTION` rather than
changing meaning.

Their common building block is a bounded byte endpoint: a byte source, sink or
duplex pair with backpressure, deadline, cancellation, EOF, half-close and one
terminal outcome. Child stdin, stdout and stderr are three uses of that
endpoint, not its definition.

This gives the same foundation to:

- synchronous and task-wrapped child processes;
- HTTP request and response body streaming;
- file and socket adapters;
- process and host channel transports; and
- future registered RXVM channel providers.

Input values are snapshotted before asynchronous I/O begins. Output bytes live
in endpoint-owned storage until the controlling execution reads or drains
them. An I/O thread never retains or mutates a live Rexx string, array, object
or register.

Existing ADDRESS and process Rexx behavior remains available through the
updated certified compiler exit and Level B adapters. New Level B code gets
`.byteendpoint`, which can read, write, stream, cancel and close without
spawning a process. Provider type `4` implements those endpoints and provider
type `5` implements structured child execution over endpoint references.

The general endpoint is exposed through the same RXAS channel roles using the
core byte-endpoint provider type. A child-process provider accepts compatible
endpoint capability references for its standard streams. HTTP-, redirect- and
process-specific opcode families are not added.

## HTTP: the required real-world consumer

Gate F must support an industrial concurrent HTTP/TLS library, not only CPU
examples. The `crexx-rag` consumer currently records CRI-16: installed
`rxhttp` opens one synchronous connection per request, sends `Connection:
close`, buffers without a configured response ceiling, and provides no
streaming or cancellation. Its provider layer also lacks a concurrent request
scheduler and connection pool.

The Gate F HTTP design must distinguish two pools:

- the task pool supplies bounded cREXX execution capacity; and
- the HTTP connection pool owns reusable network connections, per-origin
  limits and protocol multiplexing.

One is not a substitute for the other.

The intended Level G experience is typed and ordinary:

```rexx
client = .httpclient.pooled("https://api.example", 8, 32, 1048576)
scope = .taskscope.failfast(.taskpool.local(4, 64), 30000)

do parallel using scope
  generation = client.post("/generate", generationBody, headers)
  embedding = client.post("/embed", embeddingBody, headers)
end

say generation.status()
say embedding.status()
```

F1g-C ships the approved `post(path, body, headers)` surface. `headers` is a
transferable `.httpheaders` value; it safely carries credentials, media type
and idempotency keys and is revalidated after transfer. The optional
`.httppolicy` on `httpclient.pooled` configures bounded phase/observation
budgets, headers, buffered request size and opt-in replay. The containing
`.taskscope` supplies the strict whole-task monotonic deadline.

`post` is a task method on a transferable HTTP proxy. The proxy carries a
provider/channel capability and immutable configuration; it is not the current
mutable `rxhttp` object with shared `lastBody`, status and socket state. Each
response is materialized as an independent typed `.httpresponse`.

F1g-D adds `.httpbody.fixed(contentLength, capacity, responseCapacity)` and
`.httpbody.chunked(capacity, responseCapacity)`. Producers call `write` and
`finish`; `client.post_stream(path, body, headers, responseCapacity)` consumes
the request endpoint and returns response metadata whose `body_reference()`
opens the bounded response stream. Buffered `post` accepts gzip and deflate
responses and rejects decoded output beyond the configured response ceiling.

The industrial HTTP contract must cover:

- reusable keep-alive connections and bounded per-origin pooling;
- bounded request admission and response buffering;
- TLS qualification and safe credential/header handling;
- content-length, chunked and compressed responses;
- streaming as an explicit capability rather than pretending every response
  is a string;
- request and response streaming through the reusable bounded byte-endpoint
  contract;
- cooperative cancellation and deadlines;
- backpressure and connection/protocol diagnostics;
- retry only when method/idempotency policy permits it;
- teardown with no orphan request, socket or worker; and
- evidence for concurrent requests, saturation, large responses, failures and
  both VM engines.

HTTP remains a library/provider surface over the common task and channel
contract. Gate F does not add HTTP-specific VM instructions.

## What Gate F deliberately does not expose

The first surface does not expose:

- OS thread creation or thread IDs;
- numeric worker affinity;
- shared mutable module globals;
- arbitrary cross-worker references or object graphs;
- locks, condition variables, atomics or memory fences in Rexx syntax;
- writable shared-memory mappings;
- detached ordinary tasks;
- automatic replay of ambiguous side-effecting work;
- transparent distributed transactions or object replication;
- `async`/`await` or suspended Rexx frames; or
- provider-specific opcode families.

## Approved F0 choices and remaining specification gate

Adrian accepted the following user-facing choices and separately authorized
staged implementation on 2026-08-14:

- [x] Level G `name: task = type` declares typed independently schedulable
      work; Level B exposes the complete explicit class/handle lifecycle.
- [x] The same spelling inside an interface/class declares a task method whose
      receiver must be explicitly transferable.
- [x] A typed object task result crosses as `ChannelValue` and is reconstructed
      as a new controller-owned object through the declared transfer pair.
- [x] Normal task-call syntax transparently schedules work; independent task
      calls in one expression may execute concurrently.
- [x] Caller-side argument evaluation and submission remain left-to-right;
      task completion order is unconstrained except by dependencies.
- [x] Short-circuit branches are not speculatively scheduled.
- [x] `DO PARALLEL` extends one structured scope across statements; ordinary
      code remains on the controlling execution.
- [x] Pending task-result bindings keep their declared type, join on demand,
      and are all resolved at `END`.
- [x] `DO PARALLEL USING scope` consumes/closes the scope but not its pool.
- [x] `task callable` and `task .workclass(factoryArguments)` are the approved
      statically resolved task-target forms.
- [x] `.taskwork` and `.taskcontext` join the existing Level B vocabulary; no
      separate `.runnable` interface is introduced.
- [x] Level B classes expose submit/ask/join/cancel/close and the common
      channel open/start/wait/cancel/close roles; raw tickets cannot escape
      scope ownership accidentally.
- [x] Core multithreading is exposed from Level B to RXVM only through
      mandatory transport-neutral RXAS channel instructions; there is no RXPA
      task path or hidden native-payload handle in the class implementation.
- [x] Level B classes wrap `chanopen`, `chanstart`, `chanwait`, `chancancel`
      and `chanclose`; no public angle-bracket task-intrinsic family is added.
- [x] `chanopen` separates a provider type code from required-capability flags;
      core and registered RXVM-plugin provider ranges share one contract.
- [x] F0-S locks the exact RXAS/RXBIN instruction signatures, effects, signals,
      ownership, cleanup and feature/version behavior before the first opcode
      edit.
- [x] A bounded execution-local default pool makes simple task calls useful
      without setup; explicit pools/scopes provide control.
- [x] Typed task sugar raises controller-side `TASK_FAILURE` for non-success;
      explicit Level B code receives completion states as data.
- [x] Tasks remain stateless work, durable mutable state remains a single-owner
      service, and blocking nested task waits remain invalid.
- [x] Existing spawn/redirect behavior is migrated to reusable byte-endpoint
      and child-process channel providers; retired opcode slots remain reserved.
- [x] The industrial HTTP/TLS library is a required Gate F consumer with
      connection reuse, bounds, cancellation, streaming capability and
      concurrent evidence.
- [x] User-model approval alone did not start implementation; Adrian separately
      authorized the staged implementation recorded in the implementation plan.

## Current implementation and publication boundary

The maintainer/AI reference specification now contains:

1. exact grammar and contextual-keyword treatment;
2. AST and symbol/type rules for task procedures, task methods, task targets
   and pending result bindings;
3. expression dependency, short-circuit, forcing and cleanup lowering;
4. compile-checked Level B interface, factory and method declarations;
5. `ChannelValue`, codec, envelope and completion schemas;
6. scope, pool, service, cancellation, deadline and shutdown state machines;
7. RXAS/RXBIN instruction contracts, provider type/capability codes and RXVM
   provider ownership boundaries;
8. reusable byte-endpoint, child-process and existing spawn/redirect migration
   contracts;
9. HTTP provider/library contract and CRI-16 acceptance evidence;
10. diagnostics and positive/negative conformance fixtures; and
11. a coherence matrix mapping every user rule in this document to the
    specification and the existing Gate F ownership design.

F0-S completed that specification and coherence matrix before the first opcode
edit. F1a-F1f now implement the RXAS/RXBIN contract, complete local and
isolated-process providers,
canonical `ChannelValue`, lifecycle, private provider conformance seam, the
explicit Level B class surface, reusable byte endpoints and structured child
processes, sealed task procedures/methods/factory targets, typed task
expressions and `DO PARALLEL`. A Rexx programmer can use
`.taskpool.local(...)`, `.taskpool.process(...)`,
`.taskscope.failfast(...)` or `.collectall(...)`, sealed task targets, tasks,
completions and channels today; the class implementation reaches RXVM only
through the five channel instructions. Every library-development path is an
opportunity and obligation to exercise `rxc`, `rxas`, `rxlink` and `rxvm`;
the imported task-method and `.taskwork` conformance tests run that complete
pipeline in optimized/unoptimized form on both concrete VMs.

The deliberately reserved portions fail explicitly. Service `ask` and pool
statistics report unsupported operation rather than returning plausible
placeholder data. `.taskcontext.endpoint` adapts a transferable endpoint
reference inside taskwork. F1f supplies kind-1 task
procedures, kind-2 transferable task methods and kind-3 `.taskwork` factory
targets through the same local/process classes and completion values. A
process pool keeps warm isolated worker processes, but creates a fresh cREXX
execution for every task, so globals and live VM state cannot spill from one
task to the next. F1g-A transfers concrete-class results as canonical RXCV and
reconstructs new controller-owned objects. F1g-B supplies the bounded
`.httpclient.pooled` proxy, long-lived connection-owner taskwork, canonical
fixed-size admission descriptors and independent `.httpresponse` values.
F1g-C adds `.httpheaders`, `.httppolicy`, verified TLS, explicit bounded
redirect/retry rules and retained ambiguity diagnostics. F1g-D adds explicit
fixed/chunked streaming, bounded gzip/deflate decoding and the concurrent
`crexx-rag` generation/embedding integration fixture. Local implementation
and Mac qualification are complete; initial publication still requires
the separately governed portable conformance evidence. Any
contradiction or new language decision still returns to Adrian.

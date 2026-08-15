# PERF3-13 Gate F public concurrency surface design

Date: 2026-08-13

Status: **user model and staged Gate F implementation approved by Adrian on
2026-08-14; F0-S through F1g-D implementation and Mac closeout QA complete;
experimental publication remains gated by portable conformance**

This record is the normative design authority for the PERF3-13 Gate F
public concurrency surface. It records the user model, ownership and transfer
semantics that Gate F implementations must preserve. Implementation follows
the contract-first slices and first-verdict stops in
[`PERF3-13-GATE-F-IMPLEMENTATION-PLAN.md`](PERF3-13-GATE-F-IMPLEMENTATION-PLAN.md).
Experimental publication and push remain separately controlled.

Gate E remains the mechanism foundation. Its private executor, physical
doorbells, fallback progress points, worker indices and current logical
integer/string request fixture are not public contracts. Gate F starts only at
the approval point recorded in `PERF3-13-WORKLIST.md`, after the Gate E worker
model and its scale/reclamation policy have been selected.

The exact compilable Level B declarations, method signatures and machine
contract are locked by
[`PERF3-13-GATE-F-AI-SPEC.md`](PERF3-13-GATE-F-AI-SPEC.md). Its declaration
oracle compiles under the current Level B grammar. A later implementation may
not change the decisions, lifecycle or user model below without explicit
design approval.

The approved user-facing extension of this baseline is collected in
[`PERF3-13-GATE-F-USER-GUIDE.md`](PERF3-13-GATE-F-USER-GUIDE.md). That F0
record covers typed task declarations, transparently scheduled task
expressions, `DO PARALLEL`, `.taskwork` classes, the Level B-to-runtime bridge
and the industrial HTTP consumer. Adrian approved the user model and staged
implementation on 2026-08-14.

Adrian selected the core bridge on 2026-08-14: Level B concurrency classes
must call mandatory transport-neutral RXAS instructions, which RXVM implements
over the Gate E executor/provider substrate. There is no RXPA task path and no
Rexx-visible hidden native-handle contract. F0-S fixes the five instruction
signatures, opcodes, effects, signals, feature gate and binary value contract.
F1a-F1e implement that instruction family, complete type `1` local
provider, full values/lifecycle, the explicit Rexx Level B class surface,
reusable type `4` byte endpoints and type `5` child processes in both concrete
VMs, plus type `2` isolated process tasks. F1f implements the approved Level G
syntax, F1g-A closes typed task results, and F1g-B/C provide bounded pooled HTTP
ownership, safe headers/policy, verified TLS and explicit replay diagnostics.

Use these current repository contracts when implementing the design:

- [`PERF3-13-WORKLIST.md`](PERF3-13-WORKLIST.md) for Gate E/F status, approval
  and evidence sequencing;
- [`PERF3-13-GATE-F-IMPLEMENTATION-PLAN.md`](PERF3-13-GATE-F-IMPLEMENTATION-PLAN.md)
  for implementation slices, provider codes and verdict stops;
- [`PERF3-13-GATE-F-AI-SPEC.md`](PERF3-13-GATE-F-AI-SPEC.md) for the exact
  source, Level B, RXAS/RXBIN, provider and conformance contract;
- [`PERF3-13-E5-NATIVE-DOORBELL-DESIGN.md`](PERF3-13-E5-NATIVE-DOORBELL-DESIGN.md)
  for the private physical delivery boundary;
- [`RXVM_INTERPRETER.md`](../docs/ai-context/RXVM_INTERPRETER.md) for current
  VM ownership, callable descriptors and provider behavior;
- [`CREXX_LEVELB_AUTHORING.md`](../docs/ai-context/CREXX_LEVELB_AUTHORING.md),
  [`classes_and_interfaces.md`](../docs/books/crexx_language_reference/classes_and_interfaces.md)
  and [`CREXX_LIBS.md`](../docs/ai-context/CREXX_LIBS.md) for the Level B
  source and object model; and
- [`RXAS_ASSEMBLER.md`](../docs/ai-context/RXAS_ASSEMBLER.md) for the mandatory
  instruction, effect, signal, optimizer and RXBIN contract.

## Decision summary

Gate F locks these decisions:

1. **Module globals are execution-local.** A module global belongs to one
   CREXX execution/isolate. It is not an implicitly shared process variable.
2. **One owner represents mutable global state.** A durable logically global
   object is a service/actor identity whose calls are serialized per identity.
3. **Only transferable values cross a boundary.** A versioned `ChannelValue`
   is materialized into receiver-owned VM storage; live VM storage never
   crosses workers.
4. **Concurrent work has structured lifetime.** A `TaskScope` owns its child
   tasks, cancellation and deterministic join. Ordinary tasks do not detach.
5. **Large binary sharing is immutable.** Gate F supports copied, moved,
   sealed shared/mapped and serialized binary transfer, but not general shared
   mutable Rexx memory.
6. **The class and instruction contracts form one core boundary.** Level B
   classes wrap mandatory transport-neutral RXAS instructions, Level G lowers
   to those classes, and provider-specific mechanisms remain private to RXVM.
7. **Replication policy stays explicit.** Event streams build eventually
   consistent projections; versioned stores and CRDTs are opt-in libraries,
   not transparent behavior of every Rexx object.
8. **Redirects are reusable endpoints.** Spawn, process I/O, HTTP streaming
   and provider adapters share bounded byte endpoints. Pre-release
   spawn/redirect RXAS spellings may be retired to keep one coherent channel
   instruction family; their old opcode slots remain reserved.

These are user-visible semantic decisions, not merely an implementation
preference. A future provider may use threads, processes, remote hosts or a
different scheduler while preserving them.

## Goals and non-goals

Gate F is intended to support:

- independent compute and I/O work;
- bounded local worker pools;
- isolated child-process workers;
- cross-host services using the same logical channel contract;
- long-lived stateful services/actors;
- cancellation, deadlines, backpressure and deterministic teardown;
- large immutable binary payloads; and
- higher-level HTTP, database, file, timer, process and event providers.

The first Gate F surface does not provide:

- shared mutable module globals;
- arbitrary cross-worker object graphs or references;
- OS-thread creation, thread IDs or numeric worker affinity;
- lock, condition-variable, atomic or memory-fence syntax;
- writable shared-memory mappings;
- transparent distributed transactions, STM or object replication;
- implicit retries of side-effecting remote work;
- `async`/`await` language syntax or suspended Rexx frames; or
- a separate VM opcode family for each provider.

Message passing is the default ownership boundary, not a claim that channels
are optimal for every algorithm. Fine-grained shared-array algorithms and
lock-free shared structures remain outside this surface until a measured
product use case justifies a separately approved low-level design.

## User model and terminology

Gate F exposes logical work and communication concepts. Physical worker and
transport details remain provider configuration and diagnostics.

| Concept | Public meaning | Required invariant |
| --- | --- | --- |
| execution/isolate | one independently mutable CREXX execution | owns its globals, frames, registers, references and runtime overlays |
| task pool | bounded capacity for independent work | exposes capacity/policy, never physical worker identity |
| task scope | lifetime owner for a group of child tasks | close returns only after every child is terminal |
| task | one submitted unit of work | has one handle and one terminal completion |
| task target | sealed descriptor for receiver-instantiated work | no raw procedure pointer and no user-authored procedure-name string |
| service reference | transferable logical identity for stateful work | calls to one identity are serialized in accepted order |
| channel | bounded bidirectional request/completion relationship | common semantics across thread, process and host providers |
| channel value | portable value tree | contains no live RXVM storage or native pointer |
| transfer buffer | binary value with explicit copy/move/seal state | mutable bytes have one owner; shared bytes are immutable |
| completion | terminal observation of one task/request | exactly one terminal state is published to its owner |

The F1 Level B vocabulary is `.taskpool`, `.taskscope`, `.task`, `.tasktarget`,
`.taskwork`, `.taskcontext`, `.completion`, `.channel`, `.channelrequest`,
`.channelvalue`, `.channelcodec`, `.byteendpoint`, `.serviceref` and
`.transferbuffer`. The declaration oracle compile-checks these names and their
method signatures before production implementation.

## Ownership and global state

### Execution-local module globals

Module globals, including variables exposed between procedures in one module,
remain local to one execution/isolate. A pool may schedule an ordinary task on
any compatible worker, so application code must not use module-global state to
infer which earlier task ran there.

The current Gate E executor happens to retain a worker's module globals across
requests. That is useful evidence for isolated state, but it does not create a
public promise that a task is bound to an OS thread or numbered worker.

### Stateful services/actors

A program that requires persistent mutable state uses a service identity:

- one logical identity owns one mutable state history;
- one call executes at a time for that identity in the first surface;
- accepted calls have an explicitly defined per-stream order;
- the provider may pin the identity to one worker internally;
- later providers may relocate an idle identity only at a quiescent boundary
  using an explicit snapshot/restore contract; and
- a `ServiceRef` transfers identity and routing capability, not the service's
  object graph.

Services are non-reentrant by default. A handler runs one turn to completion.
The first surface does not permit a task or service handler to synchronously
join another task/channel operation; blocking join belongs to the orchestrating
parent execution. An invalid nested wait fails immediately with
`#TASK_NESTED_WAIT`, without occupying a bounded worker while waiting for its
own pool. Nested task scopes or suspended actor turns require a later
resumption/scheduler design.

Stateless task targets and stateful services remain distinct. A stateless task
may be freely scheduled; a service call preserves the logical identity's state
and ordering. The API must not make users select a worker number to obtain
state affinity.

## Structured task lifetime

A `TaskScope` owns all tasks submitted through it. The first surface provides
at least these policies:

- **fail-fast** — the first non-success terminal result requests cancellation
  of remaining children and the scope still joins every child; and
- **collect-all** — all children run to a terminal result unless the scope or
  its deadline is cancelled.

The required lifecycle is:

1. create a bounded pool or select a provider;
2. open a scope with policy and deadline;
3. submit statically described task targets or ask service references;
4. receive task handles and completions;
5. cancel explicitly or when the scope policy/deadline requires it;
6. join all children; and
7. close the scope and then the pool/provider.

Ordinary tasks cannot outlive their scope. Long-lived independent work is a
service, not a detached task. Provider shutdown stops admission, resolves
queued work, requests cancellation of running work, quarantines any worker
that cannot return safely, publishes terminal outcomes and joins all
non-quarantined runtime resources deterministically.

The Level B surface uses explicit submission and join rather than adding
`async`/`await`. The approved F0 user model uses typed task calls whose
submission and structured join are compiler-managed without exposing future
types or suspending the controlling Rexx frame. That lowering must preserve
the same scope ownership and mandatory cleanup. True `async`/`await` would
still require a separately designed suspended-frame, resumption, signal and
structured-cleanup model.

## Task targets and procedure identity

The public surface must not reproduce the Gate E PoC's string procedure names
or process-local `proc_runtime *` values. A task target is a sealed, versioned
callable descriptor produced from a statically resolved Level B procedure,
method or task-work provider. It carries the callable contract and compatible
code/generation identity needed by the receiver.

The exact Level B spelling is `task callable-reference` or
`task class-factory-expression`. The compiler lowers it to the sealed
`.tasktarget` descriptor without spelling a qualified procedure name as data.
Dynamic lookup by name, where needed for tooling, is a separate checked API and
is not the normal task-submission path.

A stateful Rexx object instance is not itself a task target. Receiver-side work
is instantiated from its selected class/factory descriptor, or addressed
through a `ServiceRef`; caller object attributes are never silently copied.

## `ChannelValue` transfer contract

`ChannelValue` is a versioned logical value tree, independent of `value`,
register layout and host ABI. Each receiver materializes it into its own
worker-owned registers and objects.

The first schema supports:

- none/void;
- `.boolean`, `.int`, `.float`, `.decimal`, `.string` and `.binary` scalars;
- ordered arrays of `ChannelValue`;
- schema-tagged ordered records whose fields are `ChannelValue` values;
- immutable `TransferBuffer` payloads; and
- `ServiceRef` handles.

Domain objects require an explicitly registered, versioned codec. The codec
produces and consumes a schema-tagged `ChannelValue` record; it does not grant
permission to transfer the object's live storage. Unknown codec/schema
versions fail as a typed terminal outcome unless an explicitly negotiated
compatible version exists.

The first schema excludes:

- Rexx references, exposed argument cells and aliases;
- arbitrary object identity or cyclic object graphs;
- closures, stack frames and suspended continuations;
- internal `value *` and `proc_runtime *` pointers; and
- native/plugin payload pointers or handles without a provider-specific
  transferable capability contract.

Aliasing is not preserved when a value tree is encoded and materialized.
Repeated source references become independent receiver-owned values unless the
item is an explicitly transferable identity such as `ServiceRef`.

## Envelope and delivery contract

Every operation uses one versioned envelope. The logical fields are:

- protocol and envelope version;
- operation kind and capability set;
- task/correlation ID;
- endpoint or service identity;
- task-target/callable descriptor version where applicable;
- sender identity and monotonically increasing stream sequence;
- payload type/schema/codec identity;
- deadline budget;
- cancellation and idempotency metadata;
- trace ID and parent trace/task ID; and
- payload or chunk/stream descriptor.

The wire encoding, network transport and authentication scheme remain F0/F2
selection points. The logical fields and failure behavior do not.

### Ordering and execution

- FIFO ordering is guaranteed only within one sender-to-endpoint stream.
- No total order is implied across different senders or endpoints.
- Within one live session, the provider gives at-most-once execution: it never
  intentionally submits the same accepted correlation ID twice.
- A provider does not automatically replay a side-effecting operation after an
  ambiguous disconnect.
- An idempotency key lets a provider or service implement explicit duplicate
  suppression; it is not a general exactly-once guarantee.
- Exactly one terminal completion is observable for each task handle. This is
  an observation guarantee, not an exactly-once execution claim.

### Bounded queues and backpressure

All queues are bounded. Admission may wait up to a deadline, return immediately
when the deadline is zero, or produce `REJECTED`/`BACKPRESSURE`. Providers must
not grow unbounded queues to preserve an apparently simple API.

Capacity, queue depth, rejection, wait time and saturation must be observable
through diagnostics. Fairness policy is provider configuration but must not
break the ordering guarantee above.

### Terminal completion

The common terminal states are:

- `SUCCEEDED` with an optional `ChannelValue` result;
- `FAILED` with a stable error code, message and optional details;
- `CANCELLED`;
- `DEADLINE_EXCEEDED`;
- `REJECTED` with a stable reason such as `BACKPRESSURE`;
- `ENDPOINT_CLOSED`;
- `TRANSPORT_LOST`; and
- `UNKNOWN_OUTCOME` when remote execution may have occurred but cannot be
  established safely.

Expected scheduling and transport outcomes remain completion states in the
explicit Level B contract rather than being replaced by catchable Rexx
signals. The approved F0 user model requires typed must-succeed task-call
sugar to project a non-success terminal completion as controller-side
`TASK_FAILURE`; that rule preserves the completion as the underlying data
and is approved. Allocation failure, invalid local API use and existing
language/runtime faults retain their normal signal contracts.
A terminal completion describes the caller's task handle. For uncooperative
native or external work, it does not prove that an underlying side effect has
stopped; the provider must use `UNKNOWN_OUTCOME` or attach an explicit
may-continue diagnostic rather than claim successful cancellation.

### Cancellation, deadlines and kill

Cancellation is cooperative task control. A queued task can be cancelled
before admission to a worker. A running Rexx task receives the selected Gate E
interrupt mechanism and terminates at a safe progress point. A native/plugin
call can only be cancelled if its contract cooperates; otherwise its worker is
quarantined until it returns or the containing process is discarded.

`CANCEL` and strong `KILL` remain internal executor events in the first public
surface. They are not added to `SIGNAL ON` or exposed as task-handler signals.
Any future catchable cancellation/cleanup construct needs its own unwind,
resource-cleanup, retry and TRACE design.

Hard termination is available only from an isolated process provider. An
in-process provider must never terminate an OS thread that may hold VM, plugin
or host locks; its strong internal action stops reuse and quarantines the
worker instead.

The API accepts a local monotonic deadline or timeout. A cross-host envelope
carries the remaining budget rather than assuming synchronized wall clocks.
Expiry requests cancellation and eventually produces `DEADLINE_EXCEEDED`; it
does not promise that an uncooperative external side effect was reversed.

## Immutable binary transfer

`TransferBuffer` provides the optimized large-payload path without weakening
the value-ownership model. Its logical states are:

1. mutable and owned by one execution;
2. moved, invalidating mutation through the sender's former handle; or
3. sealed immutable and shareable.

A provider may implement the same semantics by:

- copying small payloads;
- transferring ownership of an independently allocated mutable buffer;
- reference-counting sealed bytes within one process;
- mapping a read-only shared segment between processes; or
- serializing bytes across hosts.

The copy/share/map threshold is selected by retained measurement and is never a
language guarantee. Slices/views retain the lifetime of their sealed parent.
Mutable memory is never simultaneously visible to two executions through this
surface. Writable mapping, atomics and memory fences require a separate
advanced design and measured use case.

## Consistency above channels

### Strong per-object state

Use one service owner when callers require a single authoritative mutable
history. Service persistence may expose versioned snapshots and conditional
update, for example `read -> (version, snapshot)` and
`compareExchange(expectedVersion, replacement)`. This is an explicit service
contract rather than transparent language-level STM.

### Event streams and projections

An event hub is a Level B/G library over channels. Topics, fan-out, retention,
replay, acknowledgement, consumer offsets and projection materialization stay
outside the VM primitive. A projection is eventually consistent unless its
service contract explicitly supplies a stronger boundary.

An event-backed global view must define per-key ordering, stable event IDs,
idempotent application, schema evolution, replay behavior and conflict policy.
An event hub alone is not a shared global object. Event-sourced read models
normally accept eventual projection lag and explicit rebuild/schema costs:
[Event Sourcing pattern](https://learn.microsoft.com/en-us/azure/architecture/patterns/event-sourcing).

### CRDT and MVCC/STM boundaries

CRDTs are explicit types such as a convergent counter or set, with a proved
merge rule and delivery assumptions. They are not automatic for arbitrary
objects. Versioned transactions require immutable snapshots, retry rules and
restrictions on side effects. Gate F does not introduce transparent VM-wide
STM because ordinary Level B objects are mutable and transaction retry would
otherwise repeat I/O and native effects. Convergent replicated values require
specific algebra and delivery assumptions, not merely a version field:
[CRDT technical report](https://pages.lip6.fr/Marc.Shapiro/papers/CRDTs-beatcs-2011-06.pdf).

## Level B and Level G surface

### Level B

Level B owns the stable systems contract. It uses normal interfaces, classes,
factories and methods. Those classes call the core RXVM concurrency facility
through Level B-authored RXAS instructions. A Rexx object may retain a
validated VM channel capability, but it does not expose an RXPA payload,
native pointer or physical worker identity. Native metadata declaration alone
does not substitute for correct Rexx object construction.

The working responsibility split is:

| Interface/class | Responsibility |
| --- | --- |
| `.taskpool` | bounded capacity, admission and shutdown |
| `.taskscope` | submit/ask, policy, deadline, cancel, join and close |
| `.task` | task identity and completion access |
| `.tasktarget` | sealed statically resolved work descriptor |
| `.taskwork` | fixed receiver-side runnable class contract |
| `.taskcontext` | receiver-side deadline, cancellation and tracing context |
| `.completion` | terminal state, value/error and diagnostics |
| `.channel` | provider-neutral bounded request/completion transport |
| `.channelrequest` | advanced Level B wrapper around one local provider ticket |
| `.byteendpoint` | bounded byte source/sink for redirect, streaming and child I/O |
| `.channelvalue` | portable sum/value-tree construction and inspection |
| `.serviceref` | transferable logical stateful endpoint identity |
| `.transferbuffer` | explicit mutable-owner, move and immutable-seal lifecycle |
| `.channelcodec` | versioned domain-object encode/decode contract |

Level B currently has no generics. A single `ChannelValue` and `Completion`
contract is therefore preferred over a family such as `FutureOfString` and
`FutureOfInteger`.

The working method roles are:

- `TaskPool` named factories select a provider and create bounded capacity over
  the generic channel-open role; the resulting pool owns channel shutdown;
- `TaskScope.submit(target, request)` starts stateless work and returns a
  `Task` handle;
- `TaskScope.ask(serviceRef, request)` starts one serialized service call and
  returns a `Task` handle;
- `TaskScope.next(timeoutMilliseconds)` returns the next terminal completion in completion
  order;
- `TaskScope.join()` waits for the scope and returns completions in stable
  submission order;
- `TaskScope.cancel(reason)` requests cancellation of its non-terminal
  children, while `close()` performs the mandatory join/cleanup;
- `Task.cancel(reason)` and `Task.wait(timeoutMilliseconds)` address one child, subject to
  the parent-only blocking rule;
- the advanced `Channel.start(value, timeoutMilliseconds)` returns a
  `.channelrequest`; its `wait(timeoutMilliseconds)`, `cancel(reason)` and the
  channel `close(mode)` roles expose the common provider protocol without a
  raw ticket or physical worker;
- `.byteendpoint` composes channel open/start/wait/cancel/close into
  bounded read, write, drain, half-close and terminal-state operations; and
- `ChannelValue` provides typed factories/accessors, while `TransferBuffer`
  provides copy, move, seal and immutable slice/view operations.

These roles and their exact signatures are compile-checked by the declaration
oracle. In particular, `join()` ordering is deterministic and
does not depend on which worker finishes first. `next()` reports each child's
terminal transition at most once in completion order; `join()` still returns
the complete stable result set, including children previously observed through
`next()` or `Task.wait()`.

The intended source experience is conceptually:

```rexx
pool = .taskpool.local(4, 64)
scope = .taskscope.failfast(pool, 5000)

left = scope.submit(scannerTarget, .channelvalue.string_value("north"))
right = scope.submit(scannerTarget, .channelvalue.string_value("south"))

results = scope.join()
if results[1].succeeded() then
  say results[1].value().string()

call scope.close()
call pool.close()
```

This block illustrates the exact factory spelling; the declaration oracle
fixes method return types which the current interface grammar can express.
`scannerTarget` must be obtained from a statically resolved task-work
declaration; users must not supply a procedure-name string or worker number.

### Level G

Level G adds domain ergonomics without changing the transport contract:

- typed task declarations and task methods over generated `.taskwork`
  adapters;
- transparently structured task expressions and `DO PARALLEL`, with no
  user-visible future type or detached task;
- generated or factory-selected typed service proxies;
- normal domain methods that return `Task`/`Completion` handles;
- service/actor lifecycle and discovery helpers;
- typed codecs and schema tooling;
- publish/subscribe, event-stream and projection libraries; and
- policy-rich orchestration built on Level B scopes and channels.

An ordinary application should normally call a typed domain interface. It
should not construct raw envelopes, operation-name strings or angle-bracket
intrinsics. A proxy transmits a `ServiceRef`, callable/member descriptor and
`ChannelValue` arguments underneath that interface.

### Intrinsics and compiler lowering

Angle-bracket intrinsics remain low-level system-programmer facilities. Gate F
does not require applications to use them. The compiler may later recognize
stable Level B library calls and lower them directly while preserving the same
source contract.

The approved F0 user model adds `task` as a callable/target construct and
`DO PARALLEL` as structured syntax. It places Level B classes over an RXAS-only
core bridge, with no RXPA task path and no public angle-bracket task-intrinsic
family. F0-S has locked the compile-checked declarations, machine contract and
coherence matrix before the first compiler or opcode edit.

## Level B, RXAS, VM and provider boundary

The required implementation path is Level G syntax lowering to Level B
classes, Level B-authored assembler invoking the channel instruction family,
and RXVM implementing those instructions over the private Gate E executor and
provider substrate. RXPA is not an alternate task-start or task-wait path.

The VM may use private native structures internally, but Rexx retains only a
validated VM channel capability. The VM/provider substrate owns only:

- bounded endpoint/queue mechanics;
- wait and wakeup;
- terminal completion publication;
- cancellation/deadline delivery and quarantine;
- envelope validation;
- receiver-owned `ChannelValue` materialization;
- bounded byte endpoints, EOF/half-close and child-stdio attachment; and
- runtime-owned provider registration and lifetime pinning.

Pool policy, actor/service behavior, event routing, topic semantics,
persistence and projection logic belong in Level B/G libraries.

One logical channel contract must be implementable by:

- an in-process worker provider;
- an isolated process provider;
- an open cross-host provider; and
- at least one non-Rexx actor that uses no CREXX or RXVM headers.

The cross-host protocol must define framing, versions, capabilities, identity,
schemas/codecs, ordering, delivery, terminal errors, deadlines/cancellation,
chunks/streams, flow control and unknown-version/type/capability behavior. It
must leave explicit extension points for authentication, authorization,
integrity and confidentiality. Gate F does not choose an encoding or network
transport before that logical contract passes review.

## RXAS/RXBIN sequencing

Gate F requires a minimum transport-neutral channel instruction family. F0-S
has locked its semantic and binary contract; F1 implements it in both VMs together
with the Level B wrappers and providers. Adrian authorized staged
implementation on 2026-08-14.

The mandatory conceptual roles are:

```text
chanopen   rStatus,rChannel,rProviderType,rRequiredCapabilities,rConfiguration
chanstart  rStatus,rTicket,rChannel,rEnvelope,rWaitMicroseconds
chanwait   rStatus,rCompletion,rChannel,rWaitMicroseconds
chancancel rStatus,rChannel,rTicket,rReason
chanclose  rStatus,rChannel,rMode
```

These are the exact F0-S spellings and operand order. `providerType` selects one implementation and
`requiredCapabilities` is a separate bit mask. Core types initially cover
local task, isolated worker process, open host, bounded byte endpoint and child
process; a registered extension range permits future RXVM plugin providers
without new opcodes. Unknown types, capability bits and registration conflicts
fail deterministically. Wait `-1` means forever, `0` means nonblocking and a
positive value is a relative microsecond budget.

Generic channel/provider creation is the role of `chanopen`; pool factories
and policies remain Level B/G concerns. Service identity, event buses, topics
and persistence also remain library operations. One-off `poolnew`,
`httpstart`, `dbquery`, file, timer or process opcode families are rejected.

Because CREXX is pre-release, the current `spawn`, `redir2str`, `redir2arr`,
`str2redir`, `arr2redir` and `nullredir` RXAS surface may be retired in favor of
byte-endpoint and child-process provider types. Old numeric slots remain
reserved, stale RXBIN fails explicitly, and compiler exits, Level B ADDRESS
code and repository images are updated together. Existing socket/TLS, file and
clock/time operations remain synchronous, owner-local low-level primitives in
F1; they are not alternate asynchronous provider families. `MTIME` remains a
time-of-day primitive and is not the Gate F deadline clock.

The F0-S instruction contract defines:

- an enforced RXBIN feature/version gate;
- exact operand, result, ownership and cleanup semantics;
- the validated channel-capability representation and rejection rules;
- the portable envelope/completion value-tree boundary;
- complete effect and signal sidecars;
- conservative optimizer-barrier behavior until stronger proof exists;
- source, TRACE, profiler and debugger identity;
- cancellation/deadline and terminal-failure behavior;
- compatibility and malformed-image tests; and
- identical logical semantics in `rxbvm` and `rxtvm`.

The Level B class library reaches these instructions through its existing
Level B-only authored `assembler` facility. Ordinary Level G source cannot use
that facility directly. Profiling may later improve lowering or instruction
implementation, but it does not decide whether this core instruction boundary
exists.

## Gate F execution sequence

### F0-S — exact semantic, source and machine contract

- [x] Approve the user-oriented terminology, conceptual machine and Rexx
  surface in `PERF3-13-GATE-F-USER-GUIDE.md`.
- [x] Derive the maintainer/AI specification plus a coherence
  matrix against the user guide and this ownership design.
- [x] Compile-check the Level B interface declarations, factory forms and
  method signatures under the current grammar.
- [x] Lock the exact RXAS/RXBIN instruction signatures, value/capability types,
  effects, signals, feature/version gate, diagnostics and malformed-image
  behavior for both VMs.
- [x] Lock provider type codes, required-capability flags, runtime registration
  and provider/module lifetime rules.
- [x] Lock reusable byte-endpoint and child-process provider contracts plus the
  migration/retirement disposition of existing spawn/redirect RXAS.
- [x] Lock task-target construction without user-authored procedure strings.
- [x] Lock `ChannelValue`, codec, envelope and completion schemas.
- [x] Lock scope, service ordering, cancellation, deadline and shutdown state
  machines.
- [x] Define provider conformance tests and protocol golden vectors.
- [x] Define diagnostics, capability negotiation and version failure behavior.
- [x] Complete the coherence check before the first opcode/runtime edit; stop
  for Adrian only if it exposes a contradiction or new language decision.

### F1 — local and process providers

- [x] Implement the mandatory channel instructions identically in `rxbvm` and
  `rxtvm` over the in-process Gate E executor/provider substrate.
- [x] Implement the Level B local-provider class surface as wrappers over those
  instructions, with no RXPA task path or native payload contract.
- [x] Implement runtime-owned private provider registration, complete local
  descriptor validation/lifetime pinning and fake-provider conformance.
- [x] Implement the byte-endpoint and child-process provider descriptors and
  operations behind the same registry contract.
- [x] Generalize redirects as reusable bounded byte endpoints, update ADDRESS
  and compiler exits, retire the selected old RXAS mnemonics and preserve their
  numeric slots.
- [x] Implement a separate-process provider with the same contract.
- [x] Implement the approved core Level G task declarations, task expressions
  and `DO PARALLEL` only as lowering to that same Level B contract.
- [x] Prove no live VM value/reference/native pointer crosses the local or
  isolated-process task boundary.
- [x] Prove bounded backpressure, failure isolation, cancellation, hard process
  termination after cooperative grace, deterministic join and no-spill worker
  reuse for the local and isolated-process providers.
- [ ] Deliver the bounded concurrent HTTP/TLS consumer over tasks, channels and
  byte endpoints without an HTTP opcode family.
- [ ] Publish an experimental Level B surface only after portable local/process
  conformance and an accepted first Release verdict.

F1e implements core provider type `2` with capability mask `0x010f`: bounded
admission, cancellation, provider-owned deadlines, completion-order
observation and isolated task execution. A private versioned framed transport
uses the same canonical RXCV task and completion documents as type `1`.
Controller program generations are snapshotted only when bytecode-only; an
archive containing several semantic graphs remains a concatenation of RXBIN
007 containers so every graph keeps its own numeric callable/member identities.
Native modules, live VM storage, references and handles never enter that
snapshot.

The pool keeps a bounded number of warm worker processes, but every accepted
task creates a fresh executor and VM context inside its assigned process. Thus
process reuse amortizes launch cost without carrying module globals, frames,
registers or task state into the next request. Cancellation and deadlines send
the cooperative interrupt first and may terminate only that isolated process
after a bounded grace period. Loss before `STARTED` is `TRANSPORT_LOST`; loss
after `STARTED` is `UNKNOWN_OUTCOME`. The terminal transition is exactly once,
the failed process is replaced, all pipes and temporary snapshots are owned by
the provider, and protocol-pipe closure is contained at the private write
boundary rather than changing the host process's signal disposition.

### F2 — open host protocol and higher Level G libraries

- [ ] Select an open encoding/transport from measured and interoperability
  evidence.
- [ ] Implement a cross-host provider and one independent non-Rexx actor.
- [ ] Exercise compute, file/socket/HTTP, database, timer and child-process
  providers.
- [ ] Add typed Level G service proxies and event/projection libraries.
- [ ] Prove unknown-version/type/capability, disconnect and ambiguous-outcome
  behavior.

### F3 — specialized lowering and stabilization

- [ ] Profile the accepted Level B-over-RXAS surface and its instruction
  implementation before proposing any specialized lowering.
- [ ] Compare copies, moves and immutable sharing over the required payload
  panel.
- [ ] Optimize only the proved common paths while preserving the mandatory
  transport-neutral instruction and class contracts.
- [ ] Complete both-VM, cross-platform, sanitizer, install/package and public
  compatibility closeout.
- [ ] Stabilize the public surface only after experimental compatibility and
  operational evidence are accepted.

Each production edit follows the mandatory focused-correctness, frozen
profiling-off Release verdict and Adrian decision stop in
[`performance/AGENTS.md`](AGENTS.md).
Approval of one phase does not approve the next.

## Required evidence matrix

Gate F measurements keep correctness and semantics primary and cover:

- empty submit/completion latency and steady-state throughput;
- payload sizes of at least 16 B, 1 KiB, 64 KiB and 1 MiB;
- copy versus move versus sealed sharing/mapping/serialization;
- one worker through the selected core-count/scaling range;
- stateless CPU work, blocking I/O and stateful single-owner service work;
- industrial concurrent HTTP/TLS covering connection reuse, bounded response
  handling, cancellation, streaming capability, saturation and teardown;
- in-process, process and host providers where implemented;
- queue saturation, producer fairness and deadline-aware backpressure;
- queued cancellation, running Rexx cancellation and uncooperative native
  quarantine;
- service ordering and execution-local module-global isolation;
- teardown, retained RSS, reclamation and repeated provider reuse;
- normal single-worker product neutrality; and
- both `rxbvm` and `rxtvm` on the platforms required by the selected phase.

The public benchmark fixture must not depend on worker indices or sleep-based
ordering. Cross-host testing distinguishes transport time, serialization time,
remote execution and end-to-end latency.

## Comparative rationale

The design deliberately follows the common modern hybrid rather than claiming
that one concurrency mechanism replaces all others:

- Java retains shared variables and a happens-before memory model while adding
  cheap virtual threads and structured task lifetimes:
  [JLS 17](https://docs.oracle.com/javase/specs/jls/se26/html/jls-17.html),
  [JEP 444](https://openjdk.org/jeps/444),
  [JEP 505](https://openjdk.org/jeps/505).
- Go recommends communicating ownership while acknowledging that a mutex may
  be the clearest solution for some state:
  [Effective Go](https://go.dev/doc/effective_go).
- Rust makes cross-thread transfer/sharing explicit through `Send`/`Sync` and
  offers bounded and unbounded channels:
  [Rust concurrency traits](https://doc.rust-lang.org/stable/book/ch16-04-extensible-concurrency-sync-and-send.html),
  [`std::sync::mpsc`](https://doc.rust-lang.org/std/sync/mpsc/).
- Erlang isolates process state and copies ordinary messages while sharing
  suitable immutable binary storage internally:
  [Erlang process messages](https://www.erlang.org/docs/25/efficiency_guide/processes),
  [binary handling](https://www.erlang.org/docs/26/efficiency_guide/binaryhandling).
- Swift actors and Orleans grains expose logical isolation rather than
  physical thread identity:
  [Swift data-race safety](https://www.swift.org/migration/documentation/swift-6-concurrency-migration-guide/dataracesafety/),
  [Orleans benefits](https://learn.microsoft.com/en-us/dotnet/orleans/benefits).
- Clojure Refs show that versioned transactional state depends on immutable
  values and retry-safe effects rather than being a free replacement for
  shared variables:
  [Clojure Refs](https://clojure.org/refs).

This evidence supports isolated CREXX executions, transferable messages,
structured lifetime and single-owner logical services. It does not support
transparent sharing or versioning of arbitrary mutable Level B objects.

## Change control

The following remain measured or later-phase selections rather than public
semantic gaps:

- wire encoding and network transport;
- copy/share threshold and mapping implementation;
- provider discovery, persistence and security policy;
- fairness implementation within the stated ordering contract;
- a future catchable cancellation/cleanup language construct; and
- later evidence-led instruction optimizations within the mandatory five
  semantic roles.

A proposal to expose worker IDs, make module globals process-shared, transfer a
live object/reference, permit writable shared mapping, add transparent retry,
weaken structured join, or move event/actor policy into the VM contradicts this
record and returns to Adrian as a language/architecture decision.

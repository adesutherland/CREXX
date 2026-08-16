# cREXX concurrency architecture and maintainer reference

This document is the enduring implementation guide for cREXX concurrency. It
describes how Level G tasks, the Level B concurrency classes, RXAS/RXBIN and
RXVM providers fit together. It is written for maintainers and AI agents; the
programming and language books provide the user-facing teaching and formal
syntax.

Current status: the surface described here is implemented experimentally on
`develop` and has local Mac qualification. Portable Linux/Windows conformance,
package proof and an explicit release-publication decision are still required.
Do not describe it as released, portable or stable without checking release
tags, `docs/releases/` and the live
[`concurrency/WORKLIST.md`](../../concurrency/WORKLIST.md).

For exact current support, including deliberately unavailable declarations,
use the
[`implementation status matrix`](../../concurrency/IMPLEMENTATION-STATUS.md).
Accepted design boundaries are in
[`concurrency/DECISIONS.md`](../../concurrency/DECISIONS.md).

## The model in one picture

```text
Level G: task declarations, ordinary calls, task targets, DO PARALLEL
        |
        | compiler lowering; structured dependency and cleanup plan
        v
Level B: taskpool -> taskscope -> task/completion
         tasktarget/taskwork/taskcontext
         channel/channelrequest/channelvalue/byteendpoint
        |
        | Level B ASSEMBLER only
        v
RXAS: chanopen / chanstart / chanwait / chancancel / chanclose
        |
        | execution-local capabilities; canonical binary envelopes
        v
RXVM: channel table -> runtime provider registry -> bounded provider
        |                 |               |               |
        local tasks       processes       byte endpoints  child processes
```

HTTP remains a Rexx library above this stack. It is neither an RXAS
instruction nor a special provider type.

## Non-negotiable invariants

Every compiler, library or VM change must preserve these together:

1. One RXVM execution owns its globals, frames, registers, references,
   ordinary objects, sockets and runtime overlays. A live VM value, reference,
   object address, native payload or C pointer never crosses to another
   execution.
2. Task inputs and outputs cross as validated canonical `ChannelValue` data.
   The receiver reconstructs values and objects in receiver-owned storage.
3. A task is a stateless invocation. Durable mutable state belongs to one
   logical service/actor owner whose accepted calls are serialized. The public
   service path is reserved, not implemented yet.
4. Every accepted task belongs to a scope. Scope exit accounts for it through
   completion, cancellation and terminal observation, or an explicit unknown
   outcome. There are no detached ordinary tasks.
5. Admission queues and byte buffers are bounded. Convenience APIs must not
   hide unbounded growth.
6. Level G lowers through the public Level B classes. Those classes reach the
   VM only through the five channel instructions. There is no RXPA task API or
   hidden native-handle task path.
7. All providers implement one transport-neutral VM contract. A new transport
   or plugin must not introduce provider-specific task opcodes.
8. Raw channel and ticket integers are execution-local capabilities. They are
   not worker identities and cannot be transferred as application values.
9. A task may use synchronous owner-local I/O, but it may not block waiting for
   a different task. The controlling execution owns joins.
10. Each request publishes exactly one terminal completion despite cancel,
    deadline, close and provider-failure races.

## Terms

- **Controlling execution** or **controller**: the execution evaluating the
  expression or `DO PARALLEL`, submitting work and materializing results.
- **Task**: one independently schedulable invocation with transferred inputs
  and a terminal outcome. It is not an OS thread and has no public affinity.
- **Pool**: a bounded provider and lifecycle object. A pool is created by a
  factory such as `.taskpool.local(...)`; the pool itself is not a factory for
  user classes.
- **Scope**: the structured owner of submitted children, failure policy,
  deadline, cancellation and join.
- **Task target**: a compiler/linker-sealed descriptor for a statically known
  task procedure, transferable task method or `.taskwork` factory.
- **Task work**: the advanced Level B receiver interface
  `run(request, context)`.
- **Channel**: the provider-neutral request/completion lifecycle used beneath
  pools, scopes, endpoints and child processes.
- **Provider**: an RXVM implementation behind a provider type and capability
  mask.
- **ChannelValue**: the immutable canonical portable value tree used at every
  execution boundary.
- **Provider reference**: transferable logical identity and rights for a
  provider-owned resource. It is not a copied OS or VM handle.
- **Service**: a future single-owner durable state identity. `.serviceref`
  reserves the shape; `.taskscope.ask()` is currently unsupported.

## Level G source semantics

All syntax in this section requires `OPTIONS LEVELG`. `TASK` and `PARALLEL`
are contextual words and remain ordinary identifiers outside their grammar
positions.

### Task declarations

A module task procedure or class task method uses the existing callable shape:

```rexx
twice: task = .int
  arg value = .int
  return value * 2
```

Omitting the result type means `.void`. Task signatures cannot contain exposed
or reference arguments, varargs, untyped `.object` transfer, or another type
without an exact supported transfer contract.

A task is invoked with ordinary call syntax. Given two independent task
operands:

```rexx
total = first() + second()
```

the compiler evaluates arguments left to right, submits `first` before
`second`, permits the bodies to overlap, waits for both successful results and
then performs ordinary addition. It does not change the meaning of `+` or
ordinary call syntax. Short-circuit operators remain short-circuiting, so a
task in an unevaluated branch is not submitted.

An ordinary function in the same expression stays on the controller. The
controller may evaluate it while an already submitted independent task runs.

`CALL worker` outside a parallel block has an implicit statement scope and
completes before the next statement. Task-valued expressions similarly own an
implicit expression scope that closes before the statement completes.

### Parallel blocks

```ebnf
parallel-do ::= "do" "parallel" [ "using" expression ]
                clauses
                "end" [ symbol ]
```

The block may be a statement or an existing expression-form `DO`; expression
form still requires `LEAVE WITH expression`.

Assignments from task calls create typed pending bindings. Reading a binding
materializes it and waits if necessary. Before materialization the compiler
rejects reassignment, reference/expose use, mutation through the value and
escape from the scope.

All task calls in one block share its scope. `USING` is evaluated once and must
produce a fresh open `.taskscope`; the block consumes and closes that scope but
does not close its pool. Without `USING`, lowering creates a scope over the
execution-local default pool.

Ordinary clauses in the block still run sequentially on the controller. Only a
task call or explicit Level B submission creates child work.

### Recursion and nesting

A direct call from a task body to the same task callable is an ordinary
synchronous recursive call in the current worker. It is not a second
submission. A call from one task body to a different task callable, including
mutual recursion, would create a blocking nested wait and is rejected in the
current surface.

### Explicit task targets

```ebnf
task-target ::= "task" callable-reference
              | "task" class-factory-expression
```

`task checksum` identifies a statically resolved task callable.
`task .imagework("thumbnail")` identifies a concrete factory whose result
implements `.taskwork`; factory arguments are evaluated and transferred by the
controller, and the object is constructed in the receiving execution.

Dynamic strings, procedure variables, native addresses and worker numbers are
not task targets.

## Transferable typed objects

A typed object argument, result or task-method receiver must resolve to one
concrete transfer contract:

```rexx
from_channel: factory
  arg encoded = .channelvalue

to_channel: method = .channelvalue
```

For an argument or receiver, the controller calls `to_channel()` before
submission and the worker calls the statically resolved `from_channel()`
factory. For a result, the worker encodes and the controller reconstructs. The
encoded value contains immutable value state or a validated logical provider
reference, never an object address, local channel/ticket capability or native
payload.

This is a static route. `.channelcodec` is the explicit general interface, but
task lowering does not consult an ambient runtime codec registry.

The compiler-internal typed-value marker and the direct `.taskwork` request
path are intentionally different. `.taskscope.submit(target, request)` passes
the supplied application `ChannelValue` directly to
`.taskwork.run(request, context)`.

## Task targets and the 80-byte seal

The compiler, assembler and linker create an RXTB version-1 binding containing:

- binding kind: task procedure, task-method adapter or `.taskwork` factory;
- linked-image digest;
- semantic-graph callable id;
- callable-signature digest; and
- the adapter callable slot where required.

The human-readable target name is diagnostic text; it is not used for
dispatch. The linked descriptor lets the receiver detect a stale, substituted
or signature-incompatible target before running it. It is integrity metadata,
not encryption and not a secret.

Full validation resolves the callable and adapter against the immutable linked
graph. Each worker has a bounded cache keyed by the complete binding and
requested result mode. A hit reuses only the already validated immutable plan.
Misses and failed validation take the full path, and worker/context teardown
invalidates cached pointers.

RXLINK rebuilds and reseals task metadata and matching use-site constants after
it merges and renumbers the semantic graph. It must never copy a module-local
seal unchanged into the final image or weaken a failed seal to name dispatch.

## Level B object model

The public declarations and implementations live in
`lib/classlib/Concurrency.crexx`.

| Surface | Role and lifecycle |
| --- | --- |
| `.taskpool` | Creates a bounded local (`1`) or isolated-process (`2`) provider. The caller closes it after every scope using it has closed. |
| `.taskscope` | Owns submitted children, policy, deadline, cancellation, observations and join. `finish()` joins and raises `TASK_FAILURE` for a failed child; `abort(reason)` cancels, joins and closes. |
| `.task` | Structured wrapper for one accepted or rejected child. Its raw ticket never escapes. |
| `.completion` | Immutable observation of terminal state/value/error, or an unavailable sentinel for a timed/nonblocking observation. |
| `.tasktarget` | Compiler-created sealed descriptor. Applications use `task target` syntax rather than calling `binding()` with hand-authored bytes. |
| `.taskarguments` | Mutable controller-side compiler lowering helper. It is not the ordinary typed user surface. |
| `.taskwork` | Advanced receiver contract `run(request, context)`. |
| `.taskcontext` | Receiver view of remaining timeout, cooperative cancellation and trace identity. `endpoint(reference)` exists but remains provisional until directly covered by conformance. |
| `.channel` | Provider-neutral lifecycle owner over the five RXAS operations. |
| `.channelrequest` | Non-authority wrapper around one local ticket. |
| `.channelvalue` | Canonical immutable transfer value. |
| `.channelcodec` | Exact manual encode/decode contract; no general registry is advertised. |
| `.byteendpoint` | Reusable bounded readable, writable or duplex byte resource. |
| `.transferbuffer` | Explicit mutable-owner, moved and immutable-sealed binary lifecycle. |
| `.serviceref` | Reserved logical single-owner service identity; no public concrete service exists yet. |

Pool statistics `queued()` and `running()` and service submission `ask()`
deliberately signal unsupported status `19`. Documentation and callers must not
substitute plausible zeros or fake service behavior.

Level B timeouts use milliseconds: `-1` waits indefinitely, `0` is
nonblocking/immediate and positive values are relative waits. Values below
`-1` are invalid. The class converts positive values to checked RXAS
microseconds.

`scope.next()` observes completion order. `scope.join()` returns all children
in stable submission order, including children already observed. A timeout or
nonblocking miss returns a completion with `available() = 0`; that sentinel is
not a terminal child and never appears in `join()`.

## RXAS and RXBIN contract

Concurrency is a core RXVM capability exposed to Level B through exactly five
RXAS instructions:

| Opcode | RXAS shape |
| ---: | --- |
| `650` | `chanopen status,channel,providerType,requiredCapabilities,configuration` |
| `651` | `chanstart status,ticket,channel,envelope,waitMicroseconds` |
| `652` | `chanwait status,completion,channel,waitMicroseconds` |
| `653` | `chancancel status,channel,ticket,reason` |
| `654` | `chanclose status,channel,mode` |

The exact operand rules, effects, failures and examples are in
[`09-io-sockets-processes-and-time.md`](../reference/rxas/instructions/09-io-sockets-processes-and-time.md).
The instructions are opaque optimization barriers and do not raise VM signals;
they return operation status and failure-default companion outputs. Inputs are
snapshotted before outputs are changed, so a permitted output/input alias is
deterministic.

RXBIN 007 uses `RXBIN007_FEATURE_CHANNELS` (`1 << 3`). Writers set it when any
channel opcode is present. Readers reject channel opcodes without the feature,
unknown feature bits and every reserved opcode. Old pre-release process and
redirect slots `466..471` are reserved; their source mnemonics are retired and
images using them must be rebuilt.

The linker carries the validated union of input feature requirements. It also
preserves sealed task bindings as runtime contract metadata even when source
debug metadata is stripped.

## Providers and capabilities

Core provider types are:

| Type | Meaning | Status |
| ---: | --- | --- |
| `1` | local-thread task pool/scope | implemented |
| `2` | isolated-process task pool/scope | implemented |
| `3` | open host | reserved |
| `4` | bounded byte endpoint | implemented |
| `5` | structured child process | implemented |

Type `0` and negative values are invalid. Types `6..65535` are reserved for
future core providers; `65536` and above are the extension range. The runtime
has an internal tested registry seam, but no installed public provider-plugin
ABI is promised.

Required capability bits are separate from provider type:

| Bit | Hex | Capability |
| ---: | ---: | --- |
| `0` | `0x0001` | bounded admission/backpressure |
| `1` | `0x0002` | cancellation |
| `2` | `0x0004` | provider-owned deadlines |
| `3` | `0x0008` | completion-order observation |
| `4` | `0x0010` | streaming/chunking |
| `5` | `0x0020` | reusable byte endpoints |
| `6` | `0x0040` | child standard-stream attachment |
| `7` | `0x0080` | structured child-process execution |
| `8` | `0x0100` | isolated task execution |
| `9` | `0x0200` | open-host operation |

An open request names one provider type and the capabilities it requires. A
provider may offer more. Unknown required bits or a missing capability fail
open without leaving a live resource.

The runtime owns the provider registry, independently of any Rexx execution.
Lookup pins a descriptor/module for the channel lifetime. Providers receive
copied canonical binary values and core capability identities; callbacks do
not retain caller registers or call Rexx while holding registry/channel-table
locks.

## Capabilities and references

Channel and ticket handles are opaque positive signed 64-bit capabilities with
owner, kind, slot and generation fields. Every operation validates the owner,
kind, generation, channel relationship and lifecycle before provider access.
The integer is not a pointer, OS handle, worker number or transferable identity.
Closing a channel invalidates every copied integer for it.

A logical provider reference is different. It carries provider type, reference
version, rights, scope and opaque provider identity/integrity bytes. It contains
no VM or OS pointer. A receiving provider validates the reference and creates a
new execution-local adapter.

Reference scopes are runtime (`1`), process (`2`) and future host (`3`). Byte
endpoints currently use runtime-scoped references so attached task executions
can stream data without sharing controller VM state.

## ChannelValue and transfer buffers

RXCV is the one canonical transfer document. It represents null, boolean,
integer, float, decimal, string, binary, array, record, local capability and
provider reference nodes. Local-capability nodes are private configuration
material and are rejected as ordinary task arguments/results.

Validators enforce version, total length, node lengths, canonical record field
ordering, duplicate rejection, recursion depth, element count and trailing-byte
rules before a provider interprets the value. The receiver constructs its own
`.channelvalue`; no source `value *` survives the boundary.

`.transferbuffer` makes large binary lifecycle explicit:

- mutable owner: allocate/copy, read and write;
- moved: `move_value()` transfers bytes into an immutable ChannelValue and
  invalidates the mutable source; and
- sealed: `seal()` snapshots immutable bytes and returns the same stable
  ChannelValue on later calls while retaining read-only access to the source.

The transfer-buffer seal prevents later writes through that buffer. It does
not encrypt, authenticate or checksum the bytes. This is distinct from RXTB
task-binding validation.

## Lifecycle, completion and failure

The channel lifecycle is open, closing and closed. Close mode `1` drains;
close mode `2` cancels. A failed open leaves no live channel. Each accepted
start creates one ticket and exactly one terminal completion.

Operation status codes distinguish invalid arguments/types/providers,
unsupported capability/configuration/version, resource exhaustion,
backpressure, would-block, timeout, closed/stale/wrong-owner/unknown-ticket,
already-terminal, provider failure, shutdown, unsupported operation and
internal error. Level B maps lifecycle misuse and open/start failures to the
catchable `CHANNEL_ERROR` signal. Expected task outcomes remain completion
data.

Completion states are:

| Code | State |
| ---: | --- |
| `0` | `NONE`, observation sentinel only |
| `1` | `SUCCEEDED` |
| `2` | `FAILED` |
| `3` | `CANCELLED` |
| `4` | `DEADLINE_EXCEEDED` |
| `5` | `REJECTED` |
| `6` | `ENDPOINT_CLOSED` |
| `7` | `TRANSPORT_LOST` |
| `8` | `UNKNOWN_OUTCOME` |

Cancellation is cooperative for running local work. Process providers may
terminate an isolated worker after the cooperative grace period. A transport
loss before execution starts is `TRANSPORT_LOST`; after it starts the safe
answer is `UNKNOWN_OUTCOME`.

Fail-fast scopes request sibling cancellation after the first unsuccessful
child. Collect-all scopes keep accounting for other children. Both still join
every accepted child. Admission rejection is represented as a synthetic
`REJECTED` child so the source has one outcome per attempted submission.

## Process isolation

The process provider uses the same schemas and completion model as the local
provider. It snapshots the current bytecode-only linked generation and loads
that exact semantic graph in bounded warm worker processes. Native modules are
not process-eligible.

Each request receives a fresh executor and VM context even when its process is
reused. Globals, registers, frames, references, cancellation state and mutable
overlays do not spill between requests. Pool close joins workers/monitors,
closes private protocol endpoints and removes the temporary snapshot.

The process framing and hidden worker command are private implementation
details, not the future open-host protocol.

## Byte endpoints, child processes and ADDRESS

Type `4` endpoints provide the common bounded read/write/duplex building block.
They can export a validated provider reference that another attached execution
opens as its own adapter. Reads/writes are requests with normal completion,
deadline, cancellation, half-close and teardown accounting.

Structured child processes use provider type `5`. Standard input, output and
error redirection is expressed with endpoint/provider references rather than a
separate family of spawn/redirect instructions. The Level B ADDRESS adapters
preserve source behavior while lowering to channels and endpoints.

Synchronous owner-local file, socket, time and console instructions remain
valid. They are not transferable resource identities. If asynchronous
lifecycle is needed later, it belongs behind another provider using the same
five RXAS operations.

## Concurrent HTTP

`lib/rxfnsg/rexx/http.crexx` is a Level G concurrent HTTP library over tasks,
endpoints and the existing owner-local socket/TLS primitives. It supplies:

- bounded global/per-origin admission and reusable single-owner connections;
- verified TLS by default;
- validated headers and bounded metadata/body limits;
- explicit retry, redirect, idempotency and ambiguous-outcome policy;
- fixed and chunked request streams plus streamed responses; and
- bounded gzip, zlib and raw DEFLATE decoding.

Only provider references and canonical values cross executions; socket
integers remain with their connection owner.

The repository also contains the independent synchronous Level B client
`lib/rxfnsb/rexx/rxhttp.crexx`. Existing LLM providers currently use that
client. Do not imply that one wraps the other or migrate callers as a
documentation cleanup. The architectural decision is tracked as CONC-16.

## Deliberately absent or provisional

Do not document or implement around these as if they exist:

- `.taskscope.ask()` and concrete services;
- `.taskpool.queued()` and `.taskpool.running()` telemetry;
- provider type `3` or an open-host wire protocol;
- a public provider-plugin registration ABI;
- typed service proxies, actor/event/topic/projection libraries;
- detached tasks, public thread IDs or worker affinity;
- locks, atomics or shared writable VM values; or
- implicit replay of work with an ambiguous outcome.

`.taskcontext.endpoint()` is present and delegates to the proven endpoint
reference adapter, but it remains provisional until a direct public-contract
test is added.

## Change discipline

When changing this subsystem:

1. Preserve structured lifetime, execution ownership and canonical transfer as
   one proof. A change that makes one layer convenient by weakening another is
   not acceptable.
2. Keep Level G sugar and Level B control coherent. Syntax lowering must remain
   expressible through public Level B objects and the five RXAS instructions.
3. Keep task callable selection static and sealed. Never fall back to a
   controller-provided procedure-name string.
4. Keep provider type and capabilities separate. New technologies belong
   behind the common provider contract.
5. Keep channels/endpoints bounded, cancellation-aware and teardown-safe.
6. Update source RexxDoc, user/language/library docs, this file and the
   implementation truth matrix in the same surface change.
7. Treat library development as toolchain conformance: exercise `rxc`, `rxas`,
   `rxlink`, `rxbvm` and `rxtvm`, optimized and unoptimized where applicable.
8. Run focused malformed-value, stale/wrong-owner capability, race,
   cancellation, deadline and teardown tests before broad regression QA.
9. Route performance measurement through `performance/` governance, but keep
   concurrency scope and publication state in `concurrency/`.

## Code and evidence map

| Concern | Primary locations |
| --- | --- |
| Grammar, typing and lowering | `compiler/`, `compiler/tests/rexx_src/` |
| Level B API and RXCV adapters | `lib/classlib/Concurrency.crexx` |
| Concurrent HTTP | `lib/rxfnsg/rexx/http.crexx`, `lib/rxfnsg/tests_functional/` |
| RXAS parse/metadata/validation | `assembler/`, `common/` |
| Link-time task resealing | `linker/`, `common/` semantic graph code |
| VM channel core/provider registry | `interpreter/rxvmchannel.c` |
| Local/process executors | `interpreter/rxvmexecutor.c`, `interpreter/rxvmchannel_process.c` |
| Byte endpoint/child process providers | `interpreter/rxvmchannel_byte.c`, `interpreter/rxvmchannel_child.c` |
| Human RXAS instruction reference | `docs/reference/rxas/instructions/09-io-sockets-processes-and-time.md` |
| Live status and remaining work | `concurrency/WORKLIST.md` |
| Historical decisions and evidence | `concurrency/history/`, `performance/evidence/` |

Historical test names may retain internal development-stage labels. Those
labels identify provenance only and must not leak into enduring feature names.

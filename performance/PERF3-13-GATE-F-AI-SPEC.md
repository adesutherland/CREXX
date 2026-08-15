# PERF3-13 Gate F maintainer and AI reference specification

Date: 2026-08-15

Status: **F0-S normative contract and F1a-F1g-D implementation complete;
Mac closeout QA complete; experimental publication remains gated by portable
conformance**

This is the exact maintainer-facing specification derived from the approved
user model in
[`PERF3-13-GATE-F-USER-GUIDE.md`](PERF3-13-GATE-F-USER-GUIDE.md) and the
ownership design in
[`PERF3-13-GATE-F-DESIGN.md`](PERF3-13-GATE-F-DESIGN.md). It is the source of
truth for F1 compiler, Level B, RXAS/RXBIN and RXVM implementation work.

The original compile-only declaration oracle is
[`gate_f_levelb_contract.crexx`](../compiler/tests/rexx_src/gate_f_levelb_contract.crexx).
It proves that the Level B names and signatures in this document use the
current class/interface grammar. The F1c implementation is
[`Concurrency.crexx`](../lib/classlib/Concurrency.crexx), with functional and
bridge-inspection tests covering the executable contract.

Normative words `must`, `must not`, `should` and `may` have their usual
specification meanings. Conceptual examples in the user guide remain
authoritative for source behavior; when an earlier planning record differs
from this exact contract, this document controls F1.

## 1. Invariants

Every implementation must preserve all of these rules:

1. An RXVM execution owns its mutable globals, frames, registers, references,
   objects, sockets and runtime overlays. A live `value`, reference cell,
   stack frame, closure, object address, native payload or C pointer never
   crosses an execution boundary.
2. Task arguments and results cross as validated `ChannelValue` data. The
   receiver materializes its own values.
3. Durable mutable state has one logical service/actor owner. Tasks are
   independent work and do not acquire hidden affinity through globals.
4. Every task belongs to a scope. Scope exit accounts for every accepted child
   through terminal completion, cancellation plus terminal completion, or an
   explicit quarantined/unknown-outcome record.
5. Queues and byte buffers are bounded. No convenience surface may obtain
   apparent success by growing an unbounded queue or body buffer.
6. Level G lowers through the Level B classes. Those classes reach core
   concurrency only through the five RXAS channel operations. There is no
   RXPA task-start, task-wait, cancellation or provider-selection path.
7. Providers implement one common RXVM contract. A core or plugin provider
   does not add a provider-specific RXAS instruction.
8. Raw RXAS channel and ticket handles are execution-local integers. A
   transferable provider reference is a different, validated logical value.
9. A task body may perform synchronous I/O, but it may not synchronously wait
   for another task in F1. The controlling execution owns task joins.
10. Each request publishes exactly one terminal completion.

## 2. Layer contract

```text
Level G task declarations, calls and DO PARALLEL
        -> Level B task target, pool, scope, task and completion objects
        -> Level B-authored ASSEMBLER
        -> chanopen/start/wait/cancel/close RXAS
        -> execution-owned RXVM channel table and runtime-owned provider registry
        -> local task, process task, byte endpoint, child process or host provider
        -> Gate E sealed generations, worker-owned overlays and physical delivery
```

The Level B object owns the user lifecycle. RXVM owns capability validation,
bounded provider mechanics, completion publication and provider/module
lifetime. Gate E remains private mechanism and does not become a public worker
or affinity API.

## 3. Level G grammar

### 3.1 Contextual words

`TASK` and `PARALLEL` are contextual words, case-insensitively. They do not
become globally reserved identifiers.

All syntax in this section is Level G-only. `task` declarations, explicit
`task target` expressions and both statement/expression forms of
`DO PARALLEL` are rejected when the source selects another language level.
The same words remain ordinary identifiers outside these grammar positions.

- `task = 1`, `call task` and a routine named `task` remain valid when the
  token is not in one of the grammar positions below.
- `parallel` remains an ordinary symbol except immediately after the opening
  `DO` of a parallel block.
- Dotted class/interface names such as `.task` remain valid.

### 3.2 Task callables

The callable forms are:

```ebnf
task-procedure ::= symbol ":" "task" [ "=" type ] callable-body
task-member    ::= symbol ":" "task" [ "=" type ] callable-body
```

At module scope, `task` declares a task procedure. Inside an interface or
class it declares a task method. Omitting the result type declares `.void`.
Arguments use the existing Level B `ARG` grammar.

A task signature must not contain:

- `ARG EXPOSE`;
- a `reference` argument or return;
- varargs;
- `.object` without a statically exact transferable object contract;
- an array whose element type is not transferable; or
- a receiver without an exact transferable-proxy contract.

Task procedure identity and task-method adapter identity are emitted into
semantic metadata. They are never obtained from a user-authored procedure-name
string.

#### Transferable object contract

A task-method receiver and a typed object argument/result must resolve to a
concrete class so its encoder and reconstruction factory are statically unique.
Its exact static contract declares both members below:

```rexx
from_channel: factory
  arg encoded = .channelvalue
to_channel: method = .channelvalue
```

For a task-method receiver or typed object argument, the controller invokes
`to_channel()` synchronously before submission and the worker invokes the
statically resolved `from_channel` factory for the sealed formal type to
construct new worker-owned state. A `.channelvalue` formal receives the
canonical RXCV value directly. For a typed object result, the direction is
reversed: the worker invokes `to_channel()`, publishes canonical RXCV, and the
controller invokes the declared result class's statically resolved
`from_channel` factory. Each side owns its reconstructed object; live object
identity never crosses. Callable identities and signatures come from the
sealed semantic graph/descriptor and are not selected from controller-provided
type or method strings.

The encoded value must contain only the object's immutable value state or a
validated logical provider/service reference. It must not encode an object
address, reference, local channel/ticket handle or native payload. Declaring
the two members does not waive runtime schema validation.

Standard generated service and HTTP proxies provide this pair automatically.
`.channelcodec` remains the explicit Level B contract for manual domain-value
encoding, but F1 task signature checking does not perform an ambient runtime
codec-registry lookup: the transfer route must be statically exact.

F1g-A supports codec-backed object results in immediate expressions and
pending `DO PARALLEL` bindings. The hidden Level B bridge returns the canonical
`.channelvalue`; source lowering immediately wraps it in
`ResultClass.from_channel(...)`. The executor resolves
`ResultClass.to_channel` in the sealed task graph and encodes the returned
value before terminal completion. A missing or malformed pair is
`#TASK_NONTRANSFERABLE_TYPE`, not a late `FACTORY_NOT_FOUND`.

### 3.3 Task-target expressions

```ebnf
task-target ::= "task" callable-reference
              | "task" class-factory-expression
```

Examples:

```rexx
target = task checksum
target = task .imagework("thumbnail")
```

The first form requires a statically resolved task procedure or generated task
method adapter. The second requires a statically resolved concrete class that
implements `.taskwork` and a statically resolved factory. Factory arguments
are evaluated in the controller and encoded; the receiver object is created in
the receiving execution.

Dynamic strings, procedure variables, native procedure addresses and runtime
worker numbers are invalid task targets.

### 3.4 Parallel blocks

```ebnf
parallel-do ::= "do" "parallel" [ "using" expression ]
                clauses
                "end" [ symbol ]
```

The block may appear as a statement or in the existing expression-form `DO`
position. Expression form still requires `LEAVE WITH expression`.

`USING` is evaluated once by the controller before the block opens. Its value
must be an unused, open `.taskscope`. The block consumes and closes that scope
but not its pool. The simple form obtains a new scope from the execution-local
default pool.

Ordinary clauses run sequentially in the controlling execution. Only calls to
declared task callables and explicit Level B submissions create child work.

### 3.5 Pending result bindings

An assignment whose right side is a task call inside `DO PARALLEL` creates a
typed pending result binding. Its source type is the task's declared result
type; there is no public future type.

Before materialization, the compiler must reject:

- reassignment;
- `ARG EXPOSE`, `reference` or `dereference` use;
- indexed or attribute mutation through the binding; and
- escape from the scope through a closure, object attribute or return.

Reading the binding materializes it and waits if necessary. After successful
materialization it is an ordinary value. A non-success terminal result raises
controller-side `TASK_FAILURE` as specified in section 11.

## 4. Evaluation and lowering

### 4.1 Expression order

For an expression containing task calls, the compiler must:

1. evaluate ordinary argument and receiver expressions left to right;
2. encode a task's inputs only after all of that task's inputs are available;
3. submit ready tasks in source evaluation order;
4. allow submitted independent task bodies to overlap;
5. evaluate ordinary functions, methods and operators in their existing
   controller-side order; and
6. apply an operator only after its task operands have materialized.

Thus `task1() + task2()` submits `task1` before `task2`, permits both bodies to
overlap and performs ordinary addition after both succeed. In
`task1() + ordinaryFunction()`, the controller may evaluate
`ordinaryFunction()` while `task1` runs, but the ordinary function is not moved
to a worker.

Short-circuit operators remain short-circuiting. A task in an unevaluated
right branch is not submitted.

### 4.2 Implicit scopes

- A task-valued expression outside `DO PARALLEL` has an implicit expression
  scope which closes before the statement completes.
- `CALL taskRoutine` outside `DO PARALLEL` has an implicit statement scope and
  therefore completes before the next statement.
- Task calls in one `DO PARALLEL` share its explicit or implicit block scope.
- An early `RETURN`, `EXIT`, `LEAVE`, controller signal or failed ordinary
  expression requests cancellation and then joins before control escapes.

### 4.3 Dependency graph

The compiler may represent task expressions as a dependency graph, but the
graph is not public. Cycles are a compile-time error when statically visible.
Dynamic nested task waits from a task body fail immediately with
`#TASK_NESTED_WAIT`; they must not consume a bounded worker while waiting for
the same pool.

A direct self-call in a task body is ordinary synchronous recursion in the
current worker. The task modifier controls submission at the entry boundary;
it does not turn self-recursion into child submission. A call from one task
body to a different task (including a mutual-recursion edge between task
declarations) remains a nested task wait and is rejected in the first surface.

### 4.4 Target descriptor

A `.tasktarget` encodes this logical descriptor:

| Field | Contract |
| --- | --- |
| `kind` | `1` task procedure, `2` task method adapter, `3` `.taskwork` factory |
| `imageDigest` | 32-byte digest of the sealed executable generation contract |
| `callableId` | stable semantic-graph callable id within that image |
| `signatureDigest` | 32-byte digest of the canonical argument/result contract |
| `factoryArguments` | `ChannelValue` array; empty except for kind `3` |
| `adapterCallableId` | `0` for kind `1`; otherwise the sealed callable id plus one for the receiver `from_channel` factory (kind `2`) or `.taskwork.run` method (kind `3`) |

Local providers validate the current sealed generation and callable metadata.
Process providers load the same image digest before accepting work. A later
host registry may map the logical descriptor, but it must reject an unknown or
incompatible image/signature rather than dispatch by an unchecked string.

### 4.5 Task argument transfer shape

Primitive `.int`, `.boolean`, `.string` and `.binary` task arguments retain
their direct canonical RXCV nodes and the worker's primitive register path.
The runtime must not parse the callable signature on an ordinary primitive
cache hit merely to rediscover that shape.

A compiler-generated transferable object argument is internally marked by the
record schema `crexx.channel.task-value`, version `1`, with the single field
`value`. The field is the exact `ChannelValue` returned by `to_channel()`. The
marker is not a public wrapper object: after the sealed target has supplied the
trusted formal type, the worker extracts the field and reconstructs that exact
class through its statically resolved `from_channel` factory. A direct
`.channelvalue` formal receives the extracted canonical value as a
`.channelvalue` object.

The explicit Level B `.taskscope.submit(target, request)` and kind-3
`.taskwork.run(request, context)` contract is different: `request` is already
the canonical application value and crosses without the internal typed-object
marker. Collapsing these two paths would turn a taskwork integer request into a
record and is a conformance failure. Kind-2 receivers retain their separately
sealed receiver/factory path.

## 5. Exact Level B surface

The public F1 names are:

| Interface/class | Role |
| --- | --- |
| `.taskpool` | bounded provider capacity and shutdown |
| `.taskscope` | structured child ownership, policy, deadline and join |
| `.task` | one submitted child |
| `.tasktarget` | sealed statically resolved work descriptor |
| `.taskwork` | receiver-side advanced runnable contract |
| `.taskcontext` | receiver-side cancellation/deadline/trace context |
| `.completion` | terminal outcome, or `NONE` only for an observation timeout |
| `.channel` | provider-neutral lifecycle owner |
| `.channelrequest` | Level B wrapper around one local RXAS ticket |
| `.channelvalue` | immutable portable value tree |
| `.channelcodec` | exact domain-value encode/decode contract |
| `.byteendpoint` | reusable bounded byte source/sink/duplex endpoint |
| `.serviceref` | transferable logical single-owner service identity |
| `.transferbuffer` | mutable-owner, move and immutable-seal binary lifecycle |

The signatures are mechanically represented by the compile-only oracle linked
at the top of this document. That oracle is normative for spelling and return
types. These additional behavioral rules apply:

- `.taskpool.local(workers, admissionCapacity)` selects provider type `1`.
- `.taskpool.process(workers, admissionCapacity)` selects type `2`.
- A pool is an object created by a factory; it is not itself a factory.
- `.tasktarget.name()` is diagnostic text and `.signature()` is the canonical
  signature digest text. Dispatch uses the sealed numeric/digest descriptor in
  section 4.4, never the diagnostic name.
- `.taskscope.failfast(pool, timeoutMilliseconds)` and
  `.taskscope.collectall(...)` create a fresh child channel over the pool.
- Level B timeout arguments use milliseconds. `-1` means no observation or
  scope deadline, `0` means nonblocking/immediate and values below `-1` are
  invalid. The class converts positive values to checked RXAS microseconds.
- `.taskscope.next()` returns the next terminal child in completion order.
  `.join()` returns every child in stable submission order, including children
  previously observed through `next()` or `wait()`.
- A finite observation timeout returns `.completion` with `available() = 0`
  and state `NONE`. This sentinel is not attached to a task and is never part
  of `join()`.
- RXAS `WOULD_BLOCK` and `TIMEOUT` from an observation map to that sentinel;
  they do not raise `CHANNEL_ERROR`.
- `.task.completion()` and `.channelrequest.completion()` are nonblocking
  observations equivalent to `wait(0)`; `.terminal()` is true only after an
  available terminal completion has been cached.
- `.channelrequest` is the public wrapper around a raw ticket. Task scopes do
  not expose that wrapper; they expose `.task`, preventing a provider ticket
  from escaping structured ownership accidentally.
- `.task.identity()`, `.channelrequest.identity()` and
  `.completion.request_identity()` expose wrapper-assigned, non-authority
  correlation numbers. They never return the raw RXAS ticket capability stored
  inside the owning `.channel`.
- `.byteendpoint.provider_reference()` returns a validated logical provider
  reference. `.byteendpoint.from_reference()` opens an execution-local adapter
  to such a reference; it does not copy an OS handle.
- Factories and methods translate RXAS lifecycle misuse or open failure to the
  catchable `CHANNEL_ERROR` signal. Expected task terminal states remain data.

Implementations of these interfaces use the five channel instructions only.
An inspection test must reject an implementation containing an RXPA call or a
procedure-name string for task dispatch.

## 6. RXAS and RXBIN contract

### 6.1 Opcode allocation

The F0-S inventory was verified against parent `develop`
`b9cc911799f8dc9c7cb2895f49671901a563802b`: the dense table ends at opcode
`649`, RXBIN007 feature bits `0..2` are allocated, and the six process/redirect
operations occupy `466..471`.

F1 appends these public source opcodes to the current dense table:

| Opcode | Symbol | RXAS | Format |
| ---: | --- | --- | --- |
| `650` | `CHANOPEN_REG_REG_REG_REG_REG` | `chanopen rStatus,rChannel,rProviderType,rRequiredCapabilities,rConfiguration` | `RRRRR` |
| `651` | `CHANSTART_REG_REG_REG_REG_REG` | `chanstart rStatus,rTicket,rChannel,rEnvelope,rWaitMicroseconds` | `RRRRR` |
| `652` | `CHANWAIT_REG_REG_REG_REG` | `chanwait rStatus,rCompletion,rChannel,rWaitMicroseconds` | `RRRR` |
| `653` | `CHANCANCEL_REG_REG_REG_REG` | `chancancel rStatus,rChannel,rTicket,rReason` | `RRRR` |
| `654` | `CHANCLOSE_REG_REG_REG` | `chanclose rStatus,rChannel,rMode` | `RRR` |

All have `FLOW_NEXT | FLG_OPT_BARRIER`. F1d has migrated the compiler exit and
Level B adapters and changed the six old process/redirect slots `466..471` to
`RESERVED_466` through `RESERVED_471`; they are not reused. `spawn`,
`redir2str`, `redir2arr`, `str2redir`, `arr2redir` and `nullredir` are rejected
as retired source mnemonics. Repository and installed-development images using
the old pre-release opcodes must be rebuilt.

### 6.2 Runtime register types

| Instruction | Outputs | Inputs |
| --- | --- | --- |
| `chanopen` | status `.int`, channel `.int` | provider type `.int`, capability mask `.int`, canonical configuration `.binary` |
| `chanstart` | status `.int`, ticket `.int` | channel `.int`, canonical envelope `.binary`, wait microseconds `.int` |
| `chanwait` | status `.int`, canonical completion `.binary` | channel `.int`, wait microseconds `.int` |
| `chancancel` | status `.int` | channel `.int`, ticket `.int`, canonical reason `.binary` |
| `chanclose` | status `.int` | channel `.int`, close mode `.int` |

The assembler/link validator requires distinct output registers when an
instruction has two outputs. An output may alias an input. The VM snapshots all
inputs before changing any output so that such aliasing is deterministic.

On entry, the handler prepares failure defaults: status `INTERNAL_ERROR`,
channel/ticket `0`, and completion as empty binary. On normal return, status is
the final operation status. A non-`OK` result leaves the companion output at
its failure default. All integer output setters clear incompatible payload and
native/reference state; the completion setter owns an ordinary receiver-local
binary value.

The five operations do not raise a VM signal for malformed input, allocation
failure, provider failure, timeout or lifecycle misuse. They return an
operation status. This makes every instruction failure-atomic with no signal
continuation and no partial provider resource after failed `chanopen`.

### 6.3 Effect and signal sidecars

The initial effect entries are classified but opaque:

| Symbol | reads | writes | kills | semantics |
| --- | --- | --- | --- | --- |
| `CHANOPEN...` | `00111` | `11000` | `11000` | `RXOP_SEM_OPAQUE` |
| `CHANSTART...` | `00111` | `11000` | `11000` | `RXOP_SEM_OPAQUE` |
| `CHANWAIT...` | `0011` | `1100` | `1100` | `RXOP_SEM_OPAQUE` |
| `CHANCANCEL...` | `0111` | `1000` | `1000` | `RXOP_SEM_OPAQUE` |
| `CHANCLOSE...` | `011` | `100` | `100` | `RXOP_SEM_OPAQUE` |

Their signal contracts are classified `NONE`: no signal names, failure phase,
failure-visible partial writes or signal continuations. The reserved old slots
use reserved effect and signal entries.

No peephole, CFG, SSA or inliner rule may move, duplicate, remove or fold a
channel instruction from these initial contracts. TRACE, profiler and debugger
identity is the canonical opcode and original source anchor in both `rxbvm`
and `rxtvm`.

### 6.4 Feature gate

`RXBIN007_FEATURE_CHANNELS` is bit `1 << 3`. The supported-feature mask includes
it in F1.

- A writer sets the bit when any opcode `650..654` is present.
- A reader rejects a channel opcode when the bit is absent.
- A pre-F1 reader rejects the unknown feature bit.
- A reader rejects every reserved opcode. Slots `466..471` join that set only
  at F1d closure; transition builds still recognize their old operations.
- Linker feature flags are the validated union of input requirements.
- RXAS/RXDAS round trips retain all five mnemonics and operand order.

At F1d, all repository, installed-development and test RXBIN containing the old
six operations must be rebuilt. There is no compatibility reader which
silently maps old process instructions to the new protocol.

## 7. Constants

### 7.1 Provider type codes

| Code/range | Provider |
| ---: | --- |
| `0` | invalid/unset |
| `1` | core local-thread task/service provider |
| `2` | core isolated-process task/service provider |
| `3` | reserved core open-host provider for F2 |
| `4` | core bounded byte-endpoint provider |
| `5` | core structured child-process provider |
| `6..65535` | reserved CREXX core providers |
| `65536..INT64_MAX` | registered extension providers |
| negative | invalid/reserved |

Type `0` never means a default. Level B factories choose explicitly.

### 7.2 Required capability bits

| Bit | Hex | Meaning |
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

Bits `10..62` are reserved and bit `63` is invalid because Level B `.int` is
signed. Unknown required bits fail `chanopen`; providers may offer more than
the required mask.

### 7.3 Operation status codes

| Code | Name | Meaning |
| ---: | --- | --- |
| `0` | `OK` | operation completed as requested |
| `1` | `INVALID_ARGUMENT` | invalid range, mode or combination |
| `2` | `INVALID_VALUE_TYPE` | register payload has the wrong runtime type |
| `3` | `INVALID_PROVIDER` | provider code is invalid or unregistered |
| `4` | `PROVIDER_UNAVAILABLE` | known provider cannot run on this host/runtime |
| `5` | `UNSUPPORTED_CAPABILITY` | required capability is absent |
| `6` | `INVALID_CONFIGURATION` | malformed value or provider schema violation |
| `7` | `INCOMPATIBLE_VERSION` | unsupported value, schema or provider version |
| `8` | `RESOURCE_EXHAUSTED` | bounded table, memory or OS resource unavailable |
| `9` | `BACKPRESSURE` | admission/buffer bound reached under reject policy |
| `10` | `WOULD_BLOCK` | nonblocking observation has no result |
| `11` | `TIMEOUT` | finite operation wait elapsed without a result |
| `12` | `CHANNEL_CLOSED` | operation is not accepted in closing/closed state |
| `13` | `STALE_CAPABILITY` | slot generation no longer matches |
| `14` | `WRONG_OWNER` | capability belongs to another execution |
| `15` | `UNKNOWN_TICKET` | ticket is not owned by this channel |
| `16` | `ALREADY_TERMINAL` | cancellation raced after terminal publication |
| `17` | `PROVIDER_FAILURE` | provider could not perform the operation |
| `18` | `SHUTTING_DOWN` | runtime/provider is no longer admitting work |
| `19` | `UNSUPPORTED_OPERATION` | envelope operation is not supported |
| `20` | `INTERNAL_ERROR` | invariant failure; diagnostic must be retained |

Status values `21..INT64_MAX` are reserved. Unknown statuses are treated as
`INTERNAL_ERROR` by Level B.

### 7.4 Completion states

| Code | State |
| ---: | --- |
| `0` | `NONE` — Level B observation sentinel only, never provider terminal state |
| `1` | `SUCCEEDED` |
| `2` | `FAILED` |
| `3` | `CANCELLED` |
| `4` | `DEADLINE_EXCEEDED` |
| `5` | `REJECTED` |
| `6` | `ENDPOINT_CLOSED` |
| `7` | `TRANSPORT_LOST` |
| `8` | `UNKNOWN_OUTCOME` |

### 7.5 Other stable codes

- wait microseconds: `-1` forever, `0` nonblocking, positive relative wait,
  values below `-1` invalid;
- close mode: `1` drain, `2` cancel;
- scope failure policy: `1` fail-fast, `2` collect-all;
- byte direction: `1` readable, `2` writable, `3` duplex.

Wait values are relative operation budgets. Providers use their own monotonic
clock. `MTIME` is time-of-day and must not be used for Gate F deadlines.

## 8. Capability and provider model

### 8.1 Local capability encoding

Channel and ticket values are positive signed 64-bit integers with this opaque
layout:

```text
63       must be zero
62..33   execution owner id (30 bits, nonzero)
32       kind: 0 channel, 1 ticket
31..16   slot generation (16 bits, nonzero)
15..0    slot plus one (16 bits, nonzero)
```

The process allocates owner ids monotonically and never reuses one. Exhaustion
fails creation rather than wrapping. A slot whose generation would wrap is
retired. Each execution therefore supports at most 65,535 simultaneously live
channels and 65,535 live tickets; providers may impose lower bounds.

Every operation validates sign/reserved bits, owner, kind, slot, generation,
channel relationship and lifecycle before provider access. The integer is not
a pointer, OS handle, worker number or provider payload. Copying it copies only
the same local authority; `chanclose` invalidates every copy.

Raw local capabilities are not valid task arguments or results. The private
`ChannelValue` local-capability tag is accepted only by an exact local
provider-configuration schema, such as a task scope referring to its pool.

### 8.2 Logical provider references

A provider may export a logical reference through a completion. Such a
reference contains provider type, reference version, rights, scope and opaque
identity/authentication bytes. It contains no VM or OS pointer.

Scopes are:

- `1` runtime: reopenable by executions attached to the same RXVM runtime;
- `2` process: reopenable by registered providers in the same host process;
- `3` host: transport/provider-defined and subject to F2 authentication.

The receiving provider validates identity, rights, expiry and scope. Byte
endpoints use runtime references in F1 so a task execution can open a local
adapter and stream bytes without accessing the controller's VM values.

### 8.3 Registry

The provider registry is owned by `rxvm_runtime` and synchronized independently
of any Rexx execution. Core providers register during runtime creation.
Registration includes code, canonical name, internal ABI version,
configuration versions, capability mask and complete operation table.

Duplicate code/name, an unknown ABI, missing operation or impossible
capability claim fails registration atomically. Registry lookup pins the
descriptor and its module before `open` returns. A channel retains that pin
until logical close and any quarantined physical request can no longer invoke
the provider.

F1 exposes only a private registration function plus a fake-provider test.
The installed plugin descriptor is specified and reviewed in F2; it does not
change this RXAS contract.

Providers receive copied canonical binaries and core capability identities.
They do not retain a caller register or `value *`. Provider callbacks publish
completion into the core-owned bounded completion queue and never call Rexx
while holding a registry or channel-table lock.

## 9. ChannelValue binary encoding

### 9.1 Document header

Every RXAS configuration, envelope, reason and completion is a canonical
`.binary` document:

```text
offset  size  field
0       4     ASCII "RXCV"
4       1     major version = 1
5       1     minor version = 0
6       2     little-endian document flags
8       8     little-endian total byte length, including header
16      ...   exactly one root node
```

Flag bit `0` states that a logical provider-reference node exists. Bit `1`
states that a local-capability node exists. Other bits are zero. A decoder
recomputes both bits and rejects a mismatch, trailing data, nonzero reserved
fields, overflow, excessive nesting or a noncanonical node.

The F1 hard maximum is 16 MiB per document, nesting depth 64 and 1,048,576
container members. A provider may advertise lower limits. Streaming data uses
bounded chunks instead of increasing this limit.

### 9.2 Node framing

Every node is:

```text
tag:u8 flags:u8 reserved:u16 payloadLength:u64 payload[payloadLength]
```

Integers are little-endian. Node flags and reserved bits are zero in version 1.

| Tag | Value | Payload |
| ---: | --- | --- |
| `0` | null | empty |
| `1` | false | empty |
| `2` | true | empty |
| `3` | integer | signed 64-bit two's-complement |
| `4` | float | IEEE-754 binary64; all NaNs use `0x7ff8000000000000` |
| `5` | decimal | sign byte, signed 32-bit exponent, 64-bit digit count, one byte per digit |
| `6` | string | valid UTF-8 bytes |
| `7` | binary | uninterpreted bytes |
| `8` | array | 64-bit count followed by that many framed nodes |
| `9` | record | schema, version and sorted named fields as below |
| `10` | provider reference | provider/reference/rights/scope/identity record |
| `11` | local capability | signed 64-bit local capability plus expected kind/provider fields |

Decimal digits are `0..9`. Nonzero values have no leading or trailing zero
digit; the exponent is adjusted accordingly. Zero is sign `0`, exponent `0`,
count `1`, digit `0`. Negative zero is not encoded.

A record payload is:

```text
schemaLength:u64 schema:utf8 schemaVersion:u32 fieldCount:u64
repeat fieldCount times:
    nameLength:u64 name:utf8 value:node
```

Schema and field names are nonempty. Field names are unique and strictly
ascending by unsigned UTF-8 byte sequence. Record construction sorts fields;
decoding rejects unsorted or duplicate fields. Unknown required fields reject
the schema. A schema may explicitly designate additional fields as optional.

Provider-reference tag `10` contains provider type `i64`, reference version
`u32`, rights `u32`, scope `u8`, seven zero bytes, identity length `u64` and
opaque identity bytes. Local-capability tag `11` contains capability `i64`,
expected kind `u8`, seven zero bytes and provider type `i64`.

Public `.channelvalue` factories do not construct tags `10` or `11`; providers
and the Level B systems implementation do. Forged bytes still fail provider or
local capability validation.

Moved and sealed transfer buffers retain the same logical tag `7` encoding.
Copy, move, immutable sharing or mapping is an implementation/lifecycle choice,
not a different receiver-visible value.

### 9.3 TransferBuffer lifecycle

`.transferbuffer.allocate(capacity)` creates one mutable, execution-owned,
zeroed buffer with size zero and fixed positive capacity. `.copy_from(bytes)`
creates one with size and capacity equal to the byte length. `write(offset,
bytes)` requires a nonnegative offset and an end no greater than capacity; it
zero-fills any gap and sets size to the greater of its old value and the write
end. `read(offset, length)` requires a nonnegative range wholly within size and
returns an independent `.binary` snapshot.

- `move_value()` transfers the bytes into a `.channelvalue` and changes the
  source to `MOVED`. `mutable()` becomes false; later read, write, move or seal
  operations raise `CHANNEL_ERROR` with a moved-buffer status.
- `seal()` freezes the complete current byte range, returns an immutable
  `.channelvalue` and changes the source to `SEALED`. Reads remain valid;
  writes and moves fail. Repeated `seal()` returns the same logical bytes.
- A move may degrade to a physical copy across a process/host boundary. A seal
  may use reference counting or a read-only mapping. Neither choice changes the
  receiver's binary value or permits simultaneous mutable access.

No slice/view outlives its sealed backing. F1c may begin with copying while
retaining these source-visible states; copy/share thresholds remain an F3
measurement decision.

## 10. Standard schemas

All names below are exact, version `1`, and record field order is canonical
sorting rather than the presentation order in these tables.

### 10.1 Task pools and scopes

`crexx.channel.local-task-pool` and
`crexx.channel.process-task-pool` contain:

| Field | Type | Rule |
| --- | --- | --- |
| `admissionCapacity` | integer | `1..65535` |
| `workerCount` | integer | `1..65535`, provider/host may reject lower |

`crexx.channel.task-scope` contains:

| Field | Type | Rule |
| --- | --- | --- |
| `failurePolicy` | integer | `1` fail-fast or `2` collect-all |
| `pool` | local capability | open pool channel owned by this execution |
| `timeoutMicroseconds` | integer | `-1` unlimited, `0` immediate, positive bounded |

Opening a scope channel records its deadline from the provider's monotonic
clock. Every task ticket in the scope observes that same deadline; later
submissions do not receive a fresh full duration.

### 10.2 Task invocation

`crexx.channel.task-invoke` contains:

| Field | Type |
| --- | --- |
| `arguments` | array of transferable values |
| `target` | `crexx.channel.task-target` record from section 4.4 |

The provider assigns a monotonically increasing submission sequence within
the scope. Worker selection is private. The envelope contains no worker id or
affinity hint in F1.

`crexx.channel.service-ask` contains a validated provider reference named
`service`, a transferable `request` value and an optional idempotency key. The
provider serializes accepted calls for one service identity.

### 10.3 Completion

`crexx.channel.completion` contains:

| Field | Type |
| --- | --- |
| `details` | record or null |
| `errorCode` | integer |
| `message` | string |
| `providerType` | integer |
| `result` | transferable value or null |
| `sequence` | integer submission sequence |
| `state` | integer completion state |
| `ticket` | integer local ticket capability |

`result` is non-null only when permitted by the target return contract.
`message` is diagnostic and not a stable program branch key. `errorCode`,
state and typed `details` own programmatic interpretation.
The Level B channel consumes the internal `ticket`, resolves its owned
`.channelrequest` and exposes only the wrapper's non-authority
`request_identity`. The raw value is not returned by a public accessor.

### 10.4 Byte endpoints

Provider type `4` accepts:

- `crexx.channel.byte-memory`: direction, positive capacity and optional
  initial binary snapshot;
- `crexx.channel.byte-null`: direction; and
- `crexx.channel.byte-reference`: validated logical provider reference.

Request schemas are:

- `crexx.channel.byte-read` with positive `maximumBytes`;
- `crexx.channel.byte-write` with bounded binary `bytes`;
- `crexx.channel.byte-drain`;
- `crexx.channel.byte-half-close` with direction; and
- `crexx.channel.byte-export-reference` with requested rights/scope.

Read completion returns one bounded binary chunk, EOF in typed details, or a
terminal endpoint failure. Write completion reports accepted byte count. A
write never claims bytes not accepted into owned endpoint storage.

Input strings/arrays are converted to independent bytes before asynchronous
work. Output is accumulated only in endpoint-owned storage and materialized
into Rexx values when the controller drains it. An I/O thread never retains a
destination Rexx value.

### 10.5 Child process

Provider type `5` opens with `crexx.channel.child-process-provider`, containing
bounded concurrent-child capacity. A `crexx.channel.child-process-start`
envelope contains executable, argument array, environment record, logical
working directory, stdin/stdout/stderr endpoint references and a provider
deadline policy.

The completion contains launch state, exit status or terminating signal,
ambiguous-outcome flag and independent standard-stream terminal diagnostics.
Command strings are not shell-parsed unless the selected ADDRESS environment
explicitly chooses a shell provider.

## 11. Lifecycle, failure and cancellation

### 11.1 Channel state

```text
OPEN --chanclose(drain)--> CLOSING_DRAIN --> CLOSED
OPEN --chanclose(cancel)-> CLOSING_CANCEL -> CLOSED
OPEN --provider failure------------------> CLOSING_CANCEL -> CLOSED
```

`chanstart` is accepted only in `OPEN`. `chanwait` and `chancancel` remain valid
while closing. `DRAIN` stops admission and accounts for accepted requests.
`CANCEL` additionally requests cancellation of every nonterminal request.

`chanclose` is the release operation and invalidates the channel capability on
success. It has no wait operand. The Level B owner first applies its bounded
wait/cancel policy. For an uncooperative local native call, logical completion
may be published and the worker quarantined; the runtime retains the provider
pin and physical resource until the call returns. Final runtime destruction
joins remaining physical resources.

### 11.2 Ticket state

```text
ACCEPTED -> QUEUED -> RUNNING -> one terminal state
             |          |
             +----------+--> CANCELLED / DEADLINE_EXCEEDED
```

Submission rejection returns no ticket and is represented by operation status
for low-level `.channel`, or a synthetic `REJECTED` task completion at the
structured Level B layer. Every accepted ticket reaches exactly one state from
section 7.4.

`chanwait` returns and consumes the next unobserved provider completion in
completion order. The Level B scope caches it by ticket. A later
request-specific wait uses that cache, and `join()` retains a stable
submission-ordered copy. RXAS does not promise repeated delivery.

Consumption is failure-atomic at the VM boundary. The provider completion is
not marked observed until its canonical RXCV bytes have been materialized in
controller-worker-owned storage. If encoding or receiver allocation fails,
`chanwait` returns `RESOURCE_EXHAUSTED` and leaves that completion available for
a later observation; it must not silently lose the terminal result.

### 11.3 Races

Terminal publication uses one compare/exchange or an equivalent locked state
transition. A completion already published wins. For events observed together
before publication, the F1 priority is cancellation, scope deadline, forced
provider termination, then shutdown. Remote disconnect maps to
`TRANSPORT_LOST` only when non-execution is established; otherwise it maps to
`UNKNOWN_OUTCOME`.

Cancellation is cooperative for type `1`. It sets the worker's existing Gate E
interrupt path and becomes terminal at a safe request boundary. Type `2` may
terminate an isolated process after cooperative grace. No implementation may
force-terminate a shared in-process OS thread.

### 11.4 Source-visible failure

- Explicit `.task` and `.completion` users receive terminal states as data.
- Typed task-call sugar raises `TASK_FAILURE` when a failed result is demanded
  or when its scope closes.
- `TASK_FAILURE` carries task identity, completion state, stable error code,
  message, typed details and the complete stable completion set for a group.
- Fail-fast requests sibling cancellation but joins every child before the
  signal escapes.
- Invalid class lifecycle use and RXAS operation failure in the Level B wrapper
  raises `CHANNEL_ERROR` with operation, status and provider diagnostics.

### 11.5 Pool, scope and service lifecycle

A pool is `OPEN`, `CLOSING` or `CLOSED`. Only `OPEN` pools create scopes.
`pool.close()` stops scope admission, requests cancellation of every still-open
scope, accounts for their accepted children and releases the provider channel.
It therefore cannot strand a task merely because its caller omitted
`scope.close()`. A second close or later factory/use operation raises
`CHANNEL_ERROR`; cached terminal completion values remain ordinary values.

A scope is `OPEN`, `CANCELLING`, `JOINED` or `CLOSED`:

```text
OPEN --cancel/fail-fast/deadline--> CANCELLING --all terminal--> JOINED
OPEN -------------------------------all terminal/join---------> JOINED
JOINED -------------------------------------------------------> CLOSED
OPEN/CANCELLING --close--> account every child --> CLOSED
```

Only `OPEN` accepts `submit` or `ask`. `cancel()` is idempotent while children
remain nonterminal. `join()` may be repeated and returns the same stable
submission-ordered completion array. `close()` performs the required
cancel/join work if necessary and consumes the scope; a later operation raises
`CHANNEL_ERROR`. `DO PARALLEL USING scope` performs that consuming close at
`END` or any control escape. It never closes the pool.

Fail-fast changes `OPEN` to `CANCELLING` on the first non-success terminal
child and requests cancellation of the remainder. Collect-all does not cancel
siblings for an ordinary child failure. Both policies still account for every
accepted child. Submission backpressure before a ticket exists becomes one
synthetic `REJECTED` child completion so structured source retains one outcome
per attempted child.

One service identity has a monotonically increasing accepted-call sequence and
at most one running call. A queued cancellation reaches terminal without
running; a running cancellation is cooperative. The next accepted call does
not begin until the current call publishes its one terminal completion, so
mutable service state has one authoritative order. A `.serviceref` transfers
only logical identity and rights and does not keep a caller execution alive.

The execution-local default pool is lazily created with runtime policy and
closed during execution teardown. Its capacity is not a source guarantee;
programs requiring fixed capacity, admission bounds or failure policy create
an explicit pool and scope.

## 12. Existing-instruction disposition

### 12.1 Retire in F1d

| Current opcode(s) | Disposition |
| --- | --- |
| `spawn` | replace with provider type `5` open/start/wait/close |
| `redir2str`, `redir2arr` | type `4` writable endpoint plus controller drain adapter |
| `str2redir`, `arr2redir` | snapshotted type `4` readable endpoint |
| `nullredir` | type `4` null endpoint |

`compiler/exits/address/Address.crexx` and
`lib/rxfnsb/rexx/_address.crexx` are updated together. Existing Rexx ADDRESS
source behavior is retained, but generated RXAS uses the channel family.

### 12.2 Retain as synchronous owner-local primitives in F1

| Family | Reason and restriction |
| --- | --- |
| `sock*` TCP/TLS | context-registry-owned synchronous socket primitive used inside one execution; it cannot be transferred as `ChannelValue`; asynchronous scheduling/streaming uses tasks and type `4` endpoints |
| `fopen` through `ferror` | synchronous Level B file primitive confined to its owning execution; raw handle integers are forbidden in task values and provider schemas |
| `time`, `mtime`, `xtime` | synchronous scalar queries; they do not own asynchronous lifecycle; Gate F deadlines use provider monotonic clocks instead |
| `say*`, `readline` | synchronous current-execution console operations |

No new asynchronous file, socket, timer, database or HTTP opcode family is
permitted. A future need for provider-managed socket/file/timer lifecycle uses
a new core or extension provider type behind the same five operations and must
pass its own compatibility gate.

## 13. HTTP/TLS acceptance contract

F1g builds HTTP as an industrial consumer, not an opcode. It uses:

- type `1` or `2` task/service channels for concurrency and connection-owner
  services;
- existing owner-local TCP/TLS operations inside the owning execution;
- type `4` logical endpoint references for request/response streaming; and
- bounded `ChannelValue` metadata for request and response headers/status.

Required behavior is:

1. bounded global and per-origin concurrency;
2. per-origin reusable connections owned by a service, never a shared socket
   integer passed between workers;
3. DNS, connect, TLS, request, response and total deadlines;
4. certificate-chain and hostname verification by default;
5. bounded header count/bytes and bounded non-streamed response bodies;
6. chunked and fixed-length streaming without mandatory whole-body buffering;
7. explicit redirect and retry policy, including non-idempotent and ambiguous
   outcome rules;
8. partial read/write, early EOF, cancellation and teardown tests; and
9. concurrent `crexx-rag` generation/embedding-style integration evidence.

F1g-B implements the ownership/reuse subset and F1g-C completes the buffered
policy subset through `rxfnsg.httpclient.pooled(origin, connections, admission,
maximum_response, ?policy)`.
The transferable proxy carries immutable configuration and one type-4
admission reference. Each 192-byte admission frame is exactly two canonical
92-byte type-4 provider-reference documents plus one 8-byte request length;
each request and response body uses its own bounded endpoint. One long-lived
kind-3 `.taskwork` target owns each reusable socket and processes requests
serially on that connection. `post(path, body, headers)` is a task method
returning concrete `.httpresponse`; `.httpheaders` and `.httppolicy` use exact
`ChannelValue` records and receiver-side revalidation. Automatic replay is off;
connect-only retry may precede a send, while post-send/status retry and
same-origin 307/308 require `Idempotency-Key`. Cross-origin redirects and POST
method rewrites are refused. Attempts, followed redirects and any ambiguous
delivery history survive in the response. The current atomic socket connect
operation uses the sum of DNS/connect/TLS phase budgets; request/response
operations use their phase budgets, controller observations use the configured
completion budget, and the containing task scope owns the strict whole-task
monotonic deadline. No live endpoint object or socket integer crosses an
execution. Only the controller client closes admission and joins owners.
F1g-D adds `.httpbody.fixed`, `.httpbody.chunked` and
`.httpclient.post_stream`. Request and response streams use paired bounded
type-4 byte endpoints; response metadata carries only the encoded endpoint
reference. Buffered `post` advertises gzip/deflate and decodes RFC
1950/1951/1952 streams in pure Level B with the configured decoded-body ceiling.
The four-mode `crexx-rag` fixture exercises concurrent generation and embedding
requests, authorization/idempotency headers, request shapes and gzip/deflate
responses. No HTTP opcode or provider type is added.

## 14. Diagnostics

The compiler diagnostics are stable names even if presentation text changes:

| Diagnostic | Condition |
| --- | --- |
| `#TASK_ONLY_LEVELG` | task declaration or imported task call outside Level G |
| `#TASK_TARGET_ONLY_LEVELG` | explicit task-target expression outside Level G |
| `#PARALLEL_ONLY_LEVELG` | statement or expression `DO PARALLEL` outside Level G |
| `#TASK_EXPOSED_ARGUMENT` | `ARG EXPOSE` in task signature |
| `#TASK_REFERENCE_TYPE` | reference argument/result/receiver |
| `#TASK_NONTRANSFERABLE_TYPE` | value has no exact transfer contract |
| `#TASK_NONTRANSFERABLE_RECEIVER` | task method receiver is not a validated proxy |
| `#TASK_DYNAMIC_TARGET` | string/procedure variable/dynamic factory target |
| `#TASK_TARGET_SIGNATURE_MISMATCH` | target metadata differs from call contract |
| `#TASK_NESTED_WAIT` | task body attempts a task join |
| `#PARALLEL_SCOPE_TYPE` | `USING` is not `.taskscope` |
| `#PARALLEL_SCOPE_REUSED` | consumed/closed scope reused |
| `#PENDING_TASK_RESULT_MUTATION` | write/reference/escape before materialization |
| `#TASK_RESULT_CYCLE` | static dependency cycle |

RXAS/RXBIN diagnostics must distinguish retired mnemonic, reserved opcode,
missing `RXBIN007_FEATURE_CHANNELS`, unknown feature bit, wrong arity, duplicate
output register, wrong runtime value type, malformed RXCV and stale/wrong-owner
capability.

## 15. Conformance vectors

These vector ids are stable test names. F1 adds executable fixtures for both
optimized and unoptimized RXAS and both concrete VMs where applicable.

### 15.1 Positive

| ID | Required result |
| --- | --- |
| `GF-P01` | `task1() + task2()` submits in source order, overlaps two ready bodies and joins before addition |
| `GF-P02` | ordinary function in mixed expression runs on controller while independent task may run |
| `GF-P03` | short-circuit false branch submits no task |
| `GF-P04` | `DO PARALLEL` result binding materializes on read and all children join at `END` |
| `GF-P05` | `CALL` tasks overlap only when they share a parallel scope |
| `GF-P06` | local pool capacity one preserves results without parallel execution |
| `GF-P07` | `scope.next()` completion order differs from stable `join()` submission order without losing entries |
| `GF-P08` | byte endpoint streams bounded chunks and reports EOF/half-close |
| `GF-P09` | runtime-scoped endpoint reference reopens in an attached worker without a VM pointer |
| `GF-P10` | ADDRESS output/error behavior is unchanged after generated RXAS migration |

### 15.2 Negative/lifecycle

| ID | Required result |
| --- | --- |
| `GF-N01` | exposed/reference task argument is rejected at compile time |
| `GF-N02` | dynamic string target is rejected |
| `GF-N03` | mutation/reference escape of a pending binding is rejected |
| `GF-N04` | nested task wait fails immediately rather than deadlocking |
| `GF-N05` | queue full returns bounded rejection/backpressure and creates no ticket leak |
| `GF-N06` | wrong-owner, wrong-kind and stale handles return their exact statuses |
| `GF-N07` | repeated cancel after terminal returns `ALREADY_TERMINAL` and does not republish |
| `GF-N08` | scope deadline expires queued/running children exactly once |
| `GF-N09` | controller early exit cancels/joins all children |
| `GF-N10` | endpoint close with active I/O leaves no live Rexx destination in an I/O thread |
| `GF-N11` | a marked typed argument beyond the sealed formal list fails setup without entering the task or reading uninitialised metadata |

### 15.3 RXAS/RXBIN and malformed data

| ID | Required result |
| --- | --- |
| `GF-B01` | each new mnemonic round-trips RXAS -> RXBIN -> RXAS |
| `GF-B02` | channel opcode without feature bit is rejected |
| `GF-B03` | unknown feature bit is rejected by an older reader fixture |
| `GF-B04` | at F1d closure, opcodes `466..471` are rejected as reserved |
| `GF-B05` | at F1d closure, the old six source mnemonics are rejected as retired |
| `GF-B06` | duplicate output registers are rejected |
| `GF-B07` | malformed/truncated RXCV, noncanonical record and trailing bytes fail atomically |
| `GF-B08` | unknown required capability bit fails `chanopen` with no live resource |
| `GF-B09` | fake extension provider registers, conforms, pins and unloads safely |
| `GF-B10` | metadata/effect/signal inventories contain exactly 655 aligned entries |
| `GF-B11` | an output register may alias an input and receives the result only after every input has been consumed |

## 16. Coherence matrix

| Approved user-guide rule | Specification owner | Executable oracle/vector |
| --- | --- | --- |
| ordinary callable versus task | sections 3.2, 4.1 | `GF-P01`, `GF-P02` |
| `task1() + task2()` overlap with ordinary typed result | sections 3.5, 4.1 | `GF-P01` |
| non-task clauses stay in controlling execution | sections 3.4, 4.1 | `GF-P02` |
| `CALL` task semantics | section 4.2 | `GF-P05` |
| `DO PARALLEL`, pending bindings and join at `END` | sections 3.4, 3.5, 4.2 | `GF-P04`, `GF-N03`, `GF-N09` |
| pool versus scope and factory model | sections 5, 10.1 | Level B declaration oracle |
| `.taskwork` factory target | sections 3.3, 4.4, 5 | Level B declaration oracle, `GF-N02` |
| complete Level B control | section 5 | `gate_f_levelb_contract` CTest |
| RXAS-only core bridge | sections 2, 6 | `GF-B01`, `GF-B11`, classlib inspection test |
| `chanopen` provider type plus capability flags | sections 6.2, 7.1, 7.2 | `GF-B08` |
| no live VM values cross | sections 1, 8, 9 | `GF-P09`, `GF-N06` |
| typed object arguments and direct taskwork requests stay distinct | sections 4.5, 5 | imported task-method tests, `GF-N11` |
| structured terminal completion and `TASK_FAILURE` | sections 7.4, 11 | `GF-P07`, `GF-N07..N09` |
| stateless tasks and single-owner services | sections 1, 10.2, 11.5, 13 | service ordering tests |
| reusable redirects/endpoints | sections 10.4, 12.1 | `GF-P08`, `GF-P10`, `GF-N10` |
| pre-release RXAS retirement | sections 6.1, 6.4, 12 | `GF-B02..B05` |
| future provider plugins without opcodes | sections 7.1, 8.3 | `GF-B09` |
| concurrent industrial HTTP | section 13 | crexx-rag integration matrix |

No approved user-guide rule is left solely as an implementation convention.

## 17. F1 implementation boundary

F1a implements the opcode/feature/metadata contracts while retaining the
old six operations only as a buildable transition. F1d migrates their consumers
and then performs the reserved-slot retirement.

The completed F1c slice extends core provider type `1` over the complete RXCV
tree and typed register-image executor contract. Its runtime-owned registry
validates private descriptors, pins their modules and passes fake-provider
vector `GF-B09`. The local provider implements bounded admission,
cancellation, provider-owned deadlines and completion-order observation. The
five RXAS instructions execute on both concrete VMs with generation-checked
local capabilities, exact terminal accounting and deterministic teardown.

The Level B implementations in `Concurrency.crexx` expose the locked pool,
scope, task, target, context, completion, channel, value/codec, endpoint,
service-reference and transfer-buffer interfaces. The only runtime bridge is
the five RXAS instructions. Task targets are sealed numeric/digest descriptors;
no Rexx procedure-name string or RXPA task API occurs on the dispatch path.

Deliberately reserved operations remain failure-visible. `.taskscope.ask`,
`.taskcontext.endpoint` and pool statistics return `UNSUPPORTED_OPERATION`
rather than plausible placeholder data. F1d supplies concrete endpoint and
child-process integration, F1e the process provider, F1f kind-1 task
procedures, kind-2 transferable task methods, kind-3 `.taskwork` factories and
the gated Level G lowering, and F1g the concurrent HTTP consumer.
No public provider-plugin ABI is implied before F2.

### 17.1 F1e isolated-process provider contract

Core provider type `2` advertises `0x010f`: bounded admission, cancellation,
provider-owned deadlines, completion-order observation and isolated task
execution. It accepts the exact version-1 process-pool, task-scope and
task-invoke RXCV schemas already specified here and returns the same canonical
completion schema as type `1`. Its transport is private and versioned; F1e's
frame kinds are `READY`, `INVOKE`, `STARTED`, `RESULT`, `CANCEL` and
`SHUTDOWN`. Neither the framing nor the hidden worker command is an installed
ABI or an F2 host protocol.

Opening a process pool seals the controller's current bytecode-only program
generation into a temporary RXBIN archive. If the generation contains several
semantic graphs, the snapshot must preserve them as separate concatenated 007
containers and group all modules sharing each graph. It must not rebuild one
combined graph, because doing so can renumber numeric callable/member IDs and
invalidate an already sealed `.tasktarget`. A generation containing a native
module is not process-eligible. No live VM value, reference, frame, mutable
overlay, native payload, address or OS handle may enter the snapshot or frame
payload.

Each pool owns a bounded warm process set and admitted request count. Process
reuse is permitted only with a fresh executor and fresh VM context for each
task; module globals, registers, frames, references and cancellation state must
not spill between requests. Payloads and the provider's private byte endpoints
remain bounded. Pool close joins all worker/monitor threads, closes the
protocol endpoints and removes the temporary snapshot.

Cancellation and deadlines request the private cooperative interrupt first.
After 250 ms without a terminal result, the provider may terminate only the
isolated worker process and must replace it before further work. A transport
loss before `STARTED` is completion state `TRANSPORT_LOST`; after `STARTED` it
is `UNKNOWN_OUTCOME`. The existing exactly-once terminal priority applies to
all result/cancel/deadline/disconnect races. On POSIX, closure of the private
protocol pipe must be handled at that write boundary; an implementation must
not change the controller or host process's global `SIGPIPE` disposition.

### 17.2 F1f compiler and sealed-binding contract

F1f admits `task` declarations, explicit task-target expressions and both
forms of `DO PARALLEL` only when the source selects Level G. The compiler emits
`#TASK_ONLY_LEVELG`, `#TASK_TARGET_ONLY_LEVELG` or `#PARALLEL_ONLY_LEVELG`
outside that level. The contextual words remain available as ordinary Level B
identifiers, and the Level B concurrency classes remain directly usable.

The compiler lowers task calls to one controller-owned dependency plan. It
preserves left-to-right argument evaluation and submission, ordinary
short-circuit suppression, pending-binding restrictions, structured cleanup
and join, and synchronous execution of non-task clauses. Direct task
self-recursion is emitted as an ordinary same-worker recursive call. An edge
from one task declaration to another remains a rejected nested task wait.

RXBIN carries an 80-byte sealed task binding: image digest, callable id,
signature digest and an adapter slot containing zero or callable id plus one.
The assembler resolves definition-local placeholders. Imported task calls keep
the same deterministic relocation placeholder; the RXBIN writer/linker finds
every matching use-site constant across selected pools and reseals it against
the final linked graph. Imported class-contract stubs preserve the task-method
kind, so a call cannot silently degrade to an ordinary synchronous method. The
runtime validates the final binding before dispatch. A runtime may retain the
immutable graph digest and a bounded resolved plan only within the worker that
performed that validation. Its cache key must contain the complete binding and
requested result mode; misses and all failed validations follow the full path,
and worker/context teardown invalidates every retained pointer. Kind `2`
reconstructs its
receiver through the sealed `from_channel` factory; kind `3` constructs the
factory target in the receiver and invokes the sealed `run` adapter. Factory
arguments, requests and results cross only as `ChannelValue` data.

Imported task-method and `.taskwork` tests compile the provider library and
client separately, assert the task-lowered RXAS shape, assemble both, link them
with classlib/library, and execute
optimized/unoptimized images on `rxbvm` and `rxtvm`. This is the standing
toolchain rule for library development: exercise `rxc`, `rxas`, `rxlink` and
`rxvm`, not only the final class-library runtime.

Evidence is retained in the
[`F1a/F1b closeout`](evidence/2026-08-14-perf3-13-gate-f-f1ab-first-release-verdict/)
[`F1c closeout`](evidence/2026-08-14-perf3-13-gate-f-f1c-first-release-verdict/),
[`F1d closeout`](evidence/2026-08-15-perf3-13-gate-f-f1d-first-release-verdict/)
and
[`F1e closeout`](evidence/2026-08-15-perf3-13-gate-f-f1e-first-release-verdict/),
and the F1f evidence is retained in
[`F1f closeout`](evidence/2026-08-15-perf3-13-gate-f-f1f-first-release-verdict/).
F1g-D and the final local Gate F implementation evidence are retained in
[`F1g-D closeout`](evidence/2026-08-15-perf3-13-gate-f-f1g-d-streaming-integration-first-release-verdict/).

After the first production edit, run the minimum focused correctness checks,
freeze code, build ordinary profiling-off Release, run the smallest decisive
retained-baseline comparison, report to Adrian and stop. Do not commit that
production slice until its verdict is accepted and closeout QA makes the step
complete.

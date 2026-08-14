# PERF3-13 Gate F implementation plan

Date: 2026-08-14

Status: **implementation approved by Adrian; F0-S contract complete; F1a/F1b next**

This plan turns the approved Gate F user model and RXAS-only runtime boundary
into staged production work. It does not weaken the mandatory first ordinary
profiling-off Release verdict or the decision stops in
[`AGENTS.md`](AGENTS.md).

Authorities:

- [`PERF3-13-GATE-F-USER-GUIDE.md`](PERF3-13-GATE-F-USER-GUIDE.md) defines
  the approved Rexx user model;
- [`PERF3-13-GATE-F-DESIGN.md`](PERF3-13-GATE-F-DESIGN.md) defines ownership,
  transfer and lifecycle invariants; and
- [`PERF3-13-GATE-F-AI-SPEC.md`](PERF3-13-GATE-F-AI-SPEC.md) defines the exact
  source, Level B, RXAS/RXBIN, provider and conformance contract; and
- [`PERF3-13-WORKLIST.md`](PERF3-13-WORKLIST.md) remains the live status and
  evidence control plane.

## Locked architecture

```text
Level G task syntax and DO PARALLEL
        -> Level B task/scope/pool/channel classes
        -> authored Level B ASSEMBLER
        -> mandatory transport-neutral RXAS channel instructions
        -> RXVM channel/provider registry
        -> local, process, host or registered plugin provider
        -> Gate E executor and physical delivery substrate where applicable

Level B process/redirect classes and compiler exits
        -> the same five RXAS channel instructions
        -> reusable RXVM bounded byte-endpoint/child-process providers
        -> child stdio, channel transports, HTTP bodies and provider adapters
```

There is no RXPA task-start, task-wait or provider-selection path. A channel
provider cannot add a provider-specific opcode. It implements the common
open/start/wait/cancel/close contract behind RXVM.

The initial public instruction family has exactly five semantic roles:

```text
chanopen   status,channel,providerType,requiredCapabilities,configuration
chanstart  status,ticket,channel,envelope,waitMicroseconds
chanwait   status,completion,channel,waitMicroseconds
chancancel status,channel,ticket,reason
chanclose  status,channel,mode
```

The maintainer/AI specification locks these operand spellings, register types,
opcodes `650..654`, effects, signals and feature gate before the first opcode
edit. The implementation does not add angle-bracket task intrinsics.

## `chanopen` provider type and flags

`providerType` and `requiredCapabilities` are separate signed 64-bit integer
values.

- `providerType` selects one mutually exclusive physical implementation.
- `requiredCapabilities` is a bit mask of behavior the caller requires from
  that implementation.
- `configuration` is a versioned `ChannelValue` record interpreted by the
  selected provider.

Keeping these fields separate avoids treating mutually exclusive provider
kinds as combinable flags and allows capability negotiation without adding an
opcode.

### Provider-type code space

| Code/range | Meaning |
| --- | --- |
| `0` | invalid/unset; never silently means a default |
| `1` | core local-thread task provider |
| `2` | core isolated-process task provider |
| `3` | reserved core open-host provider for F2 |
| `4` | core bounded byte-stream/redirect endpoint provider |
| `5` | core child-process provider |
| `6..65535` | reserved for future CREXX core providers |
| `65536..INT64_MAX` | registered extension-provider codes |
| negative | invalid/reserved |

The Level B `.taskpool.local(...)` and `.taskpool.process(...)` factories hide
these numeric codes from ordinary Rexx users. The execution-local default pool
selects core local type `1` explicitly rather than relying on type `0`.

An extension provider declares one stable type code, canonical name, provider
ABI version, configuration-schema version, capability mask and the common
provider operations. Duplicate codes or names, unsupported ABI versions and
incomplete operation tables are rejected during registration.

The registry is runtime-owned, not a mutable unsynchronized process global.
Core descriptors are installed during runtime creation. F1 also provides an
internal registration hook and a fake-provider conformance fixture so the
extension seam is proved. Publication of an installed third-party RXVM plugin
ABI remains a separate F2 ABI review; it does not alter RXAS or the Level B
class contract.

### Required-capability flags

The specification assigns stable bits `0..9` for:

- bounded admission/backpressure;
- cancellation;
- deadlines;
- completion-order observation;
- streaming/chunking;
- reusable byte endpoints and child-stdio attachment;
- structured child-process execution;
- process isolation; and
- open-host operation.

F1 local task pools require bounded admission, cancellation, deadlines and
completion observation. F1 process pools additionally require process
isolation. Unknown required bits fail `chanopen`; they are never ignored.
Provider capabilities may exceed the required mask and are queryable through
Level B diagnostics.

### Configuration and failure

The configuration record starts with a schema name and version. The local task
provider initially consumes bounded worker capacity, bounded admission
capacity and provider policy. Unknown optional fields may be ignored only when
the schema marks them optional; unknown required fields fail open.

`chanopen` is failure-atomic:

- on success, status is success and `channel` contains a validated
  execution-local VM capability;
- on failure, status identifies invalid type, unavailable provider,
  unsupported capability, invalid configuration, incompatible version,
  resource limit or allocation failure, and `channel` is cleared; and
- no worker, provider reference, queue or partially opened channel remains
  live after failure.

The channel value is not a C pointer or a transferable native/plugin payload.
RXVM validates its slot, generation, owning execution and lifecycle on every
operation. Copying the containing Level B object may retain the same local
capability according to the class contract; transferring it to another worker,
process or host is rejected.

A successfully opened channel pins its provider descriptor and implementation
module until `chanclose` and final capability release. A provider cannot unload
while a channel or ticket can still call it.

## Spawn and reusable redirect endpoints

CREXX is pre-release, so RXAS consistency takes priority over retaining the
current authored `spawn`, `redir2str`, `redir2arr`, `str2redir`, `arr2redir`
and `nullredir` spellings. Gate F replaces those six lifecycle-bearing
instructions with byte-endpoint provider type `4`, child-process provider type
`5` and the common five channel instructions.

The retired numeric opcode slots remain reserved and are never silently reused.
All repository and installed-development RXBIN must be rebuilt. An image using
a retired opcode or missing the channel feature fails validation explicitly.
The compatibility commitment is at the Rexx/Level B behavior boundary, not to
pre-release RXAS or RXBIN artifacts.

The current redirect implementation already has valuable invariants: the I/O
helper owns a libc-domain byte completion, publishes one terminal state and
does not receive a worker or live VM `value *`. Gate F generalizes that
mechanism into a reusable bounded byte-endpoint substrate.

A byte endpoint has:

- one direction or an explicit duplex pair;
- bounded buffering and backpressure;
- byte chunks as the canonical payload;
- optional line/text adapters at the Level B edge;
- deadline, cancellation, EOF, half-close and terminal-error states;
- an optional child-stdio attachment capability; and
- deterministic close/join/finalization independent of whether `spawn` uses
  it.

The reusable core must not retain a destination Rexx string/array or any other
live VM value while an I/O thread is active. Output bytes are captured in
endpoint-owned storage and are materialized into receiver-owned Rexx storage
only when a Level B consumer reads or drains them. Input strings/arrays are
snapshotted before asynchronous I/O begins.

Provider type `4` exposes the general endpoint through the same
open/start/wait/cancel/close channel roles. Its envelopes describe byte read,
write, drain and half-close requests; completions contain bounded byte chunks,
EOF or typed failure. This makes it usable by process tasks, HTTP streaming,
files, sockets and future provider plugins without new domain-specific
opcodes.

Updated Level B redirect constructors become adapters over provider type `4`:

- string/array input adapters snapshot input into a readable endpoint;
- string/array output adapters create an output endpoint plus a legacy
  receiver-side drain adapter;
- the null adapter creates the null source/sink; and
- the process adapter supplies three compatible endpoint capability references
  to the child-process provider for stdin/stdout/stderr.

Existing Rexx ADDRESS behavior remains source-compatible after its compiler
exit and Level B runtime are updated. New Level B code uses `.byteendpoint`
and may read, write, stream, cancel and close it without spawning a process.
Its signatures are locked with the rest of the compile-checked Level B
declarations.

Provider type `5` owns structured child-process start, exit status,
cancellation, deadline, kill/quarantine and endpoint attachment. Synchronous
Rexx process/ADDRESS behavior is a Level B start-plus-wait wrapper; asynchronous
structured use exposes the task/channel directly. There is no separate spawn
opcode unless later evidence proves the common contract inadequate and Adrian
approves reopening the ISA.

F0-S also inventories the existing socket/TLS, file, clock/timer and process
instructions. The rule is not that every synchronous scalar primitive must be
removed. It is that lifecycle-bearing asynchronous work, reusable resources
and provider extension use the common channel model, and no second
provider-specific asynchronous family is added. Each retained direct
instruction must have a stated low-level reason and a coherent Level B owner.

## Implementation slices

### F0-S — exact specification and executable oracle — complete

Delivered:

1. the maintainer/AI reference specification and coherence matrix;
2. exact RXAS mnemonics, operand kinds and failure-visible write order;
3. provider type, capability and status constants;
4. channel and ticket lifecycle state machines;
5. `ChannelValue`, task envelope and completion schemas;
6. deadline, nonblocking wait, cancellation and close-mode rules;
7. complete opcode effect and signal contracts;
8. the direct-instruction versus channel-provider disposition for current
   spawn/redirect, socket/TLS, file and clock/timer operations; and
9. positive, negative and malformed-image test vectors.

Exit: every approved user-guide rule maps to the specification, Level B API or
diagnostic. Any contradiction returns to Adrian; otherwise the already
approved implementation proceeds without another syntax-design round.

The declaration oracle is
[`gate_f_levelb_contract.crexx`](../compiler/tests/rexx_src/gate_f_levelb_contract.crexx)
and its focused CTest compiles, assembles and disassembles the interface
metadata. The coherence matrix and exact vectors are in the maintainer/AI
specification.

### F1a — RXAS/RXBIN contract

Add the five opcode entries at the tail of the dense public opcode inventory,
without reusing retired slots. Add:

- one `RXBIN007_FEATURE_CHANNELS` bit and supported-feature validation;
- exact `rxopeffects.h` and `rxopsignals.h` entries;
- RXAS/RXDAS round-trip coverage;
- feature-bit emission and rejection tests;
- operand/register/type validation;
- optimizer-barrier and TRACE/profiler identities; and
- malformed, truncated, unknown-feature and wrong-operand fixtures.

All five operations are opaque external-state boundaries initially. Stronger
optimizer facts require later proof and are not part of the opening slice.

### F1b — local provider vertical slice

Create a private RXVM channel subsystem, expected to be isolated in
`interpreter/rxvmchannel.[ch]`, containing:

- a runtime-owned provider registry;
- generation-checked channel and ticket capability tables;
- provider descriptor lifetime pinning;
- common status/completion translation; and
- deterministic context teardown of still-open capabilities.

Refactor the existing private Gate E executor so a controller channel can seal
and attach workers to its current program generation. Retain the existing
RXBIN-path constructor for Gate E tests. The new path must not reload the
program independently or share worker-owned overlays.

Implement the five instruction handlers in the shared VM core so `rxbvm` and
`rxtvm` execute identical logical behavior. The first vertical fixture uses
the existing Gate E integer/string register-image subset and proves:

- open, bounded submission, wait, cancellation and close;
- two independent tasks and deterministic terminal completion;
- queue-full, invalid/stale capability and double-close behavior;
- queued and running cancellation;
- worker failure/quarantine and reuse; and
- zero live channels, tickets, requests or worker allocations at teardown.

After the minimum focused Debug/Release correctness needed for measurement,
freeze production changes, build the ordinary profiling-off Release product,
run the smallest decisive local-channel latency/throughput and single-worker
neutrality comparison, report it to Adrian and stop.

### F1c — complete values, lifecycle and Level B surface

Replace the Gate E fixture subset at the public boundary with the versioned
`ChannelValue` tree:

- null/boolean, integer, float, decimal, string and binary scalars;
- ordered arrays and schema-tagged records;
- copied values, moved buffers and sealed immutable buffers; and
- deterministic rejection of references, live objects, frames, closures and
  native payloads.

Complete task deadlines, nonblocking observation, one-terminal-completion,
scope cancellation, drain/cancel close modes, backpressure and cleanup.

Implement compile-checked Level B interfaces and classes:

- `.taskpool`, `.taskscope`, `.task`, `.tasktarget`, `.taskwork` and
  `.taskcontext`;
- `.completion`, `.channel`, `.channelrequest`, `.channelvalue`,
  `.channelcodec` and `.transferbuffer`; and
- named local/process pool and scope-policy factories.

The class implementations invoke only the five RXAS instructions. Tests prove
that no RXPA call or procedure-name string is present in the path. Run the
mandatory first Release verdict for this production slice and stop.

### F1d — reusable redirects and child-process integration

Extract the redirect endpoint core from `rxspawn.c` into a private reusable VM
module. Add provider types `4` and `5`, update the ADDRESS/compiler-exit and
Level B process paths to the common channel operations, including
`compiler/exits/address/Address.crexx` and `lib/rxfnsb/rexx/_address.crexx`.
Retire the six old source mnemonics, reserve their opcode slots and rebuild
generated RXBIN.

Prove:

- unused endpoints close safely without requiring a later `spawn`;
- no I/O thread retains or mutates a live VM destination;
- bounded read/write, deadline, cancellation, EOF and half-close behavior;
- string, line-array, binary and null adapters;
- independent stdin/stdout/stderr completion and teardown;
- reuse by a non-spawn in-memory/pipe fixture;
- equivalent observable Rexx behavior for existing ADDRESS/process tests;
- explicit stale-RXBIN diagnostics for each retired instruction; and
- no retired mnemonic in newly generated RXAS.

Run the required first Release verdict and stop before using the substrate in
the isolated task-process provider or HTTP library.

### F1e — isolated-process provider

Implement type `2` behind the same provider descriptor and conformance suite.
Use the same versioned envelope/completion encoding across the process
boundary. Prove:

- no pointer, live `value`, reference or native payload crosses;
- process crash and disconnect become typed terminal outcomes;
- cancellation/deadline races produce exactly one terminal completion;
- bounded I/O and queue backpressure;
- deterministic child, pipe/socket and temporary-resource teardown; and
- semantic parity with the local provider.

Freeze and stop at the process-provider first Release verdict before cross-host
or HTTP expansion.

### F1f — Level G Rexx lowering

Implement the approved `task` callable/method kind, statically resolved task
targets, normal task calls, expression dependency plans and `DO PARALLEL`.

The compiler lowers exclusively through the Level B contract. It preserves
left-to-right argument evaluation and submission, short-circuit behavior,
typed pending result bindings, controller-side `TASK_FAILURE`, structured join
and ordinary synchronous behavior for non-task calls. It does not emit a
second public intrinsic API.

Validate parser/AST/type/diagnostic fixtures, nested dependencies, failures,
early block exits and both optimized/unoptimized RXAS. Run the first Release
verdict and stop before convenience expansion.

### F1g — concurrent HTTP/TLS industrial consumer

Build the bounded multithreaded HTTP library required by `crexx-rag` over
tasks/channels, reusable byte endpoints and the existing socket/TLS substrate.
Do not add `httpstart` or other HTTP opcodes.

Required behavior includes:

- per-origin connection reuse and bounded concurrency;
- DNS/connect/TLS/request/response deadlines and cancellation;
- certificate and hostname verification;
- bounded headers and response bodies;
- streaming/chunking capability without forced whole-body buffering;
- request and response body streaming through the shared byte-endpoint
  contract;
- redirects, retry policy and ambiguous-outcome rules;
- partial I/O, connection failure and shutdown cleanup; and
- concurrent generation/embedding-style workload evidence matching the
  `crexx-rag` consumer.

Run local correctness and saturation tests, the crexx-rag integration fixture,
then the mandatory Release verdict before broader closeout.

### F2 — open host and published provider plugins

After F1 acceptance:

- select and specify an open transport and wire encoding;
- implement provider type `3` and one independent non-Rexx actor;
- publish a versioned RXVM channel-provider plugin descriptor only after ABI
  review;
- prove duplicate/unknown provider registration and plugin unload safety; and
- retain the same Level B/RXAS contract without provider-specific opcodes.

### F3 — stabilization and closeout

Complete:

- both `rxbvm` and `rxtvm`;
- required Mac, Linux and Windows provider behavior;
- focused sanitizer and allocator/teardown proof;
- install/package and rebuilt-RXBIN compatibility tests;
- source, TRACE, profiler and debugger visibility;
- payload-size copy/move/sealed-sharing comparisons; and
- ordinary single-worker product neutrality.

Only evidence-led implementation optimization occurs in F3. The five semantic
instruction roles and the Level B contract do not become optional again.

## F1a/F1b production design selection

The opening production slice records these alternatives before editing the
instruction table or VM:

1. **Status quo RXPA/provider functions plus hidden native handles —
   rejected.** This would create a second core task ABI, expose implementation
   payload ownership to Level B and fail Adrian's requirement that core RXVM
   multithreading be RXAS-only.
2. **Task-, process- and redirect-specific RXAS families — rejected.** Separate
   `poolnew`/`taskstart`/`taskwait` operations while retaining `spawn` and the
   six redirect operations would duplicate lifecycle, cancellation and
   provider-extension semantics and leave no reusable endpoint foundation.
3. **Five transport-neutral channel operations with runtime-owned providers —
   selected.** The common capability table and provider registry implement
   local tasks first, then byte endpoints, child processes and isolated tasks
   without changing the Level B/RXAS boundary. Old process/redirect slots stay
   operational only through the buildable F1a-F1c transition and become
   reserved when F1d migrates their consumers.

The selected F1a/F1b vertical slice appends opcodes `650..654`, adds the
feature/effect/signal contracts, isolates the private capability and provider
logic in `rxvmchannel.[ch]`, and attaches local workers to the controller's
sealed generation through `rxvmexecutor.[ch]`. It retains the existing
RXBIN-path executor constructor for Gate E tests and does not yet implement
Level G syntax, redirects or HTTP.

## First production edit

The first production edit is limited to F1a plus the minimum complete F1b
local-provider path. Expected files are:

- `binutils/include/rxops.h`, `rxopeffects.h`, `rxopsignals.h` and `rxbin.h`;
- `binutils/rxbin007.c` and RXAS/RXDAS validation tests;
- new private `interpreter/rxvmchannel.[ch]`;
- a generation-attaching extension of `interpreter/rxvmexecutor.[ch]`;
- shared VM handlers and CMake wiring; and
- focused RXAS, metadata, malformed-image and executor/channel tests.

It does not yet refactor redirects/process execution, implement Level G
syntax, process/host transport, a public provider-plugin ABI or HTTP. Those
remain separately measurable slices.

## Evidence and stop rules

Every slice records exact branch/commit identity, dirty scope, commands,
platform/toolchain, both concrete VMs where applicable, correctness outcomes,
raw performance samples and lifecycle/resource results.

For every production slice:

1. run the minimum focused correctness checks;
2. freeze implementation;
3. build the ordinary profiling-off Release product;
4. run the smallest decisive retained-baseline comparison;
5. report to Adrian and stop; and
6. continue to broad QA only after Adrian accepts that verdict.

No commit, push, public ABI publication or next-slice start is implied by an
accepted first verdict unless Adrian authorizes it.

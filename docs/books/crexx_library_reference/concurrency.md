# Concurrency classes {#library-concurrency}

The `concurrency` namespace is the object layer over cREXX's provider-neutral
channel instructions. It supplies bounded task pools, structured scopes,
canonical values, byte endpoints and explicit binary ownership. The language
chapter adds Level G task syntax on top; the classes themselves are available
at Level B.

The surface is initial on `develop`. Local-thread and isolated-process
providers are qualified on macOS in the current tree. Portable publication is
still subject to Linux and Windows conformance.

## Import and object relationships

```rexx
options levelb
import rxfnsb
import concurrency
```

A pool and a scope have different jobs:

- a `.taskpool` is a bounded execution provider and capacity policy;
- a `.taskscope` owns child lifetime, cancellation, deadline and joining;
- a `.task` is one submitted child; and
- a `.completion` is an immutable terminal observation.

One pool may serve several scopes. A scope is consumed once and cannot be
reused. Close every scope before closing its pool. Ordinary tasks cannot detach
from their scope.

```text
taskpool (capacity) --> taskscope (lifetime) --> task --> completion
                            |
                            +--> task --> completion
```

The common transport layer has the same ownership shape:

```text
channel (provider owner) --> channelrequest --> completion
```

Raw channel handles and request tickets are deliberately private. The wrapper
objects prevent an integer from being mistaken for transferable authority.

## Pools and structured scopes

Create a local worker-thread pool or an isolated worker-process pool:

```rexx
local_pool = .taskpool.local(4, 64)
process_pool = .taskpool.process(2, 16)
```

The first argument is maximum worker capacity. The second is bounded admission
capacity. Provider type `1` is local execution and type `2` is process
isolation. Capacity permits overlap; it does not promise a particular
schedule.

Choose a scope policy and relative deadline:

```rexx
scope = .taskscope.failfast(local_pool, 30000)
scope = .taskscope.collectall(local_pool, 30000)
```

`failfast` requests cancellation of unfinished siblings after the first
unsuccessful child. `collectall` lets the other children finish. Both account
for every accepted child. Timeout values are milliseconds: `-1` means no
deadline or an indefinite wait, `0` is an immediate poll, and a positive value
is a bounded relative wait.

The most Rexx-like way to use these objects is through Level G task syntax:

```rexx
options levelg comments_dash
import concurrency

adderwork: class implements .taskwork
  _delta = .int

  *: factory
    arg delta = .int
    _delta = delta
    return

  run: method = .channelvalue
    arg request = .channelvalue, context = .taskcontext
    if context.cancellation_requested() then return .channelvalue.null_value()
    return .channelvalue.integer_value(request.as_integer() + _delta)

main: procedure = .int
  pool = .taskpool.local(1, 4)
  scope = .taskscope.collectall(pool, 60000)

  target = task .adderwork(2)
  child = scope.submit(target, .channelvalue.integer_value(40))
  outcome = child.wait(-1)

  if outcome.succeeded() then answer = outcome.value().as_integer()
  else do
    say outcome.error_code() outcome.message()
    answer = -1
  end

  call scope.finish()
  call pool.close()
  return answer <> 42

exit main()
```

This exact pattern is compiled, assembled, linked and run in
[`concurrency_taskwork.crexx`](../crexx_programming_guide/examples/concurrency_taskwork.crexx).
`task .adderwork(2)` is Level G syntax: the compiler, assembler and linker
create and seal the target descriptor. Application code must not call
`.tasktarget.binding()` with invented bytes.

The main scope operations are:

| Operation | Meaning |
| --- | --- |
| `submit(target, request)` | submit one `.channelvalue` application request |
| `next(milliseconds)` | observe the next child in completion order |
| `join()` | return all child completions in submission order |
| `cancel(reason)` | request cancellation of unfinished children |
| `finish()` | join, close and raise `TASK_FAILURE` on child failure |
| `abort(reason)` | cancel, join and close during abnormal controller exit |

`next()` may return an unavailable sentinel with `available() = 0`. That is a
timed or nonblocking miss, not a child completion, and it never appears in
`join()`.

`.taskscope.ask()` is reserved for future single-owner services and currently
signals unsupported status `19`. `.taskpool.queued()` and `.running()` are also
reserved and signal `19`; they do not return guessed telemetry.

## Task work and task context

`.taskwork` is the advanced runnable contract below ordinary typed task calls:

```rexx
run: method = .channelvalue
  arg request = .channelvalue, context = .taskcontext
```

A factory target such as `task .adderwork(2)` creates a worker-owned class
instance. The application `request` is separate from the factory arguments.
The current factory-target lowering accepts `.boolean`, `.int`, `.string`,
`.binary` and direct `.channelvalue` arguments.

The context reports:

- `timeout_remaining()`: remaining milliseconds, or `-1` without a deadline;
- `cancellation_requested()`: cooperative cancellation state; and
- `trace_identity()`: stable invocation identity.

`endpoint(reference)` adapts a transferable type-4 provider reference to a
worker-local `.byteendpoint`. Use it inside `.taskwork.run()` when a task must
read or write an endpoint owned by its controller; ordinary controller code
can use `.byteendpoint.from_reference()` directly.

`.taskarguments` and the `submit_arguments()` / `result_*()` methods are
exposed compiler-lowering bridges. They are documented so generated code is
auditable, but ordinary programs should use typed task calls or
`submit(target, request)`.

## Completion states

Always test `available()` before treating an observation as terminal, then use
`succeeded()`, `value()`, `error_code()`, `message()` and `details()` rather
than guessing from a message string.

| State | Meaning |
| ---: | --- |
| `0` | unavailable observation sentinel |
| `1` | succeeded |
| `2` | failed |
| `3` | cancelled |
| `4` | deadline exceeded |
| `5` | rejected or target not found |
| `6` | endpoint closed or provider shutdown |
| `7` | transport lost before execution |
| `8` | outcome unknown after transport loss |
| `9` | isolated task killed |

State is provider-neutral terminal classification. `error_code()` carries the
more specific operation/provider status. `value()` is valid only when
`has_value()` is true.

## Canonical channel values

`.channelvalue` is the only ordinary value document accepted at a provider
boundary. It can represent null, boolean, integer, float, decimal, string,
binary, arrays and named versioned records. The receiver owns a validated new
value; it does not receive a pointer to the sender's object.

```rexx
names = .string[]
values = .channelvalue[]
names[1] = "name"
values[1] = .channelvalue.string_value("Ada")
names[2] = "score"
values[2] = .channelvalue.integer_value(42)

record = .channelvalue.record_value("example.result", 1, names, values)
say record.schema() record.version()
say record.field("score").as_integer()
```

Record field names must be unique. Construction writes canonical byte-name
order, so equivalent records encode identically. Accessors validate the value
kind: for example `as_integer()` does not coerce a string.

`.channelcodec` is the explicit application codec interface:

```rexx
schema: method = .string
version: method = .int
encode: method = .channelvalue
  arg item = .object
decode: method = .object
  arg item = .channelvalue
```

There is no ambient runtime codec registry. Level G transferable concrete
classes instead use their statically resolved `to_channel()` and
`from_channel()` members.

## Channels and byte endpoints

Most programs use pools, scopes and `.byteendpoint` rather than opening a raw
`.channel`. The channel object directly exposes the provider lifecycle:

```rexx
channel = .channel.open(provider_type, required_capabilities, configuration)
request = channel.start(envelope, admission_wait)
outcome = request.wait(completion_wait)
call channel.close(1)  /* drain */
```

Close mode `1` drains accepted work; mode `2` requests cancellation. A channel
owns its requests, so cancellation with a request from another channel is an
error.

Provider type `4` supplies reusable bounded byte endpoints. This complete
Level B example writes and reads through one duplex endpoint:

```rexx
options levelb
import rxfnsb
import concurrency

endpoint = .byteendpoint.memory(3, 32)

write_request = endpoint.start_write("hello" as .binary, -1)
write_result = write_request.wait(-1)
if write_result.succeeded() = 0 then exit 1

read_request = endpoint.start_read(5, -1)
read_result = read_request.wait(-1)
if read_result.succeeded() = 0 then exit 2
if read_result.value().as_binary() as .string <> "hello" then exit 3

call endpoint.half_close(2)
call endpoint.close()
exit 0
```

Endpoint direction is `1` readable, `2` writable or `3` duplex. Memory
capacity is a real backpressure bound. A transferable provider reference can
be adapted in another execution with `from_reference()` or, for an exact
encoded 92-byte reference, `from_encoded_reference()`. Close every adapter
when finished.

## Transfer buffers

`.transferbuffer` makes binary ownership transitions visible:

```rexx
buffer = .transferbuffer.copy_from("answer=41" as .binary)
call buffer.write(7, "42" as .binary)
snapshot = buffer.seal()
say snapshot.as_binary() as .string
```

The states are:

- **mutable**: read, write, `move_value()` or `seal()`;
- **moved**: the immutable value owns the transferred bytes and the source may
  no longer be read or changed; and
- **sealed**: the buffer retains read-only access and repeated `seal()` calls
  return the same stable immutable snapshot.

A transfer-buffer seal is not encryption, authentication or a checksum. It
only prevents later writes through that buffer. It is unrelated to the sealed
task-binding integrity descriptor.

## Reserved surfaces

`.serviceref` records the intended logical identity shape for a future
single-owner service. There is no concrete public service, no working
`.taskscope.ask()` path and no typed service proxy today. Provider type `3`
(open host) and the public provider-plugin ABI are also reserved.

For task-call syntax and more approachable examples, see
[Concurrent programming](../crexx_programming_guide/concurrency.md). For the
formal Level G rules, see the [Concurrency language
reference](../crexx_language_reference/concurrency.md).

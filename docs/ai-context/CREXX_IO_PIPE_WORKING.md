# cREXX IO, Pipes, and Native Handle Working Plan

Status: working design note.

This note records the review prompted by the Windows ADDRESS redirect flake and
sets out a more robust direction for IO, process pipes, and a first step toward
threading. It is intentionally a working document, not a committed language
contract.

## Problem Statement

The current ADDRESS redirect implementation passes opaque native redirect state
through Rexx `.binary` values. On Windows that payload contains raw `HANDLE`
values. Normal Level B assignment, argument passing, object construction, and
attribute storage can byte-copy those `.binary` values. The byte copy duplicates
the handle number, not ownership. The copied value can later be finalized or
closed by a different lifetime path, leaving another copy with an invalid handle.

The immediate symptom was a flaky Windows failure in:

- `ts_address_crexx_noopt`
- `ts_address_crexx_opt`

The reproduced failure signature was:

```text
PANIC: CREXX command redirect failure input=0/0/0 output=1/6/1 error=0/0/0
```

On Windows, `lastError=6` is `ERROR_INVALID_HANDLE`; `errorSource=1` is the
write side of ADDRESS CREXX redirect replay.

Short-term aliasing patches reduce the number of byte-copy sites, but they are
not a robust architecture. The underlying issue is that a native resource with
ownership semantics is being represented as an ordinary byte buffer.

## Facilities Reviewed

### Native Payloads

The VM already has native payload support on `value`:

- `value.binary_value`
- `value.native_payload_ops`
- `value.native_payload_flags`

`rxvm_native_payload_ops` carries:

- `type_name`
- `copy`
- `finalize`

The VM calls the copy hook from `copy_value()` / `copy_binary_value()` and the
finalizer from `clear_value()`. This is exactly the hook shape needed for native
handles. A redirect endpoint should not be an ordinary `.binary`; it should be
a native payload with explicit retain/duplicate/finalize behavior.

The RXPA public surface also exposes:

- `SETNATIVEPAYLOAD()`
- `GETNATIVEPAYLOAD()`
- class/interface metadata declarations such as `ADDCLASS`,
  `ADDINTERFACE`, `ADDIMPLEMENTS`, `ADDFACTORY`, and `ADDMETHOD`

Current RXPA tests already prove native payload copy/finalizer hooks fire. They
do not yet expose a pure native class constructor that stamps a returned object
without a Rexx shim.

### Class and Interface Runtime

The VM has runtime class/interface metadata and dispatch. Rexx source can define
interfaces and classes, and RXPA can publish matching metadata. Existing docs
also state that native code can advertise contracts, but object construction is
not yet a full pure-C path: complete class-shaped construction generally still
uses a small Rexx factory/class shim.

That is acceptable for an IO design. We can define public interfaces and Rexx
wrapper classes in Level B, with native payload stored inside an attribute or
the object value itself. Native methods can operate on the payload while Rexx
code sees a normal object.

### ADDRESS Runtime

ADDRESS environments are already normal Rexx objects implementing
`addressenvironment`. The native RXVML ADDRESS path carries stdin/stdout/stderr
endpoint values, and helper functions such as `rxvml_address_emit_output()` and
`rxvml_address_emit_error()` hide redirect writing from native providers.

This is a useful precedent: native providers should not manipulate raw endpoint
bytes. The missing piece is that the endpoint value itself is still an opaque
`.binary`.

### Spawn and Redirects

`interpreter/rxspawn.c` owns the current `REDIRECT` struct. It creates:

- null redirects to `NUL` or `/dev/null`
- output capture redirects to strings or arrays
- input redirects from strings or arrays
- child process stdio handles
- worker threads for pipe drain/fill

The current implementation mixes three concerns in one raw struct:

- public endpoint identity
- OS handle ownership
- per-spawn worker-thread state

Those concerns need to be split. A Rexx-visible endpoint should be a stable
object. A spawn operation should derive per-spawn handles and worker state from
that object.

### Socket Registry Precedent

`rxvmsock.c` uses a VM-context registry of socket entries and exposes integer
handles to instructions. This avoids raw OS handles in Rexx values and keeps
platform-specific cleanup in one place.

For IO streams, a native payload with a refcounted native cell is a better fit
than integer handles because it composes with classes, interfaces, method
dispatch, and ADDRESS request objects. The socket registry remains a good
precedent for central status/error fields and context-owned cleanup.

### Threading State

The current process redirect implementation already uses OS threads internally
to fill and drain pipes. Those threads are not Rexx threads. They must not call
arbitrary Rexx code or share VM frames.

A general Rexx threading API is a larger design. Process pipes can be the first
useful step: they need controlled native worker threads, lifecycle joining, and
clear ownership, but they do not require running multiple Rexx frames in
parallel.

## Design Goals

1. No raw OS handles in ordinary `.binary` values.
2. Native handle copies must either retain shared state or duplicate the OS
   handle deliberately.
3. Public IO concepts should be Rexx-visible interfaces/classes, not hidden
   ADDRESS-only internals.
4. Native and Rexx implementations should share the same interface.
5. Process pipes should become reusable library objects, not just ADDRESS
   redirect implementation details.
6. Worker threads must be implementation-owned and joined deterministically.
7. Windows, POSIX, and future host integrations should differ only behind a
   small native backend.

## Proposed Type Model

Names below are working names. Final namespace placement is open.

### Interfaces

`rxio.input`

- `read(max_bytes = -1) -> .string`
- `readb(max_bytes = -1) -> .binary`
- `close() -> .void`
- `status() -> .int`
- `error() -> .string`

`rxio.output`

- `write(text = .string) -> .int`
- `writeb(bytes = .binary) -> .int`
- `close() -> .void`
- `status() -> .int`
- `error() -> .string`

`rxio.stream implements rxio.input, rxio.output`

- duplex stream contract

`rxio.closeable`

- optional small interface if shared close/status is useful outside streams

### Classes

`rxio.nativestream`

- Native payload backed.
- Wraps files, pipes, inherited stdio, sockets later, and process endpoints.
- Owns a refcounted native cell.

`rxio.stringoutput`

- Rexx implementation that appends text to a string or string builder.
- Useful for tests and future pure-Rexx ADDRESS providers.

`rxio.arrayoutput`

- Rexx implementation that appends lines to `.string[]`.
- Mirrors current `ADDRESS ... output arr`.

`rxio.arrayinput`

- Rexx implementation that reads lines from `.string[]`.
- Mirrors current `ADDRESS ... input arr`.

`rxio.nullinput` / `rxio.nulloutput`

- Explicit discard/source-empty objects.

`rxio.pipe`

- Pair object with `read_end()` and `write_end()`.
- Native implementation first.

`rxio.process`

- Returned by future process start APIs.
- Methods: `wait()`, `kill()`, `exit_code()`, `stdin()`, `stdout()`,
  `stderr()`, `close()`.

## Native Payload Shape

The native payload should store a pointer to a refcounted cell, not the OS
handle bytes directly:

```c
typedef struct rxio_native_cell rxio_native_cell;

typedef struct rxio_payload {
    rxio_native_cell *cell;
} rxio_payload;
```

The cell should carry:

- kind: null, inherited stdio, pipe read, pipe write, process, file, memory sink
- direction flags: readable, writable, duplex
- refcount
- close state
- last status and last error text
- platform handles:
  - Windows: `HANDLE`
  - POSIX: file descriptor
- optional worker-thread state for process pipe pumps
- optional target `value*` only for legacy bridge objects, with strict lifetime
  rules

Native payload ops:

- `copy(dest, source)`: retain the cell and install a new payload pointing to it.
- `finalize(value)`: release the cell. The last release closes handles and joins
  owned workers.

For process spawning, do not pass the endpoint's stored handle directly if the
spawn will close it. Instead create a per-spawn handle view:

- Windows: duplicate inheritable child handles with `DuplicateHandle`.
- POSIX: use `dup()` where the child/spawn lifetime needs independent close.

This gives the spawn lifecycle its own handles while the Rexx endpoint object
remains valid or closes independently according to API rules.

## ADDRESS Integration

The existing ADDRESS syntax can remain:

```rexx
address crexx "run :args[]" input input_arr output output_arr error error_arr
```

The compiler/runtime lowering should change so that redirects become IO objects:

- no explicit redirect: `rxio.inherited_stdout()` / `rxio.inherited_stderr()`
  semantics, not a dummy raw binary
- `output arr`: `rxio.arrayoutput(arr)`
- `output str`: `rxio.stringoutput(str)`
- `input arr`: `rxio.arrayinput(arr)`
- explicit discard if needed: `rxio.nulloutput()`

The `addressrequest` class should store endpoint objects or interface-typed
values. It should not copy native handle bytes through constructor assignment.

Native ADDRESS providers should continue using:

- `rxvml_address_emit_output()`
- `rxvml_address_emit_error()`

Those helpers should be updated to dispatch to `rxio.output.write()` or use a
fast native endpoint path when the object is `rxio.nativestream`.

## Process and Pipe API Direction

`ADDRESS CREXX run` should eventually be implemented on top of a reusable
process API rather than having private command-run capture logic.

Working API shape:

```rexx
proc = rxio..process(args)
call proc.start(stdin, stdout, stderr)
rc = proc.wait()
```

Convenience:

```rexx
result = rxio..run(args)
out = result.stdout()
err = result.stderr()
rc = result.rc()
```

For streaming:

```rexx
pipe = rxio..pipe()
proc = rxio..process(args, pipe.read_end(), rxio..stdout(), rxio..stderr())
call pipe.write_end().write("input")
call pipe.write_end().close()
rc = proc.wait()
```

The first implementation should support simple blocking read/write/close and
wait. Nonblocking and async can come later.

## Rexx Implementations Later

The interface split allows pure Rexx implementations later:

- array-backed input/output
- string-backed output
- filtering streams
- tee streams
- logging streams
- mock streams for tests

Important constraint: native worker threads must not invoke arbitrary Rexx
methods. For non-native Rexx stream endpoints, the first implementation should
adapt at safe synchronization points:

- collect all child output in native buffers, then call Rexx output methods on
  the owning VM thread after wait
- pre-drain Rexx input objects into native buffers before spawn

True callback-from-worker-thread support should wait for a VM threading model.

## Threading Implications

This IO work can be the first controlled step toward threading without exposing
general Rexx threads.

Phase 1:

- native worker threads only for process pipe pump/drain
- no Rexx code on worker threads
- deterministic join on process wait/close/finalize

Phase 2:

- native `rxio.task` or `rxio.future` for background native operations only
- completion is polled or waited from the owning VM thread

Phase 3:

- separate `rxvm_context` per Rexx thread or explicit shared-state rules
- message-passing streams/channels as the safe sharing primitive

Do not share a single VM frame stack across OS threads.

## Suggested Implementation Plan

### Stage 0: Stabilize Existing ADDRESS Tests

If the Windows flake must be fixed before the larger refactor, finish the narrow
aliasing fix in current `_address.crexx` request construction. This is a stopgap
only.

Validation:

- `ctest -R '^(ts_address_crexx_noopt|ts_address_crexx_opt)$' --repeat until-fail:100`
- the focused ADDRESS and crexx driver smoke tests

### Stage 1: Native Payload Endpoint Core

Implement a small internal `rxio_native_cell` and payload ops in the interpreter.

Deliverables:

- retain/release/finalize tests
- copy tests proving value copies do not duplicate ownership incorrectly
- POSIX backend first for development speed on macOS
- Windows backend second with explicit `DuplicateHandle` semantics

No public language surface needs to change yet.

### Stage 2: Rexx-Visible IO Interfaces and Wrapper Classes

Add Level B interfaces/classes in a new library module, likely under `rxfnsb` or
an internal `_rxio` namespace.

Deliverables:

- `rxio.input`
- `rxio.output`
- `rxio.stream`
- native wrapper class
- array/string/null Rexx implementations
- RXPA declaration metadata if native functions are exposed directly

This stage should decide whether the native wrapper object is created by:

- a Rexx factory that calls native payload setup functions, or
- a new RXPA helper that stamps class identity from C

The Rexx factory is lower risk for the first pass.

### Stage 3: ADDRESS Redirect Migration

Replace `_noredir`, `_redir2array`, `_redir2string`, `_array2redir`, and
`_string2redir` internals so they return IO endpoint objects rather than raw
`REDIRECT` binaries.

Keep compatibility at the `SPAWN` instruction boundary temporarily:

- accept old raw `REDIRECT` binaries for existing generated bytecode
- prefer new native payload endpoints when present

Validation:

- `ts_address*`
- ADDRESS callback host tests
- native provider emit tests
- Windows repeat-until-fail stress

### Stage 4: Process API

Build a reusable process abstraction on top of the IO endpoints.

Deliverables:

- `rxio.process`
- `rxio.run(args)` convenience
- process wait/exit/kill
- pipe endpoint ownership tests
- child stdout/stderr capture tests
- stdin close/EOF tests

After this, `ADDRESS CREXX run` can delegate to the process API.

### Stage 5: Pipeline Syntax and Helpers

Once the object model is stable, add convenience helpers for pipelines:

```rexx
result = rxio..pipeline(cmd1, cmd2, cmd3)
```

or a builder:

```rexx
pipe = rxio..pipeline()
call pipe.add(cmd1)
call pipe.add(cmd2)
rc = pipe.run()
```

Do not start with syntax. Start with objects and tests.

## Testing Strategy

Core native payload:

- copy retains once
- move transfers without retain
- finalize releases exactly once
- double close is safe
- forced scalar overwrite finalizes payload

Endpoint behavior:

- null output discards
- inherited output reaches stdout/stderr
- array output preserves line behavior
- string output preserves text exactly
- binary write/read preserves bytes
- close causes EOF for readers

Process behavior:

- child stdout captured
- child stderr captured
- stdin from array/string/binary
- paths with spaces
- large output larger than pipe buffer
- child exits before parent writes
- parent closes stdin and child sees EOF

Stress:

- Windows `ts_address_crexx_*` repeat-until-fail
- macOS/Linux repeated process pipe tests
- linked and unlinked runtime modes

## Open Decisions

1. Namespace: `_rxio`, `rxio`, or part of `_rxsysb`.
2. Whether public APIs should distinguish text and binary streams as separate
   interfaces or as methods on one interface.
3. Whether RXPA should grow a native object construction helper now, or whether
   Rexx factories should remain the first implementation path.
4. Whether sockets should eventually migrate from integer registry handles to
   `rxio.stream` objects.
5. Whether `ADDRESS ... output` should expose line-oriented array behavior only,
   while `rxio.output` remains byte/text oriented.
6. How much old raw `REDIRECT` bytecode compatibility is required after beta.

## Recommendation

Use native payload-backed IO endpoint objects as the foundation. Keep the first
implementation small:

1. Native refcounted endpoint payload.
2. Rexx-visible input/output interfaces.
3. Native and Rexx array/string/null endpoint classes.
4. ADDRESS redirect migration.
5. Reusable process and pipe API.

This fixes the current Windows race by removing raw handle byte copies, and it
turns a brittle ADDRESS-only mechanism into a reusable IO facility that can later
support Rexx stream implementations, process pipelines, and controlled native
worker-thread abstractions.

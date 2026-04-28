# Level B Signals And Trace Working Design

Status: draft design record with initial syntax decisions, not yet implemented
Last updated: 2026-04-28

## Purpose

This document is the shared design record for two related Level B features:

- user-facing signal/error handling
- the `TRACE` statement, implemented through the compiler-exit/runtime-debugger path

The features should be designed together because they depend on the same runtime facts:

- VM signal/interrupt delivery
- stack-frame entry and return semantics
- breakpoint tracing
- module/address/source metadata
- filtering so runtime and compiler support code does not trace itself

Implementation should still be staged. The recommended order is:

1. Confirm and document the VM/debug context model.
2. Implement Level B signal/error handling over that model.
3. Extract the reusable debugger/trace runtime class.
4. Implement `TRACE` as a certified compiler exit over the trace runtime.

## Current VM Facts

### Signal Codes

Signal codes are defined in `interpreter/rxsignal.h`.

The current table includes ordinary runtime errors such as `FAILURE`, `ERROR`,
`DIVISION_BY_ZERO`, `CONVERSION_ERROR`, `INVALID_ARGUMENTS`, `OUT_OF_RANGE`,
and `UNICODE_ERROR`; VM/runtime signals such as `UNKNOWN_INSTRUCTION`,
`FUNCTION_NOT_FOUND`, `NOT_IMPLEMENTED`, and `INVALID_SIGNAL_CODE`; POSIX-style
interrupts such as `QUIT`, `TERM`, `POSIX_INT`, `POSIX_HUP`, `POSIX_USR1`,
`POSIX_USR2`, and `POSIX_CHLD`; plus `OTHER` and `BREAKPOINT`.

`KILL` cannot be masked. `BREAKPOINT` is the tracing/debug hook and is not an
ordinary error signal.

### Per-Frame Handler Table

Each `stack_frame` owns an `interrupt_table[RXSIGNAL_MAX]`. When a child frame
is created, `frame_f()` copies the parent table into the child.

Consequences:

- a handler installed in a frame is visible to procedures called after it is installed
- changes made in a child frame do not mutate the parent's table
- returning from a child restores the caller's handler table by returning to the caller frame
- block-scoped handler restoration is a compiler responsibility, not something the VM does today

The VM already supports these handler responses:

- `IGNORE`
- `HALT`
- `SILENT_HALT`
- `RETURN`
- `BRANCH`
- `CALL`
- `CALL_BRANCH`

RXAS exposes these through `sigignore`, `sighalt`, `sigshalt`, `sigret`,
`sigbr`, `sigcall`, and `sigcallbr`.

### Signal Delivery

Signals are represented by a global pending bitset named `interrupts`.
`DISPATCH` checks this bitset after each instruction when the current frame is
not already handling an interrupt.

Signal priority is numeric: the VM scans from low signal code to high signal
code and handles the first unignored pending signal.

Ignored signals are cleared during that scan. Non-breakpoint signals are cleared
when selected. `BREAKPOINT` remains pending until `bpoff`.

While a Rexx signal handler is running, `current_frame->is_interrupt` is set.
That disables immediate nested delivery. If a handler raises a signal, it
becomes pending and is delivered after the handler returns to a non-interrupt
frame.

### Interrupt Argument Object

For a `CALL` or `CALL_BRANCH` handler, the VM builds an object-like value with
five attributes:

1. signal code
2. module number
3. address in module
4. signal name
5. payload/message object

Current debugger code accesses these with `linkattr1`. User-facing Level B code
should not be required to know this raw VM shape.

The old language-reference note that a signal object is just `int_value` plus
`string_value` is incomplete for the current VM.

### Address Semantics

There are two address meanings that must remain distinct.

- Fault signals raised by the current instruction should report the faulting
  instruction address.
- `BREAKPOINT` and native/asynchronous interrupts should report the next
  instruction/resume address.

The fallback panic report now uses the faulting address for VM-raised errors,
and the debugger-style breakpoint path keeps the next-instruction address.

### Source Metadata

RXAS `.srcfile` and `.src` emit `META_FILE` and `META_SRC` records.
`metaloaddata module,address` returns metadata exactly at an address.

The prototype debugger has two useful patterns:

- `next_rexx()` only reports source when `META_SRC` exists at the exact address
- watch-variable lookup scans metadata backwards from `addr - 1` to find the
  most recent variable metadata

For error reporting, the right rule is closest preceding source metadata. A
fault may occur at a generated instruction inside a source clause, not only at
the instruction where `.src` was emitted.

For breakpoint stepping, exact-address source lookup is useful because stepping
should stop at authored clause boundaries in REXX mode.

## Current Compiler Exit Facts

The compiler exit bridge is now the preferred mechanism for statement-level
language extensions that can be lowered into ordinary Level B source.

Facts relevant to `TRACE`:

- certified exits can own reserved keywords
- `ADDRESS` and `PARSE` are already certified exits
- `TRACE` is listed as a built-in keyword but is not currently a certified exit
- the lexer has old `TRACE` token rules commented out, so `TRACE` can follow
  the certified-exit model
- `describe()` declares the exit, default imports, and flags
- `pre_process()` is the only structural phase
- `process()` can lower a statement into replacement source
- helper procedures can be inserted from `pre_process()`

Facts relevant to Level B signals:

- `SIGNAL` is also listed as a built-in keyword and its old lexer token is
  commented out
- a simple `SIGNAL name` statement could be implemented as a certified exit
- structured error handling may need compiler-owned AST/control-flow support,
  because exit replacement alone cannot safely own arbitrary block boundaries
  unless the syntax remains a normal statement that lowers to existing control
  flow

## Design Goals

### Signals

The Level B signal design should:

- provide comfortable, simple error handling
- support raising a signal from Rexx
- support handlers for VM/runtime signals
- expose a friendly `.signal` object
- avoid classic REXX `SIGNAL` as arbitrary `goto`
- allow procedure handlers
- allow block-local handlers without user-visible labels
- make frame and scope behavior explicit
- preserve low-level RXAS signal capabilities for debugger/runtime code

### Trace

The trace design should:

- implement the `TRACE` statement through a certified compiler exit
- reuse the rxdb breakpoint/source/metadata machinery rather than duplicating it
- move reusable debugger logic into a Level B runtime class
- support ordinary on/off trace controls first
- allow more powerful trace scopes later
- default to not tracing formal CREXX runtime/compiler namespaces
- avoid tracing the trace machinery itself

## Proposed Signal Model

### User-Facing Signal Object

Add a Level B signal interface in the Level B library namespace. The public
runtime/BIF/helper surface is already centred there, and keeping signal support
there avoids making ordinary programs import a second runtime namespace just to
handle errors.

```rexx
namespace rxfnsb expose signal signalaction

signal: interface
  *: factory
  arg name = .string
  *: factory
  arg name = .string
  arg message = .string
  *: factory
  arg name = .string
  arg payload = .object
  name: method = .string
  code: method = .int
  module: method = .int
  address: method = .int
  message: method = .string
  payload: method = .object
  file: method = .string
  line: method = .int
  column: method = .int
  source: method = .string
```

The same namespace should expose `.signalaction`:

```rexx
signalaction: interface
  retry: factory
  skip: factory
  fail: factory
  kind: method = .string
```

Named factories make the intended handler outcome readable:

```rexx
return .signalaction.skip()
return .signalaction.retry()
return .signalaction.fail()
```

Concrete classes can implement `.signal`, for example:

- `standard_signal`: a normal Rexx-created signal
- `runtime_signal`: an internal wrapper around the raw VM interrupt object

That gives the language a stable signal contract while leaving room for
multiple signal implementations. User code should normally work with `.signal`,
not with a concrete signal class.

Keep the public `.signal` factory shape focused on Rexx-created signals:
`.signal(name, message)`. The raw VM object is implementation transport. The
current `runtime_signal` implementation uses the ordinary factory to create the
wrapper object and an internal `set_raw(raw)` method to attach the VM interrupt
object inside compiler-generated handler glue. That avoids making every
`.signal` implementation support a VM-only factory signature and leaves normal
interface factory/match selection semantics undisturbed.

`runtime_signal` should use Level B's low-level physical attribute/register
mapping rather than copying the VM object through a hand-written adapter. The
raw VM interrupt object already has fixed slots for code, module, address,
name, and payload/message object. Mapping those directly into an internal class
keeps VM integration clean: the public `.signal` methods can expose friendly
fields and source metadata while the internal representation stays close to the
VM transport.

Recommended attributes:

- `code = .int`
- `name = .string`
- `module = .int`
- `address = .int`
- `message = .string`
- `payload = .object`
- `file = .string`
- `line = .int`
- `column = .int`
- `source = .string`

The raw VM interrupt object should remain an internal transport. The public
runtime should wrap it as `.signal` / `runtime_signal` by:

1. reading the five VM attributes
2. resolving closest preceding `META_FILE` / `META_SRC` for module/address
3. exposing the message/payload through the friendly interface

Decision:

- keep `payload` typed as `.object`
- derive `message` from `payload.tostring()` or equivalent when the payload is
  not already a string
- use the Level B library namespace (`rxfnsb`) for public `.signal` and
  `.signalaction`
- keep any implementation-only helpers unexposed, or in the internal runtime
  namespace when they are not part of the Level B library contract

### Raising Signals

The canonical raising form should take a `.signal` object:

```rexx
signal .signal("error")
signal .signal("error", "message")
signal rxfnsb..signal("conversion_error", payload)
```

Convenience syntax should lower to that canonical form:

```rexx
signal failure
signal failure "message"
signal failure payload
```

Recommended lowering:

```rexx
signal failure
```

means:

```rexx
signal .signal("failure")
```

and:

```rexx
signal failure "message"
```

means:

```rexx
signal .signal("failure", "message")
```

To avoid ambiguity, the compact `signal name` form should treat a bare
identifier after `signal` as a signal name, not as a variable reference. Dynamic
signal names should use the factory form:

```rexx
signal .signal(signal_name)
```

Decision:

- unknown literal names should be compile errors
- dynamic names created through `.signal(name)` should produce
  `INVALID_SIGNAL_CODE` at runtime if unresolved

Open decision:

- whether `raise` should be accepted as a clearer alias later

### Procedure Handlers

Recommended procedure-scoped handler form:

```rexx
signal on conversion_error call handle_conversion
signal on error, syntax call handle_problem
signal off conversion_error
```

The handler procedure receives one `.signal` argument.

Procedure handlers should return a small `.signalaction`, not an arbitrary
application value. The essential actions are:

- `retry`: retry the faulting instruction
- `skip`: continue after the signal point
- `fail`: propagate the signal after the handler returns

For safety, a procedure handler that falls off the end or returns `.void`
should default to `fail`, not `skip`. Accidental continuation after a VM fault
such as `CONVERSION_ERROR` can leave an output register undefined or misleading.
Handlers that deliberately consume a signal should say so explicitly:

```rexx
handle_conversion: procedure = .signalaction
  arg condition = .signal
  say condition.source()
  return .signalaction.skip()
```

`skip` maps to the current `SIGCALL` resume behaviour. `retry` and `fail`
require VM support to interpret the handler's return value when an interrupt
frame returns. The VM already records the relevant fault/resume address
distinction, but the return path must make the action explicit.

This form is for frame/procedure-scoped handlers. Block-scoped handlers use the
`do ... on signal ... end` form below.

### Block Handlers

Do not add user-visible label handlers. The block-local syntax should express
the protected body and its handlers directly:

```rexx
do
  risky_work()
  more_risky_work()
on signal conversion_error as condition
  say condition.source()
on signal error, syntax as problem
  call cleanup()
  signal problem
on signal
  call log_unhandled_signal()
end
```

Semantics:

- statements before the first `on signal` clause are the protected body
- `on signal` clauses are handlers, not labels
- `on signal` with no signal names catches all signals
- normal execution of the protected body skips all handler clauses
- `as name` binds the current `.signal` object to that local name
- if `as name` is omitted, the signal object is not available in that handler
- a handler that completes normally leaves the `do` block
- a handler can re-signal explicitly with `signal problem` when it bound the
  object with `as problem`

This avoids classic REXX `SIGNAL` as goto while keeping the handler physically
close to the protected code.

Decision:

- support `as name`
- do not create an implicit `condition` variable
- omit `as name` when a handler only needs to perform fixed cleanup/logging
- use bare `on signal` as the catch-all signal handler form
- do not add `finally` in this design; `on signal` is the broad signal trap

### Handler Restoration

Because the VM handler table is per-frame, not per-block, the compiler must
restore prior handlers around block-scoped handling.

There is no RXAS instruction today that snapshots/restores a handler table
entry. The compiler can emit simple set/unset instructions only if it knows the
previous state.

Decision:

- add VM/RXAS support to push and pop one signal handler entry
- use that support for block-scoped `do ... on signal ... end`
- keep procedure-scoped `signal on/off` mapped to the existing per-frame table
- bind any push/pop handler stack to the current stack frame
- automatically clear any unpopped pushed handlers when the stack frame exits
  or is recycled

Recommendation:

- implement procedure-scoped `signal on/off` first
- then add block-scoped handling once push/pop exists
- still emit explicit pop operations on normal block exit; frame-exit cleanup is
  the safety net for propagation, early return, and frame recycling

## Proposed Trace Model

### Runtime Trace Controller

Extract reusable logic from `debugger/rxdb.rexx` into Level B library classes:

```rexx
namespace rxfnsb expose tracecontroller tracecontext
```

Recommended user/runtime names:

- `.tracecontroller`: owns breakpoint installation, filtering, and trace output
- `.tracecontext`: immutable per-event context passed to output/filter logic

Responsibilities:

- install/uninstall the `BREAKPOINT` handler with `sigcall` and `bpon/bpoff`
- receive the raw VM interrupt object
- convert module/address to a trace context
- resolve exact REXX source for stepping and closest preceding source for
  diagnostics
- decode RXAS instructions when ASM-level trace is enabled
- apply filters
- print or otherwise emit trace lines

The existing `rxdb` can later become a client of this class rather than owning
all trace logic itself.

### Trace Context

The trace controller should build a context object with at least:

- signal code/name
- module/address
- file/source/line/column
- procedure name when resolvable from metadata
- decoded instruction when ASM trace is active
- current mode flags

This object should be distinct from `.signal`, but both can share a source
lookup helper.

### Default Exclusions

Trace should not normally trace:

- the trace controller module/class
- `rxdb`
- signal support classes such as `runtime_signal` and `signalaction`
- compiler-exit support namespaces such as `rxcp`, `rxcpexits`, and
  `rxcptest`
- core runtime namespaces such as `_rxsysb`
- standard library code in `rxfnsb`, unless explicitly included
- helper procedures inserted by compiler exits

Filtering should be based on module/procedure metadata rather than module
number. The prototype debugger's "last module only" rule is useful for rxdb
but too fragile for general TRACE.

Open decisions:

- whether the compiler should mark generated helper procedures with metadata
  to make filtering exact

### TRACE Statement Surface

Implement `TRACE` as a certified compiler exit.

Minimum useful forms:

```rexx
trace off
trace normal
trace rexx
trace asm
```

Candidate extended forms:

```rexx
trace rexx include namespace myapp
trace rexx exclude namespace rxfnsb
trace rexx procedure current
trace asm module current
trace ?rexx
```

Recommended first stage:

- `trace off`
- `trace normal`
- `trace rexx`
- `trace asm`

Recommended meaning:

- `off`: uninstall/disable trace handling
- `normal`: trace authored Rexx clauses in user code, excluding formal runtime
  namespaces
- `rexx`: like `normal`, but explicitly source-oriented and suitable for later
  source-level filtering extensions
- `asm`: include RXAS/VM instruction-level information as well as source where
  available

Defer `trace all` until include/exclude scope controls exist. Otherwise "all"
is too likely to mean "trace the trace runtime", which is rarely useful.

Lowering should call a stable runtime API, not inline large debugger logic at
each trace statement.

Example lowering shape:

```rexx
call _trace_set("rexx")
```

The trace exit should add the trace runtime import through its descriptor or
plan.

### Interaction With Signals

`TRACE` uses `BREAKPOINT`. Signal/error handling uses ordinary signal codes.
They share the same interrupt delivery loop, but they should keep separate
semantics:

- `BREAKPOINT` is a step/resume mechanism and carries next-instruction address
- error signals carry the faulting instruction address
- trace handlers should not be interrupted by their own breakpoint delivery
- ordinary signal handlers should not unexpectedly enable tracing inside
  formal runtime code

## Recommended Implementation Phases

### Phase 0: Documentation And Tests For Current VM Facts

- Add stable VM docs for signal handler tables, `is_interrupt`, handler
  responses, and interrupt object attributes.
- Add or keep RXAS tests for `SIGCALL`, `SIGCALLBR`, `SIGRET`, `BREAKPOINT`,
  and fallback panic source reporting.
- Add helper tests around closest preceding source lookup if the helper becomes
  public.

Status on 2026-04-28:

- stable VM docs now describe the current signal table, interrupt object, and
  breakpoint/fault address rules
- focused RXAS coverage exists for:
  - fallback panic module/address/source reporting
  - `BREAKPOINT` next-instruction source lookup
  - `SIGCALL` raw interrupt object attributes
  - `SIGIGNORE`, `SIGBR`, `SIGCALLBR`, `SIGRET`, `SIGSHALT`
  - per-frame handler table inheritance and child-frame isolation
  - nested pending signals while `is_interrupt` suppresses immediate delivery

### Phase 1: Procedure-Scoped Level B Signals

- Add a certified `SIGNAL` exit or compiler support for simple signal
  statements.
- Add `.signal` and raw-VM-object-backed `runtime_signal`.
- Add `.signalaction` with `retry`, `skip`, and `fail`.
- Support:
  - `signal .signal("name")`
  - `signal name`
  - `signal name message-or-payload`
  - `signal on name call procedure`
  - `signal off name`
- Add VM support for procedure-handler return actions if this phase includes
  `.signalaction`.
- Compile to existing RXAS where possible.
- Document exact handler resume semantics.

This phase deliberately avoids block-local handlers.

Status on 2026-04-28:

- added `rxfnsb.signal`, `rxfnsb.runtime_signal`, and `rxfnsb.signalaction`
- added certified `SIGNAL` compiler exit support for:
  - `signal name`
  - `signal name payload`
  - `signal .signal(name[, payload])`
  - `signal on name[, name...] call procedure`
  - `signal off name[, name...]`
- added VM/RXAS `sigcalla` for action-aware procedure handlers
- added dynamic-name RXAS signal forms backed by `INVALID_SIGNAL_CODE` on
  unknown runtime names
- procedure handlers receive `.signal`; returning `.signalaction.skip()` resumes
  after the signal, `.retry()` resumes at the interrupted address, and
  `.fail()` falls through to the default panic report with source metadata
- inline cloning now preserves named interface factory selector metadata, so
  handler code returning `.signalaction.skip()` keeps the `..skip` selector in
  optimized builds even when a generated helper body is optimized
- `signal off` restores the VM root default (`sighalt` or `sigignore` for the
  VM's default-ignored signals); true handler stack push/pop remains Phase 2

### Phase 2: Block-Scoped Error Handling

- Add handler push/pop support if not already implemented in phase 1.
- Implement `do ... on signal ... end`.
- Implement `on signal` with no names as the catch-all signal handler.
- Implement optional `as name`; without it, no signal object is bound.
- Ensure normal execution skips handler clauses.
- Ensure handler completion leaves the protected block.
- Add tests for nested blocks, nested procedures, handler restoration, and
  signal propagation.

### Phase 3: Trace Runtime Extraction

- Move rxdb source/ASM lookup into a reusable class.
- Add trace context/source helper.
- Add default namespace/procedure filtering.
- Refactor rxdb to use the class where practical.

### Phase 4: TRACE Certified Exit

- Add `TRACE` to the certified exit allowlist and `rxcexits` bundle.
- Lower trace statements to runtime API calls.
- Add tests for on/off, REXX mode, ASM mode, and default exclusions.
- Add parser-mode/highlighting tests for trace keywords.

## Approval Gates

Before implementation, these decisions need explicit approval:

1. Final spelling of `.signalaction` factories/methods.
2. Whether `retry`, `skip`, and `fail` must all be in phase 1, or whether
   `retry` can wait for a second VM pass.
3. Whether `trace normal` and `trace rexx` should be distinct in phase 1 or
   aliases until scope controls are added.

## Current Recommendation

Proceed in this order:

1. Update stable VM docs for the current signal and breakpoint model.
2. Implement `.signal`, `.signalaction`, signal raising, and procedure-scoped
   handlers.
3. Add VM/RXAS handler push/pop and block-scoped `do ... on signal ... end`.
4. Extract the trace controller after the signal context model exists.
5. Implement `TRACE` as a certified exit over that controller.

The design now deliberately avoids user-visible labels. The main remaining
risk is handler outcome semantics, especially `retry`, because it requires the
VM return path from an interrupt frame to choose between fault address, resume
address, and propagation.

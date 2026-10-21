# Optional Level L IR execution

`rxfnsl-execute.rxbin` supplies the neutral `llirexecute` prototype. It follows
IR control and owns execution frames; registered handlers supply operation and
value semantics. It knows no Classic variable names, BIFs, source grammar or
RXAS instructions. The core SDK, grammar bootstrap and static applications
remain independent of this package.

Build target `rxfnsl_execute`. A consumer imports `llirexecute` and links the
execute image with `rxfnsl-analysis.rxbin`, `rxfnsl.rxbin` and `library.rxbin`.
Normal SDK installation supplies the separately named image; `stage-l-libraries`
prepares all four SDK images without adding them to core/bootstrap imports.
The [independent fixture](../tests/library/ts_llirexecute.crexx) shows an entire
non-cREXX family, graph, registry and execution. Compiler-specific adapters and
the Classic runtime feasibility fixture live under research, outside this
public package.

## Admission and execution

Construct `registry(contract)` with the trusted frozen schema contract,
register type services, then named operation
handlers. Bind each extension opcode to a handler name, a sealed
`llireffects.summary` and an output-byte bound. Multiple opcodes can share one
named handler and its mutable resources. `registry.call` binds an opcode to a
local IR function; the coordinator owns the call/return transfer. Runtime
handlers are separate from `llir.evaluator` constant-fold callbacks, and saved
IR cannot load host code.

`plan(reference graph,registry,work,bytes)` preflights the entire frozen module,
including unreachable operations, before execution. The registry's exact
schema/family-version manifest must match that module. The plan owns a module
snapshot; replacing or releasing the original graph does not invalidate it.
The prototype admits core constants, arguments, optional arguments, argument
presence, slots, jumps, branches and returns. `core.protect`, unsupported
control, unregistered services, opaque effects and borrowing/escaping operand
profiles are rejected. Conditional extension results require a later
availability protocol. Check `valid()`, `status()` and `message()` before
constructing a session.

`session(plan,fuel,frames,values,bytes)` owns an independent copy of the prepared
plan, handler state and all frame values. Reusing a plan avoids preflight, but
does not avoid those B value copies. `start(function_id,values,present)` accepts ordered
`value` objects and explicit presence flags. A present empty value differs
from an omitted argument. Local-call inputs form a present prefix; the
remaining formal tail is omitted. This profile has no middle-omission local
call mapping. `start()` runs to completion or suspension. `run()` returns the
retained boundary result after that; it does not service a suspension.

Values carry a type name, exact binary encoding and an optional strongly owned
immutable payload. Type handlers validate/materialize values and report their
logical size. An immutable payload may wrap existing language-owned value
objects; the coordinator does not implement a second value, numeric or pool
algorithm. Type services must be pure and immutable. Runtime handlers may
mutate their session-owned resources, and each dispatch writes the changed
handler back to its single authoritative entry.

## Results and ownership

Results distinguish `done`, `failure`, `suspended`, `limit`, `cancelled`,
`discarded` and `rejected`. Completed values retain presence independently of
their bytes. Failures retain code, message, exact binary payload and operation
origin. A terminal failure publishes no partial successful result.

An explicit handler suspension returns a token for the original session and
its current generation. Keep that original session alive: a token validates
authority and does not extend the session's lifetime. `resume(token,outcome)` validates completion before
consuming the token and proceeding past the suspended operation exactly once.
Malformed completion leaves that suspension available for correction.
`discard(token)` releases it; `cancel()` also works while suspended. Foreign,
stale and already consumed tokens are rejected without changing the valid
session. Copying an active session does not copy its execution authority:
stateful use of that copy is rejected. A ready session may be copied before
starting independent runs.

Suspension owns its nested call stack, arguments, slots and pending results;
it does not borrow a handler's VM locals. Each retained named handler receives
one terminal cleanup notification. Cleanup continues if another notification
raises, preserves the primary execution failure, clears frame/pending payloads and never
turns cleanup failure into successful completion. Handler inspection returns
a value copy, without granting mutation authority over session state.
Immutable materialized constants follow the plan's lifetime.

## Resource and product limits

Preflight bounds each analysis stage's work and logical resident plan/temporary
analysis bytes. Execution bounds
coordinator fuel, call depth, frame cells and logical value bytes. Cells include
arguments, slots, immutable results and pending completion positions. Frame
admission and declared handler output reservations precede effects. Results
and resumed completion must fit the reserved types, arity and bytes before
control advances. Exceeding a handler's declared bound is a protocol failure.
The same reservation bounds retained control code/message/payload bytes;
normal outcomes contain no control packet. A supplied origin must be zero
(use the dispatched operation) or that operation's origin.
Frames conservatively retain storage; no liveness-based reclamation is claimed.

These are logical limits rather than process RSS limits. Fuel/cancellation
operate between dispatches and cannot interrupt an uncooperative callback.
Handler-owned external resources need separate host contracts and budgets.
Public execution calls are synchronous; handler reentry into the same running
session is rejected. Cancellation does not preempt an executing callback.

The prototype establishes an optional execution substrate. It does not port
RexxScript, execute the complete C22 profile or implement Classic `INTERPRET`.
In particular, retaining the runner's own frames does not prove a live compiled
caller's pool, labels, conditions or VM-frame lifetime. The
[Group 4 architecture](../../research/level-l/compilers/doc/architecture.md#group4-execution-contract)
owns that assessment and the remaining product decisions.

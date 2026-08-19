# Concurrent programming

cREXX concurrency is designed to feel like Rexx: declare work much like a
procedure, call it with ordinary call syntax, and introduce explicit control
only when you need it. The runtime supplies bounded worker threads or isolated
worker processes without exposing thread IDs, locks or shared writable VM
objects.

The current surface is initial on `develop` and requires `OPTIONS
LEVELG` for task declarations, task targets and `DO PARALLEL`. The underlying
pool, scope, channel and value classes remain available to Level B programs.
Portable publication is tracked separately from implementation status.

## Five ideas to learn first

1. A **task** is one independently schedulable invocation, not an OS thread.
2. A **pool** supplies a bounded amount of execution capacity. A pool is an
   object created by `.taskpool.local(...)` or `.taskpool.process(...)`.
3. A **scope** owns the lifetime of every task submitted through it. Leaving a
   scope accounts for every child; ordinary tasks cannot detach.
4. Values cross an execution boundary by value. Each receiver gets its own
   reconstructed data or object; ordinary mutable Rexx objects are not shared.
5. `DO PARALLEL` does not send every clause to a worker. Ordinary clauses stay
   on the controlling execution. Only task calls and explicit submissions
   create task work.

That last rule makes a parallel block predictable. File I/O, logging, result
combination and other ordinary operations keep their usual ordering unless you
put them in a task deliberately.

## Your first task calls

A task procedure replaces `PROCEDURE` with `TASK`:

```rexx
options levelg comments_dash
namespace first_concurrency

first: task = .int
  return 20

second: task = .int
  return 22

main: procedure = .int
  answer = first() + second()
  say answer
  return 0
```

`first()` and `second()` still look like Rexx function calls. Because both are
independent task calls in one expression, cREXX submits them in source order,
allows their bodies to overlap, waits for both results, and then performs the
ordinary `+`. It has not made addition asynchronous and has not changed the
meaning of a normal function call.

This is the simple surface for everyday use. A call such as
`task1() + task2()` can exploit concurrency without requiring the programmer to
create future objects or write an explicit join.

Argument and receiver expressions are still evaluated left to right. Short
circuiting is also unchanged: a task in the unselected side of `&` or `|` is
not submitted.

## Grouping work with `DO PARALLEL`

Use `DO PARALLEL` when several statements should share one structured scope:

```rexx
twice: task = .int
  arg value = .int
  return value * 2

plusone: task = .int
  arg value = .int
  return value + 1

ordinary: procedure = .int
  return 1

main: procedure = .int
  do parallel
    left = twice(20)
    right = plusone(1)

    controller_value = ordinary()

    if left <> 40 then return 1
    if controller_value <> 1 then return 2
  end

  if right <> 2 then return 3
  return 0
```

The task assignments create pending typed bindings. The tasks may run while
the controller calls `ordinary()`. Reading `left` materializes that result and
waits if necessary. Even though `right` is not read inside the block, `END`
still accounts for it before control continues.

A pending binding is deliberately not a public future object. Before it is
materialized you cannot reassign it, take a reference to it, mutate through it
or return it from the block. These restrictions keep scope cleanup reliable.

Void tasks work naturally with `CALL`:

```rexx
announce: task
  arg text = .string
  say text
  return

do parallel
  call announce "worker one"
  call announce "worker two"
end
```

The two calls share the block scope and both complete by `END`. Their output
order is not defined. Outside `DO PARALLEL`, `CALL announce ...` owns an
implicit statement scope and completes before the next statement.

The complete runnable version is
[`examples/concurrency_basic.crexx`](examples/concurrency_basic.crexx).

## Returning a value from a parallel block

The existing expression-form `DO` also supports `PARALLEL`:

```rexx
answer = do parallel
  left = twice(20)
  right = plusone(1)
  leave with left + right
end
```

`LEAVE WITH` supplies the block expression's result. The task results are
materialized before addition and the scope closes before `answer` is assigned.

## Pools are policy; scopes are lifetime

The simple forms use an execution-local default pool. Create a pool explicitly
when worker count, admission capacity, failure policy, deadline or isolation is
part of your program's design.

```rexx
square: task = .int
  arg value = .int
  return value * value

increment: task = .int
  arg value = .int
  return value + 1

run_pair: procedure = .int
  arg pool = .taskpool

  scope = .taskscope.collectall(pool, 60000)
  answer = do parallel using scope
    squared = square(6)
    next = increment(5)
    leave with squared + next
  end
  return answer

main: procedure = .int
  local_pool = .taskpool.local(2, 8)
  local_answer = run_pair(local_pool)
  call local_pool.close()

  process_pool = .taskpool.process(1, 8)
  process_answer = run_pair(process_pool)
  call process_pool.close()

  if local_answer <> 42 | process_answer <> 42 then return 1
  return 0
```

The two numbers passed to a pool factory are worker capacity and bounded
admission capacity. A local pool runs fresh Rexx executions in the same
runtime. A process pool runs fresh executions in isolated worker processes.
Changing provider does not change task syntax.

`DO PARALLEL USING scope` consumes and closes that scope at `END`, including
abnormal exits, but it does not close the pool. A scope cannot be reused.
Close the pool only after every scope created from it has closed.

Capacity controls possible overlap; it does not promise a schedule. The
process pool above deliberately has capacity one, so the two task bodies run
serially while retaining exactly the same structured result semantics.

The checked source is
[`examples/concurrency_providers.crexx`](examples/concurrency_providers.crexx).

## Choosing fail-fast or collect-all

`.taskscope.failfast(pool, milliseconds)` requests cancellation of siblings
after the first unsuccessful child. `.taskscope.collectall(...)` continues to
account for the remaining children. Both policies join every accepted child.

The timeout is a relative scope deadline in milliseconds:

- `-1` means no deadline or indefinite observation;
- `0` means immediate/nonblocking observation; and
- a positive value is a finite relative wait or deadline.

Cancellation is cooperative for a task already running in a local worker. An
isolated process provider can terminate and replace its worker if cooperative
cancellation does not finish in its grace period. Therefore task code should
check its `.taskcontext` at sensible boundaries when using the advanced
`.taskwork` interface.

## Values do not become shared objects

Primitive arguments and results cross directly as canonical values. A concrete
class can cross when it declares an exact pair of conversion members:

```rexx
numberbox: class
  _value = .int

  *: factory
    arg value = .int
    _value = value
    return

  from_channel: factory
    arg encoded = .channelvalue
    _value = encoded.as_integer()
    return

  to_channel: method = .channelvalue
    return .channelvalue.integer_value(_value)

  value: method = .int
    return _value
```

The same rule applies to the receiver of a task method:

```rexx
calculator: class
  _base = .int

  *: factory
    arg base = .int
    _base = base
    return

  from_channel: factory
    arg encoded = .channelvalue
    _base = encoded.as_integer()
    return

  to_channel: method = .channelvalue
    return .channelvalue.integer_value(_base)

  add: task = .numberbox
    arg amount = .numberbox
    return .numberbox(_base + amount.value())
```

Now ordinary method syntax performs a task invocation:

```rexx
calculator = .calculator(40)
pool = .taskpool.local(2, 8)
scope = .taskscope.collectall(pool, 60000)

answer = do parallel using scope
  leave with calculator.add(.numberbox(2))
end

call pool.close()
say answer.value()
```

The controller encodes the receiver and argument. The worker reconstructs its
own objects, performs the method, encodes the result, and the controller
reconstructs a new `.numberbox`. Neither side shares live object identity.

The full checked example is
[`examples/concurrency_transferable_objects.crexx`](examples/concurrency_transferable_objects.crexx).

## Advanced work classes and factories

Use `.taskwork` when you want an explicit target, a canonical application
request and direct completion inspection. It is the low-level runnable
interface beneath higher-level typed task calls.

```rexx
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
```

The factory expression becomes a sealed task target:

```rexx
pool = .taskpool.local(1, 4)
scope = .taskscope.collectall(pool, 60000)
target = task .adderwork(2)
child = scope.submit(target, .channelvalue.integer_value(40))
completion = child.wait(-1)

if completion.succeeded() then answer = completion.value().as_integer()
else say completion.error_code() completion.message()

call scope.finish()
call pool.close()
```

The pool does not create the `.adderwork` object in the controller and then
share it. The target records the statically known factory; its arguments cross
as values and the receiver object is constructed in the worker.

`target.name()` is diagnostic information. Dispatch uses a linker-sealed
callable descriptor, not a procedure-name string. Users should create targets
with `task callable` or `task .class(...)`, not by constructing the underlying
80-byte binding.

The full local/process example is
[`examples/concurrency_taskwork.crexx`](examples/concurrency_taskwork.crexx).

## Observing tasks and completions

The Level B path exposes these useful operations:

- `child.wait(milliseconds)` observes that particular task;
- `child.completion()` is a nonblocking observation;
- `scope.next(milliseconds)` observes the next child in completion order; and
- `scope.join()` returns every child in stable submission order.

A timed or nonblocking miss is a `.completion` with `available() = 0`, not a
failed child. A terminal completion can be `SUCCEEDED`, `FAILED`, `CANCELLED`,
`DEADLINE_EXCEEDED`, `REJECTED`, `ENDPOINT_CLOSED`, `TRANSPORT_LOST` or
`UNKNOWN_OUTCOME`.

`scope.finish()` joins, closes and raises `TASK_FAILURE` if a child did not
succeed. Use explicit `wait()`/`join()` and inspect completion data when your
application wants to handle each outcome itself. `scope.abort(reason)` is the
safe cancellation, join and close path during controller-side cleanup.

## Recursion and nested task calls

A direct self-call inside a task is ordinary synchronous recursion in that
worker:

```rexx
factorial: task = .int
  arg value = .int
  if value <= 1 then return 1
  return value * factorial(value - 1)
```

The recursive calls are not resubmitted. Calling a different task from a task
body would need a blocking nested wait and is rejected in the current surface.
Move the dependency to the controller, where cREXX can represent and join it
without consuming a worker that waits for its own bounded pool.

## HTTP uses the same task model

The Level G HTTP client declares `get`, `post`, `request` and `post_stream` as
task methods, so ordinary calls and `DO PARALLEL` use exactly the rules in this
chapter. The HTTP server uses the explicit form: its controller submits a
sealed `task .my_handler()` factory target for each complete request. A handler
implements `.httpservice .taskwork`; it is stateless task work, not the future
durable service represented by `.serviceref` and `.taskscope.ask()`.

The [HTTP client and server guide](../crexx_library_reference/concurrent_http.md)
contains a complete handler class, parallel client calls, lifecycle rules and
links to both-VM executable fixtures.

## What not to design around yet

The following names or concepts are reserved rather than usable today:

- `.taskscope.ask()` and concrete single-owner services;
- `.taskpool.queued()` and `.taskpool.running()` telemetry;
- cross-host provider type `3` and a public provider-plugin ABI;
- detached tasks, worker affinity and public thread IDs; and
- shared mutable globals, locks or atomics between Rexx executions.

Inside `.taskwork.run()`, `.taskcontext.endpoint(reference)` reconstructs a
worker-local byte endpoint from a transferable type-4 provider reference.
Controller-side code can use `.byteendpoint.from_reference()` directly.

## Checked example catalogue

These examples are compiled with `rxc`, assembled with `rxas`, linked with
`rxlink`, and executed on `rxbvm` and `rxtvm` in optimized and unoptimized
modes:

| Example | Demonstrates |
| --- | --- |
| [`concurrency_basic.crexx`](examples/concurrency_basic.crexx) | ordinary task calls, `DO PARALLEL`, controller clauses and `CALL` tasks |
| [`concurrency_providers.crexx`](examples/concurrency_providers.crexx) | explicit pool/scope policy and local/process selection |
| [`concurrency_transferable_objects.crexx`](examples/concurrency_transferable_objects.crexx) | task method receiver, object argument and object result transfer |
| [`concurrency_taskwork.crexx`](examples/concurrency_taskwork.crexx) | `.taskwork` factories, explicit submission and completion inspection |

For formal syntax and compile-time restrictions, see the language reference's
[Concurrency](../crexx_language_reference/concurrency.md) chapter. For every
Level B class and method, see the library reference's concurrency chapter.

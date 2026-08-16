# Concurrency {#language-concurrency}

This chapter defines the current cREXX task and parallel-block language
surface. It is an experimental Level G extension. The Level B class contracts
used by its lowering are specified in the library reference.

## Language level and contextual words

Task declarations, task-target expressions and both forms of `DO PARALLEL`
are accepted only when the source selects:

```rexx
options levelg
```

`TASK` and `PARALLEL` are case-insensitive contextual words. They are not
globally reserved. A variable or ordinary routine named `task` remains valid
when the token is not in one of the grammar positions defined below;
`parallel` remains an ordinary symbol except immediately after the opening
`DO` of a parallel block.

The Level B classes `.taskpool`, `.taskscope`, `.task`, `.tasktarget`,
`.taskwork`, `.taskcontext`, `.completion`, `.channel`, `.channelrequest`,
`.channelvalue`, `.channelcodec`, `.byteendpoint`, `.serviceref` and
`.transferbuffer` do not require Level G.

## Task callable declarations

The callable forms are:

```ebnf
task-procedure ::= symbol ":" "task" [ "=" type ] callable-body
task-method    ::= symbol ":" "task" [ "=" type ] callable-body
```

At module scope the form declares a task procedure. Within a class it declares
a task method. Arguments use the normal typed `ARG` statement. Omitting the
result type declares a `.void` task.

```rexx
options levelg
namespace task_example

checksum: task = .int
  arg text = .string
  return length(text)

notify: task
  arg text = .string
  say text
  return
```

The task modifier controls submission when the callable is invoked from a
controlling execution. The body itself remains an ordinary callable body
executed in one worker-owned Rexx execution.

### Transferable signature types

The current typed task-call lowering accepts these scalar argument/result
types:

- `.boolean`;
- `.int`;
- `.string`;
- `.binary`; and
- `.void` as an omitted result only.

A `.channelvalue` is accepted as a direct task argument. A concrete class is
accepted as an argument, result or task-method receiver only when the exact
static class declares the transfer contract in the next section.

Current task signatures do not accept arrays, references, varargs,
`ARG EXPOSE`, `.float`, `.decimal`, an interface result, untyped `.object`, or
another type without the exact transfer contract. RXCV can represent more
value kinds than the Level G typed-call sugar currently lowers; the broader
Level B `.channelvalue` and `.taskwork` path remains available when needed.

## Transfer contract for concrete classes

A transferable concrete class declares exactly resolved conversion members:

```rexx
from_channel: factory
  arg encoded = .channelvalue

to_channel: method = .channelvalue
```

`to_channel` has no arguments. `from_channel` has exactly one ordinary
`.channelvalue` argument. Both members must be declared on the exact concrete
class known at the task call site; an interface is not enough to select unique
reconstruction code.

The receiver and argument direction is controller `to_channel`, worker
`from_channel`. The result direction is worker `to_channel`, controller
`from_channel`. Reconstruction creates new receiver-owned objects. No live
object identity or reference crosses the boundary.

This complete task-method example uses the transfer contract for its receiver,
argument and result:

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

Its ordinary-looking method call is a task call:

```rexx
calculator = .calculator(40)
answer = calculator.add(.numberbox(2))
```

The complete executable program is
[`../crexx_programming_guide/examples/concurrency_transferable_objects.crexx`](../crexx_programming_guide/examples/concurrency_transferable_objects.crexx).

## Calling a task

A call resolved to a task callable uses the existing Rexx procedure/function
or method-call grammar. No `ASYNC` call prefix is added.

```rexx
answer = checksum("hello")
call notify "complete"
answer = calculator.add(.numberbox(2))
```

Outside `DO PARALLEL`, a task-valued expression has an implicit expression
scope that closes before its containing statement completes. A `CALL` task has
an implicit statement scope and completes before the next statement.

### Expression evaluation

For an expression containing task calls, the implementation:

1. evaluates ordinary receiver and argument expressions left to right;
2. encodes each task's inputs after those inputs are available;
3. submits ready tasks in source evaluation order;
4. permits independent submitted bodies to overlap;
5. evaluates ordinary calls and operators in their existing controller order;
   and
6. applies an operator after the task operands it needs have materialized.

Thus:

```rexx
answer = first() + second()
```

submits `first` before `second`, permits the two task bodies to overlap, waits
for both successful values, and then performs ordinary addition. It does not
make `+` asynchronous.

In `first() + ordinary()`, the controller may call `ordinary()` while
`first()` runs, but `ordinary()` is not moved to a worker. Existing
short-circuit behavior is retained. A task in an unevaluated right branch is
not submitted.

The complete positive expression/block program is
[`../crexx_programming_guide/examples/concurrency_basic.crexx`](../crexx_programming_guide/examples/concurrency_basic.crexx).

## Parallel blocks

The grammar is:

```ebnf
parallel-do ::= "do" "parallel" [ "using" expression ]
                clauses
                "end" [ symbol ]
```

The form is available wherever the corresponding ordinary `DO ... END` form
is available. In expression position the body must produce a result through
the existing `LEAVE WITH expression` statement.

```rexx
do parallel
  first_value = first()
  second_value = second()
  say "controller is still running ordinary clauses"
end
```

Only calls to task callables and explicit Level B submissions create child
work. Other clauses execute sequentially in the controlling execution.

All task calls in one block belong to one scope. Without `USING`, the compiler
creates a scope over the execution-local default pool. `USING expression` is
evaluated once before the block opens and must yield a fresh open `.taskscope`:

```rexx
pool = .taskpool.local(2, 8)
scope = .taskscope.collectall(pool, 60000)

answer = do parallel using scope
  left = first()
  right = second()
  leave with left + right
end

call pool.close()
```

Block exit consumes and closes the scope, including exit by `RETURN`, `EXIT`,
`LEAVE`, controller signal or failed ordinary expression. The exit requests
child cancellation where required and accounts for all children before control
escapes. It does not close the pool. A consumed scope cannot be reused.

The full local/process example is
[`../crexx_programming_guide/examples/concurrency_providers.crexx`](../crexx_programming_guide/examples/concurrency_providers.crexx).

### Pending result bindings

Within `DO PARALLEL`, assignment directly from a task call creates a typed
pending binding:

```rexx
do parallel
  left = first()
  right = second()

  say left       /* materializes left here */
end              /* accounts for right even if it was never read */
```

Reading the binding materializes it and waits if necessary. It then becomes an
ordinary value. Before materialization the following are invalid:

- reassignment;
- passing or taking it by reference;
- `ARG EXPOSE` use;
- indexed or attribute mutation through it; and
- returning or otherwise escaping it from the scope.

For example, this is rejected with
`#PENDING_TASK_RESULT_MUTATION`:

```rexx
do parallel
  value = first()
  value = 8
end
```

Expression-form blocks return only a materialized value:

```rexx
answer = do parallel
  left = first()
  right = second()
  leave with left + right
end
```

## Task-target expressions

The grammar is:

```ebnf
task-target ::= "task" callable-reference
              | "task" class-factory-expression
```

The callable form requires a statically resolved task procedure or generated
task-method adapter:

```rexx
target = task checksum
```

The factory form requires a statically resolved concrete factory whose class
implements `.taskwork`:

```rexx
target = task .imagework("thumbnail")
```

Factory arguments are evaluated on the controller and reconstructed in the
worker. The current factory-target lowering accepts `.boolean`, `.int`,
`.string`, `.binary` and direct `.channelvalue` arguments. It does not use the
general concrete-object `to_channel` contract for factory arguments.

Dynamic procedure names and strings are invalid:

```rexx
selected = "my_module.checksum"
target = task selected              /* #TASK_DYNAMIC_TARGET */
```

The compiler, assembler and linker seal the selected callable, signature and
linked-image identity. `.tasktarget.name()` is diagnostic only; dispatch never
uses the user-authored name string.

## The `.taskwork` adapter

An advanced work class implements:

```rexx
run: method = .channelvalue
  arg request = .channelvalue, context = .taskcontext
```

For example:

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

main: procedure = .int
  pool = .taskpool.local(1, 4)
  scope = .taskscope.collectall(pool, 60000)
  target = task .adderwork(2)
  child = scope.submit(target, .channelvalue.integer_value(40))
  completion = child.wait(-1)
  answer = completion.value().as_integer()
  call scope.finish()
  call pool.close()
  return answer <> 42
```

The complete example, including the process provider, is
[`../crexx_programming_guide/examples/concurrency_taskwork.crexx`](../crexx_programming_guide/examples/concurrency_taskwork.crexx).

The request above is already an application `.channelvalue`; the compiler does
not wrap it in the internal typed-object argument marker. The taskwork object is
constructed in the receiving execution from the target's factory arguments.

## Recursion and nested waits

A task body calling the same task callable is a normal synchronous recursive
call in that worker:

```rexx
factorial: task = .int
  arg value = .int
  if value <= 1 then return 1
  return value * factorial(value - 1)
```

It does not create a child task. A call from one task body to a different task
callable, including a mutual-recursion edge, is rejected with
`#TASK_NESTED_WAIT`. This prevents a bounded worker from blocking while waiting
for work that may require the same pool.

```rexx
inner: task = .int
  return 1

outer: task = .int
  return inner()          /* #TASK_NESTED_WAIT */
```

## Compile-time diagnostics

The following diagnostic names are the current stable identifiers for the
implemented checks:

| Diagnostic | Condition |
| --- | --- |
| `#TASK_ONLY_LEVELG` | task declaration or task call below Level G |
| `#TASK_TARGET_ONLY_LEVELG` | task-target expression below Level G |
| `#PARALLEL_ONLY_LEVELG` | statement or expression `DO PARALLEL` below Level G |
| `#TASK_EXPOSED_ARGUMENT` | `ARG EXPOSE` in a task signature |
| `#TASK_REFERENCE_TYPE` | reference argument or result |
| `#TASK_NONTRANSFERABLE_TYPE` | unsupported scalar, array, interface or class without an exact transfer contract |
| `#TASK_NONTRANSFERABLE_RECEIVER` | task-method receiver lacks the exact concrete transfer contract |
| `#TASK_DYNAMIC_TARGET` | target is a dynamic string/procedure value |
| `#TASKWORK_ADAPTER_REQUIRED` | factory target's class does not implement `.taskwork` |
| `#TASK_NESTED_WAIT` | one task body calls a different task callable |
| `#PARALLEL_SCOPE_TYPE` | `USING` does not yield `.taskscope` |
| `#PARALLEL_SCOPE_REUSED` | one consumed scope is used by another parallel block |
| `#PENDING_TASK_RESULT_MUTATION` | pending binding is reassigned, referenced, mutated or escaped |
| `#TASK_RESULT_CYCLE` | the static task-result dependency graph contains a cycle |

## Current exclusions

The language does not expose a future type, `async`/`await`, detached task,
thread identity, affinity, locks, atomics or shared writable VM objects.
Services and `.taskscope.ask()` are reserved for a later single-owner state
model. Pool telemetry and cross-host providers are also not part of the current
language surface.

Library interfaces may still use the implemented task-target form. For
example, `.httpserver.serve(task .handler())` receives a statically sealed
`.taskwork` factory target and launches independent request tasks. That does
not create a language service identity or make `.taskscope.ask()` available;
the distinction is specified in the
[HTTP client and server guide](../crexx_library_reference/concurrent_http.md).

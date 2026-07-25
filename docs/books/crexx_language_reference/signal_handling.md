# Signal Handling

## Overview

Signals report exceptional conditions that interrupt the ordinary execution of
a program. A signal may originate in a cRexx statement, in a called routine, or
in the runtime system. Examples include an explicit application failure, an
invalid conversion, division by zero, an out-of-range value, or interruption
by the operating environment.

cRexx Level B provides two complementary forms of signal handling:

* procedure-scoped handlers installed with `SIGNAL ON`; and
* block-scoped handlers written as `ON SIGNAL` clauses in a simple
  `DO ... END` group.

Procedure-scoped handlers are useful when one routine is responsible for a
condition throughout a substantial part of a call tree. Block-scoped handlers
place the protected operations and their recovery code together, making them
better suited to a particular calculation or resource operation.

Signals are represented by `.signal` objects. Such an object identifies the
condition, carries an optional message or payload, and can provide source
information for runtime errors. Procedure handlers receive this object as an
argument. A block handler can bind it to a local variable with an `AS` clause.

Level B signal handling is structured. It does not use the classic Rexx
`SIGNAL` instruction as an unrestricted branch to a label.

## Signal Names

Every signal has a name and a numeric code. Programs normally use the name,
which is clearer and does not depend on the numeric representation used by the
runtime.

Runtime and application conditions include names such as:

```text
FAILURE
ERROR
DIVISION_BY_ZERO
CONVERSION_ERROR
INVALID_ARGUMENTS
OUT_OF_RANGE
UNICODE_ERROR
UNKNOWN_INSTRUCTION
FUNCTION_NOT_FOUND
NOT_IMPLEMENTED
INVALID_SIGNAL_CODE
OTHER
```

The runtime also recognizes operating-system-style signals including:

```text
QUIT
TERM
POSIX_INT
POSIX_HUP
POSIX_USR1
POSIX_USR2
POSIX_CHLD
```

`KILL` cannot be masked and therefore cannot be consumed by an ordinary Rexx
handler. `BREAKPOINT` belongs to the tracing and debugging mechanism; it is not
an ordinary application error condition.

Signal names written literally in a `SIGNAL` statement are checked by the
compiler. An unknown literal name is a compile-time error. A name constructed
dynamically cannot be checked until execution; if it does not identify a known
signal, the runtime raises `INVALID_SIGNAL_CODE`.

## Raising a Signal

A program can raise a named signal directly:

```rexx
signal failure
```

An accompanying message or payload may follow the name:

```rexx
signal failure "The input file could not be processed"
```

The compact form treats the bare identifier as a signal name. It is
conceptually equivalent to constructing a `.signal` object:

```rexx
signal .signal("failure")
```

and:

```rexx
signal .signal("failure",
               "The input file could not be processed")
```

The object form is also used when the signal name is held in a variable:

```rexx
condition_name = "failure"
signal .signal(condition_name)
```

It can carry an object payload rather than only a string:

```rexx
signal .signal("error", diagnostic_object)
```

The qualified factory form is available when the library namespace must be
made explicit:

```rexx
signal rxfnsb..signal("conversion_error", payload)
```

A `.signal` object received by a handler can be raised again. This is commonly
used after local logging or cleanup:

```rexx
signal problem
```

where `problem` is a local variable of type `.signal`.

## The Signal Object

The public `.signal` interface separates the language-visible condition from
the runtime's internal interrupt representation. User programs should work
with `.signal`; the raw runtime signal classes are implementation details.

A signal object provides the following methods:

```rexx
name:    method = .string
code:    method = .int
module:  method = .int
address: method = .int
message: method = .string
payload: method = .object
file:    method = .string
line:    method = .int
column:  method = .int
source:  method = .string
```

The principal descriptive properties are:

* `name()` -- the symbolic signal name;
* `code()` -- the corresponding runtime signal code;
* `message()` -- a printable description of the condition; and
* `payload()` -- the object supplied when the signal was raised.

For a string payload, `message()` returns that text. When the payload is
another kind of object, the runtime derives the message from its string
representation.

Runtime-originated signals may also identify where the condition occurred:

* `module()` gives the runtime module number;
* `address()` gives the instruction address in that module;
* `file()` gives the source-file name when metadata is available;
* `line()` and `column()` identify the source location; and
* `source()` gives the associated source fragment.

For a fault raised by an instruction, the address and source information refer
to the faulting instruction. Source metadata is resolved from the closest
preceding source location, because one authored clause may generate several
runtime instructions.

Not every signal necessarily has meaningful source metadata. Native or
asynchronous signals may occur between authored clauses, and a signal created
explicitly by a program may contain only the information supplied by that
program. Handlers should therefore treat source details as diagnostic
information rather than assume that every field is populated.

For example:

```rexx
report_signal: procedure
    arg condition = .signal

    say condition.name()":" condition.message()
    if condition.file() <> "" then
        say condition.file() condition.line() condition.source()
    return
```

## Procedure-Scoped Handlers

A procedure-scoped handler is installed with:

```rexx
signal on signal-name call handler
```

Several signals can share one handler:

```rexx
signal on error, syntax call handle_problem
```

A typical installation is:

```rexx
signal on conversion_error call handle_conversion

value = perform_conversion(input)
```

The named handler must be a procedure that accepts one `.signal` argument and
returns a `.signalaction`:

```rexx
handle_conversion: procedure = .signalaction
    arg condition = .signal

    say "Conversion failed:"
    say condition.message()
    say condition.source()

    return .signalaction.skip()
```

The compiler validates the handler interface. If the procedure cannot be
found, it reports `SIGNAL_HANDLER_NOT_FOUND`. If its argument or return
signature is unsuitable, it reports `SIGNAL_HANDLER_BAD_SIGNATURE` at the
handler name in the `SIGNAL ON` statement.

### Handler Actions

A procedure handler returns one of three explicit actions:

```rexx
.signalaction.retry()
.signalaction.skip()
.signalaction.fail()
```

`retry` resumes at the interrupted instruction. The operation that raised the
signal is attempted again:

```rexx
return .signalaction.retry()
```

This is appropriate only when the handler has changed the circumstances that
caused the failure. Retrying without correcting the cause ordinarily raises
the same signal again.

`skip` resumes after the signal point:

```rexx
return .signalaction.skip()
```

The handler accepts responsibility for the condition and execution continues.
This action must be used carefully for signals raised while calculating a
value. Skipping a failed operation may leave its intended result unavailable.

`fail` allows the signal to continue to the default failure handling:

```rexx
return .signalaction.fail()
```

The runtime then produces its normal panic report, including source metadata
when available.

A handler that falls off its end or returns `.void` is treated as `fail`.
Continuation must be requested explicitly; an accidentally incomplete handler
does not silently consume a runtime error.

### Removing a Handler

A procedure-scoped handler is removed with:

```rexx
signal off conversion_error
```

Several handlers may be removed in one statement:

```rexx
signal off error, syntax
```

`SIGNAL OFF` restores the runtime's root behaviour for each named signal. That
default is either to halt or, for signals that the runtime ignores by default,
to ignore the signal.

### Scope and Called Procedures

Handlers belong to a procedure invocation. A procedure called after a handler
has been installed inherits the current handler settings:

```rexx
main: procedure
    signal on error call handle_error
    call worker
    return
```

An `ERROR` raised in `worker` can therefore be handled by `handle_error`.

The called procedure receives its own copy of the handler settings. If it
installs or removes a handler, that change is local to its invocation and does
not alter the caller's settings. When the called procedure returns, execution
continues with the caller's original handler configuration.

This gives procedure handlers a useful call-tree scope: they apply to later
calls, but child procedures cannot permanently rewrite their caller's signal
policy.

## Block-Scoped Handlers

A block-scoped handler is attached to a simple `DO ... END` group:

```rexx
do
    risky_work()
    more_risky_work()
on signal conversion_error as condition
    say condition.source()
end
```

The statements before the first `ON SIGNAL` clause form the protected body.
If they complete normally, all handler clauses are skipped. If a matching
signal occurs, execution transfers to the corresponding handler.

`ON SIGNAL` clauses are handlers, not labels. When a handler completes
normally, execution leaves the complete `DO` block; it does not return to the
statement following the fault.

### Placement

Signal clauses may be attached only to a simple `DO` group: a group without a
loop repetitor, conditional phrase, or expression-block value.

To protect an operation inside a loop, nest a simple group:

```rexx
do i = 1 to count
    do
        risky_work(i)
    on signal error as problem
        call log_problem(problem)
    end
end
```

The inner block establishes and restores its handlers for that iteration. The
outer loop retains its ordinary loop semantics.

### Handling Named Signals

One clause can handle one or more named signals:

```rexx
do
    result = convert(input)
on signal conversion_error, out_of_range as problem
    call report_problem(problem)
end
```

The names are matched against the signal delivered by the runtime. A different
signal continues to another applicable handler or propagates out of the block.

### Catching All Maskable Signals

An `ON SIGNAL` clause without names is a catch-all:

```rexx
do
    perform_operation()
on signal as problem
    call log_problem(problem)
end
```

It catches any maskable signal not handled by an earlier, more specific
clause. Signals such as `KILL`, which cannot be masked, are not made catchable
by this form.

A catch-all handler should normally follow the named handlers:

```rexx
do
    perform_operation()
on signal conversion_error as problem
    call report_bad_input(problem)
on signal
    call perform_general_cleanup()
end
```

The final clause needs no bound signal object because its body performs fixed
cleanup and does not inspect the condition.

### Binding the Signal Object

The optional `AS` phrase binds the current `.signal` object to a local name:

```rexx
on signal error as problem
```

The handler can then inspect or re-raise it:

```rexx
do
    update_resource()
on signal error as problem
    call log_problem(problem)
    signal problem
end
```

If `AS` is omitted, no implicit variable such as `condition` is created:

```rexx
on signal error
    call rollback()
```

This form is appropriate when the handler performs a fixed action and does not
need information about the signal.

### Completion and Propagation

A block handler does not return a `.signalaction`. Completing the handler
normally consumes the signal and leaves the protected block:

```rexx
do
    result = convert(input)
on signal conversion_error
    result = default_value
end
```

To propagate the condition, bind its object and raise it again:

```rexx
do
    result = convert(input)
on signal conversion_error as problem
    call log_problem(problem)
    signal problem
end
```

The enclosing block or procedure handler can then process it. If no applicable
handler exists, normal runtime failure handling applies.

There is no `FINALLY` clause in this signal model. A bare `ON SIGNAL` is a
broad error trap, not an unconditional cleanup clause: it runs only when a
signal is delivered.

## Nested Handlers

Block handlers temporarily replace the applicable handlers in the current
procedure. On every normal block exit, the previous settings are restored.
They are also restored when control leaves through a signal, an early return,
or procedure termination.

This permits blocks to specialize a broader procedure policy:

```rexx
signal on error call procedure_error

do
    special_operation()
on signal error as problem
    call local_error(problem)
end

call another_operation()
```

An `ERROR` in `special_operation()` is handled by the local block. After that
block has been left, the procedure-level `procedure_error` handler is again in
effect for `another_operation()`.

Nested blocks follow the same rule. The innermost applicable handler receives
the signal first. Re-raising the `.signal` object allows an enclosing handler
to see the same condition.

## Signals Raised While Handling a Signal

Signal delivery is suspended while a Rexx signal handler is executing. If the
handler raises another signal, the new condition becomes pending rather than
interrupting the handler immediately.

The pending signal is delivered after control returns to a frame that is not
itself handling a signal. This prevents immediate recursive interruption of
the handler, but it does not discard the newly raised condition.

Handler code should nevertheless remain conservative. A handler that
repeatedly raises the same uncorrected signal can produce a cycle of deferred
deliveries.

If several signals are pending, the runtime handles them in signal-code
priority order. Programs should not depend on the order in which unrelated
asynchronous conditions happen to arrive.

## Unhandled Signals

When no installed handler accepts a runtime error, the runtime applies the
signal's default action. For ordinary error signals this normally halts
execution and produces a panic report.

For faults raised by the current instruction, that report identifies the
faulting instruction and uses the closest available source metadata. This
normally provides the source file, line, column, and source fragment associated
with the failed operation.

Some runtime signals are ignored by default. `SIGNAL OFF` restores that
default rather than necessarily changing every signal to a halting condition.

## Signals and TRACE

Signal handling and `TRACE` share parts of the runtime interrupt mechanism, but
they have different language meanings.

Ordinary error signals describe a fault and report the faulting instruction.
Tracing uses the special `BREAKPOINT` signal as a step-and-resume mechanism,
whose address identifies the next instruction. A user signal handler should
not treat `BREAKPOINT` as an application error.

The trace runtime filters its own support code and the formal cRexx runtime by
default. Installing an ordinary signal handler does not itself enable tracing,
and tracing should not cause trace-support code to interrupt itself.

## Choosing a Handler Form

Use a procedure-scoped handler when:

* one policy should apply throughout a procedure and the routines it calls;
* recovery requires a separate, reusable handler procedure;
* the handler must explicitly choose between retry, skip, and failure; or
* several signals share the same procedure-level policy.

Use a block-scoped handler when:

* the protected operation is small and clearly delimited;
* recovery belongs beside the operation that may fail;
* different parts of one procedure need different policies; or
* the signal should be consumed by leaving a protected region.

In both forms, catch only the signals that the code can handle meaningfully.
A broad catch-all handler is useful for logging or cleanup, but it should
re-raise conditions that it cannot safely resolve.

## Complete Example

The following program combines a local conversion handler with a broader
procedure handler:

```rexx
options levelb
import rxfnsb

main: procedure
    signal on error call handle_error

    do
        value = convert_input()
        say "Converted value:" value
    on signal conversion_error as problem
        say "Invalid input:" problem.message()
        value = 0
    end

    call continue_processing(value)

    signal off error
    return

handle_error: procedure = .signalaction
    arg problem = .signal

    say problem.name()":" problem.message()
    if problem.file() <> "" then
        say problem.file() problem.line() problem.source()

    return .signalaction.fail()
```

The block consumes a `CONVERSION_ERROR` and supplies a local default. Other
`ERROR` conditions are passed to `handle_error`, which reports the condition
and deliberately selects normal failure handling.
